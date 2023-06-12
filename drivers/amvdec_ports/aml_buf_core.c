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

#include <linux/atomic.h>

#include "aml_buf_core.h"
#include "aml_vcodec_util.h"

static bool bc_sanity_check(struct buf_core_mgr_s *bc)
{
	return (bc->state == BM_STATE_ACTIVE) ? true : false;
}

static void buf_core_destroy(struct kref *kref);

static void direction_buf_get(struct buf_core_mgr_s *bc,
			  struct buf_core_entry *entry,
			  enum buf_core_user user)
{
	int type;
	switch (user) {
		case BUF_USER_DEC:
			entry->holder = BUF_HOLDER_DEC;
			entry->ref_bit_map += DEC_BIT;

			break;
		case BUF_USER_VPP:
			entry->holder = BUF_HOLDER_VPP;
			entry->ref_bit_map += VPP_BIT;

			type = bc->get_pre_user(bc, entry, user);
			if (type == BUF_USER_DEC)
				entry->ref_bit_map -= DEC_BIT;
			if (type == BUF_USER_GE2D)
				entry->ref_bit_map -= GE2D_BIT;

			break;
		case BUF_USER_GE2D:
			entry->holder = BUF_HOLDER_GE2D;
			entry->ref_bit_map += GE2D_BIT;
			entry->ref_bit_map -= DEC_BIT;

			break;
		case BUF_USER_VSINK:
			entry->holder = BUF_HOLDER_VSINK;
			entry->ref_bit_map += VSINK_BIT;

			type = bc->get_pre_user(bc, entry, user);
			if (type == BUF_USER_VPP)
				entry->ref_bit_map -= VPP_BIT;
			if (type == BUF_USER_DEC)
				entry->ref_bit_map -= DEC_BIT;
			if (type == BUF_USER_GE2D)
				entry->ref_bit_map -= GE2D_BIT;

			break;
		default:
			v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_ERROR,
			"Invalid user(%d)!\n", user);
			break;
	}
}

static void direction_buf_put(struct buf_core_mgr_s *bc,
			  struct buf_core_entry *entry,
			  enum buf_core_user user)
{
	switch (user) {
		case BUF_USER_DEC:
			if (entry->ref_bit_map & DEC_MASK)
				entry->ref_bit_map -= DEC_BIT;
			if (entry->ref_bit_map & GE2D_MASK)
				entry->holder = BUF_HOLDER_GE2D;
			if (entry->ref_bit_map & VPP_MASK)
				entry->holder = BUF_HOLDER_VPP;
			if (entry->ref_bit_map & VSINK_MASK)
				entry->holder = BUF_HOLDER_VSINK;

			break;
		case BUF_USER_VPP:
			entry->ref_bit_map -= VPP_BIT;
			if (entry->ref_bit_map & VPP_MASK)
				entry->holder = BUF_HOLDER_VPP;
			else {
				if (entry->ref_bit_map & GE2D_MASK)
					entry->holder = BUF_HOLDER_GE2D;
				if (entry->ref_bit_map & DEC_MASK)
					entry->holder = BUF_HOLDER_DEC;
			}

			break;
		case BUF_USER_GE2D:
			entry->ref_bit_map -= GE2D_BIT;
			if (entry->ref_bit_map & GE2D_MASK)
				entry->holder = BUF_HOLDER_GE2D;
			else
				entry->holder = BUF_HOLDER_DEC;

			break;
		case BUF_USER_VSINK:
			entry->ref_bit_map -= VSINK_BIT;
			entry->holder = BUF_HOLDER_DEC;

			break;
		default:
			v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_ERROR,
			"Invalid user(%d)!\n", user);
			break;
	}
}

static void buf_core_update_holder(struct buf_core_mgr_s *bc,
			  struct buf_core_entry *entry,
			  enum buf_core_user user,
			  enum buf_direction direction)
{
	if (direction == BUF_GET)
		direction_buf_get(bc, entry, user);
	else
		direction_buf_put(bc, entry, user);

	if (entry->ref_bit_map & 0x8888)
		v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"error! ref_bit_map(0x%x)\n", entry->ref_bit_map);

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, holder:%d, ref_bit_map(0x%x)\n",
		__func__, user, entry->holder, entry->ref_bit_map);

	return;
}

