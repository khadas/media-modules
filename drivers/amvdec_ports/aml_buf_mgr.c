/*
* Copyright (C) 2017 Amlogic, Inc. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
* Description:
*/

#include <linux/device.h>
#include <linux/amlogic/media/codec_mm/codec_mm.h>

#include "../frame_provider/decoder/utils/decoder_bmmu_box.h"
#include "../frame_provider/decoder/utils/decoder_mmu_box.h"
#include "aml_vcodec_drv.h"
#include "aml_vcodec_dec.h"
#include "aml_task_chain.h"
#include "aml_buf_mgr.h"
#include "aml_vcodec_util.h"
#include "vdec_drv_if.h"

static int aml_buf_box_init(struct aml_buf_mgr_s *bm)
{
	struct aml_buf_fbc_info fbc_info;
	int flag = bm->config.enable_secure ? CODEC_MM_FLAGS_TVP : 0;

	bm->fbc_array = vzalloc(sizeof(*bm->fbc_array) * BUF_FBC_NUM_MAX);
	if (!bm->fbc_array)
		return -ENOMEM;

	bm->get_fbc_info(bm, &fbc_info);

	/* init bmmu box */
	bm->mmu = decoder_mmu_box_alloc_box(bm->bc.name,
					   bm->bc.id,
					   BUF_FBC_NUM_MAX,
					   fbc_info.max_size * SZ_1M,
					   flag);
	if (!bm->mmu) {
		vfree(bm->fbc_array);
		v4l_dbg(bm->priv, V4L_DEBUG_CODEC_ERROR,
			"fail to create bmmu box\n");
		return -EINVAL;
	}

	/* init mmu box */
	flag |= (CODEC_MM_FLAGS_CMA_CLEAR | CODEC_MM_FLAGS_FOR_VDECODER);
	bm->bmmu = decoder_bmmu_box_alloc_box(bm->bc.name,
					     bm->bc.id,
					     BUF_FBC_NUM_MAX,
					     4 + PAGE_SHIFT,
					     flag, BMMU_ALLOC_FLAGS_WAIT);
	if (!bm->bmmu) {
		v4l_dbg(bm->priv, V4L_DEBUG_CODEC_ERROR,
			"fail to create mmu box\n");
		goto free_mmubox;
	}

	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_BUFMGR,
		"box init, bmmu: %px, mmu: %px\n",
		bm->bmmu, bm->mmu);

	return 0;

free_mmubox:
	vfree(bm->fbc_array);
	decoder_mmu_box_free(bm->mmu);
	bm->mmu = NULL;

	return -1;
}

static int aml_buf_fbc_init(struct aml_buf_mgr_s *bm, struct aml_buf *buf)
{
	struct aml_buf_fbc_info fbc_info;
	struct aml_buf_fbc *fbc;
	int ret, i;

	if (!bm->mmu || !bm->bmmu) {
		if (aml_buf_box_init(bm))
			return -EINVAL;
	}

	for (i = 0; i < BUF_FBC_NUM_MAX; i++) {
		if (!bm->fbc_array[i].ref)
			break;
	}

	if (i == BUF_FBC_NUM_MAX) {
		v4l_dbg(bm->priv, V4L_DEBUG_CODEC_ERROR,
			"out of fbc buf\n");
		return -EINVAL;
	}

	bm->get_fbc_info(bm, &fbc_info);

	fbc		= &bm->fbc_array[i];
	fbc->index	= i;
	fbc->hsize	= fbc_info.header_size;
	fbc->frame_size	= fbc_info.frame_size;
	fbc->bmmu	= bm->bmmu;
	fbc->mmu	= bm->mmu;
	fbc->ref	= 1;

	/* allocate header */
	ret = decoder_bmmu_box_alloc_buf_phy(bm->bmmu,
					    fbc->index,
					    fbc->hsize,
					    bm->bc.name,
					    &fbc->haddr);
	if (ret < 0) {
		v4l_dbg(bm->priv, V4L_DEBUG_CODEC_ERROR,
			"fail to alloc %dth bmmu\n", i);
		return -ENOMEM;
	}

	buf->fbc = fbc;

	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_BUFMGR,
		"%s, fbc:%d, haddr:%lx, hsize:%d, frm size:%d\n",
		__func__,
		fbc->index,
		fbc->haddr,
		fbc->hsize,
		fbc->frame_size);

	return 0;
}

