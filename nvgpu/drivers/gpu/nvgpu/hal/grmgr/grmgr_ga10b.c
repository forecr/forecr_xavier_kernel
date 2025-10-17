// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/gr/gr_instances.h>

#include "grmgr_ga10b.h"
#include <nvgpu/hw/ga10b/hw_smcarb_ga10b.h>

#ifdef CONFIG_NVGPU_MIG
#include <nvgpu/enabled.h>
#include <nvgpu/string.h>
#include <nvgpu/engines.h>
#include <nvgpu/device.h>
#include <nvgpu/utils.h>
#include <nvgpu/fbp.h>

#define NVGPU_MIG_GSP_FAULT_ID		2U
#define NVGPU_MIG_GSPLITE_FAULT_ID	20U

/**
 * TODO: Needs to be fixed in the follow up
 * by reading proper hw register.
 * http://nvbugs/4127277
 */
#define NVGPU_VEID_ROUND_DOWN_VALUE	16

#define GA10B_GRMGR_PSMCARB_ALLOWED_UGPU(gpu_instance_id, gpcgrp_id) \
	(((gpu_instance_id) == 0U))

/* Static mig config list for 2 syspipes(0x3U) + 2 GPCs + 2 Aysnc LCEs + 2:0 gpc group config */
static const struct nvgpu_mig_gpu_instance_config ga10b_gpu_instance_config = {
	.usable_gr_syspipe_count	= 2U,
	.usable_gr_syspipe_mask		= 0x3U,
	.num_config_supported		= 2U,
	.gpcgrp_gpc_count		= { 2U, 0U },
	.gpc_count			= 2U,
	.gpu_instance_config = {
		{.config_name = "2 GPU instances each with 1 GPC",
		 .num_gpu_instances = 2U,
		 .gpu_instance_static_config = {
			{.gpu_instance_id 		= 0U,
			 .gr_syspipe_id			= 0U,
			 .num_gpc			= 1U},
			{.gpu_instance_id 		= 0U,
			 .gr_syspipe_id			= 1U,
			 .num_gpc			= 1U}}},
		{.config_name = "1 GPU instance with 2 GPCs",
		 .num_gpu_instances = 1U,
		 .gpu_instance_static_config = {
			{.gpu_instance_id 		= 0U,
			 .gr_syspipe_id			= 0U,
			 .num_gpc			= 2U}}}}
};

static int nvgpu_grmgr_instance_assign_ce(struct gk20a *g)
{
	const struct nvgpu_device *grce[NVGPU_MIG_MAX_ENGINES] = { };
	const struct nvgpu_device *async_ces[NVGPU_MIG_MAX_ENGINES] = { };
	const struct nvgpu_device *lces[NVGPU_MIG_MAX_ENGINES] = { };
	const struct nvgpu_device *remain_async_ce[NVGPU_MIG_MAX_ENGINES] = { };
	const struct nvgpu_device *temp_dev = NULL;
	struct nvgpu_gpu_instance *gpu_instance = &g->mig.gpu_instance[0];
	u32 num_lce = 0U, num_async = 0U, grce_index = 0U;
	u32 lce_index = 0U, i, j, remain_async_ces = 0U;
	u32 num_instances = 0U, temp_pce_bitmask = 0U, async_ce_per_instance = 0U;

	num_lce = nvgpu_device_get_copies(g, lces, NVGPU_MIG_MAX_ENGINES);
	num_async = nvgpu_device_get_async_copies(g, async_ces, NVGPU_MIG_MAX_ENGINES);
	j = 0;
	num_instances = g->mig.num_gpu_instances;
	/**
	 * Compute the GRCE engines and populate the grce list.
	 */
	for (i = 0; i < num_lce; i++) {
		temp_dev = (struct nvgpu_device *)lces[i];
		if ((temp_dev != NULL) && (temp_dev->is_grce)) {
			grce[j] = temp_dev;
			if (nvgpu_safe_add_u32_return(j, 1, &j) == false) {
				nvgpu_err(g, "buffer overflow");
				return -EOVERFLOW;
			}
		}
	}

	/**
	 * Assign grces to gfx capable instance and corresponding
	 * async ces to the same instance.
	 */

	grce_index = 0;
	for (i = 1; i < num_instances ; i++) {
		lce_index = 0;
		if (gpu_instance[i].is_gfx_capable) {
			gpu_instance[i].lce_devs[lce_index] = grce[grce_index];
			temp_pce_bitmask = grce[grce_index]->pce_bitmask;
			grce[grce_index] = 0;
			if (nvgpu_safe_add_u32_return(lce_index, 1, &lce_index) == false) {
				nvgpu_err(g, "buffer overflow");
				return -EOVERFLOW;
			}
			if (nvgpu_safe_add_u32_return(grce_index, 1, &grce_index) == false) {
				nvgpu_err(g, "buffer overflow");
				return -EOVERFLOW;
			}
		}
		for (j = 0; j < num_async; j++) {
			temp_dev = (struct nvgpu_device *)async_ces[j];
			if (temp_dev && (temp_dev->pce_bitmask != 0) &&
					(temp_dev->pce_bitmask == temp_pce_bitmask)) {
				gpu_instance[i].lce_devs[lce_index] = async_ces[j];
				async_ces[j]  =  0;
				if (nvgpu_safe_add_u32_return(lce_index, 1, &lce_index) == false) {
					nvgpu_err(g, "buffer overflow");
					return -EOVERFLOW;
				}
			}
		}
		/**
		 * Assign number of lces as lce_index.
		 */
		gpu_instance[i].num_lce = lce_index;
	}
	/**
	 * Populate the remaining async ces in the list.
	 */
	for (j = 0; j < num_async; j++) {
		if (async_ces[j] != NULL) {
			remain_async_ce[remain_async_ces] = async_ces[j];

			if (nvgpu_safe_add_u32_return(remain_async_ces, 1,
						&remain_async_ces) == false) {
				nvgpu_err(g, "Buffer overflow");
				return -EOVERFLOW;
			}
		}
	}

	/**
	 * Compute the required async ces to the total instances.
	 * In 3 mig instances config , one treat as a physical mig
	 * One is GFX and other is Compute.
	 * GFX is already handled and remaining CE is assigned to
	 * Compute MIG.
	 */
	async_ce_per_instance = remain_async_ces / (num_instances - 1);
	if ((async_ce_per_instance == 0) && (remain_async_ces == 1)) {
		for (i = 1; i < num_instances ; i++) {
			if (!gpu_instance[i].is_gfx_capable) {
				gpu_instance[i].num_lce = 1;
				gpu_instance[i].lce_devs[0] = remain_async_ce[0];
			}
		}
	}
	/**
	 * Other  CEs should be cleared
	 * TODO: http://nvbugs/4127277
	 */
	return 0;
}