static void buf_core_free_que(struct buf_core_mgr_s *bc,
			     struct buf_core_entry *entry)
{
	entry->state = BUF_STATE_FREE;
	entry->holder = BUF_HOLDER_FREE;
	entry->ref_bit_map = 0;
	list_add_tail(&entry->node, &bc->free_que);
	bc->free_num++;

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__,
		entry->user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);
}

static void buf_core_get(struct buf_core_mgr_s *bc,
			enum buf_core_user user,
			struct buf_core_entry **out_entry,
			bool more_ref)
{
	struct buf_core_entry *entry = NULL;
	bool user_change = false;

	mutex_lock(&bc->mutex);

	if (!bc_sanity_check(bc)) {
		goto out;
	}

	if (list_empty(&bc->free_que)) {
		goto out;
	}

	entry = list_first_entry(&bc->free_que, struct buf_core_entry, node);
	list_del(&entry->node);
	bc->free_num--;

	user_change	= entry->user != user ? 1 : 0;
	entry->user	= user;
	entry->state	= BUF_STATE_USE;
	atomic_inc(&entry->ref);

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__, user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);

	if (bc->prepare /*&&
		user_change*/) {
		bc->prepare(bc, entry);
	}
	buf_core_update_holder(bc, entry, user, BUF_GET);
out:
	*out_entry = entry;

	mutex_unlock(&bc->mutex);
}

static void buf_core_put(struct buf_core_mgr_s *bc,
			struct buf_core_entry *entry)
{
	mutex_lock(&bc->mutex);

	if (!bc_sanity_check(bc)) {
		mutex_unlock(&bc->mutex);
		return;
	}

	if (!atomic_dec_return(&entry->ref)) {
		buf_core_free_que(bc, entry);
	}

	mutex_unlock(&bc->mutex);
}

static void buf_core_get_ref(struct buf_core_mgr_s *bc,
			    struct buf_core_entry *entry)
{
	mutex_lock(&bc->mutex);

	if (!bc_sanity_check(bc)) {
		mutex_unlock(&bc->mutex);
		return;
	}

	entry->state = BUF_STATE_REF;
	atomic_inc(&entry->ref);
	//kref_get(&bc->core_ref);
	buf_core_update_holder(bc, entry, BUF_USER_DEC, BUF_GET);

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__,
		entry->user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);

	mutex_unlock(&bc->mutex);
}

static void buf_core_put_ref(struct buf_core_mgr_s *bc,
			    struct buf_core_entry *entry)
{
	mutex_lock(&bc->mutex);

	if (!atomic_read(&entry->ref) && !bc_sanity_check(bc)) {
		v4l_dbg_ext(bc->id, 0,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__,
		entry->user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);
		mutex_unlock(&bc->mutex);
		return;
	}

	if (!atomic_dec_return(&entry->ref)) {
		buf_core_free_que(bc, entry);
	} else {
		entry->state = BUF_STATE_REF;
		buf_core_update_holder(bc, entry, BUF_USER_DEC, BUF_PUT);
	}

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__,
		entry->user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);

	//kref_put(&bc->core_ref, buf_core_destroy);

	mutex_unlock(&bc->mutex);
}

static int buf_core_done(struct buf_core_mgr_s *bc,
			   struct buf_core_entry *entry,
			   enum buf_core_user user)
{
	int ret = 0;

	mutex_lock(&bc->mutex);

	if (!bc_sanity_check(bc)) {
		goto out;
	}

	if (WARN_ON((entry->state != BUF_STATE_USE) &&
		(entry->state != BUF_STATE_REF) &&
		(entry->state != BUF_STATE_DONE))) {
		ret = -1;
		goto out;
	}

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__, user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);

	entry->state = BUF_STATE_DONE;

	ret = bc->output(bc, entry, user);
out:
	mutex_unlock(&bc->mutex);

	return ret;
}

static void buf_core_fill(struct buf_core_mgr_s *bc,
			    struct buf_core_entry *entry,
			    enum buf_core_user user)
{
	mutex_lock(&bc->mutex);

