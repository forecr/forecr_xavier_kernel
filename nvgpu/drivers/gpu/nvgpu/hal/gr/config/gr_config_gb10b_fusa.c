// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr.h>

#include "gr_config_gb10b.h"
#include "common/gr/gr_config_priv.h"

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>
#include <nvgpu/hw/gb10b/hw_proj_gb10b.h>

/*GB10B specific skyline info*/
#define  NVGPU_SKYLINE_SUPPORTED_NUM_TPC	   10U
#define  NVGPU_SKYLINE_GPC_TPC_MASK_SHIFT          0x4U

#define  NVGPU_SKYLINE_SUPPORTED_GPC_TPC_MASK_1    0x334
/* The singleton tpc is derived from the first GPC */
#define  NVGPU_SKYLINE_SUPPORTED_SINGLETON_MASK_1  0x4

/*244 config specific */
#define  NVGPU_SKYLINE_SUPPORTED_GPC_TPC_MASK_2    0x244
/* The singleton tpc is derived from the second GPC */
#define  NVGPU_SKYLINE_SUPPORTED_SINGLETON_MASK_2  0x2
/* This is the only supported skyline mask */
#define  NVGPU_SKYLINE_SUPPORTED_MASK  0x432

u32 gb10b_gr_config_get_pes_tpc_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index, u32 pes_index)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tmp, tmp1, tmp2;

	(void)config;

	tmp1 = nvgpu_safe_mult_u32(gpc_index, gpc_stride);
	tmp2 = nvgpu_safe_add_u32(gr_gpc0_gpm_pd_pes_tpc_id_mask_r(pes_index),
					tmp1);
	tmp = nvgpu_readl(g, tmp2);

	return gr_gpc0_gpm_pd_pes_tpc_id_mask_mask_v(tmp);
}

void gb10b_gr_config_set_live_pes_mask(struct gk20a *g,
		u32 gpc_count)
{
	u32 pes_mask = 0U;
	u32 temp_mask = 0U;
	u32 offset = 0U;
	u32 gpc_index = 0U;
	u32 reg_pes_mask;

	for (gpc_index = 0U; gpc_index < gpc_count; gpc_index++) {
		offset = nvgpu_gr_gpc_offset(g, gpc_index);
		temp_mask = nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_gpm_pd_live_physical_pes_r(), offset));
		temp_mask =
			gr_gpc0_gpm_pd_live_physical_pes_gpc0_gpm_pd_live_physical_pes_mask_f(temp_mask);
		temp_mask <<=  nvgpu_safe_mult_u32(NUMBER_OF_BITS_COMPUTE_PES_MASK,
				gpc_index);
		pes_mask = temp_mask | pes_mask;
	}
	/*
	 * When TPCs are floorswept, Corresponding PES mask needs to be updated
	 * in this register. For an example : If a GPC contains "x" TPCs and "y"
	 * PES. Every PES needs atleast a TPC to be enabled to work. If
	 * contiguous "x/y" TPCs are floorswept , then "y-1" PES will be active.
	 */
	for (gpc_index = 0U; gpc_index < gpc_count; gpc_index++) {
		offset = nvgpu_gr_gpc_offset(g, gpc_index);
		/*
		 * Each GPC occupies 4 bits, so each resiter holds information
		 * of 8 GPCs.
		 */
		reg_pes_mask = gr_gpc0_swdx_pes_mask_r(gpc_index / 8);
		nvgpu_writel(g, nvgpu_safe_add_u32(reg_pes_mask, offset), pes_mask);
	}
}