const struct nvgpu_mig_gpu_instance_config *ga10b_grmgr_get_mig_config_ptr(
		struct gk20a *g) {
	static struct nvgpu_mig_gpu_instance_config ga10b_gpu_instance_default_config;
	struct nvgpu_gpu_instance_config *gpu_instance_config =
		&ga10b_gpu_instance_default_config.gpu_instance_config[0];
	int err;

	if ((g->mig.usable_gr_syspipe_count ==
				ga10b_gpu_instance_config.usable_gr_syspipe_count) &&
			(g->mig.usable_gr_syspipe_mask ==
				ga10b_gpu_instance_config.usable_gr_syspipe_mask) &&
			(g->mig.gpc_count ==
				ga10b_gpu_instance_config.gpc_count) &&
			(g->mig.gpcgrp_gpc_count[0] ==
				ga10b_gpu_instance_config.gpcgrp_gpc_count[0]) &&
			(g->mig.gpcgrp_gpc_count[1] ==
				ga10b_gpu_instance_config.gpcgrp_gpc_count[1])) {
		nvgpu_log(g, gpu_dbg_mig,
			"Static mig config list for 2 syspipes + 2 GPCs + 2 Aysnc LCEs "
				"+ 2:0 gpc group config ");
		return &ga10b_gpu_instance_config;
	}

	/* Fall back to default config */
	ga10b_gpu_instance_default_config.usable_gr_syspipe_count =
		g->mig.usable_gr_syspipe_count;
	ga10b_gpu_instance_default_config.usable_gr_syspipe_mask =
		g->mig.usable_gr_syspipe_mask;
	ga10b_gpu_instance_default_config.num_config_supported = 1U;
	ga10b_gpu_instance_default_config.gpcgrp_gpc_count[0] =
		g->mig.gpcgrp_gpc_count[0];
	ga10b_gpu_instance_default_config.gpcgrp_gpc_count[1] =
		g->mig.gpcgrp_gpc_count[1];
	ga10b_gpu_instance_default_config.gpc_count = g->mig.gpc_count;
	err = snprintf(gpu_instance_config->config_name,
		NVGPU_MIG_MAX_CONFIG_NAME_SIZE,
		"1 GPU instance with %u GPCs", g->mig.gpc_count);
	nvgpu_assert(err > 0);

	gpu_instance_config->num_gpu_instances = 1U;
	gpu_instance_config->gpu_instance_static_config[0].gpu_instance_id = 0U;
	gpu_instance_config->gpu_instance_static_config[0].gr_syspipe_id = 0U;
	gpu_instance_config->gpu_instance_static_config[0].num_gpc =
		g->mig.gpc_count;

	nvgpu_err(g,
		"mig gpu instance config is not found for usable_gr_syspipe_count[%u %u] "
			"usable_gr_syspipe_mask[%x %x] gpc[%u %u] "
			"fall back to 1 GPU instance with %u GPCs",
		g->mig.usable_gr_syspipe_count,
		ga10b_gpu_instance_config.usable_gr_syspipe_count,
		g->mig.usable_gr_syspipe_mask,
		ga10b_gpu_instance_config.usable_gr_syspipe_mask,
		g->mig.gpc_count,
		ga10b_gpu_instance_config.gpc_count,
		g->mig.gpc_count);
	return ((const struct nvgpu_mig_gpu_instance_config *)
		&ga10b_gpu_instance_default_config);
}

void ga10b_grmgr_get_gpcgrp_count(struct gk20a *g)
{
	u32 gpcgrp_gpc_count[2] = {0U, 0U};
	u32 logical_gpc_id = 0U;
	struct nvgpu_gpc *gpcs = g->mig.gpu_instance[0].gr_syspipe.gpcs;
	u32 reg_val = nvgpu_readl(g, smcarb_ugpu_gpc_count_r());
	g->mig.gpcgrp_gpc_count[0] = smcarb_ugpu_gpc_count_ugpu0_v(reg_val);
	g->mig.gpcgrp_gpc_count[1] = smcarb_ugpu_gpc_count_ugpu1_v(reg_val);

	for (logical_gpc_id = 0U; logical_gpc_id < g->mig.gpc_count;
			logical_gpc_id++) {
		if (gpcs[logical_gpc_id].gpcgrp_id == 0U) {
			++gpcgrp_gpc_count[0];
		} else if (gpcs[logical_gpc_id].gpcgrp_id == 1U) {
			++gpcgrp_gpc_count[1];
		} else {
			nvgpu_err(g, "invalid gpcgrp_id[%d]",
				gpcs[logical_gpc_id].gpcgrp_id);
			nvgpu_assert(gpcs[logical_gpc_id].gpcgrp_id <= 1U);
		}
	}

	if ((gpcgrp_gpc_count[0] != g->mig.gpcgrp_gpc_count[0]) ||
			(gpcgrp_gpc_count[1] != g->mig.gpcgrp_gpc_count[1])) {
		nvgpu_log(g, gpu_dbg_mig,
			"expected gpcgrp0_gpc_count[%u] actual gpcgrp0_gpc_count[%u] "
				"expected gpcgrp1_gpc_count[%u] actual gpcgrp1_gpc_count[%u] "
				"g->mig.gpc_count[%u]",
			g->mig.gpcgrp_gpc_count[0], gpcgrp_gpc_count[0],
			g->mig.gpcgrp_gpc_count[1], gpcgrp_gpc_count[1],
			g->mig.gpc_count);
	}

	g->mig.gpcgrp_gpc_count[0] = gpcgrp_gpc_count[0];
	g->mig.gpcgrp_gpc_count[1] = gpcgrp_gpc_count[1];
}

static u32 ga10b_grmgr_get_local_gr_syspipe_index(struct gk20a *g,
		u32 gr_syspipe_id)
{
	u32 usable_gr_syspipe_mask = g->mig.usable_gr_syspipe_mask;
	u32 local_gr_syspipe_index = 0U;
	u32 gr_syspipe_mask = (usable_gr_syspipe_mask &
		nvgpu_safe_sub_u32(BIT32(gr_syspipe_id), 1U));

	while (gr_syspipe_mask != 0U) {
		u32 bit_position = nvgpu_safe_sub_u32(
			(u32)nvgpu_ffs(gr_syspipe_mask), 1UL);
		++local_gr_syspipe_index;
		gr_syspipe_mask ^= BIT32(bit_position);
	}

	nvgpu_log(g, gpu_dbg_mig,
		"usable_gr_syspipe_mask[%x] gr_syspipe_id[%u] "
			"local_gr_syspipe_index[%u] ",
		usable_gr_syspipe_mask, gr_syspipe_id,
		local_gr_syspipe_index);

	return local_gr_syspipe_index;
}

