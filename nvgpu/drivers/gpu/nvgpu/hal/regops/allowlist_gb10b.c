// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/regops_allowlist.h>
#include <nvgpu/profiler.h>
#include <nvgpu/regops.h>

#include <nvgpu/hw/gb10b/hw_perf_gb10b.h>

#include "allowlist_gb10b.h"

bool gb10b_is_hwpm_pma_reg_context_switched(void)
{
	return false;
}

int gb10b_init_reg_op_type_per_register_type(struct gk20a *g, struct nvgpu_profiler_object *prof,
					    enum nvgpu_profiler_pm_resource_type pm_resource)
{
	bool is_ctx_switched = false;

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC) {
		if (prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_SMPC] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
		} else {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_SMPC] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
		}
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY) {
		if (prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMON] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_CAU] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;

			/*
			 * Refer Bug 4719304
			 * PMA registers are not part of context buffer offset
			 * map yet. They are to be executed as device level
			 * regops till then irrespective of context switch mode.
			 */
			if (g->ops.regops.is_hwpm_pma_reg_context_switched != NULL) {
				is_ctx_switched =
					g->ops.regops.is_hwpm_pma_reg_context_switched();
				if (!is_ctx_switched) {
					prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
						NVGPU_DBG_REG_OP_TYPE_GLOBAL;
					prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER] =
						NVGPU_DBG_REG_OP_TYPE_GLOBAL;
				}
			}
		} else {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMON] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_CAU] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
		}
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_MM] =
			NVGPU_DBG_REG_OP_TYPE_GLOBAL;
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM) {
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_CHANNEL] =
			NVGPU_DBG_REG_OP_TYPE_GLOBAL;
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PC_SAMPLER) {
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_PC_SAMPLER] =
			NVGPU_DBG_REG_OP_TYPE_GR_CTX;
	}

	/* ctxsw support does not exist for hes right now */
	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES) {
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HES] =
			NVGPU_DBG_REG_OP_TYPE_GLOBAL;
	}

	return 0;
}

u32
gb10b_add_bitvector_to_reg_ranges(struct gk20a *g, const u32 *bitvector, u32 count,
				 u32 offset, struct nvgpu_pm_resource_register_range *ranges)
{
	u32 index = 0U;
	u32 i = 0U;
	u32 start = 0U;
	u32 end = 0U;
	u32 range_count = 0U;

	while (i < count) {
		start = i;
		end = i;

		/* Find the end of the current contiguous range */
		while ((end + 1 < count) && (bitvector[end + 1] == bitvector[end] + 4U)) {
			end++;
		}
		if (ranges != NULL) {
			/* Add this range to the map */
			ranges[index].start = nvgpu_safe_add_u32(bitvector[start], offset);
			ranges[index].end = nvgpu_safe_add_u32(bitvector[end], offset);
			nvgpu_log(g, gpu_dbg_prof, "%u: 0x%x-0x%x", index,
				 ranges[index].start, ranges[index].end);
		}
		index++;
		range_count++;
		/* Move to the next possible range */
		i = end + 1;
	}

	return range_count;
}

const struct nvgpu_pm_resource_register_range *
gb10b_get_hwpm_perfmon_bitvector_register_ranges(struct gk20a *g,
					struct nvgpu_profiler_object *prof, u32 *count)
{
	struct nvgpu_pm_resource_register_range *ranges;
	u32 perfmon_stride = perf_pmmsys_perdomain_offset_v();
	const u32 *perfmon_bitvector = NULL;
	u32 bitvector_count = 0U;
	u32 bitvector_range_count = 0U;
	u32 range_count = 0U;
	u32 gr_instance_id = nvgpu_grmgr_get_gr_instance_id(g, prof->gpu_instance_id);
	u32 num_gpc = nvgpu_grmgr_get_gr_num_gpcs(g, gr_instance_id);
	u32 num_fbp = nvgpu_grmgr_get_gr_num_fbps(g, gr_instance_id);
	u32 num_sys = 1U;
	/* To account GPCS and FBPS space registers, 1U is added to chiplet count */
	u32 gpc_perfmon_count = nvgpu_safe_mult_u32(nvgpu_safe_add_u32(num_gpc, 1U),
						   g->num_gpc_perfmon);
	u32 fbp_perfmon_count = nvgpu_safe_mult_u32(nvgpu_safe_add_u32(num_fbp, 1U),
						   g->num_fbp_perfmon);
	u32 chiplet_reg_base = 0U;
	u32 chiplet_stride = 0U;
	u32 chiplet_offset = 0U;
	u32 perfmon_offset = 0U;
	u32 perfmon = 0U;
	u32 offset = 0U;
	u32 chiplet = 0U;

