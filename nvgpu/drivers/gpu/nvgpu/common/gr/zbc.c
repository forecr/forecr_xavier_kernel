// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/bug.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/gr/zbc.h>
#include <nvgpu/string.h>
#include <nvgpu/power_features/pg.h>
#ifdef CONFIG_NVGPU_LS_PMU
#include <nvgpu/pmu/pmu_pg.h>
#endif

#define ZBC_ENTRY_UPDATED	1
#define ZBC_ENTRY_ADDED		2

static void nvgpu_gr_zbc_update_stencil_reg(struct gk20a *g,
			     struct nvgpu_gr_zbc_entry *stencil_val, u32 index)
{
	/* update l2 table */
	if (g->ops.ltc.set_zbc_s_entry != NULL) {
		g->ops.ltc.set_zbc_s_entry(g, stencil_val->stencil, index);
	}

	/* update zbc stencil registers */
	g->ops.gr.zbc.add_stencil(g, stencil_val, index);
}

#ifdef CONFIG_NVGPU_ZBC_USER
static int nvgpu_gr_zbc_add_stencil(struct gk20a *g, struct nvgpu_gr_zbc *zbc,
			struct nvgpu_gr_zbc_entry *stencil_val)
{
	struct zbc_stencil_table *s_tbl;
	u32 i;
	int entry_added = -ENOSPC;
	bool entry_exist = false;

	/* search existing tables */
	for (i = zbc->min_stencil_index; i <= zbc->max_used_stencil_index;
		i++) {

		s_tbl = &zbc->zbc_s_tbl[i];

		if ((s_tbl->ref_cnt != 0U) &&
		    (s_tbl->stencil == stencil_val->stencil) &&
		    (s_tbl->format == stencil_val->format)) {
			if (s_tbl->ref_cnt == U32_MAX) {
				return entry_added;
			}
			s_tbl->ref_cnt = s_tbl->ref_cnt + 1U;
			entry_exist = true;
			entry_added = ZBC_ENTRY_UPDATED;
			break;
		}
	}
	/* add new table */
	if (!entry_exist &&
		(zbc->max_used_stencil_index < zbc->max_stencil_index)) {

		s_tbl = &zbc->zbc_s_tbl[zbc->max_used_stencil_index + 1U];
		if (s_tbl->ref_cnt != 0U) {
			return entry_added;
		}

		/* Increment used index and add new entry at that index */
		zbc->max_used_stencil_index =
			zbc->max_used_stencil_index + 1U;

		/* update sw copy */
		s_tbl->stencil = stencil_val->stencil;
		s_tbl->format = stencil_val->format;
		s_tbl->ref_cnt = 1U;

		nvgpu_gr_zbc_update_stencil_reg(g, stencil_val,
			zbc->max_used_stencil_index);

		entry_added = ZBC_ENTRY_ADDED;
	}
	return entry_added;
}
#endif

static void nvgpu_gr_zbc_update_depth_reg(struct gk20a *g,
			struct nvgpu_gr_zbc_entry *depth_val, u32 index)
{
	/* update l2 table */
	g->ops.ltc.set_zbc_depth_entry(g, depth_val->depth, index);

	/* update zbc registers */
	g->ops.gr.zbc.add_depth(g, depth_val, index);
}

#ifdef CONFIG_NVGPU_ZBC_USER
static int nvgpu_gr_zbc_add_depth(struct gk20a *g, struct nvgpu_gr_zbc *zbc,
			struct nvgpu_gr_zbc_entry *depth_val)
{
	struct zbc_depth_table *d_tbl;
	u32 i;
	int entry_added = -ENOSPC;
	bool entry_exist = false;