static int ga10b_grmgr_validate_config(struct gk20a *g,
		u32 config_id, struct nvgpu_gpu_instance gpu_instance[],
		u32 *temp_num_gpu_instances)
{
	const struct nvgpu_mig_gpu_instance_config *mig_gpu_instance_config =
			g->ops.grmgr.get_mig_config_ptr(g);
	u32 num_gpc = g->mig.gpc_count;


	if ((mig_gpu_instance_config == NULL) || (num_gpc > NVGPU_MIG_MAX_GPCS)) {
		nvgpu_err(g, "mig_gpu_instance_config NULL "
			"or (num_gpc > NVGPU_MIG_MAX_GPCS)[%u %u] ",
			num_gpc, NVGPU_MIG_MAX_GPCS);
		return -EINVAL;
	}

	*temp_num_gpu_instances =
		mig_gpu_instance_config->gpu_instance_config[config_id].num_gpu_instances;

	if ((config_id >= mig_gpu_instance_config->num_config_supported) ||
		(gpu_instance == NULL) ||
		(*temp_num_gpu_instances > g->ops.grmgr.get_max_sys_pipes(g))) {
		nvgpu_err(g,
			"[Invalid param] conf_id[%u %u] num_gpu_inst[%u %u] ",
			config_id, mig_gpu_instance_config->num_config_supported,
			*temp_num_gpu_instances, g->ops.grmgr.get_max_sys_pipes(g));
		return -EINVAL;
	}
	return 0;
}

static int ga10b_grmgr_compute_lce_gpc_mask(struct gk20a *g, u32 *num_lce,
		u32 *num_gr, u32 temp_num_gpu_instances, u32 *lce_mask,
		u32 *gpc_mask, u32 **gr_instance_id_per_swizzid,
		const struct nvgpu_device **lces)
{
	u32 allowed_swizzid_size = g->ops.grmgr.get_allowed_swizzid_size(g);

	*num_lce = nvgpu_device_get_copies(g, lces, NVGPU_MIG_MAX_ENGINES);
	nvgpu_assert(*num_lce > 0U);

	*num_gr = g->mig.usable_gr_syspipe_count;
	if (*num_gr < temp_num_gpu_instances) {
		nvgpu_err(g, "(num_gr < temp_num_gpu_instances)[%u %u]",
			*num_gr, temp_num_gpu_instances);
		return -EINVAL;
	}

	*lce_mask = nvgpu_safe_cast_u64_to_u32(nvgpu_safe_sub_u64(BIT64(*num_lce), 1ULL));
	*gpc_mask = nvgpu_safe_cast_u64_to_u32(nvgpu_safe_sub_u64(BIT64(g->mig.gpc_count), 1ULL));

	*gr_instance_id_per_swizzid = (u32 *)nvgpu_kzalloc(g,
		nvgpu_safe_mult_u32(sizeof(u32), allowed_swizzid_size));
	if (*gr_instance_id_per_swizzid == NULL) {
		nvgpu_err(g, "(gr_instance_id_per_swizzid- kzalloc failed");
		return -ENOMEM;
	}

	nvgpu_log(g, gpu_dbg_mig, "num_gr[%u] num_lce[%u] ", *num_gr, *num_lce);

	return 0;
}

static int ga10b_grmgr_validate_gr_syspipe_id(struct gk20a *g,
		u32 local_gr_syspipe_index,
		u32 num_gr,
		u32 gr_syspipe_id)
{
	int err = 0;

	if (local_gr_syspipe_index >= num_gr) {
		nvgpu_err(g,
			"GR index config mismatch, "
			"num_gr[%d] actual_gr_index[%u] ",
			num_gr, local_gr_syspipe_index);
		err = -EINVAL;
	}

	if ((g->mig.usable_gr_syspipe_instance_id[local_gr_syspipe_index] !=
			gr_syspipe_id)) {
		nvgpu_err(g,
			"GR SYSPIPE ID mismatch expected[%u] actual[%u] "
			"or (gr_engine_info == NULL) ",
			gr_syspipe_id,
			g->mig.usable_gr_syspipe_instance_id[local_gr_syspipe_index]);
		err = -EINVAL;
	}
	return err;
}

static void ga10b_grmgr_init_instance_from_config(struct gk20a *g,
		const struct nvgpu_gpu_instance_static_config *gpu_instance_static_config,
		struct nvgpu_gr_syspipe *gr_syspipe, u32 index,
		struct nvgpu_gpu_instance *gpu_instance)
{

	gpu_instance[index].gpu_instance_id =
		gpu_instance_static_config[index].gpu_instance_id;
	gpu_instance[index].is_gfx_capable = gpu_instance_static_config[index].is_gfx_capable;
	gpu_instance[index].is_memory_partition_supported = gpu_instance_static_config[index].is_memory_partition_supported;
	gpu_instance[index].veid_mask = gpu_instance_static_config[index].veid_mask;
	gpu_instance[index].lts_mask = gpu_instance_static_config[index].lts_mask;
	gpu_instance[index].fbp_en_mask = gpu_instance_static_config[index].fbp_en_mask;
	gpu_instance[index].num_fbp = gpu_instance_static_config[index].num_fbps;
	gpu_instance[index].swizzle_id = gpu_instance_static_config[index].swizzle_id;
	gr_syspipe->gr_instance_id = gpu_instance_static_config[index].gr_syspipe_id;
	gr_syspipe->gr_syspipe_id =
		gpu_instance_static_config[index].gr_syspipe_id;
	gr_syspipe->gpc_mask = nvgpu_safe_sub_u32(
		BIT32(gr_syspipe->num_gpc), 1U);
	gr_syspipe->gr_dev = nvgpu_device_get(g, NVGPU_DEVTYPE_GRAPHICS,
	      gr_syspipe->gr_syspipe_id);
	nvgpu_assert(gr_syspipe->gr_dev != NULL);
	gpu_instance[index].num_ofa = gpu_instance_static_config[index].num_ofa;
	gpu_instance[index].num_nvjpg = gpu_instance_static_config[index].num_nvjpg;
	gpu_instance[index].num_nvenc = gpu_instance_static_config[index].num_nvenc;
	gpu_instance[index].num_nvdec = gpu_instance_static_config[index].num_nvdec;
	return;
}

static void nvgpu_grmgr_assign_gsp(struct gk20a *g,
		u32 temp_num_gpu_instances,
		struct nvgpu_gpu_instance gpu_instance[])
{
	u32 num_engines = 0U;
	const struct nvgpu_device *gsp_engines[2] = {};
	u32 index = 0U;
	u32 i = 0U;