static int aml_buf_fbc_release(struct aml_buf_mgr_s *bm, struct aml_buf *buf)
{
	struct aml_buf_fbc *fbc = buf->fbc;
	int ret;

	ret = decoder_bmmu_box_free_idx(bm->bmmu, fbc->index);
	if (ret < 0) {
		v4l_dbg(bm->priv, V4L_DEBUG_CODEC_ERROR,
			"fail to free %dth bmmu\n", fbc->index);
		return -ENOMEM;
	}

	ret = decoder_mmu_box_free_idx(bm->mmu, fbc->index);
	if (ret < 0) {
		v4l_dbg(bm->priv, V4L_DEBUG_CODEC_ERROR,
			"fail to free %dth mmu\n", fbc->index);
		return -ENOMEM;
	}

	fbc->ref = 0;

	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_BUFMGR,
		"%s, fbc:%d, haddr:%lx, hsize:%d, frm size:%d\n",
		__func__,
		fbc->index,
		fbc->haddr,
		fbc->hsize,
		fbc->frame_size);

	return 0;
}

static void aml_buf_get_fbc_info(struct aml_buf_mgr_s *bm,
				struct aml_buf_fbc_info *info)
{
	struct vdec_comp_buf_info comp_info;

	if (vdec_if_get_param(bm->priv, GET_PARAM_COMP_BUF_INFO, &comp_info)) {
		v4l_dbg(bm->priv, V4L_DEBUG_CODEC_ERROR,
			"fail to get comp info\n");
		return;
	}

	info->max_size		= comp_info.max_size;
	info->header_size	= comp_info.header_size;
	info->frame_size	= comp_info.frame_buffer_size;
}

static void aml_buf_fbc_destroy(struct aml_buf_mgr_s *bm)
{
	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_PRINFO, "%s %d\n", __func__, __LINE__);
	decoder_bmmu_box_free(bm->bmmu);
	decoder_mmu_box_free(bm->mmu);
	vfree(bm->fbc_array);
}

static void aml_buf_mgr_destroy(struct kref *kref)
{
	struct aml_buf_mgr_s *bm =
		container_of(kref, struct aml_buf_mgr_s, ref);

	if (bm->config.enable_fbc) {
		aml_buf_fbc_destroy(bm);
	}
}

static void aml_buf_set_planes_v4l2(struct aml_buf_mgr_s *bm,
				   struct aml_buf *aml_buf,
				   void *priv)
{
	struct vb2_buffer *vb = (struct vb2_buffer *)priv;
	struct vb2_v4l2_buffer *vb2_v4l2 = to_vb2_v4l2_buffer(vb);
	struct aml_v4l2_buf *aml_vb = container_of(vb2_v4l2, struct aml_v4l2_buf, vb);
	struct aml_buf_config *cfg = &bm->config;
	char plane_n[3] = {'Y','U','V'};
	int i;

	aml_buf->vb		= vb;
	aml_buf->num_planes	= vb->num_planes;
	aml_vb->aml_buf		= aml_buf;

	for (i = 0 ; i < vb->num_planes ; i++) {
		if (i == 0) {
			//Y
			if (vb->num_planes == 1) {
				aml_buf->planes[0].length	= cfg->luma_length + cfg->chroma_length;
				aml_buf->planes[0].offset	= cfg->luma_length;
			} else {
				aml_buf->planes[0].length	= cfg->luma_length;
				aml_buf->planes[0].offset	= 0;
			}
		} else {
			if (vb->num_planes == 2) {
				//UV
				aml_buf->planes[1].length	= cfg->chroma_length;
				aml_buf->planes[1].offset	= cfg->chroma_length >> 1;
			} else {
				aml_buf->planes[i].length	= cfg->chroma_length >> 1;
				aml_buf->planes[i].offset	= 0;
			}
		}

		aml_buf->planes[i].addr	= vb2_dma_contig_plane_dma_addr(vb, i);
		aml_buf->planes[i].dbuf	= vb->planes[i].dbuf;

		v4l_dbg(bm->priv, V4L_DEBUG_CODEC_BUFMGR,
			"idx: %u, %c:(0x%lx, %d)\n",
			vb->index,
			plane_n[i],
			aml_buf->planes[i].addr,
			aml_buf->planes[i].length);
	}
}

