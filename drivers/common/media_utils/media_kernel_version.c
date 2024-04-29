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
#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 15, 0)
ssize_t media_write(struct file *file, const void *buf, size_t count, loff_t *pos)
{
	return kernel_write(file, buf, count, pos);
}
EXPORT_SYMBOL(media_write);

ssize_t media_read(struct file *file, void *bufs, size_t count, loff_t *pos)
{
	return kernel_read(file, bufs, count, pos);
}
EXPORT_SYMBOL(media_read);

struct file *media_open(const char *filename, int flags, umode_t mode)
{
	return filp_open(filename, flags, mode);
}
EXPORT_SYMBOL(media_open);

int media_close(struct file *filp, fl_owner_t id)
{
	return filp_close(filp, id);
}
EXPORT_SYMBOL(media_close);
#else
ssize_t media_write(struct file *file, const void *buf, size_t count, loff_t *pos)
{
	return 0;
}
EXPORT_SYMBOL(media_write);

ssize_t media_read(struct file *file, void *bufs, size_t count, loff_t *pos)
{
	return 0;
}
EXPORT_SYMBOL(media_read);

struct file *media_open(const char *filename, int flags, umode_t mode)
{
	return NULL;
}
EXPORT_SYMBOL(media_open);

int media_close(struct file *filp, fl_owner_t id)
{
	return 0;
}
EXPORT_SYMBOL(media_close);
#endif
