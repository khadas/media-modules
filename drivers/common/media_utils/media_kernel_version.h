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
#ifndef __MEDIA_KERNEL_VERSION_H__
#define __MEDIA_KERNEL_VERSION_H__

#include <linux/version.h>
ssize_t media_write(struct file *, const void *, size_t, loff_t *);
ssize_t media_read(struct file *, void *, size_t, loff_t *);
struct file *media_open(const char *, int, umode_t);
int media_close(struct file *, fl_owner_t);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(6, 3, 13)
#define KV_CLASS_CONST
#define KV_CLASS_ATTR_CONST
#else
#define KV_CLASS_CONST const      //kernel 6.6 change the type of parameter to const
#define KV_CLASS_ATTR_CONST const //kernel 6.6 change the type of parameter to const
#endif
#endif