	num_engines = nvgpu_device_get_by_type(g, gsp_engines,
			NVGPU_DEVTYPE_NVGSP, 2);
	if (num_engines == 0) {
		return;
	}
	for (index = 0U; index < temp_num_gpu_instances; index++) {
		for (i = 0; i < num_engines; i++) {
			if (!gpu_instance[index].is_gfx_capable &&
					(gsp_engines[i]->type == NVGPU_DEVTYPE_NVGSP) &&
					(gsp_engines[i]->fault_id == NVGPU_MIG_GSPLITE_FAULT_ID)) {
				gpu_instance[index].nvgsplite_devs[0] = gsp_engines[i];
				gpu_instance[index].num_nvgsplite =  1;
				g->ops.fb.set_swizzid(g, gsp_engines[i]->fault_id,
					gpu_instance[index].swizzle_id);
			} else {
				if ((gpu_instance[index].is_gfx_capable) &&
						(gsp_engines[i]->fault_id == NVGPU_MIG_GSP_FAULT_ID)) {
					gpu_instance[index].nvgsp_devs[0] = gsp_engines[i];
					g->ops.fb.set_swizzid(g, gsp_engines[i]->fault_id,
						gpu_instance[index].swizzle_id);
					gpu_instance[index].num_nvgsp =  1;
				}
			}

		}
	}
}

static void nvgpu_print_mig_info(struct gk20a *g, u32 index,
		struct nvgpu_gr_syspipe *gr_syspipe,
		struct nvgpu_gpu_instance *gpu_instance,
		u32 max_fbps_count)
{

	nvgpu_log(g, gpu_dbg_mig,
		"[%d] gpu_instance_id[%u] gr_instance_id[%u] "
		"gr_syspipe_id[%u] num_gpc[%u] gr_engine_id[%u] "
		"max_veid_count_per_tsg[%u] veid_start_offset[%u] "
		"veid_end_offset[%u] "
		"is_memory_partition_support[%d] num_lce[%u] "
		"max_fbps_count[%u] num_fbp[%u] fbp_en_mask [0x%x] ",
		index, gpu_instance[index].gpu_instance_id,
		gr_syspipe->gr_instance_id,
		gr_syspipe->gr_syspipe_id,
		gr_syspipe->num_gpc,
		gr_syspipe->gr_dev->engine_id,
		gr_syspipe->max_veid_count_per_tsg,
		gr_syspipe->veid_start_offset,
		nvgpu_safe_sub_u32(
			nvgpu_safe_add_u32(gr_syspipe->veid_start_offset,
				gr_syspipe->max_veid_count_per_tsg), 1U),
		gpu_instance[index].is_memory_partition_supported,
		gpu_instance[index].num_lce,
		max_fbps_count,
		gpu_instance[index].num_fbp,
		gpu_instance[index].fbp_en_mask);
}

static void ga10b_grmgr_assign_gpc_mask(struct gk20a *g, u32 *gpc_mask,
		const struct nvgpu_gpu_instance_static_config *gpu_instance_static_config,
		struct nvgpu_gpc *gpcs,
		struct nvgpu_gr_syspipe *gr_syspipe, u32 index,
		u32 *gpu_instance_gpcgrp_id,
		u32 temp_num_gpu_instances)
{
	u32 temp_gpc_cnt = 0U;
	u32 temp_gpc_mask = *gpc_mask;
	u32 logical_gpc_id = 0U;

	gr_syspipe->num_gpc = 0;
	while (temp_gpc_mask && (temp_gpc_cnt <
			(gpu_instance_static_config[index].num_gpc))) {
		logical_gpc_id = nvgpu_safe_sub_u32(
			(u32)nvgpu_ffs(temp_gpc_mask), 1UL);

		if ((gpcs[logical_gpc_id].gpcgrp_id ==
				gpu_instance_gpcgrp_id[index]) ||
				(temp_num_gpu_instances == 1U)) {
			gr_syspipe->gpcs[temp_gpc_cnt].logical_id =
				gpcs[logical_gpc_id].logical_id;
			gr_syspipe->gpcs[temp_gpc_cnt].physical_id =
				gpcs[logical_gpc_id].physical_id;
			gr_syspipe->gpcs[temp_gpc_cnt].gpcgrp_id =
				gpcs[logical_gpc_id].gpcgrp_id;

			*gpc_mask ^= BIT32(logical_gpc_id);

			nvgpu_log(g, gpu_dbg_mig,
				"gpu_instance_id[%u] "
				"gr_instance_id[%u] gr_syspipe_id[%u] "
				"gpc_local_id[%u] gpc_logical_id[%u] "
				"gpc_physical_id[%u]  gpc_grpid[%u] "
				"free_gpc_mask[%x] gr_syspipe_id[%u]",
				gpu_instance_static_config[index].gpu_instance_id,
				index,
				gpu_instance_static_config[index].gr_syspipe_id,
				temp_gpc_cnt,
				gr_syspipe->gpcs[temp_gpc_cnt].logical_id,
				gr_syspipe->gpcs[temp_gpc_cnt].physical_id,
				gr_syspipe->gpcs[temp_gpc_cnt].gpcgrp_id,
				*gpc_mask,
				gpu_instance_static_config[index].gr_syspipe_id);

			++temp_gpc_cnt;
			gr_syspipe->num_gpc = nvgpu_safe_add_u32(
					gr_syspipe->num_gpc, 1U);
		}
			temp_gpc_mask ^= BIT32(logical_gpc_id);
	}
}

static int nvgpu_grmgr_validate_num_gpc(struct gk20a *g,
		const struct nvgpu_gpu_instance_static_config *gpu_instance_static_config,
		struct nvgpu_gr_syspipe *gr_syspipe, u32 index)

{
	if (gr_syspipe->num_gpc !=
			gpu_instance_static_config[index].num_gpc) {
		nvgpu_err(g,
			"GPC config mismatch, [%d] gpu_instance_id[%u] "
				"gr_syspipe_id[%u] available[%u] expected[%u] ",
				index,
				gpu_instance_static_config[index].gpu_instance_id,
				gpu_instance_static_config[index].gr_syspipe_id,
				gr_syspipe->num_gpc,
				gpu_instance_static_config[index].num_gpc);
		return -EINVAL;
	}
return 0;
}

