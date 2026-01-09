// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/grmgr.h>
#include <hal/fifo/fifo_utils_ga10b.h>
#include <hal/fb/fb_gb10b.h>
#include <nvgpu/hw/gb10b/hw_smcarb_gb10b.h>
#include <nvgpu/hw/gb10b/hw_runlist_gb10b.h>
#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>
#include <nvgpu/fbp.h>

#include "grmgr_gb10b.h"

struct gk20a;
#define NVGPU_VEID_SHIFT  0x3

#ifdef CONFIG_NVGPU_MIG

static const struct nvgpu_mig_gpu_instance_config gb10b_gpu_instance_config_2_syspipes_2_gpcgrp = {
	.usable_gr_syspipe_count	= 2U,
	.usable_gr_syspipe_mask	 	= 0x3U,
	.num_config_supported	   	= 1U,
	.gpcgrp_gpc_count	       	= {2U, 0U},
	.gpc_count		      	= 2U,
	.gpu_instance_config = {
		{.config_name = "2 GPU instances each with 1 GPC  THOR U",
		 .num_gpu_instances = 2U,
		 .gpu_instance_static_config = {
			{.gpu_instance_id	= 1U,
			 .gr_syspipe_id		= 0U,
			 .num_gpc		= 1U,
			 .is_gfx_capable	= true,
			 .swizzle_id		= 2,
			 .lts_mask		= 0xf000,
			 .veid_mask		= 0xf,
			 .num_fbps		= 0x1,
			 .fbp_en_mask           = 0x8,
			 .is_memory_partition_supported = true,
			 .num_ofa		= 0,
			 .num_nvenc		= 1,
			 .num_nvdec		= 0,
			 .num_nvjpg		= 1},
			{.gpu_instance_id	= 2U,
			 .gr_syspipe_id		= 1U,
			 .num_gpc		= 1U,
			 .is_gfx_capable	= false,
			 .swizzle_id		= 1,
			 .lts_mask		= 0xff,
			 .veid_mask		= 0xf0,
			 .num_fbps		= 0x2,
			 .fbp_en_mask		= 0x3,
			 .is_memory_partition_supported = true,
			 .num_ofa		= 1,
			 .num_nvenc		= 1,
			 .num_nvdec		= 1,
			 .num_nvjpg		= 1}}}}
};

/* Static mig config list for 2 syspipes(0x3U) + 3 GPCs + 3:0 gpc group config */
static const struct nvgpu_mig_gpu_instance_config gb10b_gpu_instance_config_3_syspipes_3_gpcgrp = {
	.usable_gr_syspipe_count	= 2U,
	.usable_gr_syspipe_mask	 	= 0x3U,
	.num_config_supported	   	= 1U,
	.gpcgrp_gpc_count	       	= {3U, 0U},
	.gpc_count		      	= 3U,
	.gpu_instance_config = {
		{.config_name = "2 GPU instances one with 1 GPC and the other with 2 GPCs",
		 .num_gpu_instances = 2U,
		 .gpu_instance_static_config = {
			{.gpu_instance_id	= 1U,
			 .gr_syspipe_id		= 0U,
			 .num_gpc		= 1U,
			 .is_gfx_capable	= true,
			 .swizzle_id		= 2,
			 .lts_mask		= 0xf000,
			 .veid_mask 		= 0x3,
			 .num_fbps		= 0x1,
			 .fbp_en_mask           = 0x8,
			 .is_memory_partition_supported = true,
			 .num_ofa		= 0,
			 .num_nvenc		= 1,
			 .num_nvdec		= 1,
			 .num_nvjpg		= 1},
			{.gpu_instance_id	= 2U,
			 .gr_syspipe_id		= 1U,
			 .num_gpc		= 2U,
			 .is_gfx_capable	= false,
			 .lts_mask 		= 0xfff,
			 .veid_mask 		= 0xfc,
			 .num_fbps 		= 0x3,
			 .fbp_en_mask           = 0x7,
			 .is_memory_partition_supported = true,
			 .swizzle_id		= 1,
			 .num_ofa		= 1,
			 .num_nvenc		= 1,
			 .num_nvdec		= 1,
			 .num_nvjpg		= 1}}}}
};

