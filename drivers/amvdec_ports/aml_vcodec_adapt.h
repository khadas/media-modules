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
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Description:
 */
#ifndef VDEC_ADAPT_H
#define VDEC_ADAPT_H

#include <linux/amlogic/media/utils/vformat.h>
#include <linux/amlogic/media/utils/amstream.h>
#include <linux/amlogic/media/codec_mm/dmabuf_manage.h>
#include "../stream_input/amports/streambuf.h"
#include "../frame_provider/decoder/utils/vdec_input.h"
#include "aml_vcodec_drv.h"

struct aml_vdec_adapt {
	int format;
	void *vsi;
	int32_t failure;
	uint32_t inst_addr;
	unsigned int signaled;
	struct aml_vcodec_ctx *ctx;
	wait_queue_head_t wq;
	struct file *filp;
	struct vdec_s *vdec;
	struct stream_port_s port;
	struct dec_sysinfo dec_prop;
	struct v4l2_config_parm config;
	int video_type;
	char *frm_name;
};

int video_decoder_init(struct aml_vdec_adapt *ada_ctx);

int video_decoder_release(struct aml_vdec_adapt *ada_ctx);

int vdec_vbuf_write(struct aml_vdec_adapt *ada_ctx,
	const char *buf, unsigned int count);

int vdec_vframe_write(struct aml_vdec_adapt *ada_ctx, const char *buf,
	unsigned int count, u64 timestamp, ulong meta_ptr, chunk_free free);

void vdec_vframe_input_free(void *priv, u32 handle);

int vdec_vframe_write_with_dma(struct aml_vdec_adapt *ada_ctx,
	ulong addr, u32 count, u64 timestamp, u32 handle,
	chunk_free free, void *priv);

bool vdec_input_full(struct aml_vdec_adapt *ada_ctx);

void aml_decoder_flush(struct aml_vdec_adapt *ada_ctx);

int aml_codec_reset(struct aml_vdec_adapt *ada_ctx, int *flag);
void aml_codec_connect(struct aml_vdec_adapt *ada_ctx);
void aml_codec_disconnect(struct aml_vdec_adapt *ada_ctx);

extern void dump_write(const char __user *buf, size_t count);

bool is_input_ready(struct aml_vdec_adapt *ada_ctx);

int vdec_frame_number(struct aml_vdec_adapt *ada_ctx);

void vdec_set_dmabuf_type(struct aml_vdec_adapt *ada_ctx, bool dmabuf_type);

int vdec_get_instance_num(void);

bool vdec_check_is_available(u32 fmt);

void vdec_set_duration(s32 duration);

void vdec_set_vf_duration(s32 duration);

void vdec_write_stream_data(struct aml_vdec_adapt *ada_ctx, u32 addr, u32 size);

void vdec_write_stream_data_inner(struct aml_vdec_adapt *ada_ctx, char *addr, u32 size, u64 timestamp);

void v4l2_set_ext_buf_addr(struct aml_vdec_adapt *ada_ctx, struct dmabuf_dmx_sec_es_data *es_data, int offset);

int vdec_get_vdec_id(struct aml_vdec_adapt *ada_ctx);

void vdec_thread_wakeup(struct aml_vdec_adapt *ada_ctx);

#endif /* VDEC_ADAPT_H */

