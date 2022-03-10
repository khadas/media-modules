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
#include "vdec_v4l2_buffer_ops.h"
#include <media/v4l2-mem2mem.h>
#include <linux/printk.h>
#include <media/v4l2-mem2mem.h>

/**
 * struct v4l2_m2m_dev - per-device context
 * @source:		&struct media_entity pointer with the source entity
 *			Used only when the M2M device is registered via
 *			v4l2_m2m_unregister_media_controller().
 * @source_pad:		&struct media_pad with the source pad.
 *			Used only when the M2M device is registered via
 *			v4l2_m2m_unregister_media_controller().
 * @sink:		&struct media_entity pointer with the sink entity
 *			Used only when the M2M device is registered via
 *			v4l2_m2m_unregister_media_controller().
 * @sink_pad:		&struct media_pad with the sink pad.
 *			Used only when the M2M device is registered via
 *			v4l2_m2m_unregister_media_controller().
 * @proc:		&struct media_entity pointer with the M2M device itself.
 * @proc_pads:		&struct media_pad with the @proc pads.
 *			Used only when the M2M device is registered via
 *			v4l2_m2m_unregister_media_controller().
 * @intf_devnode:	&struct media_intf devnode pointer with the interface
 *			with controls the M2M device.
 * @curr_ctx:		currently running instance
 * @job_queue:		instances queued to run
 * @job_spinlock:	protects job_queue
 * @job_work:		worker to run queued jobs.
 * @m2m_ops:		driver callbacks
 */
struct v4l2_m2m_dev {
	struct v4l2_m2m_ctx	*curr_ctx;
#ifdef CONFIG_MEDIA_CONTROLLER
	struct media_entity	*source;
	struct media_pad	source_pad;
	struct media_entity	sink;
	struct media_pad	sink_pad;
	struct media_entity	proc;
	struct media_pad	proc_pads[2];
	struct media_intf_devnode *intf_devnode;
#endif

	struct list_head	job_queue;
	spinlock_t		job_spinlock;
	struct work_struct	job_work;

	const struct v4l2_m2m_ops *m2m_ops;
};


int vdec_v4l_get_pic_info(struct aml_vcodec_ctx *ctx,
	struct vdec_pic_info *pic)
{
	int ret = 0;

	if (ctx->drv_handle == 0)
		return -EIO;

	ret = ctx->dec_if->get_param(ctx->drv_handle,
		GET_PARAM_PIC_INFO, pic);

	return ret;
}
EXPORT_SYMBOL(vdec_v4l_get_pic_info);

int vdec_v4l_set_cfg_infos(struct aml_vcodec_ctx *ctx,
	struct aml_vdec_cfg_infos *cfg)
{
	int ret = 0;

	if (ctx->drv_handle == 0)
		return -EIO;

	ret = ctx->dec_if->set_param(ctx->drv_handle,
		SET_PARAM_CFG_INFO, cfg);

	return ret;
}
EXPORT_SYMBOL(vdec_v4l_set_cfg_infos);

int vdec_v4l_set_ps_infos(struct aml_vcodec_ctx *ctx,
	struct aml_vdec_ps_infos *ps)
{
	int ret = 0;

	if (ctx->drv_handle == 0)
		return -EIO;

	ret = ctx->dec_if->set_param(ctx->drv_handle,
		SET_PARAM_PS_INFO, ps);

	return ret;
}
EXPORT_SYMBOL(vdec_v4l_set_ps_infos);

int vdec_v4l_set_comp_buf_info(struct aml_vcodec_ctx *ctx,
		struct vdec_comp_buf_info *info)
{
	int ret = 0;

	if (ctx->drv_handle == 0)
		return -EIO;

	ret = ctx->dec_if->set_param(ctx->drv_handle,
		SET_PARAM_COMP_BUF_INFO, info);

	return ret;

}
EXPORT_SYMBOL(vdec_v4l_set_comp_buf_info);

