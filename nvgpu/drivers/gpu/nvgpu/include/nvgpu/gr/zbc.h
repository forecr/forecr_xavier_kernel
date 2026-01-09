/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZBC_H
#define NVGPU_GR_ZBC_H

#include <nvgpu/types.h>


#define NVGPU_GR_ZBC_COLOR_VALUE_SIZE	4U  /* RGBA */

/* index zero reserved to indicate "not ZBCd" */
#define NVGPU_GR_ZBC_STARTOF_TABLE	1U

#define NVGPU_GR_ZBC_TYPE_INVALID	0U
#define NVGPU_GR_ZBC_TYPE_COLOR		1U
#define NVGPU_GR_ZBC_TYPE_DEPTH		2U
#define NVGPU_GR_ZBC_TYPE_STENCIL	3U

/* Opaque black (i.e. solid black, fmt 0x28 = A8B8G8R8) */
#define GR_ZBC_SOLID_BLACK_COLOR_FMT		0x28
/* Transparent black = (fmt 1 = zero) */
#define GR_ZBC_TRANSPARENT_BLACK_COLOR_FMT	0x1
/* Opaque white (i.e. solid white) = (fmt 2 = uniform 1) */
#define GR_ZBC_SOLID_WHITE_COLOR_FMT		0x2
/* z format with fp32 */
#define GR_ZBC_Z_FMT_VAL_FP32			0x1

#define GR_ZBC_STENCIL_CLEAR_FMT_INVAILD	0U
#define GR_ZBC_STENCIL_CLEAR_FMT_U8		1U

/* GA10y+ supports only 0 and 1 depth values */
#define MAX_DEPTH_INDEX		2U

struct gk20a;

struct nvgpu_gr_zbc_table_indices {
	u32 min_color_index;
	u32 min_depth_index;
	u32 min_stencil_index;
	u32 max_color_index;
	u32 max_depth_index;
	u32 max_stencil_index;
};

struct nvgpu_gr_zbc_query_params {
	u32 color_ds[NVGPU_GR_ZBC_COLOR_VALUE_SIZE];
	u32 color_l2[NVGPU_GR_ZBC_COLOR_VALUE_SIZE];
	u32 depth;
	u32 stencil;
	u32 ref_cnt;
	u32 format;
	u32 type;	/* color or depth */
	u32 index_size;	/* [out] size, [in] index */
};

struct zbc_color_table {
	u32 color_ds[NVGPU_GR_ZBC_COLOR_VALUE_SIZE];
	u32 color_l2[NVGPU_GR_ZBC_COLOR_VALUE_SIZE];
	u32 format;
	u32 ref_cnt;
};

struct zbc_depth_table {
	u32 depth;
	u32 format;
	u32 ref_cnt;
};

struct zbc_stencil_table {
	u32 stencil;
	u32 format;
	u32 ref_cnt;
};

struct nvgpu_gr_zbc_entry {
	u32 color_ds[NVGPU_GR_ZBC_COLOR_VALUE_SIZE];
	u32 color_l2[NVGPU_GR_ZBC_COLOR_VALUE_SIZE];
	u32 depth;
	u32 stencil;
	u32 type;
	u32 format;
};
/*
 * HW ZBC table valid entries start at index 1.
 * Entry 0 is reserved to mean "no matching entry found, do not use ZBC"
 */
struct nvgpu_gr_zbc {
	struct nvgpu_mutex zbc_lock;	/* Lock to access zbc table */
	struct zbc_color_table *zbc_col_tbl; /* SW zbc color table pointer */
	struct zbc_depth_table *zbc_dep_tbl; /* SW zbc depth table pointer */
	struct zbc_stencil_table *zbc_s_tbl; /* SW zbc stencil table pointer */
	u32 min_color_index;	/* Minimum valid color table index */
	u32 min_depth_index;	/* Minimum valid depth table index */
	u32 min_stencil_index;	/* Minimum valid stencil table index */
	u32 max_color_index;	/* Maximum valid color table index */
	u32 max_depth_index;	/* Maximum valid depth table index */
	u32 max_stencil_index;	/* Maximum valid stencil table index */
	u32 max_used_color_index; /* Max used color table index */
	u32 max_used_depth_index; /* Max used depth table index */
	u32 max_used_stencil_index; /* Max used stencil table index */
};

int nvgpu_gr_zbc_init(struct gk20a *g, struct nvgpu_gr_zbc **zbc);
void nvgpu_gr_zbc_deinit(struct gk20a *g, struct nvgpu_gr_zbc *zbc);
void nvgpu_gr_zbc_load_table(struct gk20a *g, struct nvgpu_gr_zbc *zbc);
#ifdef CONFIG_NVGPU_ZBC_USER
int nvgpu_gr_zbc_query_table(struct gk20a *g, struct nvgpu_gr_zbc *zbc,
			     struct nvgpu_gr_zbc_query_params *query_params);
int nvgpu_gr_zbc_set_table(struct gk20a *g, struct nvgpu_gr_zbc *zbc,
			   struct nvgpu_gr_zbc_entry *zbc_val);

struct nvgpu_gr_zbc_entry *nvgpu_gr_zbc_entry_alloc(struct gk20a *g);
void nvgpu_gr_zbc_entry_free(struct gk20a *g, struct nvgpu_gr_zbc_entry *entry);
void nvgpu_gr_zbc_set_entry_color_ds(struct nvgpu_gr_zbc_entry *entry,
		u32 idx, u32 ds);
void nvgpu_gr_zbc_set_entry_color_l2(struct nvgpu_gr_zbc_entry *entry,
		u32 idx, u32 l2);
void nvgpu_gr_zbc_set_entry_depth(struct nvgpu_gr_zbc_entry *entry, u32 depth);
void nvgpu_gr_zbc_set_entry_stencil(struct nvgpu_gr_zbc_entry *entry,
		u32 stencil);
u32 nvgpu_gr_zbc_get_entry_type(struct nvgpu_gr_zbc_entry *entry);
void nvgpu_gr_zbc_set_entry_type(struct nvgpu_gr_zbc_entry *entry, u32 type);
void nvgpu_gr_zbc_set_entry_format(struct nvgpu_gr_zbc_entry *entry,
		u32 format);
u32 nvgpu_gr_zbc_get_entry_color_ds(struct nvgpu_gr_zbc_entry *entry, u32 idx);
#endif
u32 nvgpu_gr_zbc_get_entry_stencil(struct nvgpu_gr_zbc_entry *entry);
u32 nvgpu_gr_zbc_get_entry_depth(struct nvgpu_gr_zbc_entry *entry);
u32 nvgpu_gr_zbc_get_entry_format(struct nvgpu_gr_zbc_entry *entry);
u32 nvgpu_gr_zbc_get_entry_color_l2(struct nvgpu_gr_zbc_entry *entry, u32 idx);

#endif /* NVGPU_GR_ZBC_H */