	perfmon_bitvector =
		g->ops.regops.get_hwpm_perfmon_register_offset_allowlist(&bitvector_count);
	bitvector_range_count = gb10b_add_bitvector_to_reg_ranges(g, perfmon_bitvector,
							bitvector_count,
							0U, NULL);


	range_count = nvgpu_safe_add_u32(gpc_perfmon_count, fbp_perfmon_count);
	range_count = nvgpu_safe_add_u32(range_count,
					nvgpu_safe_mult_u32(num_sys, g->num_sys_perfmon));
	range_count = nvgpu_safe_mult_u32(bitvector_range_count, range_count);
	ranges = nvgpu_kzalloc(g, sizeof(struct nvgpu_pm_resource_register_range) * range_count);
	if (ranges == NULL) {
		nvgpu_err(g, "Failed to allocate ranges");
		return NULL;
	}

	nvgpu_log(g, gpu_dbg_prof,
		"num_gpc_perfmon: %u, num_fbp_perfmon: %u, num_sys_perfmon: %u",
		g->num_gpc_perfmon, g->num_fbp_perfmon, g->num_sys_perfmon);
	*count = 0U;
	chiplet_reg_base = g->ops.regops.get_hwpm_perfmon_gpc_reg_base();
	chiplet_stride = g->ops.perf.get_pmmgpc_per_chiplet_offset();
	for (chiplet = 0U; chiplet < nvgpu_safe_add_u32(num_gpc, 1U); chiplet++) {
		if (chiplet == num_gpc) {
			chiplet_offset = g->ops.perf.get_hwpm_gpcs_base_addr();
		} else {
			chiplet_offset = nvgpu_safe_add_u32(chiplet_reg_base,
							   nvgpu_safe_mult_u32(chiplet,
									      chiplet_stride));
		}
		for (perfmon = 0U; perfmon < g->num_gpc_perfmon; perfmon++) {
			perfmon_offset = nvgpu_safe_mult_u32(perfmon, perfmon_stride);
			offset = nvgpu_safe_add_u32(chiplet_offset, perfmon_offset);
			nvgpu_log(g, gpu_dbg_prof,
				"%u:%u, gpc_offset: 0x%x, perfmon_offset: 0x%x, offset: 0x%x",
				chiplet, perfmon, chiplet_offset, perfmon_offset, offset);
			range_count = gb10b_add_bitvector_to_reg_ranges(g, perfmon_bitvector,
								bitvector_count, offset,
								ranges + *count);
			*count += range_count;
		}
	}

	chiplet_reg_base = g->ops.regops.get_hwpm_perfmon_fbp_reg_base();
	chiplet_stride = g->ops.perf.get_pmmfbp_per_chiplet_offset();
	for (chiplet = 0U; chiplet < nvgpu_safe_add_u32(num_fbp, 1U); chiplet++) {
		if (chiplet == num_fbp) {
			chiplet_offset = g->ops.perf.get_hwpm_fbps_base_addr();
		} else {
			chiplet_offset = nvgpu_safe_add_u32(chiplet_reg_base,
							   nvgpu_safe_mult_u32(chiplet,
									      chiplet_stride));
		}

		for (perfmon = 0U; perfmon < g->num_fbp_perfmon; perfmon++) {
			perfmon_offset = nvgpu_safe_mult_u32(perfmon, perfmon_stride);
			offset = nvgpu_safe_add_u32(chiplet_offset, perfmon_offset);
			nvgpu_log(g, gpu_dbg_prof,
				"%u:%u, fbp_offset: 0x%x, perfmon_offset: 0x%x, offset: 0x%x",
				chiplet, perfmon, chiplet_offset, perfmon_offset, offset);
			range_count = gb10b_add_bitvector_to_reg_ranges(g, perfmon_bitvector,
								bitvector_count, offset,
								ranges + *count);
			*count += range_count;
		}
	}