	if (!bc_sanity_check(bc)) {
		goto out;
	}

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__, user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);

	if (WARN_ON((entry->state != BUF_STATE_INIT) &&
		(entry->state != BUF_STATE_DONE) &&
		(entry->state != BUF_STATE_REF))) {
		goto out;
	}

	if (bc->external_process)
		bc->external_process(bc, entry);

	if (!atomic_dec_return(&entry->ref)) {
		buf_core_free_que(bc, entry);
	} else {
		entry->state = BUF_STATE_REF;
		buf_core_update_holder(bc, entry, user, BUF_PUT);
	}

out:
	mutex_unlock(&bc->mutex);
}

static int buf_core_ready_num(struct buf_core_mgr_s *bc)
{
	if (!bc_sanity_check(bc)) {
		return 0;
	}

	return bc->free_num;
}

static bool buf_core_empty(struct buf_core_mgr_s *bc)
{
	if (!bc_sanity_check(bc)) {
		return true;
	}

	return list_empty(&bc->free_que);
}

static void buf_core_reset(struct buf_core_mgr_s *bc)
{
	struct buf_core_entry *entry, *tmp;
	struct hlist_node *h_tmp;
	ulong bucket;

	mutex_lock(&bc->mutex);

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"%s, core st:%d, core ref:%d, free:%d\n",
		__func__,
		bc->state,
		kref_read(&bc->core_ref),
		bc->free_num);

	list_for_each_entry_safe(entry, tmp, &bc->free_que, node) {
		list_del(&entry->node);
	}

	hash_for_each_safe(bc->buf_table, bucket, h_tmp, entry, h_node) {
		entry->user = BUF_USER_MAX;
		entry->state = BUF_STATE_INIT;
		atomic_set(&entry->ref, 1);
	}

	bc->free_num = 0;

	mutex_unlock(&bc->mutex);
}

static void buf_core_destroy(struct kref *kref)
{
	struct buf_core_mgr_s *bc =
		container_of(kref, struct buf_core_mgr_s, core_ref);
	struct buf_core_entry *entry, *tmp;

	list_for_each_entry_safe(entry, tmp, &bc->free_que, node) {
		list_del(&entry->node);
	}

	bc->free_num	= 0;
	bc->buf_num	= 0;
	bc->state	= BM_STATE_EXIT;

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR, "%s\n", __func__);
}

static int buf_core_attach(struct buf_core_mgr_s *bc, ulong key, void *priv)
{
	int ret = 0;
	struct buf_core_entry *entry;
	struct hlist_node *tmp;

	mutex_lock(&bc->mutex);

	hash_for_each_possible_safe(bc->buf_table, entry, tmp, h_node, key) {
		if (key == entry->key) {
			v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
				"reuse buffer, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
				entry->user,
				entry->key,
				entry->state,
				bc->state,
				atomic_read(&entry->ref),
				kref_read(&bc->core_ref),
				bc->free_num);

			entry->user	= BUF_USER_MAX;
			entry->state	= BUF_STATE_INIT;
			entry->vb2	= priv;
			//atomic_set(&entry->ref, 1);

			bc->prepare(bc, entry);

			goto out;
		}
	}

	ret = bc->mem_ops.alloc(bc, &entry, priv);
	if (ret) {
		goto out;
	}

	entry->key	= key;
	entry->priv	= bc;
	entry->vb2	= priv;
	entry->user	= BUF_USER_MAX;
	entry->state	= BUF_STATE_INIT;
	atomic_set(&entry->ref, 1);

	hash_add(bc->buf_table, &entry->h_node, key);

	bc->state	= BM_STATE_ACTIVE;
	bc->buf_num++;
	kref_get(&bc->core_ref);

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
		"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
		__func__,
		entry->user,
		entry->key,
		entry->state,
		bc->state,
		atomic_read(&entry->ref),
		kref_read(&bc->core_ref),
		bc->free_num);
out:
	mutex_unlock(&bc->mutex);

	return ret;
}

static void buf_core_detach(struct buf_core_mgr_s *bc, ulong key)
{
	struct buf_core_entry *entry;
	struct hlist_node *h_tmp;

	mutex_lock(&bc->mutex);

	hash_for_each_possible_safe(bc->buf_table, entry, h_tmp, h_node, key) {
		if (key == entry->key) {
			v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR,
				"%s, user:%d, key:%lx, st:(%d, %d), ref:(%d, %d), free:%d\n",
				__func__,
				entry->user,
				entry->key,
				entry->state,
				bc->state,
				atomic_read(&entry->ref),
				kref_read(&bc->core_ref),
				bc->free_num);

			entry->state = BUF_STATE_ERR;
			hash_del(&entry->h_node);
			bc->mem_ops.free(bc, entry);

			kref_put(&bc->core_ref, buf_core_destroy);
			break;
		}
	}

	mutex_unlock(&bc->mutex);
}

