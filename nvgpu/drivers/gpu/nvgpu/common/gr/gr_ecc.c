/*
 * Copyright (c) 2019-2023, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/gr/gr_ecc.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/string.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/kmem.h>
#include <nvgpu/ecc.h>
#include "common/gr/gr_config_priv.h"

int nvgpu_ecc_counter_init_per_gr(struct gk20a *g,
		struct nvgpu_ecc_stat **stat, const char *name)
{
	struct nvgpu_ecc_stat *stats;
	u32 i;
	char gr_str[10] = {0};

	stats = nvgpu_kzalloc(g, nvgpu_safe_mult_u64(sizeof(*stats),
			g->num_gr_instances));
	if (stats == NULL) {
		return -ENOMEM;
	}

	for (i = 0; i < g->num_gr_instances; i++) {
		/**
		 * Store stats name as below:
		 * gr<gr_index>_<name_string>
		 */
		(void)strcpy(stats[i].name, "gr");
		(void)nvgpu_strnadd_u32(gr_str, i, sizeof(gr_str), 10U);
		(void)strncat(stats[i].name, gr_str,
					NVGPU_ECC_STAT_NAME_MAX_SIZE -
					strlen(stats[i].name));
		(void)strncat(stats[i].name, "_",
					NVGPU_ECC_STAT_NAME_MAX_SIZE -
					strlen(stats[i].name));
		(void)strncat(stats[i].name, name,
					NVGPU_ECC_STAT_NAME_MAX_SIZE -
					strlen(stats[i].name));

		nvgpu_ecc_stat_add(g, &stats[i]);
	}

	*stat = stats;
	return 0;
}

int nvgpu_ecc_counter_init_per_tpc(struct gk20a *g,
		struct nvgpu_ecc_stat ***stat, const char *name)
{
	struct nvgpu_ecc_stat **stats;
	struct nvgpu_gr_config *gr_config = nvgpu_gr_get_config_ptr(g);
	u32 cur_gr_instance = nvgpu_gr_get_cur_instance_id(g);
	u32 gpc_count = nvgpu_gr_config_get_gpc_count(gr_config);
	u32 max_gpc_count = gr_config->max_gpc_count;
	u32 gpc, tpc, gpc_phys_id, tpc_phys_id;
	char gpc_str[10] = {0}, tpc_str[10] = {0};
	int err = 0;

	stats = nvgpu_kzalloc(g, nvgpu_safe_mult_u64(sizeof(*stats),
			      max_gpc_count));
	if (stats == NULL) {
		return -ENOMEM;
	}

	for (gpc = 0; gpc < gpc_count; gpc++) {
		gpc_phys_id = nvgpu_grmgr_get_gr_gpc_phys_id(g, cur_gr_instance, gpc);
		stats[gpc_phys_id] = nvgpu_kzalloc(g,
			nvgpu_safe_mult_u64(sizeof(*stats[gpc_phys_id]),
				nvgpu_gr_config_get_max_tpc_per_gpc_count(gr_config)));
		if (stats[gpc_phys_id] == NULL) {
			nvgpu_err(g, "Mem alloc failed for %s\n", name);
			err = -ENOMEM;
			goto fail;
		}
	}

	for (gpc = 0; gpc < gpc_count; gpc++) {
		gpc_phys_id = nvgpu_grmgr_get_gr_gpc_phys_id(g, cur_gr_instance, gpc);
		// For getting tpc count, gpc id is logical because we read it using gpc_stride.
		for (tpc = 0; tpc < nvgpu_gr_config_get_gpc_tpc_count(gr_config, gpc); tpc++) {
			tpc_phys_id = gr_config->gpc_tpc_physical_id_map[gpc_phys_id][tpc];
			/**
			 * Store stats name as below:
			 * gpc<gpc_value>_tpc<tpc_value>_<name_string>
			 */
			(void)strcpy(stats[gpc_phys_id][tpc_phys_id].name, "gpc");
			(void)nvgpu_strnadd_u32(gpc_str, gpc_phys_id,
							sizeof(gpc_str), 10U);
			(void)strncat(stats[gpc_phys_id][tpc_phys_id].name, gpc_str,
						NVGPU_ECC_STAT_NAME_MAX_SIZE -
						strlen(stats[gpc_phys_id][tpc_phys_id].name));
			(void)strncat(stats[gpc_phys_id][tpc_phys_id].name, "_tpc",
						NVGPU_ECC_STAT_NAME_MAX_SIZE -
						strlen(stats[gpc_phys_id][tpc_phys_id].name));
			(void)nvgpu_strnadd_u32(tpc_str, tpc_phys_id,
							sizeof(tpc_str), 10U);
			(void)strncat(stats[gpc_phys_id][tpc_phys_id].name, tpc_str,
						NVGPU_ECC_STAT_NAME_MAX_SIZE -
						strlen(stats[gpc_phys_id][tpc_phys_id].name));
			(void)strncat(stats[gpc_phys_id][tpc_phys_id].name, "_",
						NVGPU_ECC_STAT_NAME_MAX_SIZE -
						strlen(stats[gpc_phys_id][tpc_phys_id].name));
			(void)strncat(stats[gpc_phys_id][tpc_phys_id].name, name,
						NVGPU_ECC_STAT_NAME_MAX_SIZE -
						strlen(stats[gpc_phys_id][tpc_phys_id].name));

			nvgpu_ecc_stat_add(g, &stats[gpc_phys_id][tpc_phys_id]);
		}
	}

	*stat = stats;

fail:
	if (err != 0) {
		while (gpc-- != 0u) {
			nvgpu_kfree(g, stats[gpc]);
		}

		nvgpu_kfree(g, stats);
	}