#ifdef CONFIG_NVGPU_SM_DIVERSITY
static void gb10b_gr_config_set_redex_sminfo(struct gk20a *g,
		struct nvgpu_gr_config *gr_config, u32 num_sm,
		u32 sm_per_tpc, struct tpc_vgpc_table *vgpc_table)
{
	u32 sm;
	u32 tpc = nvgpu_gr_config_get_tpc_count(gr_config);
	u32 sm_id = 0;
	u32 glboal_index = 0;

	for (sm_id = 0; sm_id < num_sm; sm_id += sm_per_tpc) {
		tpc = nvgpu_safe_sub_u32(tpc, 1U);
		for (sm = 0; sm < sm_per_tpc; sm++) {
			u32 index = nvgpu_safe_add_u32(sm_id, sm);
			struct nvgpu_sm_info *sm_info =
				nvgpu_gr_config_get_redex_sm_info(
					gr_config, index);
			nvgpu_gr_config_set_sm_info_gpc_index(sm_info,
							vgpc_table[tpc].gpc_id);
			nvgpu_gr_config_set_sm_info_tpc_index(sm_info,
							vgpc_table[tpc].tpc_id);
			nvgpu_gr_config_set_sm_info_sm_index(sm_info, sm);
			nvgpu_gr_config_set_sm_info_global_tpc_index(
				sm_info, glboal_index);
			nvgpu_gr_config_set_sm_info_virtual_gpc_index(sm_info,
				vgpc_table[tpc].virtual_gpc_id);

			nvgpu_log(g, gpu_dbg_info | gpu_dbg_gr,
				"gpc : %d tpc %d sm_index %d global_index: %d "
				"vgpc_index: %d",
				nvgpu_gr_config_get_sm_info_gpc_index(sm_info),
				nvgpu_gr_config_get_sm_info_tpc_index(sm_info),
				nvgpu_gr_config_get_sm_info_sm_index(sm_info),
				nvgpu_gr_config_get_sm_info_global_tpc_index(
					sm_info),
				nvgpu_gr_config_get_sm_info_virtual_gpc_index(
					sm_info));

		}
		glboal_index = nvgpu_safe_add_u32(glboal_index, 1U);
	}
}
#endif

static void gb10b_gr_config_set_sminfo(struct gk20a *g,
		struct nvgpu_gr_config *gr_config, u32 num_sm,
		u32 sm_per_tpc, struct tpc_vgpc_table *vgpc_table)
{
	u32 sm;
	u32 tpc = 0;
	u32 sm_id = 0;

	for (sm_id = 0; sm_id < num_sm; sm_id += sm_per_tpc) {
		for (sm = 0; sm < sm_per_tpc; sm++) {
			u32 index = nvgpu_safe_add_u32(sm_id, sm);
			struct nvgpu_sm_info *sm_info =
				nvgpu_gr_config_get_sm_info(gr_config, index);
			nvgpu_assert(sm_info != NULL);

			nvgpu_gr_config_set_sm_info_gpc_index(sm_info,
							vgpc_table[tpc].gpc_id);
			nvgpu_gr_config_set_sm_info_tpc_index(sm_info,
							vgpc_table[tpc].tpc_id);
			nvgpu_gr_config_set_sm_info_sm_index(sm_info, sm);
			nvgpu_gr_config_set_sm_info_global_tpc_index(sm_info,
					tpc);
			nvgpu_gr_config_set_sm_info_virtual_gpc_index(sm_info,
				vgpc_table[tpc].virtual_gpc_id);

			nvgpu_log(g, gpu_dbg_info | gpu_dbg_gr,
				"gpc : %d tpc %d sm_index %d global_index: %d "
				"vgpc_index: %d",
				nvgpu_gr_config_get_sm_info_gpc_index(sm_info),
				nvgpu_gr_config_get_sm_info_tpc_index(sm_info),
				nvgpu_gr_config_get_sm_info_sm_index(sm_info),
				nvgpu_gr_config_get_sm_info_global_tpc_index(
					sm_info),
				nvgpu_gr_config_get_sm_info_virtual_gpc_index(
					sm_info));

		}
		tpc = nvgpu_safe_add_u32(tpc, 1U);
	}

#ifdef CONFIG_NVGPU_SM_DIVERSITY
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_SM_DIVERSITY)) {
		gb10b_gr_config_set_redex_sminfo(g, gr_config, num_sm,
			sm_per_tpc, vgpc_table);
	}