	chiplet_reg_base = g->ops.regops.get_hwpm_perfmon_sys_reg_base();
	chiplet_stride = g->ops.perf.get_pmmsys_per_chiplet_offset();
	for (chiplet = 0U; chiplet < num_sys; chiplet++) {
		chiplet_offset = nvgpu_safe_mult_u32(chiplet, chiplet_stride);
		chiplet_offset = nvgpu_safe_add_u32(chiplet_offset, chiplet_reg_base);
		for (perfmon = 0U; perfmon < g->num_sys_perfmon; perfmon++) {
			perfmon_offset = nvgpu_safe_mult_u32(perfmon, perfmon_stride);
			offset = nvgpu_safe_add_u32(chiplet_offset, perfmon_offset);
			nvgpu_log(g, gpu_dbg_prof,
				"%u:%u, sys_offset: 0x%x, perfmon_offset: 0x%x, offset: 0x%x",
				chiplet, perfmon, chiplet_offset, perfmon_offset, offset);
			range_count = gb10b_add_bitvector_to_reg_ranges(g, perfmon_bitvector,
								bitvector_count, offset,
								ranges + *count);
			*count += range_count;
		}
	}

	return ranges;
}

const struct nvgpu_pm_resource_register_range *
gb10b_get_hwpm_router_bitvector_register_ranges(struct gk20a *g,
					       struct nvgpu_profiler_object *prof, u32 *count)
{
	struct nvgpu_pm_resource_register_range *ranges;
	const u32 *router_bitvector = NULL;
	u32 bitvector_count = 0U;
	u32 bitvector_range_count = 0U;
	u32 range_count = 0U;
	u32 chiplet = 0U;
	u32 gr_instance_id = nvgpu_grmgr_get_gr_instance_id(g, prof->gpu_instance_id);
	u32 num_gpc = nvgpu_grmgr_get_gr_num_gpcs(g, gr_instance_id);
	u32 num_fbp = nvgpu_grmgr_get_gr_num_fbps(g, gr_instance_id);
	u32 num_sys = 1U;
	u32 stride = g->ops.regops.get_hwpm_router_gpc_reg_stride();
	u32 chiplet_router_reg_base = 0U;
	u32 chiplet_count;
	u32 offset = 0U;

	router_bitvector =
		g->ops.regops.get_hwpm_router_register_offset_allowlist(&bitvector_count);
	bitvector_range_count =
		gb10b_add_bitvector_to_reg_ranges(g, router_bitvector, bitvector_count,
						 0U, NULL);

	/* To account GPCSROUTER and FBPSROUTER, 1U is added to chiplet count */
	chiplet_count = nvgpu_safe_add_u32(nvgpu_safe_add_u32(num_gpc, 1U),
					  nvgpu_safe_add_u32(num_fbp, 1U));
	chiplet_count = nvgpu_safe_add_u32(chiplet_count, num_sys);
	range_count = nvgpu_safe_mult_u32(bitvector_range_count, chiplet_count);
	ranges = nvgpu_kzalloc(g, sizeof(struct nvgpu_pm_resource_register_range) * range_count);
	if (ranges == NULL) {
		nvgpu_err(g, "Failed to allocate ranges");
		return NULL;
	}

	*count = 0U;
	chiplet_router_reg_base = g->ops.regops.get_hwpm_router_gpc_reg_base();
	for (chiplet = 0U; chiplet < nvgpu_safe_add_u32(num_gpc, 1U); chiplet++) {
		if (chiplet == num_gpc) {
			offset = g->ops.perf.get_hwpm_gpcsrouter_base_addr();
		} else {
			offset = nvgpu_safe_add_u32(chiplet_router_reg_base,
						   nvgpu_safe_mult_u32(chiplet, stride));
		}
		nvgpu_log(g, gpu_dbg_prof, "GPC:%u, offset: 0x%x", chiplet, offset);
		range_count =
			gb10b_add_bitvector_to_reg_ranges(g, router_bitvector, bitvector_count,
							 offset, ranges + *count);
		*count += range_count;
	}

	chiplet_router_reg_base = g->ops.regops.get_hwpm_router_fbp_reg_base();
	for (chiplet = 0U; chiplet < nvgpu_safe_add_u32(num_fbp, 1U); chiplet++) {
		if (chiplet == num_fbp) {
			offset = g->ops.perf.get_hwpm_fbpsrouter_base_addr();
		} else {
			offset = nvgpu_safe_add_u32(chiplet_router_reg_base,
						   nvgpu_safe_mult_u32(chiplet, stride));
		}
		nvgpu_log(g, gpu_dbg_prof, "FBP:%u, offset: 0x%x", chiplet, offset);
		range_count =
			gb10b_add_bitvector_to_reg_ranges(g, router_bitvector, bitvector_count,
							 offset, ranges + *count);
		*count += range_count;
	}