	return err;
}

int nvgpu_ecc_counter_init_per_gpc(struct gk20a *g,
		struct nvgpu_ecc_stat **stat, const char *name)
{
	struct nvgpu_ecc_stat *stats;
	struct nvgpu_gr_config *gr_config = nvgpu_gr_get_config_ptr(g);
	u32 cur_gr_instance = nvgpu_gr_get_cur_instance_id(g);
	u32 gpc_count = nvgpu_gr_config_get_gpc_count(gr_config);
	u32 max_gpc_count = gr_config->max_gpc_count;
	u32 gpc, gpc_phys_id;
	char gpc_str[10] = {0};

	stats = nvgpu_kzalloc(g, nvgpu_safe_mult_u64(sizeof(*stats),
						     max_gpc_count));
	if (stats == NULL) {
		return -ENOMEM;
	}

	for (gpc = 0; gpc < gpc_count; gpc++) {
		gpc_phys_id = nvgpu_grmgr_get_gr_gpc_phys_id(g, cur_gr_instance, gpc);
		/**
		 * Store stats name as below:
		 * gpc<gpc_value>_<name_string>
		 */
		(void)strcpy(stats[gpc_phys_id].name, "gpc");
		(void)nvgpu_strnadd_u32(gpc_str, gpc_phys_id, sizeof(gpc_str), 10U);
		(void)strncat(stats[gpc_phys_id].name, gpc_str,
					NVGPU_ECC_STAT_NAME_MAX_SIZE -
					strlen(stats[gpc_phys_id].name));
		(void)strncat(stats[gpc_phys_id].name, "_",
					NVGPU_ECC_STAT_NAME_MAX_SIZE -
					strlen(stats[gpc_phys_id].name));
		(void)strncat(stats[gpc_phys_id].name, name,
					NVGPU_ECC_STAT_NAME_MAX_SIZE -
					strlen(stats[gpc_phys_id].name));

		nvgpu_ecc_stat_add(g, &stats[gpc_phys_id]);
	}

	*stat = stats;
	return 0;
}

void nvgpu_ecc_counter_deinit_per_gr(struct gk20a *g,
				     struct nvgpu_ecc_stat **stats_p)
{
	struct nvgpu_ecc_stat *stats = NULL;
	u32 i;

	if (*stats_p != NULL) {
		stats = *stats_p;

		for (i = 0; i < g->num_gr_instances; i++) {
			nvgpu_ecc_stat_del(g, &stats[i]);
		}

		nvgpu_kfree(g, stats);
		*stats_p = NULL;
	}
}

void nvgpu_ecc_counter_deinit_per_tpc(struct gk20a *g,
				      struct nvgpu_ecc_stat ***stats_p)
{
	struct nvgpu_gr_config *gr_config = nvgpu_gr_get_config_ptr(g);
	struct nvgpu_ecc_stat **stats = NULL;
	u32 gpc_count;
	u32 gpc, tpc;
	u32 gpc_phys_id, tpc_phys_id;
	u32 cur_gr_instance = nvgpu_gr_get_cur_instance_id(g);

	if (*stats_p != NULL) {
		gpc_count = nvgpu_gr_config_get_gpc_count(gr_config);
		stats = *stats_p;

		for (gpc = 0; gpc < gpc_count; gpc++) {
			gpc_phys_id = nvgpu_grmgr_get_gr_gpc_phys_id(g, cur_gr_instance, gpc);
			if (stats[gpc_phys_id] == NULL) {
				continue;
			}

			for (tpc = 0;
			     tpc < nvgpu_gr_config_get_gpc_tpc_count(gr_config, gpc);
			     tpc++) {
				tpc_phys_id = gr_config->gpc_tpc_physical_id_map[gpc_phys_id][tpc];
				nvgpu_ecc_stat_del(g, &stats[gpc_phys_id][tpc_phys_id]);
			}

			nvgpu_kfree(g, stats[gpc_phys_id]);
			stats[gpc_phys_id] = NULL;
		}

		nvgpu_kfree(g, stats);
		*stats_p = NULL;
	}
}

void nvgpu_ecc_counter_deinit_per_gpc(struct gk20a *g,
				      struct nvgpu_ecc_stat **stats_p)
{
	struct nvgpu_gr_config *gr_config = nvgpu_gr_get_config_ptr(g);
	struct nvgpu_ecc_stat *stats = NULL;
	u32 cur_gr_instance = nvgpu_gr_get_cur_instance_id(g);
	u32 gpc_count;
	u32 gpc, gpc_phys_id;

	if (*stats_p != NULL) {
		gpc_count = nvgpu_gr_config_get_gpc_count(gr_config);
		stats = *stats_p;

		for (gpc = 0; gpc < gpc_count; gpc++) {
			gpc_phys_id = nvgpu_grmgr_get_gr_gpc_phys_id(g, cur_gr_instance, gpc);
			nvgpu_ecc_stat_del(g, &stats[gpc_phys_id]);
		}

		nvgpu_kfree(g, stats);
		*stats_p = NULL;
	}
}

void nvgpu_gr_ecc_free(struct gk20a *g)
{
	struct nvgpu_gr_config *gr_config = nvgpu_gr_get_config_ptr(g);

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (gr_config == NULL) {
		return;
	}

	if (g->ops.gr.ecc.fecs_ecc_deinit != NULL) {
		g->ops.gr.ecc.fecs_ecc_deinit(g);
	}

	if (g->ops.gr.ecc.gpc_tpc_ecc_deinit != NULL) {
		g->ops.gr.ecc.gpc_tpc_ecc_deinit(g);
	}
}
