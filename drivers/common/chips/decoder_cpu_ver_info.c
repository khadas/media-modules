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
#include <linux/kernel.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/amlogic/media/registers/cpu_version.h>
#include "decoder_cpu_ver_info.h"
#include "../register/register.h"

#define DECODE_CPU_VER_ID_NODE_NAME "cpu_ver_name"
#define CODEC_DOS_DEV_ID_NODE_NAME  "vcodec_dos_dev"

#define AM_SUCCESS 0
#define MAJOR_ID_START AM_MESON_CPU_MAJOR_ID_M6

static bool codec_dos_dev = 0;

inline bool is_support_new_dos_dev(void)
{
	return codec_dos_dev;
}
EXPORT_SYMBOL(is_support_new_dos_dev);

static enum AM_MESON_CPU_MAJOR_ID cpu_ver_id = AM_MESON_CPU_MAJOR_ID_MAX; //AM_MESON_CPU_MAJOR_ID_MAX;
static int cpu_sub_id = 0;

struct dos_of_dev_s {
	enum AM_MESON_CPU_MAJOR_ID chip_id;
	reg_compat_func reg_compat;
};

static struct dos_of_dev_s dos_dev_data[AM_MESON_CPU_MAJOR_ID_MAX - MAJOR_ID_START] = {
	[AM_MESON_CPU_MAJOR_ID_M8B - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_M8B,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_GXL - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_GXL,
		.reg_compat = NULL,
	},

	[AM_MESON_CPU_MAJOR_ID_G12A - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_G12A,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_G12B - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_G12B,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_GXLX2 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_GXLX2,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_SM1 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_SM1,
		.reg_compat = NULL,
		},
	[AM_MESON_CPU_MAJOR_ID_TL1 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_TL1,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_TM2 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_TM2,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_SC2 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_SC2,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_T5 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_T5,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_T5D - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_T5D,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_T7 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_T7,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_S4 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_S4,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_T3 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_T3,
		.reg_compat = t3_mm_registers_compat,
	},
	[AM_MESON_CPU_MAJOR_ID_S4D - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_S4D,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_T5W - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_T5W,
		.reg_compat = NULL,
	},
	[AM_MESON_CPU_MAJOR_ID_S5 - MAJOR_ID_START] = {
		.chip_id = AM_MESON_CPU_MAJOR_ID_S5,
		.reg_compat = s5_mm_registers_compat,
	},
};

static const struct of_device_id cpu_ver_of_match[] = {
	{
		.compatible = "amlogic, cpu-major-id-axg",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_AXG - MAJOR_ID_START],
	},

	{
		.compatible = "amlogic, cpu-major-id-g12a",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_G12A - MAJOR_ID_START],
	},

	{
		.compatible = "amlogic, cpu-major-id-gxl",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_GXL - MAJOR_ID_START],
	},

	{
		.compatible = "amlogic, cpu-major-id-gxm",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_GXM - MAJOR_ID_START],
	},

	{
		.compatible = "amlogic, cpu-major-id-txl",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_TXL - MAJOR_ID_START],
	},

	{
		.compatible = "amlogic, cpu-major-id-txlx",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_TXLX - MAJOR_ID_START],
	},

	{
		.compatible = "amlogic, cpu-major-id-sm1",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_SM1 - MAJOR_ID_START],
	},

	{
		.compatible = "amlogic, cpu-major-id-tl1",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_TL1 - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-tm2",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_TM2 - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-sc2",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_SC2 - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-t5",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_T5 - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-t5d",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_T5D - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-t7",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_T7 - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-s4",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_S4 - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-t3",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_T3 - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-p1",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_P1 - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-s4d",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_S4D - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-t5w",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_T5W - MAJOR_ID_START],
	},
	{
		.compatible = "amlogic, cpu-major-id-s5",
		.data = &dos_dev_data[AM_MESON_CPU_MAJOR_ID_S5 - MAJOR_ID_START],
	},
	{},
};

static struct dos_of_dev_s dos_dev_sub_table[] = {
	{
		.chip_id = AM_MESON_CPU_MINOR_ID_REVB_G12B,
	},
	{
		.chip_id = AM_MESON_CPU_MINOR_ID_REVB_TM2,
	},
	{
		.chip_id = AM_MESON_CPU_MINOR_ID_S4_S805X2,
	},
};