const struct nvgpu_mig_gpu_instance_config *gb10b_grmgr_get_mig_config_ptr(
		struct gk20a *g) {

	if ((g->mig.usable_gr_syspipe_count ==
				gb10b_gpu_instance_config_3_syspipes_3_gpcgrp.usable_gr_syspipe_count) &&
			(g->mig.usable_gr_syspipe_mask ==
				gb10b_gpu_instance_config_3_syspipes_3_gpcgrp.usable_gr_syspipe_mask) &&
			(g->mig.gpc_count ==
				gb10b_gpu_instance_config_3_syspipes_3_gpcgrp.gpc_count) &&
			(g->mig.gpcgrp_gpc_count[0] ==
				gb10b_gpu_instance_config_3_syspipes_3_gpcgrp.gpcgrp_gpc_count[0]) &&
			(g->mig.gpcgrp_gpc_count[1] ==
				gb10b_gpu_instance_config_3_syspipes_3_gpcgrp.gpcgrp_gpc_count[1])) {
		nvgpu_log(g, gpu_dbg_mig,
			"Static mig config list for 2 syspipes + 3 GPCs"
				"+ 3:0 gpc group config ");
		return &gb10b_gpu_instance_config_3_syspipes_3_gpcgrp;
	} else if ((g->mig.usable_gr_syspipe_count ==
				gb10b_gpu_instance_config_2_syspipes_2_gpcgrp.usable_gr_syspipe_count) &&
			(g->mig.usable_gr_syspipe_mask ==
				gb10b_gpu_instance_config_2_syspipes_2_gpcgrp.usable_gr_syspipe_mask) &&
			(g->mig.gpc_count ==
				gb10b_gpu_instance_config_2_syspipes_2_gpcgrp.gpc_count) &&
			(g->mig.gpcgrp_gpc_count[0] ==
				gb10b_gpu_instance_config_2_syspipes_2_gpcgrp.gpcgrp_gpc_count[0]) &&
			(g->mig.gpcgrp_gpc_count[1] ==
				gb10b_gpu_instance_config_2_syspipes_2_gpcgrp.gpcgrp_gpc_count[1])) {
		nvgpu_log(g, gpu_dbg_mig,
			"Static mig config list for 2 syspipes + 2 GPCs"
				"+ 2:0 gpc group config ");
		return &gb10b_gpu_instance_config_2_syspipes_2_gpcgrp;
	}
	return NULL;
}

u32 gb10b_grmgr_get_allowed_swizzid_size(struct gk20a *g)
{
	(void)g;
	return smcarb_allowed_swizzid__size1_v();
}

bool gb10b_grmgr_is_gpc_gfx_capable(struct gk20a *g, u32 physical_gpc_id)
{
	u32 reg_val = 0U;
	u32 gpc_physical_offset;

	gpc_physical_offset = nvgpu_gr_gpc_offset(g, physical_gpc_id);
	reg_val = nvgpu_readl(g, nvgpu_safe_add_u32(gr_gpc0_fs_gpc_r(),
				gpc_physical_offset));
	return (reg_val & gr_gpc0_fs_gpc_gpc0_fs_gpc_graphics_capable_yes_f());
}

void gb10b_grmgr_set_syspipe_gfx_or_compute(struct gk20a *g, u32 syspipe_id,
		bool is_gfx_supported)
{
	u32 reg_val = 0U;

	(void)syspipe_id;

	reg_val = nvgpu_readl(g, smcarb_sys_pipe_eng_type_r());
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) {
		if (is_gfx_supported){
			/* Compute mode is set by default. So set gfx mode if needed. */
			reg_val = set_field(reg_val, smcarb_sys_pipe_eng_type_mode_m(),
					smcarb_sys_pipe_eng_type_mode_gfx_f());
			nvgpu_writel(g, smcarb_sys_pipe_eng_type_r(),
					reg_val);
		}
	}
}