static int ga10b_grmgr_get_gpu_instance(struct gk20a *g,
		u32 config_id,
		struct nvgpu_gpu_instance gpu_instance[],
		struct nvgpu_gpc gpcs[])
{
	int err = 0;
	u32 num_gr = 0U;
	const struct nvgpu_device *lces[NVGPU_MIG_MAX_ENGINES] = {NULL};
	const struct nvgpu_device *ofa_engines[2] = {NULL};
	const struct nvgpu_device *nvjpg_engines[2] = {NULL};
	const struct nvgpu_device *nvdec_engines[2] = {NULL};
	const struct nvgpu_device *nvenc_engines[2] = {NULL};
	u32 num_ofa_total = 0U;
	u32 num_nvjpg_total = 0U;
	u32 num_nvdec_total = 0U;
	u32 num_nvenc_total = 0U;
	u32 num_lce = 0U;
	u32 num_ofa_engines_per_partition = 0U;
	u32 num_nvenc_engines_per_partition = 0U;
	u32 num_nvdec_engines_per_partition = 0U;
	u32 num_nvjpg_engines_per_partition = 0U;
	struct nvgpu_gr_syspipe *gr_syspipe = NULL;
	u32 index = 0U;
	u32 temp_num_gpu_instances = 0U;
	u32 lce_mask = 0U;
	u32 gpc_mask = 0U;
	u32 local_gr_syspipe_index = 0U;
	u32 veid_count_per_gpc = 0U;
	u32 veid_start_offset = 0U;
	u32 num_gpc = g->mig.gpc_count;
	u32 gpu_instance_gpcgrp_id[NVGPU_MIG_MAX_GPU_INSTANCES];
	const struct nvgpu_gpu_instance_static_config *gpu_instance_static_config;
	u32 *gr_instance_id_per_swizzid;
	const struct nvgpu_mig_gpu_instance_config *mig_gpu_instance_config =
		g->ops.grmgr.get_mig_config_ptr(g);
	u32 max_subctx_count = g->ops.gr.init.get_max_subctx_count();
	u32 max_fbps_count = g->mig.max_fbps_count;
	bool is_mm_required = true;
	bool is_gsp_required = false;

	err = ga10b_grmgr_validate_config(g, config_id, gpu_instance,
				&temp_num_gpu_instances);
	if (err != 0) {
		return err;
	}

	gpu_instance_static_config =
		mig_gpu_instance_config->gpu_instance_config[config_id].gpu_instance_static_config;
	nvgpu_log(g, gpu_dbg_mig,
		"temp_num_gpu_instances[%u] config_name[%s] ",
		temp_num_gpu_instances,
		mig_gpu_instance_config->gpu_instance_config[config_id].config_name);


	err = ga10b_grmgr_compute_lce_gpc_mask(g, &num_lce, &num_gr,
			temp_num_gpu_instances, &lce_mask,
			&gpc_mask, &gr_instance_id_per_swizzid,
			lces);
	if (err != 0) {
		return err;
	}

	nvgpu_assert(max_subctx_count > 0U);
	/* Round down to multiple of 16 */
	veid_count_per_gpc = round_down((max_subctx_count / num_gpc),
			NVGPU_VEID_ROUND_DOWN_VALUE);

	nvgpu_log(g, gpu_dbg_mig, "veid_count_per_gpc[%u] num_gpc[%u] ",
		veid_count_per_gpc, num_gpc);

	for (index = 0U; index < temp_num_gpu_instances; index++) {
		u32 gr_syspipe_id =
			gpu_instance_static_config[index].gr_syspipe_id;

		local_gr_syspipe_index =
			ga10b_grmgr_get_local_gr_syspipe_index(g, gr_syspipe_id);

		err = ga10b_grmgr_validate_gr_syspipe_id(g, local_gr_syspipe_index,
							num_gr, gr_syspipe_id);
		if (err != 0) {
			goto exit;
		}

		gr_syspipe = &gpu_instance[index].gr_syspipe;

		if (g->ops.grmgr.get_gpc_instance_gpcgrp_id(g,
				gpu_instance_static_config[index].gpu_instance_id,
				gpu_instance_static_config[index].gr_syspipe_id,
				&gpu_instance_gpcgrp_id[index]) != 0) {
			nvgpu_err(g,
				"g->ops.grmgr.get_gpc_instance_gpcgrp_id -failed");
			err = -EINVAL;
			goto exit;
		}
		ga10b_grmgr_assign_gpc_mask(g, &gpc_mask, gpu_instance_static_config,
				gpcs, gr_syspipe, index, gpu_instance_gpcgrp_id,
				temp_num_gpu_instances);
		err = nvgpu_grmgr_validate_num_gpc(g, gpu_instance_static_config,
				gr_syspipe, index);
		if (err != 0) {
			goto exit;
		}

		ga10b_grmgr_init_instance_from_config(g, gpu_instance_static_config,
				gr_syspipe, index, gpu_instance);

		gr_syspipe->max_veid_count_per_tsg = nvgpu_safe_mult_u32(
				veid_count_per_gpc, gr_syspipe->num_gpc);
		/* Add extra VEIDs in last gpu instance */
		if ((temp_num_gpu_instances > 1) &&
				(index == temp_num_gpu_instances - 1)) {
			gr_syspipe->max_veid_count_per_tsg =
				nvgpu_safe_add_u32(gr_syspipe->max_veid_count_per_tsg,
						(max_subctx_count -
						 (veid_count_per_gpc * num_gpc)));
		}
		gr_syspipe->veid_start_offset = veid_start_offset;
		veid_start_offset = nvgpu_safe_add_u32(
				veid_start_offset,
				gr_syspipe->max_veid_count_per_tsg);
		gpu_instance[index].gpu_instance_type = NVGPU_MIG_TYPE_MIG;
		g->ops.grmgr.setup_veids_mask(g, index);
		gpu_instance[index].veid_mask = gr_syspipe->veid_mask;

		gpu_instance[index].fbp_l2_en_mask =
			nvgpu_kzalloc(g,
				nvgpu_safe_mult_u64(max_fbps_count, sizeof(u32)));
		if (gpu_instance[index].fbp_l2_en_mask == NULL) {
			nvgpu_err(g,
				"gpu_instance[%d].fbp_l2_en_mask aloc failed",
				index);
			err = -ENOMEM;
			goto exit;
		}

		if (is_mm_required) {
			if (index == 0U) {
				num_ofa_total = nvgpu_device_get_by_type(g, ofa_engines, NVGPU_DEVTYPE_OFA, 2);
				num_nvjpg_total = nvgpu_device_get_by_type(g, nvjpg_engines, NVGPU_DEVTYPE_NVJPG, 2);
				num_nvenc_total = nvgpu_device_get_by_type(g, nvenc_engines, NVGPU_DEVTYPE_NVENC, 2);
				num_nvdec_total = nvgpu_device_get_by_type(g, nvdec_engines, NVGPU_DEVTYPE_NVDEC, 2);
			}

			num_ofa_engines_per_partition = 0U;
			num_nvenc_engines_per_partition = 0U;
			num_nvdec_engines_per_partition = 0U;
			num_nvjpg_engines_per_partition = 0U;
			if ((gpu_instance[index].num_ofa != 0) &&
					(num_ofa_total >= gpu_instance[index].num_ofa)) {
				num_ofa_total = num_ofa_total - gpu_instance[index].num_ofa;
				num_ofa_engines_per_partition = nvgpu_grmgr_assign_ofa(g, gpu_instance[index].num_ofa,
							gpu_instance[index].swizzle_id, index + 1,
							(struct nvgpu_device **)ofa_engines);
			}
			gpu_instance[index].num_ofa = num_ofa_engines_per_partition;

			if ((gpu_instance[index].num_nvjpg != 0) &&
					(num_nvjpg_total >= gpu_instance[index].num_nvjpg)) {
				num_nvjpg_total = num_nvjpg_total - gpu_instance[index].num_nvjpg;
				num_nvjpg_engines_per_partition = nvgpu_grmgr_assign_nvjpg(g, gpu_instance[index].num_nvjpg,
						gpu_instance[index].swizzle_id, index + 1,
						(struct nvgpu_device **)nvjpg_engines);
			}
			gpu_instance[index].num_nvjpg = num_nvjpg_engines_per_partition;

			if ((gpu_instance[index].num_nvenc != 0) &&
					(num_nvenc_total >= gpu_instance[index].num_nvenc)) {

				num_nvenc_total = num_nvenc_total - gpu_instance[index].num_nvenc;
				num_nvenc_engines_per_partition = nvgpu_grmgr_assign_nvenc(g, gpu_instance[index].num_nvenc,
						gpu_instance[index].swizzle_id, index + 1,
						(struct nvgpu_device **)nvenc_engines);
			}
			gpu_instance[index].num_nvenc = num_nvenc_engines_per_partition;

			if ((gpu_instance[index].num_nvdec != 0) &&
					(num_nvdec_total >= gpu_instance[index].num_nvdec)) {

				num_nvdec_total = num_nvdec_total - gpu_instance[index].num_nvdec;
				num_nvdec_engines_per_partition= nvgpu_grmgr_assign_nvdec(g, gpu_instance[index].num_nvdec,
						gpu_instance[index].swizzle_id, index + 1,
						(struct nvgpu_device **)nvdec_engines);
			}
			gpu_instance[index].num_nvdec = num_nvdec_engines_per_partition;
		} else {
			gpu_instance[index].num_ofa = 0U;
			gpu_instance[index].num_nvjpg = 0U;
			gpu_instance[index].num_nvenc = 0U;
			gpu_instance[index].num_nvdec = 0U;
		}
	}
	nvgpu_print_mig_info(g, index, gr_syspipe, gpu_instance,
		max_fbps_count);