static const struct of_device_id cpu_sub_id_of_match[] = {
	{
		.compatible = "amlogic, cpu-major-id-g12b-b",
		.data = &dos_dev_sub_table[0],
	},
	{
		.compatible = "amlogic, cpu-major-id-tm2-b",
		.data = &dos_dev_sub_table[1],
	},
	{
		.compatible = "amlogic, cpu-major-id-s4-805x2",
		.data = &dos_dev_sub_table[2],
	},
};

static struct platform_device *get_dos_dev_from_dtb(void)
{
	struct device_node *pnode = NULL;

	pnode = of_find_node_by_name(NULL, CODEC_DOS_DEV_ID_NODE_NAME);
	if (pnode == NULL) {
		pnode = of_find_node_by_name(NULL, DECODE_CPU_VER_ID_NODE_NAME);
		if (pnode == NULL) {
			pr_err("No find node.\n");
			return NULL;
		}
	} else {
		codec_dos_dev = 1;
	}

	return of_find_device_by_node(pnode);
}

static struct dos_of_dev_s * get_dos_of_dev_data(struct platform_device *pdev)
{
	const struct of_device_id *pmatch = NULL;

	if (pdev == NULL)
		return NULL;

	pmatch = of_match_device(cpu_ver_of_match, &pdev->dev);
	if (NULL == pmatch) {
		pmatch = of_match_device(cpu_sub_id_of_match, &pdev->dev);
		if (NULL == pmatch) {
			pr_err("No find of_match_device\n");
			return NULL;
		}
	}

	return (struct dos_of_dev_s *)pmatch->data;
}

struct platform_device *initial_dos_device(void)
{
	struct platform_device *pdev = NULL;
	struct dos_of_dev_s *of_dev_data;

	pdev = get_dos_dev_from_dtb();

	of_dev_data = get_dos_of_dev_data(pdev);

	if (of_dev_data) {
		cpu_ver_id = of_dev_data->chip_id & MAJOY_ID_MASK;
		cpu_sub_id = (of_dev_data->chip_id & SUB_ID_MASK) >> 8;
	} else {
		cpu_ver_id = (enum AM_MESON_CPU_MAJOR_ID)get_cpu_type();
		cpu_sub_id = (is_meson_rev_b()) ? CHIP_REVB : CHIP_REVA;
	}
	if ((cpu_ver_id == AM_MESON_CPU_MAJOR_ID_G12B) && (cpu_sub_id == CHIP_REVB))
		cpu_ver_id = AM_MESON_CPU_MAJOR_ID_TL1;

	pr_info("vdec init cpu id: 0x%x(%d)", cpu_ver_id, cpu_sub_id);

	if (pdev && of_dev_data)
		dos_register_probe(pdev, of_dev_data->reg_compat);

	pr_info("initial_dos_device end\n");

	return pdev;
}
EXPORT_SYMBOL(initial_dos_device);


enum AM_MESON_CPU_MAJOR_ID get_cpu_major_id(void)
{
	if (AM_MESON_CPU_MAJOR_ID_MAX == cpu_ver_id)
		initial_dos_device();

	return cpu_ver_id;
}
EXPORT_SYMBOL(get_cpu_major_id);

int get_cpu_sub_id(void)
{
	return cpu_sub_id;
}
EXPORT_SYMBOL(get_cpu_sub_id);

bool is_cpu_meson_revb(void)
{
	if (AM_MESON_CPU_MAJOR_ID_MAX == cpu_ver_id)
		initial_dos_device();

	return (cpu_sub_id == CHIP_REVB);
}
EXPORT_SYMBOL(is_cpu_meson_revb);

bool is_cpu_tm2_revb(void)
{
	return ((get_cpu_major_id() == AM_MESON_CPU_MAJOR_ID_TM2)
		&& (is_cpu_meson_revb()));
}
EXPORT_SYMBOL(is_cpu_tm2_revb);

bool is_cpu_s4_s805x2(void)
{
	return ((get_cpu_major_id() == AM_MESON_CPU_MAJOR_ID_S4)
		&& (get_cpu_sub_id() == CHIP_REVX));
}
EXPORT_SYMBOL(is_cpu_s4_s805x2);

