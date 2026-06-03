/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved. */

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/regops_allowlist.h>
#include <nvgpu/profiler.h>
#include <nvgpu/regops.h>

#include "allowlist_gb10b.h"
#include "allowlist_gb20c.h"

int gb20c_init_reg_op_type_per_register_type(struct gk20a *g, struct nvgpu_profiler_object *prof,
						enum nvgpu_profiler_pm_resource_type pm_resource)
{
	bool is_gr_ctx;

	(void)g;

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC) {
		if (prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_SMPC] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_CAU] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
		} else {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_SMPC] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_CAU] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
		}
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY) {
		if (prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMON] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_MM] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
		} else {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMON] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_MM] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
		}
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM) {
		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY] &&
		   prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
			is_gr_ctx = true;
		} else if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC] &&
		   prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
			is_gr_ctx = true;
		} else if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES] &&
		   prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES]) {
			is_gr_ctx = true;
		} else if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF] &&
		   prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF]) {
			is_gr_ctx = true;
		} else {
			is_gr_ctx = false;
		}

		if (is_gr_ctx) {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_CHANNEL] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
		} else {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_CHANNEL] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
		}
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PC_SAMPLER) {
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_PC_SAMPLER] =
			NVGPU_DBG_REG_OP_TYPE_GR_CTX;
	}

	/* ctxsw support does not exist for hes right now */
	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES) {
		if (prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES]) {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HES] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
		} else {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HES] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
		}
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF) {
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_CCU_PROF] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
	}

	return 0;
}

#define NV_PERF_CCUPROF_GPCS 0x000f6000
#define NV_PERF_CCUPROF_FBPS 0x000f5000

const struct nvgpu_pm_resource_register_range
*gb20c_get_hwpm_ccuprof_bitvector_register_ranges(struct gk20a *g,
						 struct nvgpu_profiler_object *prof, u32 *count)
{
	u32 gr_instance_id = nvgpu_grmgr_get_gr_instance_id(g, prof->gpu_instance_id);
	u32 num_gpc = nvgpu_grmgr_get_gr_num_gpcs(g, gr_instance_id);
	u32 num_fbp = nvgpu_grmgr_get_gr_num_fbps(g, gr_instance_id);
	struct nvgpu_pm_resource_register_range *ranges;
	const u32 *ccuprof_bitvector = NULL;
	u32 bitvector_count = 0U;
	u32 bitvector_range_count = 0U;
	u32 range_count = 0U;
	u32 offset = 0U;
	u32 ccuprof_count = 0U;
	u32 gpc_stride = 0U;
	u32 fbp_stride = 0U;
	u32 ccuprof = 0U;
	u32 gpc = 0U;
	u32 fbp = 0U;

	ccuprof_count =
		nvgpu_safe_mult_u32(g->ops.regops.get_hwpm_perf_ccuprof_units_per_gpc(),
				   nvgpu_safe_add_u32(num_gpc, 1U));
	ccuprof_count =
		nvgpu_safe_add_u32(ccuprof_count,
			nvgpu_safe_mult_u32(g->ops.regops.get_hwpm_perf_ccuprof_units_per_fbp(),
			nvgpu_safe_add_u32(num_fbp, 1U)));

	ccuprof_bitvector =
		g->ops.regops.get_ccuprof_register_offset_allowlist(&bitvector_count);
	bitvector_range_count =
		gb10b_add_bitvector_to_reg_ranges(g, ccuprof_bitvector, bitvector_count,
						 0U, NULL);
	range_count = nvgpu_safe_mult_u32(bitvector_range_count, ccuprof_count);
	nvgpu_log(g, gpu_dbg_gpu_dbg, "Allocating %u ranges", range_count);

	ranges = nvgpu_kzalloc(g, sizeof(struct nvgpu_pm_resource_register_range) * range_count);
	if (ranges == NULL) {
		nvgpu_err(g, "Failed to allocate ranges");
		return NULL;
	}