#endif
}

static void gb10b_gr_config_set_vgpc_in_mpc_gpm(struct gk20a *g,
		struct nvgpu_gr_config *gr_config)
{
	u32 gpc_id, tpc_id, vgpc, reg_val, tpc_cnt;
	u32 vtpc_id, gpm_vgpc, tpc_offset;
	u32 gpc_cnt = nvgpu_gr_config_get_gpc_count(gr_config);
	u32 gpc_base = nvgpu_get_litter_value(g, GPU_LIT_GPC_BASE);
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_in_gpc_base = nvgpu_get_litter_value(g,
			GPU_LIT_TPC_IN_GPC_BASE);
	u32 tpc_in_gpc_stride = nvgpu_get_litter_value(g,
			GPU_LIT_TPC_IN_GPC_STRIDE);
	u32 gpc_base_addr, tpc_base_addr, vgpc_id;
	u32 tpc_id_org = 0U;

	for (gpc_id = 0; gpc_id < gpc_cnt; gpc_id++) {
		gpc_base_addr =  nvgpu_safe_add_u32(gpc_base,
				(nvgpu_safe_mult_u32(gpc_id, gpc_stride)));
		tpc_cnt = nvgpu_gr_config_get_gpc_tpc_count(gr_config, gpc_id);
		for (vtpc_id = 0; vtpc_id < tpc_cnt; vtpc_id++) {
			tpc_id_org = g->ops.gr.init.get_nonpes_aware_tpc(g, gpc_id, vtpc_id, gr_config);
			if (tpc_id_org != vtpc_id) {
				nvgpu_log(g, gpu_dbg_gr, "logical tpc_id not matching with vtpc_id"
						"tpc_id_org 0x%x vtpc_id 0x%x", tpc_id_org,
						vtpc_id);
			}
			tpc_base_addr =  nvgpu_safe_add_u32(gpc_base_addr,
					(nvgpu_safe_add_u32(tpc_in_gpc_base,
					(nvgpu_safe_mult_u32(tpc_id_org,
						tpc_in_gpc_stride)))));
			nvgpu_log(g, gpu_dbg_gr, "NV_PTPC_PRI_MPC_VGPC: "
					"GPC(0x%x), REG_ADDR(0x%x), "
					"WR_DATA(0x%x)", gpc_id,
					nvgpu_safe_add_u32(gr_mpc_vgpc_r(),
						tpc_base_addr),
					gr_mpc_vgpc_id_f(
						nvgpu_gr_config_get_virtual_gpc_id(
							gr_config, gpc_id,
							vtpc_id)));
			vgpc_id = nvgpu_gr_config_get_virtual_gpc_id( gr_config,
					gpc_id, vtpc_id);
			nvgpu_writel(g, nvgpu_safe_add_u32(gr_mpc_vgpc_r(),
					tpc_base_addr),
					gr_mpc_vgpc_id_f(vgpc_id));
		}

		/*
		 * NV_PTPC_PRI_GPM_VGPC specifies virtual GPC id for a given
		 * virtual TPC. Each register can hold information for 4 TPCs.
		 */
		for (gpm_vgpc = 0; gpm_vgpc < gr_gpm_pd_vgpc__size_1_v();
				gpm_vgpc++) {
			reg_val = 0U;
			for (tpc_offset = 0; tpc_offset < 4; tpc_offset++) {
				tpc_id = (gpm_vgpc << 2) + tpc_offset;
				if (tpc_id >= tpc_cnt) {
					vgpc = ILLEGAL_VGPC;
				} else {
					vgpc = nvgpu_gr_config_get_virtual_gpc_id(
							gr_config, gpc_id,
							tpc_id);
				}
				reg_val |= (gr_gpm_pd_vgpc_tpc0_f(vgpc)) <<
						(gr_gpm_pd_vgpc_tpc0_s() *
						 tpc_offset);
			}
			nvgpu_log(g, gpu_dbg_gr, "NV_PGPC_PRI_GPM_PD_VGPC(%d): "
					"GPC(0x%x), REG_ADDR(0x%x), "
					"WR_DATA(0x%x)", gpm_vgpc, gpc_id,
					nvgpu_safe_add_u32(gr_gpm_pd_vgpc_r(gpm_vgpc),
					 gpc_base_addr), reg_val);
			nvgpu_writel(g, nvgpu_safe_add_u32(
						gr_gpm_pd_vgpc_r(gpm_vgpc),
						gpc_base_addr), reg_val);
		}
	}
}