u32 gb10b_runlist_config_reg_off(void)
{
	return runlist_config_r();
}

void gb10b_grmgr_add_grce_syspipe_gfx(struct gk20a *g,
		u32 syspipe_id,
		struct nvgpu_runlist *runlist,
		bool is_gfx_supported)
{
	u32 reg_off = 0U, reg_val = 0U;

	if (syspipe_id != 0) {
		return;
	}

	reg_off = g->ops.grmgr.get_runlist_config_reg_off();
	reg_val = nvgpu_runlist_readl(g, runlist, reg_off);

	if (is_gfx_supported) {
		reg_val = set_field(reg_val, runlist_config_subch4_m(),
			runlist_config_subch4_subch4_active_f());
	} else {
		reg_val = set_field(reg_val, runlist_config_subch4_m(),
				runlist_config_subch4_subch4_inactive_f());
	}

	nvgpu_runlist_writel(g, runlist, reg_off, reg_val);
}

void gb10b_grmgr_setup_veids_mask(struct gk20a *g, u32 index)
{
	u32 num_bits, num_shift, tmp_mask;
	struct nvgpu_gr_syspipe *gr_syspipe;
	/*
	 * every bit represents 8 veids.
	 * So for 64 veids mask will be 0xff
	 */
	gr_syspipe = &g->mig.gpu_instance[nvgpu_safe_add_u32(index, 1)].gr_syspipe;

	num_bits = gr_syspipe->max_veid_count_per_tsg >> NVGPU_VEID_SHIFT;
	num_shift = gr_syspipe->veid_start_offset >> NVGPU_VEID_SHIFT;
	tmp_mask = BIT64(num_bits);
	tmp_mask = nvgpu_safe_sub_u32(tmp_mask, 1);
	gr_syspipe->veid_mask = ((tmp_mask) << num_shift) & 0xff;
}

int gb10b_grmgr_choose_mig_vpr_setting(
	struct gk20a *g,
	bool *out_swizzid_checks,
	u32 *out_gfx_swizzid)
{
	bool tmp_swizzid_checks;
	u32 tmp_gfx_swizzid;

	if (nvgpu_grmgr_is_multi_gr_enabled(g))
	{
		u32 i;
		struct nvgpu_gpu_instance *partition;
		bool found_gfx_capable_partition = false;
		const u32 num_instances = g->mig.num_gpu_instances;
		/*
		 * MIG: enable MIG swizzid checks and restrict VPR to GFX-capable partition
		 */
		tmp_swizzid_checks = true;
		/*
		 * The start index is always index=1 because index=0 is the full config instance.
		 */
		for (i = 1U; i < num_instances; i++) {
			partition = &g->mig.gpu_instance[i];
			if (partition->is_gfx_capable) {
				/* Only one GFX-capable partition can exist */
				nvgpu_assert(found_gfx_capable_partition == false);
				tmp_gfx_swizzid = partition->swizzle_id;
				found_gfx_capable_partition = true;
			}
		}
		if (!found_gfx_capable_partition) {
			nvgpu_err(g, "Failed to find a GFX-capable MIG partition");
			return -EINVAL;
		}
	} else {
		/*
		 * No MIG: enable MIG swizzid checks but allow only swizid=0.
		 */
		tmp_swizzid_checks = true;
		tmp_gfx_swizzid = 0;
	}
	*out_swizzid_checks = tmp_swizzid_checks;
	*out_gfx_swizzid = tmp_gfx_swizzid;
	return 0;
}

