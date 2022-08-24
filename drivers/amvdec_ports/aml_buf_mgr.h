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
#ifndef _AML_DEC_BUFMGR_H_
#define _AML_DEC_BUFMGR_H_

#include <media/videobuf2-core.h>
#include <media/videobuf2-v4l2.h>
#include <media/v4l2-event.h>
#include <media/v4l2-mem2mem.h>
#include <media/videobuf2-dma-contig.h>
#include <media/videobuf2-dma-sg.h>

#include "aml_buf_core.h"

#define BUF_FBC_NUM_MAX		(64)
#define BUF_MAX_PLANES		(3)

struct aml_buf_mgr_s;

/*
 * struct aml_buf_config - Parameter configuration structure.
 *
 * @enable_extbuf	: Configure the external buffer mode.
 * @buf_size		: The size of buffer that should be allocated.
 * @enable_fbc		: Enables the AFBC feature.
 * @enable_secure	: Indicates the secure mode.
 * @memory_mode		: memory mode used by v4l2 vb queue.
 * @planes		: The number of planes used.
 * @luma_length		: The size of the image brightness data.
 * @chroma_length	: The size of the image chroma data.
 */
struct aml_buf_config {
	bool	enable_extbuf;
	bool	enable_fbc;
	bool 	enable_secure;
	int	memory_mode;
	int	planes;
	u32	luma_length;
	u32	chroma_length;
};

/*
 * struct aml_buf_plane - Buffer plane information.
 *
 * @dbuf	: The address of dmabuf.
 * @addr	: Indicates the associated physical address.
 * @vaddr	: Indicates the virtual address associated with plane.
 * @length	: The actual size of the buffer.
 * @bytes_used	: The size of the buffer to be used.
 * @offset	: The offset position used by buffer.
 * @stride	: The size of each line of image data.
 * @uvm_fd	: The file handle of UVM buffer.
 */
struct aml_buf_plane {
	struct dma_buf	*dbuf;
	ulong		addr;
	void		*vaddr;
	u32		length;
	u32		bytes_used;
	u32		offset;
	int		stride;
	int		uvm_fd;
};

/*
 * struct aml_buf_fbc_info - AFBC buffer size information.
 *
 * @max_size	: Max size needed for mmu box.
 * @header_size	: Continuous size for the compressed header.
 * @frame_size	: SG page number to store the frame.
 */
struct aml_buf_fbc_info {
	u32		max_size;
	u32		header_size;
	u32		frame_size;
};

typedef void (*get_fbc_info)(struct aml_buf_mgr_s *,
			    struct aml_buf_fbc_info *);

/*
 * struct aml_buf_fbc - AFBC buffer information.
 *
 * @index	: The serial number of the buffer.
 * @bmmu	: The context of bmmu box.
 * @mmu		: The context of mmu box.
 * @ref		: Reference count of AFBC buffer.
 * @haddr	: The address of header data.
 * @hsize	: The size of header data.
 * @frame_size	: the size of AFBC data per frame.
 */
struct aml_buf_fbc {
	u32		index;
	void		*bmmu;
	void		*mmu;
	int		ref;
	ulong		haddr;
	u32		hsize;
	u32		frame_size;
	int		used[BUF_FBC_NUM_MAX];
};

/*
 * struct aml_buf - aml_buf structure.
 *
 * @index	: The serial number of the buffer.
 * @state	: Indicates the usage status of the aml_buf.
 * @num_planes	: The number of planes used.
 * @planes	: Buffer plane information.
 * @fbc		: AFBC buffer information.
 * @entry	: Buffer entity embedded in aml_buf.
 * @task	: The context of task chain.
 * @vframe	: The video frame struct.
 * @vb		: The vb2 struct defined by v4l2.
 * @meta_ptr	: The handle of meta date.
 */
struct aml_buf {
	u32			index;
	int			state;
	u32			num_planes;
	u64			timestamp;
	struct aml_buf_plane	planes[BUF_MAX_PLANES];
	struct aml_buf_fbc	*fbc;
	struct buf_core_entry	entry;

	struct task_chain_s	*task;
	struct vframe_s		vframe;
	struct vb2_buffer	*vb;
	ulong			meta_ptr;
	void			*vpp_buf;
	void			*ge2d_buf;
};

/*
 * struct aml_buf_mgr_s - aml_buf manager context.
 *
 * @bc		: The buffer core manager context.
 * @ref		: The reference count of buffer manager.
 * @priv	: Records the context of the caller.
 * @config	: Parameter configuration structure.
 * @bmmu	: The context of bmmu box.
 * @mmu		: The context of mmu box.
 * @fbc_array	: AFBC buffer array data.
 * @get_fbc_info : Used to get AFBC data size information.
 */
struct aml_buf_mgr_s {
	struct buf_core_mgr_s		bc;
	struct kref			ref;
	void				*priv;

	struct aml_buf_config		config;

	/* fbc information */
	void				*bmmu;
	void				*mmu;
	struct aml_buf_fbc		*fbc_array;
	get_fbc_info			get_fbc_info;
};

/*
 * bc_to_bm() - Used for BC to BM.
 *
 * @bc		: Pointer to &struct buf_core_mgr_s buffer core manager context.
 *
 * It is easy to convert BC to BM.
 *
 * Return	: returns bm context.
 */
static inline struct aml_buf_mgr_s *bc_to_bm(struct buf_core_mgr_s *bc)
{
	return container_of(bc, struct aml_buf_mgr_s, bc);
}

/*
 * entry_to_aml_buf() - Used for entry to aml_buf.
 *
 * @bc		: Pointer to &struct buf_core_mgr_s buffer core manager context.
 *
 * It is easy to convert entry to aml_buf.
 *
 * Return	: returns aml_buf
 */
static inline struct aml_buf *entry_to_aml_buf(void *entry)
{
	return container_of(entry, struct aml_buf, entry);
}

/*
 * aml_buf_mgr_init() - buffer core management initialization.
 *
 * @bm		: Pointer to &struct aml_buf_mgr_s buffer manager context.
 * @name	: The name of buffer manager.
 * @id		: The instance ID of buffer manager.
 * @priv	: The private date from caller.
 *
 * Used to initialize the buffer manager context.
 *
 * Return	: returns zero on success; an error code otherwise
 */
int aml_buf_mgr_init(struct aml_buf_mgr_s *bm, char *name, int id, void *priv);

/*
 * aml_buf_mgr_release() - buffer core management initialization.
 *
 * @bm		: pointer to &struct buf_core_mgr_s buffer manager context.
 *
 * Used to initialize the buffer manager context.
 */
void aml_buf_mgr_release(struct aml_buf_mgr_s *bm);

#endif //_AML_DEC_BUFMGR_H_