static void gb10b_gr_config_setup_skyline(struct gk20a *g,
		struct nvgpu_gr_config *gr_config,
		struct tpc_vgpc_table *vgpc_table,
		u32 *virtual_gpc_to_logical_gpc,
		u32 gpc_in_skyline_cnt, u32 gpc_cnt,
		u32 *virtual_gpc_tpc_count)
{
	u32 tpc_id = 0, gpc_id, global_tpc_id = 0, skip_gpc_cnt;
	u32 unused_tpc = 0U;
	u32 num_gpc = nvgpu_gr_config_get_gpc_count(gr_config);
	u32 litter_num_tpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_TPC_PER_GPC);
	u32 total_singletons = 0U;
	u32 num_tpc = nvgpu_gr_config_get_tpc_count(gr_config);
	u32 singleton_id = 0U;
	/*
	 * Walk through the virtual GPC's and assign the global TPC id breadth
	 * first, until we hit the skyline of each GPC. An example config:
	 * CWD_SM_ID_VGPC(0): 0x9060300
	 * TPC0=0 TPC1=3 TPC2=6 TPC3=9
	 * CWD_SM_ID_VGPC(1): 0xa070401
	 * TPC0=1 TPC1=4 TPC2=7 TPC3=10
	 * CWD_SM_ID_VGPC(2): 0xb080502
	 * TPC0=2 TPC1=5 TPC2=8 TPC3=11
	 */
	global_tpc_id = 0;
	skip_gpc_cnt = nvgpu_safe_sub_u32(num_gpc, gpc_in_skyline_cnt);
	for (tpc_id = 0; tpc_id < litter_num_tpc; tpc_id++) {
		for (gpc_id = 0; gpc_id < gpc_cnt; gpc_id++) {
			if (tpc_id < nvgpu_gr_config_get_gpc_skyline(gr_config, gpc_id)) {
				vgpc_table[global_tpc_id].virtual_gpc_id
					= gpc_id;
				vgpc_table[global_tpc_id].global_tpc_id
					= global_tpc_id;
				vgpc_table[global_tpc_id].gpc_id =
					virtual_gpc_to_logical_gpc[
						nvgpu_safe_add_u32(
							gpc_id, skip_gpc_cnt)];
				vgpc_table[global_tpc_id].tpc_id = tpc_id;
				nvgpu_gr_config_set_virtual_gpc_id(gr_config,
						vgpc_table, global_tpc_id,
						gpc_id, tpc_id);
				global_tpc_id = nvgpu_safe_add_u32(
						global_tpc_id, 1U);
			}
		}
	}

	total_singletons = gr_config->num_singletons;
	singleton_id = 0U;
	while (total_singletons != 0) {
		for (gpc_id = 0; gpc_id < num_gpc; gpc_id++) {
			if (gr_config->virtual_gpc_singleton_cnt[gpc_id] != 0) {
				vgpc_table[global_tpc_id].virtual_gpc_id =
					gr_config->singleton_to_vgpc[singleton_id];
				vgpc_table[global_tpc_id].global_tpc_id = global_tpc_id;
				vgpc_table[global_tpc_id].gpc_id = virtual_gpc_to_logical_gpc[gpc_id];
				vgpc_table[global_tpc_id].tpc_id = virtual_gpc_tpc_count[gpc_id] -
					gr_config->virtual_gpc_singleton_cnt[gpc_id];
				gr_config->gpc_info[vgpc_table[global_tpc_id].gpc_id].virtual_gpc_info
					[vgpc_table[global_tpc_id].tpc_id] =
						gr_config->singleton_to_vgpc[singleton_id];
				global_tpc_id++;
				gr_config->virtual_gpc_singleton_cnt[gpc_id]--;
				singleton_id++;
				total_singletons--;

				if (total_singletons == 0)
					break;
			}
		}
	}

	unused_tpc = num_tpc - gr_config->num_tpc_in_skyline - gr_config->num_singletons;

	while ((gr_config->num_singletons != 0U) && (unused_tpc != 0)) {
		for (gpc_id = 0; gpc_id < num_gpc; gpc_id++) {
			if (gr_config->virtual_gpc_singleton_cnt[gpc_id] != 0) {
				vgpc_table[global_tpc_id].virtual_gpc_id =
					ILLEGAL_VGPC;
				vgpc_table[global_tpc_id].global_tpc_id = global_tpc_id;
				vgpc_table[global_tpc_id].gpc_id =
					virtual_gpc_to_logical_gpc[gpc_id];
				vgpc_table[global_tpc_id].tpc_id =
					virtual_gpc_tpc_count[gpc_id] -
						gr_config->virtual_gpc_singleton_cnt[gpc_id];

				 gr_config->gpc_info[vgpc_table[global_tpc_id].gpc_id].
					 virtual_gpc_info[vgpc_table[global_tpc_id].tpc_id] =
						ILLEGAL_VGPC;

				global_tpc_id++;
				gr_config->virtual_gpc_singleton_cnt[gpc_id]--;
				unused_tpc--;
				if (unused_tpc == 0) {
					break;
				}

			}
		}
	}
}

