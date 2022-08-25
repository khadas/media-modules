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
#include <linux/io.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <trace/hooks/v4l2.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-v4l2.h>
#include "../../amvdec_ports/aml_vcodec_drv.h"
#include "vendor_hooks.h"

void v4l2_meta_ptr_update(void *data, struct vb2_v4l2_buffer *vbuf, struct v4l2_buffer *b)
{
	vbuf->android_kabi_reserved1 = (ulong)(((u64)b->reserved2 << 32) | b->reserved);
}

void v4l2_fill_fmtdesc(void *data, struct v4l2_fmtdesc *format)
{
	const char *descr = NULL;
	u32 flags = V4L2_FMT_FLAG_COMPRESSED;

	switch (format->pixelformat) {
	case V4L2_PIX_FMT_AV1:		descr = "AV1"; break;
	case V4L2_PIX_FMT_AVS:		descr = "AVS"; break;
	case V4L2_PIX_FMT_AVS2:		descr = "AVS2"; break;
	default:
		flags = 0;
		pr_err("Unknown pixelformat 0x%08x\n",
			format->pixelformat);
	}

	strscpy(format->description, descr,
			sizeof(format->description));
	format->flags = format->flags | flags;
}

void register_media_modules_vendor_hooks()
{
	register_trace_android_vh_v4l2_meta_ptr_update(v4l2_meta_ptr_update, NULL);
	register_trace_android_vh_v4l2_fill_fmtdesc(v4l2_fill_fmtdesc, NULL);
}
EXPORT_SYMBOL(register_media_modules_vendor_hooks);

void unregister_media_modules_vendor_hooks()
{
	unregister_trace_android_vh_v4l2_meta_ptr_update(v4l2_meta_ptr_update, NULL);
	unregister_trace_android_vh_v4l2_fill_fmtdesc(v4l2_fill_fmtdesc, NULL);
}
EXPORT_SYMBOL(unregister_media_modules_vendor_hooks);