	chiplet_router_reg_base = g->ops.regops.get_hwpm_router_sys_reg_base();
	for (chiplet = 0U; chiplet < num_sys; chiplet++) {
		offset = nvgpu_safe_add_u32(chiplet_router_reg_base,
					   nvgpu_safe_mult_u32(chiplet, stride));
		nvgpu_log(g, gpu_dbg_prof, "Sys:%u, offset: 0x%x", chiplet, offset);
		range_count =
			gb10b_add_bitvector_to_reg_ranges(g, router_bitvector, bitvector_count,
							 offset, ranges + *count);
		*count += range_count;
	}

	return ranges;
}

const struct nvgpu_pm_resource_register_range *
gb10b_get_hwpm_pma_trigger_bitvector_register_ranges(struct gk20a *g, u32 *count)
{
	struct nvgpu_pm_resource_register_range *ranges;
	const u32 *trigger_bitvector = NULL;
	u32 bitvector_count = 0U;
	u32 bitvector_range_count = 0U;
	u32 range_count = 0U;
	u32 offset = 0U;

	trigger_bitvector =
		g->ops.regops.get_hwpm_pma_trigger_register_offset_allowlist(&bitvector_count);
	bitvector_range_count = gb10b_add_bitvector_to_reg_ranges(g, trigger_bitvector,
							bitvector_count,
							0U, NULL);

	ranges = nvgpu_kzalloc(g, sizeof(struct nvgpu_pm_resource_register_range) *
				bitvector_range_count);
	if (ranges == NULL) {
		nvgpu_err(g, "Failed to allocate ranges");
		return NULL;
	}

	offset = g->ops.regops.get_hwpm_perf_pmasys_reg_base();
	nvgpu_log(g, gpu_dbg_prof, "offset: 0x%x", offset);
	range_count = gb10b_add_bitvector_to_reg_ranges(g, trigger_bitvector, bitvector_count,
		offset, ranges);

	*count = range_count;
	return ranges;
}

const struct nvgpu_pm_resource_register_range *
gb10b_get_hwpm_pma_channel_bitvector_register_ranges(struct gk20a *g, u32 *count)
{
	struct nvgpu_pm_resource_register_range *ranges;
	const u32 *pma_channel_bitvector = NULL;
	u32 bitvector_count = 0U;
	u32 bitvector_range_count = 0U;
	u32 range_count = 0U;
	u32 offset = 0U;

	pma_channel_bitvector =
		g->ops.regops.get_hwpm_pma_channel_register_offset_allowlist(&bitvector_count);
	bitvector_range_count = gb10b_add_bitvector_to_reg_ranges(g, pma_channel_bitvector,
							bitvector_count,
							0U, NULL);

	ranges = nvgpu_kzalloc(g, sizeof(struct nvgpu_pm_resource_register_range) *
	bitvector_range_count);
	if (ranges == NULL) {
		nvgpu_err(g, "Failed to allocate ranges");
		return NULL;
	}

	offset = g->ops.regops.get_hwpm_perf_pmasys_reg_base();
	nvgpu_log(g, gpu_dbg_prof, "offset: 0x%x", offset);
	range_count = gb10b_add_bitvector_to_reg_ranges(g, pma_channel_bitvector, bitvector_count,
		offset, ranges);

	*count = range_count;
	return ranges;
}

const struct nvgpu_pm_resource_register_range *
gb10b_get_hwpm_smpc_bitvector_register_ranges(struct gk20a *g,
					     struct nvgpu_profiler_object *prof, u32 *count)
{
	u32 gr_instance_id = nvgpu_grmgr_get_gr_instance_id(g, prof->gpu_instance_id);
	struct nvgpu_gr_config *config = nvgpu_gr_get_config_ptr(g);
	u32 num_gpc = nvgpu_grmgr_get_gr_num_gpcs(g, gr_instance_id);
	struct nvgpu_pm_resource_register_range *ranges;
	const u32 *smpc_bitvector = NULL;
	u32 bitvector_count = 0U;
	u32 bitvector_range_count = 0U;
	u32 range_count = 0U;
	u32 offset = 0U;
	u32 num_tpc = 0U;
	u32 tpc_count = 0U;
	u32 gpc = 0U;
	u32 tpc = 0U;
	u32 tpc_offset = 0U;
	u32 gpc_pri_base = 0U;
	u32 tpc_in_gpc_base = 0U;
	u32 tpc_stride = 0U;
	u32 gpc_stride = 0U;
	u32 tpc_shared_base_in_gpc = 0U;