static void gb10b_gr_config_sort_gpc(struct gk20a *g, u32 gpc_cnt,
		u32 *virtual_gpc_tpc_cnt,
		u32 *virtual_gpc_to_logical_gpc)
{
	u32 gpc_id;
	bool are_gpcs_sorted = false;
	u32 value;

	(void)g;
	/*
	 * “Virtual GPC ID” is obtained by numbering GPCs in increasing order of
	 * TPC counts, i.e GPC with highest number of floor-swept TPCs (meaning
	 * lowest number of functioning TPCs) is the lowest virtual GPC ID.
	 * Sort GPCs in terms of increasing number of TPCs (using bubble sort).
	 */
	while (!are_gpcs_sorted) {
		are_gpcs_sorted = true;
		for (gpc_id = 0; gpc_id < nvgpu_safe_sub_u32(gpc_cnt, 1U); gpc_id++) {
			if (virtual_gpc_tpc_cnt[gpc_id + 1] <
					virtual_gpc_tpc_cnt[gpc_id]) {
				are_gpcs_sorted = false;
				/* Swap TPC number */
				value = virtual_gpc_tpc_cnt[gpc_id];
				virtual_gpc_tpc_cnt[gpc_id]
					= virtual_gpc_tpc_cnt[gpc_id + 1];
				virtual_gpc_tpc_cnt[gpc_id + 1] = value;
				/* Swap the virtual to logical GPC index to match */
				value = virtual_gpc_to_logical_gpc[gpc_id];
				virtual_gpc_to_logical_gpc[gpc_id]
					= virtual_gpc_to_logical_gpc[gpc_id + 1];
				virtual_gpc_to_logical_gpc[gpc_id + 1] = value;
			}
		}
	}
}