static void aml_buf_set_planes(struct aml_buf_mgr_s *bm,
			      struct aml_buf *buf)
{
	//todo
}

static int aml_buf_set_default_parms(struct aml_buf_mgr_s *bm,
				     struct aml_buf *buf,
				     void *priv)
{
	int ret;

	buf->index = bm->bc.buf_num;

	if (bm->config.enable_extbuf) {
		aml_buf_set_planes_v4l2(bm, buf, priv);

		ret = aml_uvm_buff_attach(buf->vb);
		if (ret) {
			v4l_dbg(bm->priv, V4L_DEBUG_CODEC_ERROR,
				"uvm buffer attach failed.\n");
			return ret;
		}
	} else {
		// alloc buffer
		aml_buf_set_planes(bm, buf);
	}

	ret = task_chain_init(&buf->task, bm->priv, buf, buf->index);
	if (ret) {
		v4l_dbg(bm->priv, V4L_DEBUG_CODEC_ERROR,
			"task chain init failed.\n");
	}

	return ret;
}

static int aml_buf_alloc(struct buf_core_mgr_s *bc,
			struct buf_core_entry **entry,
			void *priv)
{
	int ret;
	struct aml_buf *buf;
	struct aml_buf_mgr_s *bm = bc_to_bm(bc);

	buf = vzalloc(sizeof(struct aml_buf));
	if (buf == NULL) {
		return -ENOMEM;
	}

	/* afbc init. */
	if (bm->config.enable_fbc) {
		ret = aml_buf_fbc_init(bm, buf);
		if (ret) {
			goto err1;
		}
	}

	ret = aml_buf_set_default_parms(bm, buf, priv);
	if (ret) {
		goto err2;
	}

	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_BUFMGR,
		"%s, entry:%px, free:%d\n",
		__func__,
		&buf->entry,
		bc->free_num);

	kref_get(&bm->ref);

	*entry = &buf->entry;

	return 0;

err2:
	aml_buf_fbc_destroy(bm);
err1:
	vfree(buf);

	return ret;
}

static void aml_buf_free(struct buf_core_mgr_s *bc,
			struct buf_core_entry *entry)
{
	struct aml_buf_mgr_s *bm = bc_to_bm(bc);
	struct aml_buf *buf = entry_to_aml_buf(entry);

	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_BUFMGR,
		"%s, entry:%px, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d)\n",
		__func__,
		entry,
		entry->user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref));

	/* afbc free */
	if (bm->config.enable_fbc) {
		aml_buf_fbc_release(bm, buf);
	}

	/* task chain clean */
	task_chain_clean(buf->task);
	/* task chain release */
	task_chain_release(buf->task);

	kref_put(&bm->ref, aml_buf_mgr_destroy);

	if (buf->vpp_buf)
		vfree(buf->vpp_buf);

	if (buf->ge2d_buf)
		vfree(buf->ge2d_buf);

	vfree(buf);
}

static void aml_buf_configure(struct buf_core_mgr_s *bc, void *cfg)
{
	struct aml_buf_mgr_s *bm = bc_to_bm(bc);

	bm->config = *(struct aml_buf_config *)cfg;
}