	/* search existing tables */
	for (i = zbc->min_depth_index; i <= zbc->max_used_depth_index; i++) {

		d_tbl = &zbc->zbc_dep_tbl[i];

		if ((d_tbl->ref_cnt != 0U) &&
		    (d_tbl->depth == depth_val->depth) &&
		    (d_tbl->format == depth_val->format)) {
			if (d_tbl->ref_cnt == U32_MAX) {
				return entry_added;
			}
			d_tbl->ref_cnt = d_tbl->ref_cnt + 1U;
			entry_exist = true;
			entry_added = ZBC_ENTRY_UPDATED;
			break;
		}
	}
	/* add new table */
	if (!entry_exist &&
		(zbc->max_used_depth_index < zbc->max_depth_index)) {

		d_tbl = &zbc->zbc_dep_tbl[zbc->max_used_depth_index + 1U];
		if (d_tbl->ref_cnt != 0U) {
			return entry_added;
		}

		/* Increment used index and add new entry at that index */
		zbc->max_used_depth_index =
			zbc->max_used_depth_index + 1U;

		/* update sw copy */
		d_tbl->depth = depth_val->depth;
		d_tbl->format = depth_val->format;
		d_tbl->ref_cnt = 1U;

		nvgpu_gr_zbc_update_depth_reg(g, depth_val,
			zbc->max_used_depth_index);

		entry_added = ZBC_ENTRY_ADDED;
	}

	return entry_added;
}
#endif

static void nvgpu_gr_zbc_update_color_reg(struct gk20a *g,
			struct nvgpu_gr_zbc_entry *color_val, u32 index)
{
	/* update l2 table */
	g->ops.ltc.set_zbc_color_entry(g, color_val->color_l2, index);

	/* update zbc registers */
	g->ops.gr.zbc.add_color(g, color_val, index);
}

#ifdef CONFIG_NVGPU_ZBC_USER
static int nvgpu_gr_zbc_add_color(struct gk20a *g, struct nvgpu_gr_zbc *zbc,
			struct nvgpu_gr_zbc_entry *color_val)
{
	struct zbc_color_table *c_tbl;
	u32 i;
	int entry_added = -ENOSPC;
	bool entry_exist = false;

	/* search existing table */
	for (i = zbc->min_color_index; i <= zbc->max_used_color_index; i++) {

		c_tbl = &zbc->zbc_col_tbl[i];

		if ((c_tbl->ref_cnt != 0U) &&
			(nvgpu_memcmp((u8 *)c_tbl->color_l2,
				(u8 *)color_val->color_l2,
				sizeof(color_val->color_l2)) == 0)) {

			if (c_tbl->ref_cnt == U32_MAX) {
				return entry_added;
			}
			c_tbl->ref_cnt = c_tbl->ref_cnt + 1U;
			entry_exist = true;
			entry_added = ZBC_ENTRY_UPDATED;
			break;
		}
	}

	/* add new entry */
	if (!entry_exist &&
		(zbc->max_used_color_index < zbc->max_color_index)) {

		c_tbl = &zbc->zbc_col_tbl[zbc->max_used_color_index + 1U];
		if (c_tbl->ref_cnt != 0U) {
			return entry_added;
		}

		/* Increment used index and add new entry at that index */
		zbc->max_used_color_index =
			zbc->max_used_color_index + 1U;

		/* update local copy */
		for (i = 0; i < NVGPU_GR_ZBC_COLOR_VALUE_SIZE; i++) {
			c_tbl->color_l2[i] = color_val->color_l2[i];
			c_tbl->color_ds[i] = color_val->color_ds[i];
		}
		c_tbl->format = color_val->format;
		c_tbl->ref_cnt = 1U;

		nvgpu_gr_zbc_update_color_reg(g, color_val,
			zbc->max_used_color_index);

		entry_added = ZBC_ENTRY_ADDED;
	}

	return entry_added;
}

static int nvgpu_gr_zbc_add(struct gk20a *g, struct nvgpu_gr_zbc *zbc,
			    struct nvgpu_gr_zbc_entry *zbc_val)
{
	int added = 0;
	u32 cur_instance_id = 0U;
#if defined(CONFIG_NVGPU_LS_PMU) && defined(CONFIG_NVGPU_POWER_PG)
	u32 entries;
#endif