	for (gpc = 0U; gpc < nvgpu_safe_add_u32(num_gpc, 1U); gpc++) {
		if (gpc == num_gpc) {
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, 0U);
		} else {
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, gpc);
		}
		tpc_count += nvgpu_safe_add_u32(num_tpc, 1U);
	}

	smpc_bitvector =
		g->ops.regops.get_smpc_register_offset_allowlist(&bitvector_count);
	bitvector_range_count =
		gb10b_add_bitvector_to_reg_ranges(g, smpc_bitvector, bitvector_count,
						 0U, NULL);
	range_count = nvgpu_safe_mult_u32(bitvector_range_count, tpc_count);

	ranges = nvgpu_kzalloc(g, sizeof(struct nvgpu_pm_resource_register_range) * range_count);
	if (ranges == NULL) {
		nvgpu_err(g, "Failed to allocate ranges");
		return NULL;
	}

	gpc_pri_base = nvgpu_get_litter_value(g, GPU_LIT_GPC_BASE);
	gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	tpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	tpc_in_gpc_base = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_BASE);
	tpc_shared_base_in_gpc = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_SHARED_BASE);

	*count = 0U;
	for (gpc = 0; gpc < nvgpu_safe_add_u32(num_gpc, 1U); gpc++) {
		if (gpc == num_gpc) {
			offset = nvgpu_get_litter_value(g, GPU_LIT_GPC_SHARED_BASE);
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, 0U);
		} else {
			offset = nvgpu_safe_add_u32(gpc_pri_base,
						   nvgpu_safe_mult_u32(gpc, gpc_stride));
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, gpc);
		}

		for (tpc = 0; tpc < nvgpu_safe_add_u32(num_tpc, 1U); tpc++) {
			if (tpc == num_tpc) {
				tpc_offset = nvgpu_safe_add_u32(offset, tpc_shared_base_in_gpc);
			} else {
				tpc_offset = nvgpu_safe_add_u32(tpc_in_gpc_base,
							       nvgpu_safe_mult_u32(tpc,
										  tpc_stride));
				tpc_offset = nvgpu_safe_add_u32(offset, tpc_offset);
			}
			nvgpu_log(g, gpu_dbg_prof,
				"%u:%u, tpc_offset: 0x%x", gpc, tpc, tpc_offset);
			range_count =
				gb10b_add_bitvector_to_reg_ranges(g, smpc_bitvector,
								 bitvector_count,
								 tpc_offset, ranges + *count);
			*count += range_count;
		}
	}

	return ranges;
}

const struct nvgpu_pm_resource_register_range *
gb10b_get_hwpm_cau_bitvector_register_ranges(struct gk20a *g,
				struct nvgpu_profiler_object *prof, u32 *count)
{
	u32 gr_instance_id = nvgpu_grmgr_get_gr_instance_id(g, prof->gpu_instance_id);
	struct nvgpu_gr_config *config = nvgpu_gr_get_config_ptr(g);
	u32 num_gpc = nvgpu_grmgr_get_gr_num_gpcs(g, gr_instance_id);
	struct nvgpu_pm_resource_register_range *ranges;
	const u32 *cau_bitvector = NULL;
	u32 bitvector_count = 0U;
	u32 bitvector_range_count = 0U;
	u32 range_count = 0U;
	u32 num_tpc = 0U;
	u32 tpc_count = 0U;
	u32 gpc = 0U;
	u32 tpc = 0U;
	u32 cau_idx = 0U;
	u32 gpc_offset = 0U;
	u32 tpc_offset = 0U;
	u32 gpc_pri_base = 0U;
	u32 tpc_in_gpc_base = 0U;
	u32 tpc_stride = 0U;
	u32 gpc_stride = 0U;
	u32 cau_base = 0U;
	u32 tpc_shared_base_in_gpc = 0U;