	if (is_gsp_required) {
		nvgpu_grmgr_assign_gsp(g, temp_num_gpu_instances, gpu_instance);
	}
	g->mig.num_gpu_instances = nvgpu_safe_add_u32(temp_num_gpu_instances, 1U);
	if (temp_num_gpu_instances > 1) {
		(void)nvgpu_grmgr_instance_assign_ce(g);
	}
exit:
	nvgpu_kfree(g, gr_instance_id_per_swizzid);
	return err;
}

static void ga10b_grmgr_set_smc_state(struct gk20a *g, bool enable)
{
	u32 smc_state = 0U;

	smc_state = nvgpu_readl(g, smcarb_sys_pipe_info_r());

	if (smcarb_sys_pipe_info_mode_v(smc_state) != enable) {
		smc_state &= ~smcarb_sys_pipe_info_mode_m();
		if (enable) {
			smc_state |= smcarb_sys_pipe_info_mode_f(
				smcarb_sys_pipe_info_mode_smc_v());
		} else {
			smc_state |= smcarb_sys_pipe_info_mode_f(
				smcarb_sys_pipe_info_mode_legacy_v());
		}
		nvgpu_writel(g, smcarb_sys_pipe_info_r(), smc_state);
		nvgpu_log(g, gpu_dbg_mig, "MIG boot reg_val[%x] enable[%d]",
			smc_state, enable);
	}
}

static int ga10b_grmgr_config_gpc_smc_map(struct gk20a *g, bool enable)
{
	u32 physical_gpc_id = 0U;
	u32 local_gpc_id = 0U;
	u32 logical_gpc_id = 0U;
	u32 gpu_instance_id = 0U;
	u32 gr_sys_pipe_id = 0U;
	u32 ugpu_id = 0U;
	u32 reg_val = 0U;
	struct nvgpu_gr_syspipe *gr_syspipe = NULL;

	for (gpu_instance_id = 0; gpu_instance_id < g->mig.num_gpu_instances;
			++gpu_instance_id) {

		if (!nvgpu_grmgr_is_mig_type_gpu_instance(
				&g->mig.gpu_instance[gpu_instance_id])) {
			nvgpu_log(g, gpu_dbg_mig, "skip physical instance[%u]",
				gpu_instance_id);
			/* Skip physical device gpu instance when MIG is enabled */
			continue;
		}

		gr_syspipe = &g->mig.gpu_instance[gpu_instance_id].gr_syspipe;
		gr_sys_pipe_id = gr_syspipe->gr_syspipe_id;
		local_gpc_id = 0;

		while (local_gpc_id < gr_syspipe->num_gpc) {
			ugpu_id = gr_syspipe->gpcs[local_gpc_id].gpcgrp_id;
			physical_gpc_id = gr_syspipe->gpcs[local_gpc_id].physical_id;
			logical_gpc_id = gr_syspipe->gpcs[local_gpc_id].logical_id;

			reg_val = nvgpu_readl(g,
				smcarb_smc_partition_gpc_map_r(logical_gpc_id));

			if (enable == false) {
				reg_val = set_field(reg_val,
					smcarb_smc_partition_gpc_map_valid_m(),
					smcarb_smc_partition_gpc_map_valid_f(
						smcarb_smc_partition_gpc_map_valid_false_v()));
			}
			else if (enable && (physical_gpc_id ==
					smcarb_smc_partition_gpc_map_physical_gpc_id_v(reg_val)) &&
					(ugpu_id == smcarb_smc_partition_gpc_map_ugpu_id_v(reg_val))) {
				reg_val = set_field(reg_val,
					smcarb_smc_partition_gpc_map_sys_pipe_local_gpc_id_m(),
					smcarb_smc_partition_gpc_map_sys_pipe_local_gpc_id_f(
						local_gpc_id));
				reg_val = set_field(reg_val,
					smcarb_smc_partition_gpc_map_sys_pipe_id_m(),
					smcarb_smc_partition_gpc_map_sys_pipe_id_f(
						gr_sys_pipe_id));
				reg_val = set_field(reg_val,
					smcarb_smc_partition_gpc_map_valid_m(),
					smcarb_smc_partition_gpc_map_valid_f(
						smcarb_smc_partition_gpc_map_valid_true_v()));
			} else {
				nvgpu_err(g, "wrong mig config found [%u %u %u %u %u]",
					logical_gpc_id,
					physical_gpc_id,
					smcarb_smc_partition_gpc_map_physical_gpc_id_v(reg_val),
					ugpu_id,
					smcarb_smc_partition_gpc_map_ugpu_id_v(reg_val));
				return -EINVAL;
			}

			nvgpu_writel(g, smcarb_smc_partition_gpc_map_r(logical_gpc_id), reg_val);
			nvgpu_log(g, gpu_dbg_mig,
				"[%d] gpu_instance_id[%u] gr_instance_id[%u] "
					"gr_syspipe_id[%u] logical_gpc_id[%u] physical_gpc_id[%u] "
					" local_gpc_id[%u] gpcgrp_id[%u] reg_val[%x] enable[%d] ",
				gpu_instance_id, g->mig.gpu_instance[gpu_instance_id].gpu_instance_id,
				gr_syspipe->gr_instance_id,
				gr_sys_pipe_id, logical_gpc_id, physical_gpc_id,
				local_gpc_id, ugpu_id, reg_val, enable);
			++local_gpc_id;
		}
	}

	if (g->ops.priv_ring.config_gpc_rs_map(g, enable) != 0) {
		nvgpu_err(g, "g->ops.priv_ring.config_gpc_rs_map-failed");
		return -EINVAL;
	}

	if (g->ops.fb.set_smc_eng_config(g, enable) != 0) {
		nvgpu_err(g, "g->ops.fb.set_smc_eng_config-failed");
		return -EINVAL;
	}

	return 0;
}