	cur_instance_id = nvgpu_gr_get_cur_instance_id(g);
	/* no endian swap ? */
	nvgpu_mutex_acquire(&zbc->zbc_lock);
	nvgpu_speculation_barrier();
	switch (zbc_val->type) {
	case NVGPU_GR_ZBC_TYPE_COLOR:
		added = nvgpu_gr_zbc_add_color(g, zbc, zbc_val);
		break;
	case NVGPU_GR_ZBC_TYPE_DEPTH:
		added = nvgpu_gr_zbc_add_depth(g, zbc, zbc_val);
		break;
	case NVGPU_GR_ZBC_TYPE_STENCIL:
		if (nvgpu_is_gr_class_enabled(g,
				NVGPU_SUPPORT_ZBC_STENCIL, cur_instance_id)) {
			added =  nvgpu_gr_zbc_add_stencil(g, zbc, zbc_val);
		} else {
			nvgpu_err(g,
			"invalid zbc table type %d", zbc_val->type);
			added = -EINVAL;
		}
		break;
	default:
		nvgpu_err(g,
			"invalid zbc table type %d", zbc_val->type);
		added = -EINVAL;
		break;
	}

#if defined(CONFIG_NVGPU_LS_PMU) && defined(CONFIG_NVGPU_POWER_PG)
	if (added == ZBC_ENTRY_ADDED) {
		/* update zbc for elpg only when new entry is added */
		entries = max(
			nvgpu_safe_sub_u32(zbc->max_used_color_index,
				zbc->min_color_index),
			nvgpu_safe_sub_u32(zbc->max_used_depth_index,
				zbc->min_depth_index));
		if (g->elpg_enabled) {
			nvgpu_pmu_save_zbc(g, entries);
		}
	}
#endif

	nvgpu_mutex_release(&zbc->zbc_lock);
	if (added < 0) {
		return added;
	}
	return 0;
}

int nvgpu_gr_zbc_set_table(struct gk20a *g, struct nvgpu_gr_zbc *zbc,
			   struct nvgpu_gr_zbc_entry *zbc_val)
{
	nvgpu_log(g, gpu_dbg_zbc, " zbc_val->type %u", zbc_val->type);

	return nvgpu_pg_elpg_protected_call(g,
		nvgpu_gr_zbc_add(g, zbc, zbc_val));
}

/* get a zbc table entry specified by index
 * return table size when type is invalid */
int nvgpu_gr_zbc_query_table(struct gk20a *g, struct nvgpu_gr_zbc *zbc,
			struct nvgpu_gr_zbc_query_params *query_params)
{
	u32 index = query_params->index_size;
	u32 i;
	u32 cur_instance_id = 0U;
	int error = 0;