	/* 1U is added to num_gpc to account for shared GPC */
	for (gpc = 0U; gpc < nvgpu_safe_add_u32(num_gpc, 1U); gpc++) {
		if (gpc == num_gpc) {
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, 0U);
		} else {
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, gpc);
		}
		tpc_count += nvgpu_safe_add_u32(num_tpc, 1U);
	}

	cau_bitvector =
		g->ops.regops.get_cau_register_offset_allowlist(&bitvector_count);
	bitvector_range_count =
		gb10b_add_bitvector_to_reg_ranges(g, cau_bitvector, bitvector_count,
						 0U, NULL);
	range_count = nvgpu_safe_mult_u32(bitvector_range_count, tpc_count);
	range_count = nvgpu_safe_mult_u32(range_count, 2U);
	ranges = nvgpu_kzalloc(g, sizeof(struct nvgpu_pm_resource_register_range) * range_count);
	if (ranges == NULL) {
		nvgpu_err(g, "Failed to allocate ranges");
		return NULL;
	}

	gpc_pri_base = nvgpu_get_litter_value(g, GPU_LIT_GPC_BASE);
	gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	tpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	tpc_in_gpc_base = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_BASE);
	tpc_shared_base_in_gpc = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_SHARED_BASE);

	cau_base = g->ops.regops.get_hwpm_gpc0_tpc0_cau_base();
	cau_base = nvgpu_safe_sub_u32(cau_base,
				nvgpu_safe_add_u32(gpc_pri_base, tpc_in_gpc_base));

	*count = 0U;
	for (gpc = 0; gpc < nvgpu_safe_add_u32(num_gpc, 1U); gpc++) {
		if (gpc == num_gpc) {
			gpc_offset = nvgpu_get_litter_value(g, GPU_LIT_GPC_SHARED_BASE);
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, 0U);
		} else {
			gpc_offset = nvgpu_safe_add_u32(gpc_pri_base,
						   nvgpu_safe_mult_u32(gpc, gpc_stride));
			num_tpc = nvgpu_gr_config_get_gpc_tpc_count(config, gpc);
		}
		gpc_offset = nvgpu_safe_add_u32(gpc_offset, cau_base);

		for (tpc = 0; tpc < nvgpu_safe_add_u32(num_tpc, 1U); tpc++) {
			/* since num of cau per tpc is 1, we can directly add tpc_in_gpc_base */
			if (tpc == num_tpc) {
				tpc_offset = nvgpu_safe_add_u32(gpc_offset, tpc_shared_base_in_gpc);
			} else {
				tpc_offset = nvgpu_safe_add_u32(tpc_in_gpc_base,
							       nvgpu_safe_mult_u32(tpc,
										  tpc_stride));
				tpc_offset = nvgpu_safe_add_u32(gpc_offset, tpc_offset);
			}
			/* TODO: replace 2U with NV_PGRAPH_PRI_GPCS_TPC0_CAU_CONTROL__SIZE_1 */
			for (cau_idx = 0; cau_idx < 2U; cau_idx++) {
				tpc_offset = nvgpu_safe_add_u32(tpc_offset, nvgpu_safe_mult_u32(cau_idx, 64U));
				nvgpu_log(g, gpu_dbg_prof,
					"%u:%u:%u, gpc_offset: 0x%x, tpc_offset: 0x%x",
					gpc, tpc, cau_idx, gpc_offset, tpc_offset);
				range_count = gb10b_add_bitvector_to_reg_ranges(g, cau_bitvector,
					bitvector_count,
					tpc_offset, ranges + *count);
				*count += range_count;
			}
		}
	}

	return ranges;
}

const struct nvgpu_pm_resource_register_range *
gb10b_get_hwpm_hes_bitvector_register_ranges(struct gk20a *g, u32 *count)
{
	struct nvgpu_pm_resource_register_range *ranges;
	const u32 *hes_bitvector = NULL;
	u32 bitvector_count = 0U;
	u32 bitvector_range_count = 0U;
	u32 range_count = 0U;
	u32 offset = 0U;

	hes_bitvector =
		g->ops.regops.get_hes_register_offset_allowlist(&bitvector_count);
	bitvector_range_count = gb10b_add_bitvector_to_reg_ranges(g, hes_bitvector, bitvector_count,
							0U, NULL);

	/* bitvector covers whole of NV_PERF_HEM_SYS0 */
	ranges = nvgpu_kzalloc(g, sizeof(struct nvgpu_pm_resource_register_range) *
			      bitvector_range_count);
	if (ranges == NULL) {
		nvgpu_err(g, "Failed to allocate ranges");
		return NULL;
	}

	offset = g->ops.regops.get_hwpm_hem_sys0_base();
	nvgpu_log(g, gpu_dbg_prof, "offset: 0x%x", offset);
	range_count = gb10b_add_bitvector_to_reg_ranges(g, hes_bitvector, bitvector_count,
		offset, ranges);

	*count = range_count;
	return ranges;
}