	*count = 0U;
	gpc_stride = g->ops.regops.get_hwpm_perf_ccuprof_gpc_reg_stride();
	ccuprof_count = g->ops.regops.get_hwpm_perf_ccuprof_units_per_gpc();
	for (gpc = 0; gpc < nvgpu_safe_add_u32(num_gpc, 1U); gpc++) {
		if (gpc == num_gpc) {
			offset = g->ops.regops.get_hwpm_perf_ccuprof_gpcs_reg_base();
		} else {
			offset = g->ops.regops.get_hwpm_perf_ccuprof_gpc0_reg_base();
			offset = nvgpu_safe_add_u32(offset, nvgpu_safe_mult_u32(gpc, gpc_stride));
		}
		for (ccuprof = 0; ccuprof < ccuprof_count; ccuprof++) {
			range_count =
				gb10b_add_bitvector_to_reg_ranges(g, ccuprof_bitvector,
								 bitvector_count,
								 offset, ranges + *count);
			nvgpu_log(g, gpu_dbg_gpu_dbg,
				 "gpc: %u, ccuprof: %u, offset: 0x%x, range_count: %u",
				 gpc, ccuprof, offset, range_count);
			*count = nvgpu_safe_add_u32(*count, range_count);
			offset = nvgpu_safe_add_u32(offset,
				g->ops.regops.get_hwpm_perf_ccuprof_unit_reg_stride());
		}
	}

	ccuprof_count = g->ops.regops.get_hwpm_perf_ccuprof_units_per_fbp();
	fbp_stride = g->ops.regops.get_hwpm_perf_ccuprof_fbp_reg_stride();
	for (fbp = 0U; fbp < nvgpu_safe_add_u32(num_fbp, 1U); fbp++) {
		if (fbp == num_fbp) {
			offset = g->ops.regops.get_hwpm_perf_ccuprof_fbps_reg_base();
		} else {
			offset = g->ops.regops.get_hwpm_perf_ccuprof_fbp0_reg_base();
			offset = nvgpu_safe_add_u32(offset, nvgpu_safe_mult_u32(fbp,
					fbp_stride));
		}

		for (ccuprof = 0; ccuprof < ccuprof_count; ccuprof++) {
			range_count = gb10b_add_bitvector_to_reg_ranges(g, ccuprof_bitvector,
								       bitvector_count, offset,
								       ranges + *count);
			nvgpu_log(g, gpu_dbg_gpu_dbg,
				 "fbp: %u, ccuprof: %u, offset: 0x%x, range_count: %u",
				 fbp, ccuprof, offset, range_count);
			*count = nvgpu_safe_add_u32(*count, range_count);
			offset = nvgpu_safe_add_u32(offset,
				g->ops.regops.get_hwpm_perf_ccuprof_unit_reg_stride());
		}
	}

	return ranges;
}

static void
gb20c_allowlist_misc_register_ranges_static(struct nvgpu_pm_resource_register_range_map *map,
					   u32 *map_index)
{
	/* TODO: See if following registers can be provided from bitvector. */
	map[*map_index].start = 0x0000cc98U;
	map[*map_index].end = 0x0000cc98U;
	map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX;
	(*map_index)++;

	map[*map_index].start = 0x00d00194U;
	map[*map_index].end = 0x00d00194U;
	map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX;
	(*map_index)++;

	map[*map_index].start = 0x00d00198U;
	map[*map_index].end = 0x00d00198U;
	map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX;
	(*map_index)++;

	map[*map_index].start = 0x00d01194U;
	map[*map_index].end = 0x00d01194U;
	map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX;
	(*map_index)++;

	map[*map_index].start = 0x00d01594U;
	map[*map_index].end = 0x00d01594U;
	map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX;
	(*map_index)++;

	map[*map_index].start = 0x00d02594U;
	map[*map_index].end = 0x00d02594U;
	map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX;
	(*map_index)++;

	map[*map_index].start = 0x00d03594U;
	map[*map_index].end = 0x00d03594U;
	map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX;
	(*map_index)++;

	map[*map_index].start = 0x00d03598U;
	map[*map_index].end = 0x00d03598U;
	map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX;
	(*map_index)++;

	map[*map_index].start = 0x00d05594U;
	map[*map_index].end = 0x00d05594U;
	map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX;
	(*map_index)++;
}