static int ga10b_grmgr_init_gr_manager_main(struct gk20a *g)
{
	struct nvgpu_gpu_instance *gpu_instance;
	int err = 0;
	u32 i, index;

	if (g->ops.fb.set_fbp_mask != NULL) {
		err = g->ops.fb.set_fbp_mask(g, g->mig.num_gpu_instances);
		if (err != 0) {
			nvgpu_err(g, "failed to set fbp mask");
			return err;
		}
	}

	nvgpu_grmgr_init_fbp_mapping(g, &g->mig.gpu_instance[1],
					nvgpu_safe_sub_u32(g->mig.num_gpu_instances, 1U));

#ifdef CONFIG_NVGPU_NON_FUSA
	for (index = 0; index < g->mig.num_gpu_instances; index++) {
		gpu_instance = &g->mig.gpu_instance[index];
		for_each_set_bit(i, (const unsigned long *)&gpu_instance->fbp_en_mask,
								g->mig.max_fbps_count) {
			gpu_instance->fbp_l2_en_mask[i] = nvgpu_fbp_get_l2_en_mask(g->fbp, i);
		}
	}
#endif

	err = ga10b_grmgr_config_gpc_smc_map(g, true);
	if (err != 0) {
		nvgpu_err(g, "ga10b_grmgr_config_gpc_smc_map-failed[%d]", err);
		return err;
	}

	err = g->ops.fb.config_veid_smc_map(g, true);
	if (err != 0) {
		return err;
	}

#ifdef CONFIG_NVGPU_VPR
	if (g->ops.grmgr.setup_vpr != NULL) {
		err = g->ops.grmgr.setup_vpr(g);
		if (err != 0) {
			nvgpu_err(g, "Failed to setup VPR. Error: %d", err);
			return err;
		}
	}
#endif

	err = g->ops.fb.set_remote_swizid(g, true);
	if (err != 0) {
		nvgpu_err(g, "g->ops.fb.set_remote_swizid-failed[%d]", err);
		return err;
	}

	ga10b_grmgr_set_smc_state(g, true);

	nvgpu_log(g, gpu_dbg_mig,
		"MIG boot success num_gpu_instances[%u] "
			"num_gr_sys_pipes_enabled[%u] gr_syspipe_en_mask[%x]",
		g->mig.num_gpu_instances,
		g->mig.num_gr_sys_pipes_enabled,
		g->mig.gr_syspipe_en_mask);

	return err;
}

int ga10b_grmgr_init_gr_manager(struct gk20a *g)
{
	struct nvgpu_gr_syspipe *gr_syspipe = NULL;
	int err;
	u32 index = 0U;
	const struct nvgpu_device *gr_dev = NULL;
	u32 max_veid_count_per_tsg = g->ops.gr.init.get_max_subctx_count();

	/* Init physical device gpu instance */
	err = nvgpu_init_gr_manager(g);
	if (err != 0) {
		nvgpu_err(g, "nvgpu_init_gr_manager-failed[%d]", err);
		return err;
	}

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG) ||
			(g->mig.gpc_count < 2U)) {
		/*
		 * Fall back to 1 GPU instance.
		 * It can be Physical/legacy or MIG mode based NVGPU_SUPPORT_MIG.
		 */
		nvgpu_log(g, gpu_dbg_mig,
			"Fall back to 1 GPU instance - mode[%s]",
			(nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG) ?
				"MIG_Physical" : "Physical"));
		return 0;
	}

	g->mig.is_nongr_engine_sharable = false;
	if (max_veid_count_per_tsg < 64U) {
		nvgpu_err(g,
			"re-generate mig gpu instance config based on floorsweep config veid[%u]",
			max_veid_count_per_tsg);
		return -EINVAL;
	}

	err = ga10b_grmgr_get_gpu_instance(g, g->mig.current_gpu_instance_config_id,
			&g->mig.gpu_instance[1],
			g->mig.gpu_instance[0].gr_syspipe.gpcs);
	if (err != 0) {
		nvgpu_err(g, "ga10b_grmgr_get_gpu_instance-failed[%d]", err);
		return err;
	}

	g->mig.max_gr_sys_pipes_supported = g->ops.grmgr.get_max_sys_pipes(g);

	g->mig.gr_syspipe_en_mask = 0;
	g->mig.num_gr_sys_pipes_enabled = 0U;
	g->mig.current_gr_syspipe_id = NVGPU_MIG_INVALID_GR_SYSPIPE_ID;
	nvgpu_mutex_init(&g->mig.gr_syspipe_lock);

	for (index = 0U; index  < g->mig.num_gpu_instances; index++) {
		if (!nvgpu_grmgr_is_mig_type_gpu_instance(
				&g->mig.gpu_instance[index])) {
			/* Skip physical device gpu instance when MIG is enabled */
			nvgpu_log(g, gpu_dbg_mig, "skip physical instance[%u]", index);
			continue;
		}
		gr_syspipe = &g->mig.gpu_instance[index].gr_syspipe;
		g->mig.gr_syspipe_en_mask |= BIT32(gr_syspipe->gr_syspipe_id);

		gr_dev = nvgpu_device_get(g, NVGPU_DEVTYPE_GRAPHICS,
			gr_syspipe->gr_syspipe_id);

		nvgpu_assert(gr_dev != NULL);

		/*
		 * HW recommended to put GR engine into reset before programming
		 * config_gpc_rs_map (ga10b_grmgr_config_gpc_smc_map()).
		 */
		if (g->ops.gr.init.set_engine_reset != NULL) {
			nvgpu_gr_exec_for_instance(g, gr_syspipe->gr_syspipe_id,
					g->ops.gr.init.set_engine_reset(g, true));
		} else {
			err = g->ops.mc.enable_dev(g, gr_dev, false);
			if (err != 0) {
				nvgpu_err(g, "GR engine reset failed gr_syspipe_id[%u %u]",
						gr_syspipe->gr_syspipe_id, gr_dev->inst_id);
				return err;
			}
		}

		++g->mig.num_gr_sys_pipes_enabled;
	}

	if (g->ops.gr.init.set_gpcs_reset != NULL) {
		g->ops.gr.init.set_gpcs_reset(g, true);
	}
	err = ga10b_grmgr_init_gr_manager_main(g);
	if (err != 0) {
		nvgpu_err(g, "ga10b_grmgr_init_gr_manager_main failed");
		return err;
	}

	return 0;
}