	cur_instance_id = nvgpu_gr_get_cur_instance_id(g);
	nvgpu_speculation_barrier();
	switch (query_params->type) {
	case NVGPU_GR_ZBC_TYPE_INVALID:
		nvgpu_log(g, gpu_dbg_zbc, "Query zbc size");
		if ((zbc->max_color_index == U32_MAX) || (zbc->max_color_index < zbc->min_color_index)) {
			nvgpu_err(g, "zbc color max/min index wrong max=%u min=%u",
				zbc->max_color_index, zbc->min_color_index);
			error = -EINVAL;
			break;
		}
		query_params->index_size = zbc->max_color_index - zbc->min_color_index + 1U;
		break;
	case NVGPU_GR_ZBC_TYPE_COLOR:
		if ((index < zbc->min_color_index) ||
				(index > zbc->max_color_index)) {
			nvgpu_err(g, "invalid zbc color table index %u", index);
			error = -EINVAL;
			break;
		}
		nvgpu_log(g, gpu_dbg_zbc, "Query zbc color at index %u", index);

		nvgpu_speculation_barrier();
		for (i = 0; i < NVGPU_GR_ZBC_COLOR_VALUE_SIZE; i++) {
			query_params->color_l2[i] =
				zbc->zbc_col_tbl[index].color_l2[i];
			query_params->color_ds[i] =
				zbc->zbc_col_tbl[index].color_ds[i];
		}
		query_params->format = zbc->zbc_col_tbl[index].format;
		query_params->ref_cnt = zbc->zbc_col_tbl[index].ref_cnt;

		break;
	case NVGPU_GR_ZBC_TYPE_DEPTH:
		if ((index < zbc->min_depth_index) ||
				(index > zbc->max_depth_index)) {
			nvgpu_err(g, "invalid zbc depth table index %u", index);
			error = -EINVAL;
			break;
		}
		nvgpu_log(g, gpu_dbg_zbc, "Query zbc depth at index %u", index);

		nvgpu_speculation_barrier();
		query_params->depth = zbc->zbc_dep_tbl[index].depth;
		query_params->format = zbc->zbc_dep_tbl[index].format;
		query_params->ref_cnt = zbc->zbc_dep_tbl[index].ref_cnt;
		break;
	case NVGPU_GR_ZBC_TYPE_STENCIL:
		if (nvgpu_is_gr_class_enabled(g,
					NVGPU_SUPPORT_ZBC_STENCIL, cur_instance_id)) {
			if ((index < zbc->min_stencil_index) ||
					(index > zbc->max_stencil_index)) {
				nvgpu_err(g,
					"invalid zbc stencil table index %u",
					index);
				error = -EINVAL;
				break;
			}
			nvgpu_log(g, gpu_dbg_zbc,
				"Query zbc stencil at index %u", index);

			nvgpu_speculation_barrier();
			query_params->stencil = zbc->zbc_s_tbl[index].stencil;
			query_params->format = zbc->zbc_s_tbl[index].format;
			query_params->ref_cnt = zbc->zbc_s_tbl[index].ref_cnt;
		} else {
			nvgpu_err(g, "invalid zbc table type");
			error = -EINVAL;
		}
		break;
	default:
		nvgpu_err(g, "invalid zbc table type");
		error = -EINVAL;
		break;
	}

	return error;
}
#endif

/*
 * Update zbc table registers as per sw copy of zbc tables
 */
void nvgpu_gr_zbc_load_table(struct gk20a *g, struct nvgpu_gr_zbc *zbc)
{
	unsigned int i;
	u32 cur_instance_id = 0U;

	cur_instance_id = nvgpu_gr_get_cur_instance_id(g);
	for (i = zbc->min_color_index; i <= zbc->max_used_color_index; i++) {
		struct zbc_color_table *c_tbl = &zbc->zbc_col_tbl[i];
		struct nvgpu_gr_zbc_entry zbc_val;

		zbc_val.type = NVGPU_GR_ZBC_TYPE_COLOR;
		nvgpu_memcpy((u8 *)zbc_val.color_ds,
			(u8 *)c_tbl->color_ds, sizeof(zbc_val.color_ds));
		nvgpu_memcpy((u8 *)zbc_val.color_l2,
			(u8 *)c_tbl->color_l2, sizeof(zbc_val.color_l2));
		zbc_val.format = c_tbl->format;

		nvgpu_gr_zbc_update_color_reg(g, &zbc_val, i);
	}

	for (i = zbc->min_depth_index; i <= zbc->max_used_depth_index; i++) {
		struct zbc_depth_table *d_tbl = &zbc->zbc_dep_tbl[i];
		struct nvgpu_gr_zbc_entry zbc_val;

		zbc_val.type = NVGPU_GR_ZBC_TYPE_DEPTH;
		zbc_val.depth = d_tbl->depth;
		zbc_val.format = d_tbl->format;

		nvgpu_gr_zbc_update_depth_reg(g, &zbc_val, i);
	}

	if (nvgpu_is_gr_class_enabled(g,
				NVGPU_SUPPORT_ZBC_STENCIL, cur_instance_id)) {
		for (i = zbc->min_stencil_index;
			i <= zbc->max_used_stencil_index; i++) {
			struct zbc_stencil_table *s_tbl = &zbc->zbc_s_tbl[i];
			struct nvgpu_gr_zbc_entry zbc_val;

			zbc_val.type = NVGPU_GR_ZBC_TYPE_STENCIL;
			zbc_val.stencil = s_tbl->stencil;
			zbc_val.format = s_tbl->format;

			nvgpu_gr_zbc_update_stencil_reg(g, &zbc_val, i);
		}
	}
}

