/*
 * Copyright (c) 2020-2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef ZBC_GA10B_H
#define ZBC_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_zbc_table_indices;
struct nvgpu_gr_zbc;

void ga10b_gr_zbc_init_table_indices(struct gk20a *g,
			struct nvgpu_gr_zbc_table_indices *zbc_indices);
void ga10b_gr_zbc_add_color(struct gk20a *g,
			struct nvgpu_gr_zbc_entry *color_val, u32 index);
void ga10b_gr_zbc_load_static_table(struct gk20a *g,
					struct nvgpu_gr_zbc *zbc);
#define NVGPU_ZBC_SET_COLOR_ATTR(tbl, ind, i)				\
{									\
	(tbl)[ind].color_ds[0] = (NVGPU_ZBC_CL_VAL_1_IND_##i);		\
	(tbl)[ind].color_ds[1] = (NVGPU_ZBC_CL_VAL_2_IND_##i);		\
	(tbl)[ind].color_ds[2] = (NVGPU_ZBC_CL_VAL_3_IND_##i);		\
	(tbl)[ind].color_ds[3] = (NVGPU_ZBC_CL_VAL_4_IND_##i);		\
	(tbl)[ind].color_l2[0] = (NVGPU_ZBC_CL_VAL_1_IND_##i);		\
	(tbl)[ind].color_l2[1] = (NVGPU_ZBC_CL_VAL_2_IND_##i);		\
	(tbl)[ind].color_l2[2] = (NVGPU_ZBC_CL_VAL_3_IND_##i);		\
	(tbl)[ind].color_l2[3] = (NVGPU_ZBC_CL_VAL_4_IND_##i);		\
	(tbl)[ind].ref_cnt = nvgpu_safe_add_u32((tbl)[ind].ref_cnt, 1U);\
	ind = nvgpu_safe_add_u32(ind, 1U);				\
}

#define NVGPU_ZBC_CL_VAL_1_IND_1		0xff000000U
#define NVGPU_ZBC_CL_VAL_2_IND_1		0xff000000U
#define NVGPU_ZBC_CL_VAL_3_IND_1		0xff000000U
#define NVGPU_ZBC_CL_VAL_4_IND_1		0xff000000U

#define NVGPU_ZBC_CL_VAL_1_IND_2		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_2		0x00000000U
#define NVGPU_ZBC_CL_VAL_3_IND_2		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_2		0x00000000U

#define NVGPU_ZBC_CL_VAL_1_IND_3		0xffffffffU
#define NVGPU_ZBC_CL_VAL_2_IND_3		0xffffffffU
#define NVGPU_ZBC_CL_VAL_3_IND_3		0xffffffffU
#define NVGPU_ZBC_CL_VAL_4_IND_3		0xffffffffU

#ifndef CONFIG_NVGPU_NON_FUSA
#define NVGPU_ZBC_CL_VAL_1_IND_4		0x7f000000U
#define NVGPU_ZBC_CL_VAL_2_IND_4		0x7f000000U
#define NVGPU_ZBC_CL_VAL_3_IND_4		0x7f000000U
#define NVGPU_ZBC_CL_VAL_4_IND_4		0x7f000000U

#define NVGPU_ZBC_CL_VAL_1_IND_5		0x40000000U
#define NVGPU_ZBC_CL_VAL_2_IND_5		0x40000000U
#define NVGPU_ZBC_CL_VAL_3_IND_5		0x40000000U
#define NVGPU_ZBC_CL_VAL_4_IND_5		0x40000000U

#define NVGPU_ZBC_CL_VAL_1_IND_6		0xbf000000U
#define NVGPU_ZBC_CL_VAL_2_IND_6		0xbf000000U
#define NVGPU_ZBC_CL_VAL_3_IND_6		0xbf000000U
#define NVGPU_ZBC_CL_VAL_4_IND_6		0xbf000000U

#define NVGPU_ZBC_CL_VAL_1_IND_7		0xff0000ffU
#define NVGPU_ZBC_CL_VAL_2_IND_7		0xff0000ffU
#define NVGPU_ZBC_CL_VAL_3_IND_7		0xff0000ffU
#define NVGPU_ZBC_CL_VAL_4_IND_7		0xff0000ffU

#define NVGPU_ZBC_CL_VAL_1_IND_8		0x00ffffffU
#define NVGPU_ZBC_CL_VAL_2_IND_8		0x00ffffffU
#define NVGPU_ZBC_CL_VAL_3_IND_8		0x00ffffffU
#define NVGPU_ZBC_CL_VAL_4_IND_8		0x00ffffffU

#define NVGPU_ZBC_CL_VAL_1_IND_9		0x7fffffffU
#define NVGPU_ZBC_CL_VAL_2_IND_9		0x7fffffffU
#define NVGPU_ZBC_CL_VAL_3_IND_9		0x7fffffffU
#define NVGPU_ZBC_CL_VAL_4_IND_9		0x7fffffffU

#define NVGPU_ZBC_CL_VAL_1_IND_10		0xffff0000U
#define NVGPU_ZBC_CL_VAL_2_IND_10		0xffff0000U
#define NVGPU_ZBC_CL_VAL_3_IND_10		0xffff0000U
#define NVGPU_ZBC_CL_VAL_4_IND_10		0xffff0000U

#define NVGPU_ZBC_CL_VAL_1_IND_11		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_11		0x3c000000U
#define NVGPU_ZBC_CL_VAL_3_IND_11		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_11		0x3c000000U

#define NVGPU_ZBC_CL_VAL_1_IND_12		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_12		0x38000000U
#define NVGPU_ZBC_CL_VAL_3_IND_12		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_12		0x38000000U

#define NVGPU_ZBC_CL_VAL_1_IND_13		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_13		0x34000000U
#define NVGPU_ZBC_CL_VAL_3_IND_13		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_13		0x34000000U

#define NVGPU_ZBC_CL_VAL_1_IND_14		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_14		0x3a000000U
#define NVGPU_ZBC_CL_VAL_3_IND_14		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_14		0x3a000000U

#define NVGPU_ZBC_CL_VAL_1_IND_15		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_15		0x3c003c00U
#define NVGPU_ZBC_CL_VAL_3_IND_15		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_15		0x3c003c00U

#define NVGPU_ZBC_CL_VAL_1_IND_16		0x3c003c00U
#define NVGPU_ZBC_CL_VAL_2_IND_16		0x3c003c00U
#define NVGPU_ZBC_CL_VAL_3_IND_16		0x3c003c00U
#define NVGPU_ZBC_CL_VAL_4_IND_16		0x3c003c00U

#define NVGPU_ZBC_CL_VAL_1_IND_17		0x3c003c00U
#define NVGPU_ZBC_CL_VAL_2_IND_17		0x00003c00U
#define NVGPU_ZBC_CL_VAL_3_IND_17		0x3c003c00U
#define NVGPU_ZBC_CL_VAL_4_IND_17		0x00003c00U

#define NVGPU_ZBC_CL_VAL_1_IND_18		0x3c003c00U
#define NVGPU_ZBC_CL_VAL_2_IND_18		0x38003c00U
#define NVGPU_ZBC_CL_VAL_3_IND_18		0x3c003c00U
#define NVGPU_ZBC_CL_VAL_4_IND_18		0x38003c00U

#define NVGPU_ZBC_CL_VAL_1_IND_19		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_19		0x00000000U
#define NVGPU_ZBC_CL_VAL_3_IND_19		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_19		0x3f800000U

#define NVGPU_ZBC_CL_VAL_1_IND_20		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_20		0x00000000U
#define NVGPU_ZBC_CL_VAL_3_IND_20		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_20		0x3f000000U

#define NVGPU_ZBC_CL_VAL_1_IND_21		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_21		0x00000000U
#define NVGPU_ZBC_CL_VAL_3_IND_21		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_21		0x3e800000U

#define NVGPU_ZBC_CL_VAL_1_IND_22		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_22		0x00000000U
#define NVGPU_ZBC_CL_VAL_3_IND_22		0x00000000U
#define NVGPU_ZBC_CL_VAL_4_IND_22		0x3f400000U

#define NVGPU_ZBC_CL_VAL_1_IND_23		0x00000000U
#define NVGPU_ZBC_CL_VAL_2_IND_23		0x00000000U
#define NVGPU_ZBC_CL_VAL_3_IND_23		0x3f800000U
#define NVGPU_ZBC_CL_VAL_4_IND_23		0x3f800000U

#define NVGPU_ZBC_CL_VAL_1_IND_24		0x3f800000U
#define NVGPU_ZBC_CL_VAL_2_IND_24		0x3f800000U
#define NVGPU_ZBC_CL_VAL_3_IND_24		0x3f800000U
#define NVGPU_ZBC_CL_VAL_4_IND_24		0x3f800000U

#define NVGPU_ZBC_CL_VAL_1_IND_25		0x3f800000U
#define NVGPU_ZBC_CL_VAL_2_IND_25		0x3f800000U
#define NVGPU_ZBC_CL_VAL_3_IND_25		0x3f800000U
#define NVGPU_ZBC_CL_VAL_4_IND_25		0x00000000U

#define NVGPU_ZBC_CL_VAL_1_IND_26		0x3f800000U
#define NVGPU_ZBC_CL_VAL_2_IND_26		0x3f800000U
#define NVGPU_ZBC_CL_VAL_3_IND_26		0x3f800000U
#define NVGPU_ZBC_CL_VAL_4_IND_26		0x3f000000U
#endif

#define NVGPU_ZBC_SET_DEPTH_ATTR(tbl, i)			\
{								\
	(tbl).depth = (NVGPU_ZBC_DPTH_##i);			\
	(tbl).format = (NVGPU_ZBC_DPTH_FMT_##i);		\
	(tbl).ref_cnt = nvgpu_safe_add_u32((tbl).ref_cnt, 1U);	\
}

#define NVGPU_ZBC_DPTH_1		0x3f800000U
#define NVGPU_ZBC_DPTH_FMT_1		GR_ZBC_Z_FMT_VAL_FP32
#define NVGPU_ZBC_DPTH_2		0U
#define NVGPU_ZBC_DPTH_FMT_2		GR_ZBC_Z_FMT_VAL_FP32

#define NVGPU_ZBC_SET_STENCIL_ATTR(tbl, i)			\
{								\
	(tbl).stencil = (NVGPU_ZBC_SCIL_##i);			\
	(tbl).format = (NVGPU_ZBC_SCIL_FMT_##i);		\
	(tbl).ref_cnt = nvgpu_safe_add_u32((tbl).ref_cnt, 1U);	\
}

#define NVGPU_ZBC_SCIL_1		0x00000000U
#define NVGPU_ZBC_SCIL_FMT_1		GR_ZBC_STENCIL_CLEAR_FMT_U8
#define NVGPU_ZBC_SCIL_2		0x00000001U
#define NVGPU_ZBC_SCIL_FMT_2		GR_ZBC_STENCIL_CLEAR_FMT_U8
#define NVGPU_ZBC_SCIL_3		0xffU
#define NVGPU_ZBC_SCIL_FMT_3		GR_ZBC_STENCIL_CLEAR_FMT_U8
#endif /* ZBC_GA10B_H */