static int nvgpu_gr_config_get_skyline_info(struct gk20a *g,
		struct nvgpu_gr_config *gr_config,
		u32 *virtual_gpc_tpc_count)
{
	u32 sky_val;
	u32 mask = 0U;
	u32 i;
	u32 value = 0U;
	u32 gpc_tpc_mask = 0U;
	/*read from dt*/
	bool skyline_enabled  = true;
	u32 singleton_mask = 0U;
	u32 gpc_skyline_cnt = 0U;
	u32 last_sky_val = 0U;
	u32 gpc_id;
	u32 num_singletons = 0U;
	u32 num_gpc = nvgpu_gr_config_get_gpc_count(gr_config);
	u32 num_singleton_gpc = nvgpu_get_litter_value(g,
				GPU_LIT_NUM_SINGLETON_GPCS);
	u32 num_litter_gpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_GPCS);
	u32 num_tpc = nvgpu_gr_config_get_tpc_count(gr_config);

	if (num_tpc != NVGPU_SKYLINE_SUPPORTED_NUM_TPC) {
		skyline_enabled = false;
	}

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) {
		skyline_enabled = false;
	}
	/* Read gpc tpc info and set the mask */
	for (gpc_id = 0; gpc_id < num_gpc; gpc_id++) {
		gpc_tpc_mask = (gpc_tpc_mask << NVGPU_SKYLINE_GPC_TPC_MASK_SHIFT) |
			nvgpu_gr_config_get_gpc_tpc_count(gr_config, gpc_id);
	}

	/*read the mask and check the validity*/
	if (skyline_enabled) {
		mask = NVGPU_SKYLINE_SUPPORTED_MASK;
		if (gpc_tpc_mask == NVGPU_SKYLINE_SUPPORTED_GPC_TPC_MASK_1) {
			singleton_mask = NVGPU_SKYLINE_SUPPORTED_SINGLETON_MASK_1;
		} else if (gpc_tpc_mask == NVGPU_SKYLINE_SUPPORTED_GPC_TPC_MASK_2) {
			singleton_mask = NVGPU_SKYLINE_SUPPORTED_SINGLETON_MASK_2;
		} else {
			/* unsupported config and disable skyline.*/
			skyline_enabled = false;
		}
	}
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_gr, "Skyline info :skyline_enabled %d",
			skyline_enabled);
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_gr, "Skyline info: mask :0x%x singleton_mask: 0x%x",
			mask, singleton_mask);
	if (!skyline_enabled) {
		nvgpu_gr_config_set_singleton_mask(gr_config, 0U);
		nvgpu_gr_config_set_num_singletons(gr_config, 0U);
		nvgpu_gr_config_set_num_tpc_in_skyline(gr_config, 0U);
		gr_config->is_skyline_enabled = false;
		for (gpc_id = 0; gpc_id < num_gpc; gpc_id++) {
			nvgpu_gr_config_set_gpc_skyline(gr_config, gpc_id,
				virtual_gpc_tpc_count[gpc_id]);
			value = nvgpu_gr_config_get_num_tpc_in_skyline(gr_config);
			value = nvgpu_safe_add_u32(value, virtual_gpc_tpc_count[gpc_id]);
			nvgpu_gr_config_set_num_tpc_in_skyline(gr_config, value);
		}
		return 0;
	}

	gr_config->num_singletons = 0U;
	gr_config->num_tpc_in_skyline = 0U;
	for (gpc_id = 0; gpc_id < num_gpc; gpc_id++) {
		sky_val = mask & 0xf;
		mask = mask >> NVGPU_SKYLINE_GPC_TPC_MASK_SHIFT;
		gr_config->num_tpc_in_skyline += sky_val;
		gr_config->gpc_skyline[gpc_id] = sky_val;
		if (sky_val != 0) {
			gpc_skyline_cnt++;
		}
	}

	gr_config->skip_gpc_cnt = num_gpc - gpc_skyline_cnt;
	for (gpc_id = 0; gpc_id < num_gpc; gpc_id++) {
		if (gpc_id < gr_config->skip_gpc_cnt) {
			gr_config->virtual_gpc_singleton_cnt[gpc_id] = virtual_gpc_tpc_count[gpc_id];
		} else {
			sky_val = gr_config->gpc_skyline[gpc_id - gr_config->skip_gpc_cnt];
			if (sky_val > virtual_gpc_tpc_count[gpc_id]  ||
					((sky_val != 0U) && (sky_val < last_sky_val))) {
				return -EINVAL;
			}

			if (sky_val != 0U) {
				last_sky_val = sky_val;
			}
			gr_config->virtual_gpc_singleton_cnt[gpc_id] =
				virtual_gpc_tpc_count[gpc_id] - sky_val;
		}
		gr_config->num_singletons += gr_config->virtual_gpc_singleton_cnt[gpc_id];
	}

	for (i = 0; i < num_singleton_gpc; i++) {
		if (BIT32(i) & singleton_mask) {
			gr_config->singleton_to_vgpc[num_singletons] =  i + num_litter_gpc;
			num_singletons++;
		}
	}

	if (gr_config->num_singletons < num_singletons) {
		return -EINVAL;
	}

	gr_config->num_singletons = num_singletons;
	gr_config->singleton_mask = singleton_mask;
	gr_config->is_skyline_enabled = true;
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_gr, "Skyline info: num_singletons:0x%x, singleton_mask: 0x%x",
			num_singletons, singleton_mask);
	return 0;
}