static void aml_external_process(struct buf_core_mgr_s *bc,
				struct buf_core_entry *entry)
{
	struct aml_buf *aml_buf = entry_to_aml_buf(entry);
	struct aml_buf_mgr_s *bm =
			container_of(bc, struct aml_buf_mgr_s, bc);

	aml_vdec_recycle_dec_resource(bm->priv, aml_buf);
}

static void aml_buf_prepare(struct buf_core_mgr_s *bc,
				struct buf_core_entry *entry)
{
	struct aml_buf_mgr_s *bm = bc_to_bm(bc);
	struct aml_buf *buf = entry_to_aml_buf(entry);

	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__,
		entry->user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);

	if (!task_chain_empty(buf->task)) {
		task_chain_clean(buf->task);
	}

	/* afbc update */
	if (bm->config.enable_fbc) {
		struct aml_buf_fbc_info fbc_info;

		bm->get_fbc_info(bm, &fbc_info);
		if (buf->fbc &&
			((fbc_info.frame_size != buf->fbc->frame_size) ||
			(fbc_info.header_size != buf->fbc->hsize))) {
			aml_buf_fbc_release(bm, buf);
			aml_buf_fbc_init(bm, buf);
		}
	}

	if (bm->config.enable_extbuf) {
		aml_buf_set_planes_v4l2(bm, buf, entry->vb2);
	} else {
		aml_buf_set_planes(bm, buf);
	}

	if (entry->user != BUF_USER_MAX)
		aml_creat_pipeline(bm->priv, buf, entry->user);
}

static void aml_buf_output(struct buf_core_mgr_s *bc,
			  struct buf_core_entry *entry,
			  enum buf_core_user user)
{
	struct aml_buf_mgr_s *bm = bc_to_bm(bc);
	struct aml_buf *buf = entry_to_aml_buf(entry);

	if (task_chain_empty(buf->task))
		return;

	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__,
		entry->user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);

	buf->task->submit(buf->task, user_to_task(user));
}

static void aml_buf_input(struct buf_core_mgr_s *bc,
		   struct buf_core_entry *entry,
		   enum buf_core_user user)
{
	struct aml_buf_mgr_s *bm = bc_to_bm(bc);
	struct aml_buf *buf = entry_to_aml_buf(entry);

	if (task_chain_empty(buf->task))
		return;

	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__,
		entry->user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);

	buf->task->recycle(buf->task, user_to_task(user));
}

int aml_buf_mgr_init(struct aml_buf_mgr_s *bm, char *name, int id, void *priv)
{
	int ret = -1;

	bm->bc.id		= id;
	bm->bc.name		= name;
	bm->priv		= priv;
	bm->get_fbc_info	= aml_buf_get_fbc_info;

	bm->bc.config		= aml_buf_configure;
	bm->bc.prepare		= aml_buf_prepare;
	bm->bc.input		= aml_buf_input;
	bm->bc.output		= aml_buf_output;
	bm->bc.external_process	= aml_external_process;
	bm->bc.mem_ops.alloc	= aml_buf_alloc;
	bm->bc.mem_ops.free	= aml_buf_free;

	kref_init(&bm->ref);

	ret = buf_core_mgr_init(&bm->bc);
	if (ret) {
		v4l_dbg(priv, V4L_DEBUG_CODEC_ERROR,
			"%s, init fail.\n", __func__);
	} else {
		v4l_dbg(priv, V4L_DEBUG_CODEC_PRINFO,
			"%s\n", __func__);
	}

	return ret;
}

void aml_buf_mgr_release(struct aml_buf_mgr_s *bm)
{
	v4l_dbg(bm->priv, V4L_DEBUG_CODEC_PRINFO, "%s\n", __func__);

	kref_put(&bm->ref, aml_buf_mgr_destroy);
	buf_core_mgr_release(&bm->bc);
}