static void nvgpu_gr_zbc_init_indices(struct gk20a *g, struct nvgpu_gr_zbc *zbc)
{
	struct nvgpu_gr_zbc_table_indices zbc_indices;

	g->ops.gr.zbc.init_table_indices(g, &zbc_indices);

	zbc->min_color_index = zbc_indices.min_color_index;
	zbc->max_color_index = zbc_indices.max_color_index;
	zbc->min_depth_index = zbc_indices.min_depth_index;
	zbc->max_depth_index = zbc_indices.max_depth_index;
	zbc->min_stencil_index = zbc_indices.min_stencil_index;
	zbc->max_stencil_index = zbc_indices.max_stencil_index;

	nvgpu_log(g, gpu_dbg_zbc, "zbc->min_color_index %u",
		zbc->min_color_index);
	nvgpu_log(g, gpu_dbg_zbc, "zbc->max_color_index %u",
		zbc->max_color_index);
	nvgpu_log(g, gpu_dbg_zbc, "zbc->min_depth_index %u",
		zbc->min_depth_index);
	nvgpu_log(g, gpu_dbg_zbc, "zbc->max_depth_index %u",
		zbc->max_depth_index);
	nvgpu_log(g, gpu_dbg_zbc, "zbc->min_stencil_index %u",
		zbc->min_stencil_index);
	nvgpu_log(g, gpu_dbg_zbc, "zbc->max_stencil_index %u",
		zbc->max_stencil_index);
}

static void nvgpu_gr_zbc_load_default_sw_table(struct gk20a *g,
					struct nvgpu_gr_zbc *zbc)
{
	nvgpu_mutex_init(&zbc->zbc_lock);

	if (g->ops.gr.zbc.load_default_sw_table != NULL) {
		g->ops.gr.zbc.load_default_sw_table(g, zbc);
	}

}

static int gr_zbc_allocate_local_tbls(struct gk20a *g, struct nvgpu_gr_zbc *zbc)
{
	u32 zbc_col_size = nvgpu_safe_add_u32(zbc->max_color_index,
				zbc->min_color_index);
	u32 zbc_dep_size = nvgpu_safe_add_u32(zbc->max_depth_index,
				zbc->min_depth_index);
	u32 zbc_s_size = nvgpu_safe_add_u32(zbc->max_stencil_index,
				zbc->min_stencil_index);

	zbc->zbc_col_tbl = nvgpu_kzalloc(g,
			sizeof(struct zbc_color_table) * zbc_col_size);
	if (zbc->zbc_col_tbl == NULL) {
		goto alloc_col_tbl_err;
	}

	zbc->zbc_dep_tbl = nvgpu_kzalloc(g,
			sizeof(struct zbc_depth_table) * zbc_dep_size);

	if (zbc->zbc_dep_tbl == NULL) {
		goto alloc_dep_tbl_err;
	}

	zbc->zbc_s_tbl = nvgpu_kzalloc(g,
			sizeof(struct zbc_stencil_table) * zbc_s_size);
	if (zbc->zbc_s_tbl == NULL) {
		goto alloc_s_tbl_err;
	}

	return 0;

alloc_s_tbl_err:
	nvgpu_kfree(g, zbc->zbc_dep_tbl);
alloc_dep_tbl_err:
	nvgpu_kfree(g, zbc->zbc_col_tbl);
alloc_col_tbl_err:
	return -ENOMEM;
}