u32 ga10b_grmgr_get_max_sys_pipes(struct gk20a *g)
{
	(void)g;
	return smcarb_max_partitionable_sys_pipes_v();
}

u32 ga10b_grmgr_get_allowed_swizzid_size(struct gk20a *g)
{
	(void)g;
	return smcarb_allowed_swizzid__size1_v();
}

int ga10b_grmgr_get_gpc_instance_gpcgrp_id(struct gk20a *g,
		u32 gpu_instance_id, u32 gr_syspipe_id, u32 *gpcgrp_id)
{

	if ((gpu_instance_id >= g->ops.grmgr.get_allowed_swizzid_size(g)) ||
		(gr_syspipe_id >= g->ops.grmgr.get_max_sys_pipes(g)) ||
		(gpcgrp_id == NULL)) {
		nvgpu_err(g,
			"[Invalid_param] gr_syspipe_id[%u %u] gpu_instance_id[%u %u] "
				"or gpcgrp_id == NULL ",
				gr_syspipe_id, g->ops.grmgr.get_max_sys_pipes(g),
				gpu_instance_id, g->ops.grmgr.get_allowed_swizzid_size(g));
		return -EINVAL;
	}

	*gpcgrp_id = 0U;
	nvgpu_log(g, gpu_dbg_mig,
			"Found [%u] gpcgrp id for gpu_instance_id[%u] "
				"gr_syspipe_id[%u] ",
			*gpcgrp_id,
			gpu_instance_id,
			gr_syspipe_id);
	return 0;
}

int ga10b_grmgr_remove_gr_manager(struct gk20a *g)
{
	int err;
	u32 index = 0U;

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) {
		/* Fall back to non MIG gr manager remove ops - noop */
		return 0;
	}

	ga10b_grmgr_set_smc_state(g, false);
	err = ga10b_grmgr_config_gpc_smc_map(g, false);
	err |= g->ops.fb.config_veid_smc_map(g, false);
	err |= g->ops.fb.set_remote_swizid(g, false);

	/* Free only MIG instance fbp_l2_en_mask */
	for (index = 1U; index  < g->mig.num_gpu_instances; index++) {
		if (g->mig.gpu_instance[index].fbp_l2_en_mask !=
				NULL) {
			nvgpu_kfree(g,
				g->mig.gpu_instance[index].fbp_l2_en_mask);
			g->mig.gpu_instance[index].fbp_l2_en_mask = NULL;
			g->mig.gpu_instance[index].num_fbp = 0U;
			g->mig.gpu_instance[index].fbp_en_mask = 0U;
		}
	}
	nvgpu_mutex_destroy(&g->mig.gr_syspipe_lock);

	(void)memset(&g->mig, 0, sizeof(struct nvgpu_mig));

	nvgpu_log(g, gpu_dbg_mig, "success");

	return err;
}

int ga10b_grmgr_get_mig_gpu_instance_config(struct gk20a *g,
		const char **config_name,
		u32 *num_config_supported) {

	u32 config_id = 0U;
	const struct nvgpu_mig_gpu_instance_config *mig_gpu_instance_config =
		g->ops.grmgr.get_mig_config_ptr(g);

	if (num_config_supported == NULL) {
		return -EINVAL;
	}

	*num_config_supported = mig_gpu_instance_config->num_config_supported;

	if (config_name != NULL) {
		for (config_id = 0U; config_id < *num_config_supported;
			config_id++) {
				config_name[config_id] =
					mig_gpu_instance_config->gpu_instance_config[config_id].config_name;
		}
	}
	return 0;
}

#endif

void ga10b_grmgr_load_smc_arb_timestamp_prod(struct gk20a *g)
{
	u32 reg_val = 0U;

	/* set prod value for smc arb timestamp ctrl disable tick */
	reg_val = nvgpu_readl(g, smcarb_timestamp_ctrl_r());
	reg_val = set_field(reg_val,
			smcarb_timestamp_ctrl_disable_tick_m(),
                        smcarb_timestamp_ctrl_disable_tick__prod_f());
	nvgpu_writel(g, smcarb_timestamp_ctrl_r(), reg_val);

}

int ga10b_grmgr_discover_gpc_ids(struct gk20a *g,
		u32 num_gpc, struct nvgpu_gpc *gpcs)
{
	u32 logical_gpc_id = 0U;
	u32 reg_val = 0U;

	if (gpcs == NULL) {
		nvgpu_err(g, "no valid gpcs ptr");
		return -EINVAL;
	}

	for (logical_gpc_id = 0U; logical_gpc_id < num_gpc; logical_gpc_id++) {
		reg_val = nvgpu_readl(g,
			smcarb_smc_partition_gpc_map_r(logical_gpc_id));
		gpcs[logical_gpc_id].logical_id = logical_gpc_id;
		gpcs[logical_gpc_id].physical_id =
			smcarb_smc_partition_gpc_map_physical_gpc_id_v(reg_val);
		gpcs[logical_gpc_id].gpcgrp_id =
			smcarb_smc_partition_gpc_map_ugpu_id_v(reg_val);
		nvgpu_log(g, gpu_dbg_mig,
			"index[%u] gpc_logical_id[%u] "
				"gpc_physical_id[%u]  gpc_grpid[%u] ",
			logical_gpc_id,
			gpcs[logical_gpc_id].logical_id,
			gpcs[logical_gpc_id].physical_id,
			gpcs[logical_gpc_id].gpcgrp_id);
	}
	return 0;
}