int gb10b_grmgr_setup_vpr(struct gk20a *g)
{
	int err;
	bool swizzid_checks;
	u32 gfx_swizzid;

	if (nvgpu_platform_is_silicon(g)) {
		// Skip on silicon where secure boot is enabled, but keep for pre-si
		// environments where VPR programming may not be yet done.
		return 0;
	}

	err = gb10b_grmgr_choose_mig_vpr_setting(g, &swizzid_checks, &gfx_swizzid);
	if (err < 0) {
		nvgpu_err(g, "Failed to choose MIG VPR setting. Error %d", err);
		return err;
	}
	nvgpu_log(g, gpu_dbg_mig, "VPR swizzid checks: %u. VPR-capable swizzid: %u",
		(u32)swizzid_checks, gfx_swizzid);
	/*
	 * This is a hack until the design and implementation for dynamic MIG + VPR
	 * is properly done. This would be handled under https://nvbugs/4232026.
	 *
	 * Once PRI security is enabled, the CPU would not be able to program
	 * the MMU VPR settings.
	*/
	gb10b_fb_configure_vpr_mmu_mig(g, swizzid_checks, gfx_swizzid);
	return 0;
}
#endif

int gb10b_grmgr_setup_ltc_partition(struct gk20a *g, u32 syspipe_id)
{
	int ret = 0;
	u32 ret_val = 0;
	u32 gpu_instance_id = syspipe_id;
	u32 lts_mask, lts_index;
	struct nvgpu_gpu_instance *gpu_instance;

	gpu_instance = &g->mig.gpu_instance[0];
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) {
		if (nvgpu_safe_add_u32_return(syspipe_id, 1, &gpu_instance_id) == false) {
			nvgpu_err(g, "buffer overflow");
			return -EOVERFLOW;
		}
	}
	lts_mask = gpu_instance[gpu_instance_id].logical_lts_mask;
	lts_index = 0U;
	ret = g->ops.gr.falcon.ctrl_ctxsw(g,
			NVGPU_GR_FALCON_METHOD_SET_SMC_LTS_MASK_INDEX, lts_index, &ret_val);
	if (ret != 0U) {
		nvgpu_err(g, "Set lts mask index failed");
		return ret;
	}

	ret = g->ops.gr.falcon.ctrl_ctxsw(g,
			NVGPU_GR_FALCON_METHOD_ASSIGN_SMC_LTS_MASK, lts_mask, &ret_val);
	if (ret != 0U) {
		nvgpu_err(g, "Set lts mask method failed");
	}

	return ret;
}

void gb10b_grmgr_setup_fbp_id_mapping(struct gk20a *g, struct nvgpu_gpu_instance *gpu_instance)
{
	u32 fbp_en_mask = gpu_instance->fbp_en_mask;
	u32 i, j = 0;

	if (!nvgpu_grmgr_is_mig_type_gpu_instance(gpu_instance)) {
		for (i = 0; i < gpu_instance->num_fbp; i++) {
			gpu_instance->fbp_mappings[i] = i;
			nvgpu_log(g, gpu_dbg_info, "gpu_instance_id %u: fbp_mapping: "
					"local[%u] -> logical[%u]",
				gpu_instance->gpu_instance_id, i, gpu_instance->fbp_mappings[i]);
		}
		return;
	}

	/*
	 * Memory partition is supported on gb10b. As per memory partition across
	 * MIG, setup mapping of fbp logical id to local id.
	 */
	for_each_set_bit(i, (const unsigned long *)&fbp_en_mask, sizeof(fbp_en_mask) * 8) {
		gpu_instance->fbp_mappings[j] = nvgpu_fbp_get_fbp_logical_index(g->fbp, i);
		nvgpu_log(g, gpu_dbg_info, "gpu_instance_id %u: fbp_mapping: "
				"local[%u] -> logical[%u] -> physical[%u]",
			gpu_instance->gpu_instance_id, j, gpu_instance->fbp_mappings[j], i);
		j = nvgpu_safe_add_u32(j, 1);
	}
}