/* allocate the struct and load the table */
int nvgpu_gr_zbc_init(struct gk20a *g, struct nvgpu_gr_zbc **zbc)
{
	int ret = -ENOMEM;
	struct nvgpu_gr_zbc *gr_zbc = NULL;

	*zbc = NULL;

	gr_zbc = nvgpu_kzalloc(g, sizeof(*gr_zbc));
	if (gr_zbc == NULL) {
		return ret;
	}

	nvgpu_gr_zbc_init_indices(g, gr_zbc);

	ret = gr_zbc_allocate_local_tbls(g, gr_zbc);
	if (ret != 0) {
		goto alloc_err;
	}

	nvgpu_gr_zbc_load_default_sw_table(g, gr_zbc);

	*zbc = gr_zbc;
	return ret;

alloc_err:
	nvgpu_kfree(g, gr_zbc);
	return ret;
}

/* deallocate the memory for the struct */
void nvgpu_gr_zbc_deinit(struct gk20a *g, struct nvgpu_gr_zbc *zbc)
{
	if (zbc == NULL) {
		return;
	}

	nvgpu_kfree(g, zbc->zbc_col_tbl);
	nvgpu_kfree(g, zbc->zbc_dep_tbl);
	nvgpu_kfree(g, zbc->zbc_s_tbl);
	nvgpu_kfree(g, zbc);
}

#ifdef CONFIG_NVGPU_ZBC_USER
struct nvgpu_gr_zbc_entry *nvgpu_gr_zbc_entry_alloc(struct gk20a *g)
{
	return nvgpu_kzalloc(g, sizeof(struct nvgpu_gr_zbc_entry));
}
void nvgpu_gr_zbc_entry_free(struct gk20a *g, struct nvgpu_gr_zbc_entry *entry)
{
	nvgpu_kfree(g, entry);
}

void nvgpu_gr_zbc_set_entry_color_ds(struct nvgpu_gr_zbc_entry *entry,
		u32 idx, u32 ds)
{
	entry->color_ds[idx] = ds;
}

void nvgpu_gr_zbc_set_entry_color_l2(struct nvgpu_gr_zbc_entry *entry,
		u32 idx, u32 l2)
{
	entry->color_l2[idx] = l2;
}

void nvgpu_gr_zbc_set_entry_depth(struct nvgpu_gr_zbc_entry *entry,
		u32 depth)
{
	entry->depth = depth;
}

void nvgpu_gr_zbc_set_entry_stencil(struct nvgpu_gr_zbc_entry *entry,
		u32 stencil)
{
	entry->stencil = stencil;
}

u32 nvgpu_gr_zbc_get_entry_type(struct nvgpu_gr_zbc_entry *entry)
{
	return entry->type;
}

void nvgpu_gr_zbc_set_entry_type(struct nvgpu_gr_zbc_entry *entry,
		u32 type)
{
	entry->type = type;
}

void nvgpu_gr_zbc_set_entry_format(struct nvgpu_gr_zbc_entry *entry,
		u32 format)
{
	entry->format = format;
}

u32 nvgpu_gr_zbc_get_entry_color_ds(struct nvgpu_gr_zbc_entry *entry,
		u32 idx)
{
	return entry->color_ds[idx];
}
#endif

u32 nvgpu_gr_zbc_get_entry_stencil(struct nvgpu_gr_zbc_entry *entry)
{
	return entry->stencil;
}

u32 nvgpu_gr_zbc_get_entry_depth(struct nvgpu_gr_zbc_entry *entry)
{
	return entry->depth;
}

u32 nvgpu_gr_zbc_get_entry_color_l2(struct nvgpu_gr_zbc_entry *entry,
		u32 idx)
{
	return entry->color_l2[idx];
}

u32 nvgpu_gr_zbc_get_entry_format(struct nvgpu_gr_zbc_entry *entry)
{
	return entry->format;
}
