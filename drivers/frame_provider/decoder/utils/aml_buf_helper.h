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
#ifndef _AML_BUF_HELPER_H_
#define _AML_BUF_HELPER_H_

#include "../../../amvdec_ports/aml_buf_mgr.h"

/*
 * aml_buf_configure() - Interface for parameter configuration.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @cfg		: Parameter configuration structure.
 *
 * Interface for parameter configuration.
 */
static inline void aml_buf_configure(struct aml_buf_mgr_s *bm,
				    struct aml_buf_config *cfg)
{
	bm->bc.config(&bm->bc, cfg);
}

/*
 * aml_buf_attach() - Use to attach buffers that need to be managed.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @key		: The key information of buffer.
 * @priv	: The priv data from caller.
 *
 * Use to attach buffers that need to be managed.
 */
static inline int aml_buf_attach(struct aml_buf_mgr_s *bm,
				 ulong key, void *priv)
{
	return bm->bc.attach(&bm->bc, key, priv);
}

/*
 * aml_buf_detach() - Use to detach buffers from buffer manager.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @key		: The key information of buffer.
 *
 * Use to detach buffers from buffer manager.
 */
static inline void aml_buf_detach(struct aml_buf_mgr_s *bm, ulong key)
{
	bm->bc.detach(&bm->bc, key);
}

/*
 * aml_buf_reset() - Interface used to reset the state of the buffer manager.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 *
 * Interface used to reset the state of the buffer manager.
 */
static inline void aml_buf_reset(struct aml_buf_mgr_s *bm)
{
	bm->bc.reset(&bm->bc);
}

/*
 * aml_buf_ready_num() - Query the number of buffers in the free queue.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 *
 * Query the number of buffers in the free queue.
 *
 * Return	: returns free numbers in the queue.
 */
static inline int aml_buf_ready_num(struct aml_buf_mgr_s *bm)
{
	return bm->bc.buf_ops.ready_num(&bm->bc);
}

/*
 * aml_buf_empty() - Check whether the free queue is empty.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 *
 * Check whether the free queue is empty.
 *
 * Return	: returns the state of free queue.
 */
static inline bool aml_buf_empty(struct aml_buf_mgr_s *bm)
{
	return bm->bc.buf_ops.empty(&bm->bc);
}

/*
 * aml_buf_done() - The done interface is called if the user finishes fill the data.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @buf		: The structure of ambuf.
 * @user	: The current buffer user.
 *
 * The done interface is called if the user finishes fill the data.
 */
static inline void aml_buf_done(struct aml_buf_mgr_s *bm,
			       struct aml_buf *buf,
			       enum buf_core_user user)
{
	bm->bc.buf_ops.done(&bm->bc, &buf->entry, user);
}

/*
 * aml_buf_fill() - The fill interface is called if the user consumes the data.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @buf		: The structure of ambuf.
 * @user	: The current buffer user.
 *
 * The fill interface is called if the user consumes the data.
 */
static inline void aml_buf_fill(struct aml_buf_mgr_s *bm,
			       struct aml_buf *buf,
			       enum buf_core_user user)
{
	bm->bc.buf_ops.fill(&bm->bc, &buf->entry, user);
}

/*
 * aml_buf_get() - Get a free buffer from free queue.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @user	: The current buffer user.
 * @more_ref	: Need to get more reference.
 *
 * Get a free buffer from free queue.
 *
 * Return	: returns a buffer entry.
 */
static inline struct aml_buf *aml_buf_get(struct aml_buf_mgr_s *bm, int user, bool more_ref)
{
	struct buf_core_entry *entry = NULL;

	bm->bc.buf_ops.get(&bm->bc, user, &entry, more_ref);

	return entry ? entry_to_ambuf(entry) : NULL;
}

/*
 * aml_buf_put() - Put an unused buffer to the free queue.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @buf		: The structure of ambuf
 *
 * Put an unused buffer to the free queue.
 */
static inline void aml_buf_put(struct aml_buf_mgr_s *bm, struct aml_buf *buf)
{
	bm->bc.buf_ops.put(&bm->bc, &buf->entry);
}

/*
 * aml_buf_get_ref() - Increase a reference count to the buffer.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @buf		: The structure of ambuf.
 *
 * Increase a reference count to the buffer.
 */
static inline void aml_buf_get_ref(struct aml_buf_mgr_s *bm,
				  struct aml_buf *buf)
{
#if 0
	if (buf->planes[0].dbuf)
		get_dma_buf(buf->planes[0].dbuf);
	if (buf->planes[1].dbuf)
		get_dma_buf(buf->planes[1].dbuf);
#endif
	bm->bc.buf_ops.get_ref(&bm->bc, &buf->entry);
}

/*
 * aml_buf_put_ref() - Decrease a reference count to the buffer.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @buf		: The structure of ambuf.
 *
 * Decrease a reference count to the buffer.
 */
static inline void aml_buf_put_ref(struct aml_buf_mgr_s *bm, struct aml_buf *buf)
{
	bm->bc.buf_ops.put_ref(&bm->bc, &buf->entry);
#if 0
	if (buf->planes[0].dbuf)
		dma_buf_put(buf->planes[0].dbuf);
	if (buf->planes[1].dbuf)
		dma_buf_put(buf->planes[1].dbuf);
#endif
}

#endif //_AML_BUF_HELPER_H_