int gb10b_gr_config_init_sm_id_table(struct gk20a *g,
		struct nvgpu_gr_config *gr_config)
{
	u32 gpc_id;
	u32 sm_per_tpc = nvgpu_gr_config_get_sm_count_per_tpc(gr_config);
	u32 gpc_cnt = nvgpu_gr_config_get_gpc_count(gr_config);
	u32 gpc_in_skyline_cnt = gpc_cnt;
	u32 tpc_cnt = nvgpu_gr_config_get_tpc_count(gr_config);
	u32 num_sm = nvgpu_safe_mult_u32(sm_per_tpc, tpc_cnt);
	u32 num_singleton_gpc = nvgpu_get_litter_value(g,
			GPU_LIT_NUM_SINGLETON_GPCS);
	u32 num_tpc_per_gpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_TPC_PER_GPC);
	u32 num_gpcs = nvgpu_get_litter_value(g, GPU_LIT_NUM_GPCS);
	u32 max_tpc_cnt = 0U;
	u32 *virtual_gpc_tpc_cnt = NULL;
	u32 *virtual_gpc_to_logical_gpc = NULL;
	u32 *virtual_gpc_singleton_cnt = NULL;
	u32 *singleton_to_vgpc = NULL;
	struct tpc_vgpc_table *vgpc_table = NULL;
	int err = 0;
	u32 tbl_size = 0U;

	max_tpc_cnt = nvgpu_safe_mult_u32(num_gpcs, num_tpc_per_gpc);
	tbl_size = nvgpu_safe_mult_u32(max_tpc_cnt,
			(u32)sizeof(struct tpc_vgpc_table));
	vgpc_table = nvgpu_kzalloc(g, tbl_size);
	tbl_size = nvgpu_safe_mult_u32(num_singleton_gpc, (u32)sizeof(u32));
	singleton_to_vgpc = nvgpu_kzalloc(g, tbl_size);
	tbl_size = nvgpu_safe_mult_u32((u32)sizeof(u32),
				nvgpu_get_litter_value(g, GPU_LIT_NUM_GPCS));
	virtual_gpc_tpc_cnt = nvgpu_kzalloc(g, tbl_size);
	virtual_gpc_to_logical_gpc = nvgpu_kzalloc(g, tbl_size);
	virtual_gpc_singleton_cnt = nvgpu_kzalloc(g, tbl_size);

	/*
	 * There is a change in the numbering of GPCs and TPCs in the compute
	 * pipe. On Ampere, GPCs have a physical id, and a logical id which is
	 * used for PRI addressing after floorsweeping. From Hopper, there is a
	 * concept of a virtual GPC_id which is used for CGA scheduling and
	 * to support TPC migration. The virtual GPCs are numbered based on the
	 * number of non-floorswept TPCs that exist per GPC. They are numbered
	 * from largest to smallest (floorswept) TPC count, independent of which
	 * GPCs support graphics.
	 */
	if ((vgpc_table == NULL) ||
	    (singleton_to_vgpc == NULL) ||
	    (virtual_gpc_tpc_cnt == NULL) ||
	    (virtual_gpc_to_logical_gpc == NULL) ||
	    (virtual_gpc_singleton_cnt == NULL)) {
		nvgpu_err(g, "Error allocating memory for sm tables");
		err = -ENOMEM;
		goto exit_build_table;
	}

	for (gpc_id = 0; gpc_id < gpc_cnt; gpc_id++) {
		virtual_gpc_tpc_cnt[gpc_id] = nvgpu_gr_config_get_gpc_tpc_count(
				gr_config, gpc_id);
		virtual_gpc_to_logical_gpc[gpc_id] = gpc_id;
	}

	gb10b_gr_config_sort_gpc(g, gpc_cnt, virtual_gpc_tpc_cnt,
			virtual_gpc_to_logical_gpc);

	/*
	 * On Ampere, TPCs have a physical id, a logical id which is used for
	 * PRI space addressing after floorsweeping, and a virtual TPC id that
	 * is PES-aware for SCG scheduling. From Hopper, there is a concept of a
	 * migration TPC_id which is used for CGA scheduling and to support TPC
	 * migration. The migration TPC_id matches the virtual TPC_id except
	 * for a few TPCs which are pulled out of the virtual GPC for CGA
	 * scheduling. They are called “singleton TPCs” and always have a
	 * migration TPC_id of 0. Set singletonMask to 0 */

	err = nvgpu_gr_config_get_skyline_info(g, gr_config, virtual_gpc_tpc_cnt);
	if (err != 0) {
		goto exit_build_table;
	}
	nvgpu_gr_config_set_singleton_mask(gr_config, gr_config->singleton_mask);

	/*
	 * The shape of migratable TPCs looks like a silhouette of a mountain,
	 * and this template is called as “skyline” of a SKU. To begin with,
	 * set the skyline as same as the full TPC count in each virtual GPC.
	 */
	nvgpu_gr_config_set_num_singletons(gr_config, gr_config->num_singletons);
	nvgpu_gr_config_set_num_tpc_in_skyline(gr_config, gr_config->num_tpc_in_skyline);
	gpc_in_skyline_cnt = gpc_cnt - gr_config->skip_gpc_cnt;
	gb10b_gr_config_setup_skyline(g, gr_config, vgpc_table,
			virtual_gpc_to_logical_gpc, gpc_in_skyline_cnt,
			gpc_cnt, virtual_gpc_tpc_cnt);

	nvgpu_gr_config_set_no_of_sm(gr_config, num_sm);
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_gr, "total number of sm = %d",
			num_sm);

	gb10b_gr_config_set_sminfo(g, gr_config, num_sm, sm_per_tpc,
					vgpc_table);

	gb10b_gr_config_set_vgpc_in_mpc_gpm(g, gr_config);

exit_build_table:
	if (vgpc_table != NULL) {
		nvgpu_kfree(g, vgpc_table);
	}
	if (singleton_to_vgpc != NULL) {
		nvgpu_kfree(g, singleton_to_vgpc);
	}
	if (virtual_gpc_tpc_cnt != NULL) {
		nvgpu_kfree(g, virtual_gpc_tpc_cnt);
	}
	if (virtual_gpc_to_logical_gpc != NULL) {
		nvgpu_kfree(g, virtual_gpc_to_logical_gpc);
	}
	if (virtual_gpc_singleton_cnt != NULL) {
		nvgpu_kfree(g, virtual_gpc_singleton_cnt);
	}

	return err;
}