void
gb20c_allowlist_misc_register_ranges(struct gk20a *g, struct nvgpu_profiler_object *prof,
				    struct nvgpu_pm_resource_register_range_map *map,
				    u32 *map_index, u32 *count)
{
	u32 gr_instance_id = nvgpu_grmgr_get_gr_instance_id(g, prof->gpu_instance_id);
	u32 num_gpc = nvgpu_grmgr_get_gr_num_gpcs(g, gr_instance_id);
	u32 num_fbp = nvgpu_grmgr_get_gr_num_fbps(g, gr_instance_id);
	u32 num_ltc_slices = g->ops.top.get_max_lts_per_ltc(g);
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);
	struct nvgpu_gr_config *config = nvgpu_gr_get_config_ptr(g);
	u32 ccuprof_reg_offset = 0U;
	u32 gpc_offset = 0U;
	u32 fbp_offset = 0U;
	u32 gpc_stride = 0U;
	u32 tpc_stride = 0U;
	u32 offset = 0U;
	u32 num_tpc = 0U;
	u32 gpc = 0U;
	u32 tpc = 0U;
	u32 fbp = 0U;
	u32 lts = 0U;
	u32 range_count = 9U;

	num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, 0U);
	range_count = nvgpu_safe_add_u32(range_count,
					nvgpu_safe_mult_u32(nvgpu_safe_add_u32(num_gpc, 1U),
							   nvgpu_safe_add_u32(num_tpc, 1U)));

	range_count = nvgpu_safe_add_u32(range_count,
					nvgpu_safe_mult_u32(nvgpu_safe_add_u32(num_fbp, 1U),
							   nvgpu_safe_add_u32(num_ltc_slices, 1U)));

	*count = range_count;
	if (map == NULL) {
		return;
	}

	nvgpu_log(g, gpu_dbg_gpu_dbg, "Adding %u misc register ranges", range_count);
	gb20c_allowlist_misc_register_ranges_static(map, map_index);

	gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	tpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);

	ccuprof_reg_offset =
		g->ops.regops.get_hwpm_gpc0_tpc0_mpc_ccuprof_ulcmux_cntl0();
	nvgpu_log(g, gpu_dbg_gpu_dbg, "gpc ccuprof register offset: 0x%x", ccuprof_reg_offset);
	for (gpc = 0; gpc < nvgpu_safe_add_u32(num_gpc, 1U); gpc++) {
		if (gpc == num_gpc) {
			gpc_offset = nvgpu_safe_sub_u32(ccuprof_reg_offset,
							gpc_stride);
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, 0U);
		} else {
			gpc_offset = nvgpu_safe_add_u32(ccuprof_reg_offset,
						   nvgpu_safe_mult_u32(gpc, gpc_stride));
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, gpc);
		}
		for (tpc = 0; tpc < nvgpu_safe_add_u32(num_tpc, 1U); tpc++) {
			if (tpc == num_tpc) {
				offset = nvgpu_safe_sub_u32(gpc_offset, tpc_stride);
			} else {
				offset = nvgpu_safe_add_u32(gpc_offset,
							       nvgpu_safe_mult_u32(tpc,
										  tpc_stride));
			}
			nvgpu_log(g, gpu_dbg_gpu_dbg,
				 "gpc: %u, tpc: %u, offset: 0x%x", gpc, tpc, offset);
			map[*map_index].start = offset;
			map[*map_index].end = offset;
			map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_CCU_PROF;
			(*map_index)++;
		}
	}

	ccuprof_reg_offset =
		g->ops.regops.get_hwpm_ltc0_lts0_ccuprof_ulcmux_cntl0();
	nvgpu_log(g, gpu_dbg_gpu_dbg, "fbp ccuprof register offset: 0x%x", ccuprof_reg_offset);
	for (fbp = 0; fbp < nvgpu_safe_add_u32(num_fbp, 1U); fbp++) {
		if (fbp == num_fbp) {
			fbp_offset = nvgpu_safe_sub_u32(ccuprof_reg_offset,
							ltc_stride);
		} else {
			fbp_offset = nvgpu_safe_add_u32(ccuprof_reg_offset,
						   nvgpu_safe_mult_u32(fbp, ltc_stride));
		}
		for (lts = 0; lts < nvgpu_safe_add_u32(num_ltc_slices, 1U); lts++) {
			if (lts == num_ltc_slices) {
				offset = nvgpu_safe_sub_u32(fbp_offset, lts_stride);
			} else {
				offset = nvgpu_safe_add_u32(fbp_offset,
							nvgpu_safe_mult_u32(lts, lts_stride));
			}
			nvgpu_log(g, gpu_dbg_gpu_dbg,
				 "fbp: %u, lts: %u, offset: 0x%x", fbp, lts, offset);
			map[*map_index].start = offset;
			map[*map_index].end = offset;
			map[*map_index].type = NVGPU_HWPM_REGISTER_TYPE_CCU_PROF;
			(*map_index)++;
		}
	}

	return;
}