int vdec_v4l_set_hdr_infos(struct aml_vcodec_ctx *ctx,
	struct aml_vdec_hdr_infos *hdr)
{
	int ret = 0;

	if (ctx->drv_handle == 0)
		return -EIO;

	ret = ctx->dec_if->set_param(ctx->drv_handle,
		SET_PARAM_HDR_INFO, hdr);

	return ret;
}
EXPORT_SYMBOL(vdec_v4l_set_hdr_infos);

void aml_vdec_pic_info_update(struct aml_vcodec_ctx *ctx)
{
	if (ctx != NULL)
		ctx->vdec_pic_info_update(ctx);
}

int vdec_v4l_post_evet(struct aml_vcodec_ctx *ctx, u32 event)
{
	int ret = 0;

	if (ctx->drv_handle == 0)
		return -EIO;
	if (event == 1)
		ctx->reset_flag = 2;
	ret = ctx->dec_if->set_param(ctx->drv_handle,
		SET_PARAM_POST_EVENT, &event);

	return ret;
}
EXPORT_SYMBOL(vdec_v4l_post_evet);

int vdec_v4l_res_ch_event(struct aml_vcodec_ctx *ctx)
{
	int ret = 0;
	struct aml_vcodec_dev *dev = ctx->dev;

	if (ctx->drv_handle == 0)
		return -EIO;

	aml_vdec_pic_info_update(ctx);

	mutex_lock(&ctx->state_lock);

	ctx->state = AML_STATE_FLUSHING;/*prepare flushing*/

	pr_info("[%d]: vcodec state (AML_STATE_FLUSHING-RESCHG)\n", ctx->id);

	mutex_unlock(&ctx->state_lock);

	while (ctx->m2m_ctx->job_flags & TRANS_RUNNING) {
		v4l2_m2m_job_pause(dev->m2m_dev_dec, ctx->m2m_ctx);
	}

	return ret;
}
EXPORT_SYMBOL(vdec_v4l_res_ch_event);


int vdec_v4l_write_frame_sync(struct aml_vcodec_ctx *ctx)
{
	int ret = 0;

	if (ctx->drv_handle == 0)
		return -EIO;

	ret = ctx->dec_if->set_param(ctx->drv_handle,
		SET_PARAM_WRITE_FRAME_SYNC, NULL);

	return ret;
}
EXPORT_SYMBOL(vdec_v4l_write_frame_sync);

int vdec_v4l_get_dw_mode(struct aml_vcodec_ctx *ctx,
	unsigned int *dw_mode)
{
	int ret = -1;

	if (ctx->drv_handle == 0)
		return -EIO;

	ret = ctx->dec_if->get_param(ctx->drv_handle,
		GET_PARAM_DW_MODE, dw_mode);

	return ret;
}
EXPORT_SYMBOL(vdec_v4l_get_dw_mode);

void v4l2_m2m_job_pause(struct v4l2_m2m_dev *m2m_dev,
			struct v4l2_m2m_ctx *m2m_ctx)
{
	unsigned long flags;

	spin_lock_irqsave(&m2m_dev->job_spinlock, flags);
	if (!m2m_dev->curr_ctx || m2m_dev->curr_ctx != m2m_ctx) {
		spin_unlock_irqrestore(&m2m_dev->job_spinlock, flags);
		printk("Called by an instance not currently running\n");
		return;
	}

	list_del(&m2m_dev->curr_ctx->queue);
	m2m_dev->curr_ctx->job_flags &= ~(TRANS_QUEUED | TRANS_RUNNING);
	m2m_dev->curr_ctx->job_flags |= TRANS_ABORT;
	wake_up(&m2m_dev->curr_ctx->finished);
	m2m_dev->curr_ctx = NULL;

	spin_unlock_irqrestore(&m2m_dev->job_spinlock, flags);
}
EXPORT_SYMBOL(v4l2_m2m_job_pause);