void buf_core_walk(struct buf_core_mgr_s *bc)
{
	struct buf_core_entry *entry, *tmp;
	struct hlist_node *h_tmp;
	ulong bucket;
	int dec_holders = 0;
	int ge2d_holders = 0;
	int vpp_holders = 0;
	int vsink_holders = 0;

	mutex_lock(&bc->mutex);

	pr_info("\nFree queue elements:\n");
	list_for_each_entry_safe(entry, tmp, &bc->free_que, node) {
		v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_PRINFO,
			"--> key:%lx, user:%d, holder:%d, st:(%d, %d), ref:(%d, %d), free:%d\n",
			entry->key,
			entry->user,
			entry->holder,
			entry->state,
			bc->state,
			atomic_read(&entry->ref),
			kref_read(&bc->core_ref),
			bc->free_num);
	}

	pr_info("\nHash table elements:\n");
	hash_for_each_safe(bc->buf_table, bucket, h_tmp, entry, h_node) {
		if (entry->holder == BUF_HOLDER_DEC)
			dec_holders++;
		if (entry->holder == BUF_HOLDER_GE2D)
			ge2d_holders++;
		if (entry->holder == BUF_HOLDER_VPP)
			vpp_holders++;
		if (entry->holder == BUF_HOLDER_VSINK)
			vsink_holders++;
		v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_PRINFO,
			"--> key:%lx, user:%d, holder:%d, st:(%d, %d), ref:(%d, %d), free:%d, ref_map:0x%x\n",
			entry->key,
			entry->user,
			entry->holder,
			entry->state,
			bc->state,
			atomic_read(&entry->ref),
			kref_read(&bc->core_ref),
			bc->free_num,
			entry->ref_bit_map);
	}
	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_PRINFO, "holders: dec(%d), ge2d(%d), vpp(%d), vsink(%d)\n",
		dec_holders, ge2d_holders, vpp_holders, vsink_holders);

	mutex_unlock(&bc->mutex);
}
EXPORT_SYMBOL(buf_core_walk);

int buf_core_mgr_init(struct buf_core_mgr_s *bc)
{
	/* Sanity check of mandatory interfaces. */
	if (WARN_ON(!bc->config) ||
		WARN_ON(!bc->input) ||
		WARN_ON(!bc->output) ||
		WARN_ON(!bc->mem_ops.alloc) ||
		WARN_ON(!bc->mem_ops.free)) {
		return -1;
	}

	hash_init(bc->buf_table);
	INIT_LIST_HEAD(&bc->free_que);
	mutex_init(&bc->mutex);
	kref_init(&bc->core_ref);

	bc->free_num		= 0;
	bc->buf_num		= 0;
	bc->state		= BM_STATE_INIT;

	/* The external interfaces of BC context. */
	bc->attach		= buf_core_attach;
	bc->detach		= buf_core_detach;
	bc->reset		= buf_core_reset;

	/* The interface set of the buffer core operation. */
	bc->buf_ops.get		= buf_core_get;
	bc->buf_ops.put		= buf_core_put;
	bc->buf_ops.get_ref	= buf_core_get_ref;
	bc->buf_ops.put_ref	= buf_core_put_ref;
	bc->buf_ops.done	= buf_core_done;
	bc->buf_ops.fill	= buf_core_fill;
	bc->buf_ops.ready_num	= buf_core_ready_num;
	bc->buf_ops.empty	= buf_core_empty;
	bc->buf_ops.update_holder = buf_core_update_holder;

	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR, "%s\n", __func__);

	return 0;
}
EXPORT_SYMBOL(buf_core_mgr_init);

void buf_core_mgr_release(struct buf_core_mgr_s *bc)
{
	v4l_dbg_ext(bc->id, V4L_DEBUG_CODEC_BUFMGR, "%s\n", __func__);

	kref_put(&bc->core_ref, buf_core_destroy);
}
EXPORT_SYMBOL(buf_core_mgr_release);

