// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include <nvgpu/grmgr.h>

#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/ctx_mappings.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr_instances.h>

#include "nvgpu/gr/gr_utils.h"

#include "gr_init_gb10b.h"
#include "common/gr/gr_config_priv.h"
#include "common/gr/obj_ctx_priv.h"
#include "common/gr/gr_config_priv.h"

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>

#define NVGPU_GR_ENGINE_RESET_DELAY_US		(20U)
#define NVGPU_FECS_PWR_MODE_TIMEOUT_MAX_US	(2000U)
#define NVGPU_FECS_PWR_MODE_TIMEOUT_DEFAULT_US	(10U)

/*
 * Each gpc can have maximum 32 tpcs, so each tpc index need
 * 5 bits. Each map register(32bits) can hold 6 tpcs info.
 */
#define GR_TPCS_INFO_FOR_MAPREGISTER 6U

#ifdef CONFIG_NVGPU_GRAPHICS
int gb10b_gr_init_rop_mapping(struct gk20a *g,
				struct nvgpu_gr_config *gr_config)
{
	u32 map;
	u32 i, j = 1U;
	u32 base = 0U;
	u32 mapreg_num, offset, mapregs, tile_cnt, tpc_cnt;
	u32 num_gpcs = nvgpu_get_litter_value(g, GPU_LIT_NUM_GPCS);
	u32 num_tpc_per_gpc = nvgpu_get_litter_value(g,
				GPU_LIT_NUM_TPC_PER_GPC);
	u32 num_tpcs = nvgpu_safe_mult_u32(num_gpcs, num_tpc_per_gpc);

	nvgpu_log_fn(g, " ");

	if ((g->numa_node_id ==1 ) || (nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG) &&
			!(nvgpu_grmgr_is_cur_instance_support_gfx(g)))) {
		nvgpu_log_fn(g, " MIG is enabled, skipped rop mapping");
		return 0;
	}

	nvgpu_writel(g, gr_crstr_map_table_cfg_r(),
		gr_crstr_map_table_cfg_row_offset_f(
			nvgpu_gr_config_get_map_row_offset(gr_config)) |
		gr_crstr_map_table_cfg_num_entries_f(
			nvgpu_gr_config_get_tpc_count(gr_config)));
	/*
	 * 6 tpc can be stored in one map register.
	 * But number of tpcs are not always multiple of six,
	 * so adding additional check for valid number of
	 * tpcs before programming map register.
	 */
	/* This if-else logic implements DIV_ROUND_UP in a safe way */
	if (nvgpu_safe_add_u32_return(num_tpcs, GR_TPCS_INFO_FOR_MAPREGISTER, &mapregs) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	} else if (nvgpu_safe_sub_u32_return(mapregs, 1, &mapregs) == false) {
		nvgpu_err(g, "buffer underflow");
		return -EOVERFLOW;
	} else {
		mapregs = mapregs / GR_TPCS_INFO_FOR_MAPREGISTER;
	}

	for (mapreg_num = 0U; mapreg_num < mapregs; mapreg_num++) {
		map = 0U;
		offset = 0U;
		while ((offset < GR_TPCS_INFO_FOR_MAPREGISTER)
			&& (num_tpcs > 0U)) {
			tile_cnt = nvgpu_gr_config_get_map_tile_count(gr_config,
						nvgpu_safe_add_u32(base, offset));
			if (offset == 0U) {
				map = map | gr_crstr_gpc_map_tile0_f(tile_cnt);
			} else if (offset == 1U) {
				map = map | gr_crstr_gpc_map_tile1_f(tile_cnt);
			} else if (offset == 2U) {
				map = map | gr_crstr_gpc_map_tile2_f(tile_cnt);
			} else if (offset == 3U) {
				map = map | gr_crstr_gpc_map_tile3_f(tile_cnt);
			} else if (offset == 4U) {
				map = map | gr_crstr_gpc_map_tile4_f(tile_cnt);
			} else if (offset == 5U) {
				map = map | gr_crstr_gpc_map_tile5_f(tile_cnt);
			} else {
				nvgpu_err(g, "incorrect rop mapping %x",
					  offset);
			}
			num_tpcs--;
			offset++;
		}

		nvgpu_writel(g, gr_crstr_gpc_map_r(mapreg_num), map);
		nvgpu_writel(g, gr_ppcs_wwdx_map_gpc_map_r(mapreg_num), map);
		nvgpu_writel(g, gr_rstr2d_gpc_map_r(mapreg_num), map);

		base = nvgpu_safe_add_u32(base, GR_TPCS_INFO_FOR_MAPREGISTER);
	}

	nvgpu_writel(g, gr_ppcs_wwdx_map_table_cfg_r(),
		gr_ppcs_wwdx_map_table_cfg_row_offset_f(
			nvgpu_gr_config_get_map_row_offset(gr_config)) |
		gr_ppcs_wwdx_map_table_cfg_num_entries_f(
			nvgpu_gr_config_get_tpc_count(gr_config)));

	for (i = 0U; i < gr_ppcs_wwdx_map_table_cfg_coeff__size_1_v(); i++) {
		tpc_cnt = nvgpu_gr_config_get_tpc_count(gr_config);
		nvgpu_writel(g, gr_ppcs_wwdx_map_table_cfg_coeff_r(i),
			gr_ppcs_wwdx_map_table_cfg_coeff_0_mod_value_f(
			   (BIT32(j) % tpc_cnt)) |
			gr_ppcs_wwdx_map_table_cfg_coeff_1_mod_value_f(
			   (BIT32(nvgpu_safe_add_u32(j, 1U)) % tpc_cnt)) |
			gr_ppcs_wwdx_map_table_cfg_coeff_2_mod_value_f(
			   (BIT32(nvgpu_safe_add_u32(j, 2U)) % tpc_cnt)) |
			gr_ppcs_wwdx_map_table_cfg_coeff_3_mod_value_f(
			   (BIT32(nvgpu_safe_add_u32(j, 3U)) % tpc_cnt)));
			j = nvgpu_safe_add_u32(j, 4U);
	}

	nvgpu_writel(g, gr_rstr2d_map_table_cfg_r(),
		gr_rstr2d_map_table_cfg_row_offset_f(
			nvgpu_gr_config_get_map_row_offset(gr_config)) |
		gr_rstr2d_map_table_cfg_num_entries_f(
			nvgpu_gr_config_get_tpc_count(gr_config)));

	return 0;
}
#endif /* CONFIG_NVGPU_GRAPHICS */

#define GB10B_MAX_TPC_PER_GPC	4U

/* Program CWD_GPC_TPC_ID registers */
static void gb10b_gr_init_cwd_gpc_tpc_id(struct gk20a *g, u32 tpc_cnt,
		u32 *tpc_sm_id, u32 sm_per_tpc, u32 no_of_sm, u32 num_gpcs,
		u32 *sm_id_to_vgpc_id, u32 *sm_id_to_singleton_vgpc_id,
		struct nvgpu_gr_config *gr_config,
		struct nvgpu_gr_ctx *gr_ctx, bool patch)
{
	u32 reg_num, tpc_offset;
	u32 reg, sm_id, bits, bit_stride, index;
	u32 tpc_index, gpc_index, vgpc_index, tpc_id;

	/*
	 * NV_PGRAPH_PRI_CWD_GPC_TPC_ID(i) specifies local GPC and virtual
	 * TPC IDs of four TPCs with compute-mode SM_IDs.
	 * Note this mapping must be consistent with PRI_GPM_PD_SM_ID.
	 */
	for (reg_num = 0U; reg_num <= (nvgpu_safe_sub_u32(tpc_cnt, 1U) / GB10B_MAX_TPC_PER_GPC);
			reg_num++) {
		reg = 0;
		/* Each NV_PGRAPH_PRI_CWD_GPC_TPC_ID can store 4 TPCs.*/
		for (tpc_offset = 0U; tpc_offset < GB10B_MAX_TPC_PER_GPC;
				tpc_offset++) {
			struct nvgpu_sm_info *sm_info;
			bit_stride = nvgpu_safe_add_u32(
						gr_cwd_gpc_tpc_id_gpc0_s(),
						gr_cwd_gpc_tpc_id_tpc0_s());

			tpc_id = (reg_num * GB10B_MAX_TPC_PER_GPC) + tpc_offset;
			sm_id = nvgpu_safe_mult_u32(tpc_id, sm_per_tpc);

			if (sm_id >= no_of_sm) {
				break;
			}
#ifdef CONFIG_NVGPU_SM_DIVERSITY
			if ((gr_ctx == NULL) ||
				nvgpu_gr_ctx_get_sm_diversity_config(gr_ctx) ==
				NVGPU_DEFAULT_SM_DIVERSITY_CONFIG) {
				sm_info =
					nvgpu_gr_config_get_sm_info(
						gr_config, sm_id);
			} else {
				sm_info =
					nvgpu_gr_config_get_redex_sm_info(
						gr_config, sm_id);
			}
#else
			sm_info =
				nvgpu_gr_config_get_sm_info(gr_config, sm_id);
#endif
			nvgpu_assert(sm_info != NULL);

			gpc_index =
				nvgpu_gr_config_get_sm_info_gpc_index(sm_info);
			vgpc_index =
				nvgpu_gr_config_get_sm_info_virtual_gpc_index(
						sm_info);
			tpc_index =
				nvgpu_gr_config_get_sm_info_tpc_index(sm_info);

			nvgpu_log(g, gpu_dbg_gr, "TPC_ID(%d), SM_ID(%d), "
					"GPC_IDX(%d), TPC_IDX(%d), vGPC_IDX(%d)"
					, tpc_id, sm_id, gpc_index, tpc_index,
					vgpc_index);

			bits = gr_cwd_gpc_tpc_id_gpc0_f(gpc_index) |
				gr_cwd_gpc_tpc_id_tpc0_f(tpc_index);
			reg |= bits << nvgpu_safe_mult_u32(tpc_offset,
					bit_stride);

			index = nvgpu_safe_mult_u32(num_gpcs,
					((tpc_index & 0xC) >> 2U));
			index = nvgpu_safe_add_u32(gpc_index, index);
			tpc_sm_id[index] |= (tpc_id <<
						nvgpu_safe_mult_u32(
							(tpc_index & 3U),
							bit_stride));

			/*
			 * Handle setup for the vGPC depending on whether it is
			 * for a singleton tpc or not.
			 */
			if (vgpc_index < num_gpcs) {
				index = nvgpu_safe_mult_u32(num_gpcs,
						((tpc_index & 0xC) >> 2U));
				index = nvgpu_safe_add_u32(vgpc_index, index);
				bit_stride = gr_cwd_sm_id_vgpc_tpc1_s();
				sm_id_to_vgpc_id[index] |= (tpc_id <<
							nvgpu_safe_mult_u32(
								(tpc_index & 3U),
								bit_stride));
			} else if (vgpc_index != ILLEGAL_VGPC) {
				index = ((vgpc_index - num_gpcs) & 0x1C) >> 2U;
				bit_stride = gr_cwd_sm_id_singleton_tpc_gpc1_s();
				sm_id_to_singleton_vgpc_id[index] |= (tpc_id <<
							nvgpu_safe_mult_u32(
								((vgpc_index -
								  num_gpcs) & 3U),
								bit_stride));
			}
		}

		// CWD_GPC_TPC_ID is supported in gb10b but is not supported in next chips
		if (g->ops.gr.init.is_cwd_gpc_tpc_id_removed == NULL) {
			nvgpu_log(g, gpu_dbg_gr, "CWD_GPC_TPC_ID(%d): 0x%x",
					reg_num, reg);
			nvgpu_gr_ctx_patch_write(g, gr_ctx,
					gr_cwd_gpc_tpc_id_r(reg_num),
					reg,
					patch);
		}
	}
}

/* Program CWD_VGPC_MTPC_ID(i) */
static void gb10b_gr_init_cwd_vgpc_mtpc_id(struct gk20a *g, u32 tpc_cnt,
		u32 sm_per_tpc, u32 no_of_sm, u32 num_gpcs,
		struct nvgpu_gr_config *gr_config,
		struct nvgpu_gr_ctx *gr_ctx, bool patch)
{
	u32 reg_num, tpc_offset;
	u32 reg, sm_id, bits, bit_stride;
	u32 tpc_index, vgpc_index, tpc_id;

	/*
	 * Hopper SM_ID -> <Virtual GPC ID, Migratable TPC ID (*)>
	 * NV_PGRAPH_PRI_CWD_VGPC_MTPC_ID(i) specifies virtual GPC and
	 * migratable TPC IDs of two TPCs with compute-mode SM_IDs.
	 * 31           24 23           16 15            8 7             0
	 * .-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-.
	 * |      GPC1     |      TPC1     |      GPC0     |      TPC0     |
	 * `-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-'
	 */
	for (reg_num = 0U; reg_num <= (nvgpu_safe_sub_u32(tpc_cnt, 1) >> 1); reg_num++) {
		reg = 0;
		/* Each NV_PGRAPH_PRI_CWD_VGPC_MTPC_ID can store 2 TPCs.*/
		for (tpc_offset = 0U; tpc_offset < 2; tpc_offset++) {
			struct nvgpu_sm_info *sm_info;
			bit_stride = nvgpu_safe_add_u32(
						gr_cwd_vgpc_mtpc_id_gpc0_s(),
						gr_cwd_vgpc_mtpc_id_tpc0_s());

			tpc_id = (reg_num << 1) + tpc_offset;
			sm_id = nvgpu_safe_mult_u32(tpc_id, sm_per_tpc);

			if (sm_id >= no_of_sm) {
				break;
			}
#ifdef CONFIG_NVGPU_SM_DIVERSITY
			if ((gr_ctx == NULL) ||
				nvgpu_gr_ctx_get_sm_diversity_config(gr_ctx) ==
				NVGPU_DEFAULT_SM_DIVERSITY_CONFIG) {
				sm_info =
					nvgpu_gr_config_get_sm_info(
						gr_config, sm_id);
			} else {
				sm_info =
					nvgpu_gr_config_get_redex_sm_info(
						gr_config, sm_id);
			}
#else
			sm_info =
				nvgpu_gr_config_get_sm_info(gr_config, sm_id);
#endif
			nvgpu_assert(sm_info != NULL);

			vgpc_index =
				nvgpu_gr_config_get_sm_info_virtual_gpc_index(sm_info);
			tpc_index =
				nvgpu_gr_config_get_sm_info_tpc_index(sm_info);

			if (vgpc_index >= num_gpcs) {
				tpc_index = 0;
			}

			bits = gr_cwd_vgpc_mtpc_id_gpc0_f(vgpc_index) |
				gr_cwd_vgpc_mtpc_id_tpc0_f(tpc_index);
			reg |= bits << nvgpu_safe_mult_u32(tpc_offset,
					bit_stride);
		}

		nvgpu_log(g, gpu_dbg_gr, "CWD_VGPC_MTPC_ID(%d): 0x%x",
				reg_num, reg);
		nvgpu_gr_ctx_patch_write(g, gr_ctx,
				gr_cwd_vgpc_mtpc_id_r(reg_num),
				reg,
				patch);
	}
}

/* Loads SMID tables into CWD */
int gb10b_gr_init_sm_id_config(struct gk20a *g, u32 *tpc_sm_id,
				struct nvgpu_gr_config *gr_config,
				struct nvgpu_gr_ctx *gr_ctx,
				bool patch)
{
	u32 reg_num, data;
	u32 sm_per_tpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_SM_PER_TPC);
	u32 num_gpcs = nvgpu_get_litter_value(g, GPU_LIT_NUM_GPCS);
	u32 no_of_sm = nvgpu_gr_config_get_no_of_sm(gr_config);
	u32 tpc_cnt = nvgpu_gr_config_get_tpc_count(gr_config);
	u32 *sm_id_to_vgpc_id = NULL;
	u32 *sm_id_to_singleton_vgpc_id = NULL;
	int err = 0;

	sm_id_to_vgpc_id = nvgpu_kzalloc(g, nvgpu_safe_mult_u32(
				gr_cwd_sm_id_vgpc__size_1_v(), sizeof(u32)));
	if (sm_id_to_vgpc_id == NULL) {
		nvgpu_log(g, gpu_dbg_gr, "Insufficient memory");
		err = -ENOMEM;
		goto exit_build_table;
	}

	sm_id_to_singleton_vgpc_id = nvgpu_kzalloc(g, nvgpu_safe_mult_u32(
			gr_cwd_sm_id_singleton_tpc__size_1_v(), sizeof(u32)));
	if (sm_id_to_singleton_vgpc_id == NULL) {
		nvgpu_log(g, gpu_dbg_gr, "Insufficient memory");
		err = -ENOMEM;
		goto exit_build_table;
	}

	/* Program CWD_GPC_TPC_ID(i) */
	gb10b_gr_init_cwd_gpc_tpc_id(g, tpc_cnt, tpc_sm_id, sm_per_tpc,
			no_of_sm, num_gpcs, sm_id_to_vgpc_id,
			sm_id_to_singleton_vgpc_id,
			gr_config, gr_ctx, patch);

	/* Program CWD_VGPC_MTPC_ID(i) */
	gb10b_gr_init_cwd_vgpc_mtpc_id(g, tpc_cnt, sm_per_tpc,
			no_of_sm, num_gpcs, gr_config, gr_ctx, patch);

	/* Program CWD_SM_ID(i) */
	for (reg_num = 0U; reg_num < g->ops.gr.init.get_sm_id_size();
			reg_num++) {
		nvgpu_log(g, gpu_dbg_gr, "CWD_SM_ID(%d): 0x%x", reg_num,
				tpc_sm_id[reg_num]);
		nvgpu_gr_ctx_patch_write(g, gr_ctx,
				gr_cwd_sm_id_r(reg_num),
				tpc_sm_id[reg_num],
				patch);
	}

	/* Program CWD_SM_ID_VGPC(i) */
	for (reg_num = 0U; reg_num < gr_cwd_sm_id_vgpc__size_1_v();
			reg_num++) {
		data = sm_id_to_vgpc_id[reg_num];
		nvgpu_log(g, gpu_dbg_gr, "CWD_SM_ID_VGPC(%d): 0x%x",
				reg_num, data);
		nvgpu_gr_ctx_patch_write(g, gr_ctx,
				gr_cwd_sm_id_vgpc_r(reg_num),
				data,
				patch);
	}

	/* Program CWD_SM_ID_SINGLETON_TPC(i) */
	for (reg_num = 0U; reg_num < gr_cwd_sm_id_singleton_tpc__size_1_v();
			reg_num++) {
		data = sm_id_to_singleton_vgpc_id[reg_num];
		nvgpu_log(g, gpu_dbg_gr, "CWD_SM_ID_SINGLETON_TPC(%d): 0x%x",
				reg_num, data);
		nvgpu_gr_ctx_patch_write(g, gr_ctx,
				gr_cwd_sm_id_singleton_tpc_r(reg_num),
				data,
				patch);
	}

	/* Program CWD_NUM_MTPC_PER_VGPC.
	 * In GB10B, the number of GPCs varies from 2 to 3. Based on the number
	 * of GPCs available, program the field related to GPC2 in CWD register.
	 */
        data = (gr_cwd_num_mtpc_per_vgpc_gpc0_f(
			nvgpu_gr_config_get_gpc_skyline(gr_config, 0U)) |
                gr_cwd_num_mtpc_per_vgpc_gpc1_f(
			nvgpu_gr_config_get_gpc_skyline(gr_config, 1U)));
	if (g->ops.priv_ring.get_gpc_count(g) ==
			nvgpu_get_litter_value(g, GPU_LIT_NUM_GPCS)) {
        	data = data | (gr_cwd_num_mtpc_per_vgpc_gpc2_f(
				nvgpu_gr_config_get_gpc_skyline(gr_config, 2U)));
	}
        nvgpu_log(g, gpu_dbg_gr, "CWD_NUM_MTPC_PER_VGPC: 0x%x", data);
	nvgpu_gr_ctx_patch_write(g, gr_ctx,
			gr_cwd_num_mtpc_per_vgpc_r(),
			data,
			patch);

	/* Program CWD_SINGLETON_TPC_MASK */
        data = nvgpu_gr_config_get_singleton_mask(gr_config);
        nvgpu_log(g, gpu_dbg_gr, "CWD_SINGLETON_TPC_MASK: 0x%x", data);
	nvgpu_gr_ctx_patch_write(g, gr_ctx,
			gr_cwd_singleton_tpc_mask_r(),
			data,
			patch);

exit_build_table:
	if (sm_id_to_vgpc_id != NULL) {
		nvgpu_kfree(g, sm_id_to_vgpc_id);
	}

	if (sm_id_to_singleton_vgpc_id != NULL) {
		nvgpu_kfree(g, sm_id_to_singleton_vgpc_id);
	}

	return err;
}

u32 gb10b_gr_init_get_sm_id_size(void)
{
	return gr_cwd_sm_id__size_1_v();
}

void gb10b_gr_init_sm_id_numbering(struct gk20a *g, u32 gpc, u32 tpc, u32 smid,
				struct nvgpu_gr_config *gr_config,
				struct nvgpu_gr_ctx *gr_ctx,
				bool patch)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_in_gpc_stride = nvgpu_get_litter_value(g,
					GPU_LIT_TPC_IN_GPC_STRIDE);
	u32 gpc_offset = nvgpu_safe_mult_u32(gpc_stride, gpc);
	u32 global_tpc_index;
	u32 tpc_offset;
	u32 offset_sum = 0U;
	struct nvgpu_sm_info *sm_info;

	nvgpu_log(g, gpu_dbg_gr, "SM id %u ", smid);

#ifdef CONFIG_NVGPU_SM_DIVERSITY
	sm_info = (((gr_ctx == NULL) ||
		(nvgpu_gr_ctx_get_sm_diversity_config(gr_ctx) ==
		NVGPU_DEFAULT_SM_DIVERSITY_CONFIG)) ?
			nvgpu_gr_config_get_sm_info(gr_config, smid) :
			nvgpu_gr_config_get_redex_sm_info(gr_config, smid));
#else
		sm_info = nvgpu_gr_config_get_sm_info(gr_config, smid);
#endif
	nvgpu_assert(sm_info != NULL);

	global_tpc_index =
		nvgpu_gr_config_get_sm_info_global_tpc_index(sm_info);

	tpc = g->ops.gr.init.get_nonpes_aware_tpc(g, gpc, tpc, gr_config);
	tpc_offset = nvgpu_safe_mult_u32(tpc_in_gpc_stride, tpc);

	offset_sum = nvgpu_safe_add_u32(gpc_offset, tpc_offset);

	nvgpu_gr_ctx_patch_write(g, gr_ctx,
			nvgpu_safe_add_u32(gr_gpc0_tpc0_sm_cfg_r(), offset_sum),
			gr_gpc0_tpc0_sm_cfg_tpc_id_f(global_tpc_index),
			patch);
	nvgpu_gr_ctx_patch_write(g, gr_ctx,
			nvgpu_safe_add_u32(
				gr_gpc0_gpm_pd_sm_id_r(tpc), gpc_offset),
			gr_gpc0_gpm_pd_sm_id_id_f(global_tpc_index),
			patch);
}

u32 gb10b_gr_init_get_attrib_cb_default_size(struct gk20a *g)
{
	(void)g;
	return gr_gpc0_ppc0_cbm_beta_cb_size_v_default_v();
}

u32 gb10b_gr_init_get_alpha_cb_default_size(struct gk20a *g)
{
	(void)g;
	return gr_gpc0_ppc0_cbm_alpha_cb_size_v_default_v();
}

void gb10b_gr_set_num_gfx_capable_tpcs(struct gk20a *g)
{
	u32 reg_value;
	u32 reg_offset;
	u32 gpc_stride;
	u32 cur_gr_instance_id;
	u32 gpc_count;
	u32 gpc_local_index;
	u32 num_gfx_capable_tpcs;
	u32 gpc_logical_index;

	gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	cur_gr_instance_id = nvgpu_gr_get_cur_instance_id(g);
	gpc_count = nvgpu_grmgr_get_gr_num_gpcs(g, cur_gr_instance_id);

	for (gpc_local_index = 0U; gpc_local_index < gpc_count; ++gpc_local_index) {
		/* Query the number of available TPCs for GFX */
		gpc_logical_index = nvgpu_grmgr_get_gr_gpc_logical_id(g, cur_gr_instance_id, gpc_local_index);
		nvgpu_log(g, gpu_dbg_gr, "Check gfx-capable tpcs for GPC (local=%u, logical=%u)",
			gpc_local_index, gpc_logical_index);
		reg_offset = nvgpu_safe_mult_u32(gpc_stride, gpc_logical_index);
		reg_offset = nvgpu_safe_add_u32(reg_offset, gr_gpc0_fs_gpc_r());
		reg_value = nvgpu_readl(g, reg_offset);
		num_gfx_capable_tpcs = gr_gpc0_fs_gpc_num_available_graphics_tpcs_v(reg_value);

		/* Explicitly set to maximum available GFX TPCs */
		nvgpu_log(g, gpu_dbg_gr, "set GPC%u to have %u GFX-capable TPCs", gpc_logical_index, num_gfx_capable_tpcs);

		reg_offset = nvgpu_safe_mult_u32(gpc_stride, gpc_logical_index);
		reg_offset = nvgpu_safe_add_u32(reg_offset, gr_pri_gpc0_gpccs_tpcs_usable_by_gfx_r());

		/* Sanity check for an overflow of the register */
		nvgpu_assert((gr_pri_gpc0_gpccs_tpcs_usable_by_gfx_num_tpcs_m() | num_gfx_capable_tpcs) ==
			gr_pri_gpc0_gpccs_tpcs_usable_by_gfx_num_tpcs_m());

		reg_value = nvgpu_readl(g, reg_offset);
		reg_value = set_field(reg_value, gr_pri_gpc0_gpccs_tpcs_usable_by_gfx_num_tpcs_m(), num_gfx_capable_tpcs);

		nvgpu_writel(g, reg_offset, reg_value);
	}
}

void gb10b_gr_init_fs_state(struct gk20a *g)
{
	u32 data;
#ifdef CONFIG_NVGPU_NON_FUSA
	u32 ecc_val;
#endif
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr, " ");

	data = nvgpu_readl(g, gr_gpcs_tpcs_sm_texio_control_r());
	data = set_field(data,
		gr_gpcs_tpcs_sm_texio_control_oor_addr_check_mode_m(),
		gr_gpcs_tpcs_sm_texio_control_oor_addr_check_mode_63_56_match_f());
	nvgpu_writel(g, gr_gpcs_tpcs_sm_texio_control_r(), data);

	data = nvgpu_readl(g, gr_gpcs_tpcs_sm_disp_ctrl_r());
	data = set_field(data, gr_gpcs_tpcs_sm_disp_ctrl_re_suppress_m(),
			 gr_gpcs_tpcs_sm_disp_ctrl_re_suppress_disable_f());
	nvgpu_writel(g, gr_gpcs_tpcs_sm_disp_ctrl_r(), data);

#ifdef CONFIG_NVGPU_NON_FUSA
	ecc_val = nvgpu_gr_get_override_ecc_val(g);
	if (ecc_val != 0U) {
		g->ops.fuse.write_feature_override_ecc(g, ecc_val);
	}
#endif

	/*
	 * Removed the errata for CTA_SUBPARTITION_SKEW as this functionality is
	 * removed.
	 */
	data = nvgpu_readl(g, gr_debug_0_r());
	data = set_field(data,
		gr_debug_0_scg_force_slow_drain_tpc_m(),
		gr_debug_0_scg_force_slow_drain_tpc_enabled_f());
	nvgpu_writel(g, gr_debug_0_r(), data);
}

void gb10b_gr_init_override_context_reset(struct gk20a *g)
{
	nvgpu_writel(g, gr_fecs_ctxsw_reset_ctl_r(),
			gr_fecs_ctxsw_reset_ctl_sys_halt_disabled_f() |
			gr_fecs_ctxsw_reset_ctl_sys_engine_reset_disabled_f() |
			gr_fecs_ctxsw_reset_ctl_sys_context_reset_enabled_f());

	nvgpu_writel(g, gr_gpccs_ctxsw_reset_ctl_r(),
			gr_gpccs_ctxsw_reset_ctl_gpc_halt_disabled_f() |
			gr_gpccs_ctxsw_reset_ctl_gpc_reset_disabled_f() |
			gr_gpccs_ctxsw_reset_ctl_gpc_context_reset_enabled_f() |
			gr_gpccs_ctxsw_reset_ctl_zcull_reset_enabled_f());

	nvgpu_udelay(FECS_CTXSW_RESET_DELAY_US);
	(void) nvgpu_readl(g, gr_fecs_ctxsw_reset_ctl_r());
	(void) nvgpu_readl(g, gr_gpccs_ctxsw_reset_ctl_r());

	/* Deassert reset */
	nvgpu_writel(g, gr_fecs_ctxsw_reset_ctl_r(),
			gr_fecs_ctxsw_reset_ctl_sys_halt_disabled_f() |
			gr_fecs_ctxsw_reset_ctl_sys_engine_reset_disabled_f() |
			gr_fecs_ctxsw_reset_ctl_sys_context_reset_disabled_f());

	nvgpu_writel(g, gr_gpccs_ctxsw_reset_ctl_r(),
			gr_gpccs_ctxsw_reset_ctl_gpc_halt_disabled_f() |
			gr_gpccs_ctxsw_reset_ctl_gpc_reset_disabled_f() |
			gr_gpccs_ctxsw_reset_ctl_gpc_context_reset_disabled_f() |
			gr_gpccs_ctxsw_reset_ctl_zcull_reset_disabled_f());

	nvgpu_udelay(FECS_CTXSW_RESET_DELAY_US);
	(void) nvgpu_readl(g, gr_fecs_ctxsw_reset_ctl_r());
	(void) nvgpu_readl(g, gr_gpccs_ctxsw_reset_ctl_r());
}

void gb10b_gr_init_detect_sm_arch(struct gk20a *g)
{
	u32 v = nvgpu_readl(g, gr_gpc0_tpc0_sm_arch_r());

	g->params.sm_arch_spa_version =
		gr_gpc0_tpc0_sm_arch_spa_version_v(v);
	g->params.sm_arch_sm_version =
		gr_gpc0_tpc0_sm_arch_sm_version_v(v);
	g->params.sm_arch_warp_count =
		gr_gpc0_tpc0_sm_arch_warp_count_v(v);
}

void gb10b_gr_init_commit_global_pagepool(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, u64 addr, size_t size, bool patch,
	bool global_ctx)
{
	u32 pp_addr;
	u32 pp_size;

	addr = addr >> gr_scc_pagepool_base_addr_39_8_align_bits_v();

	if (global_ctx) {
		size = size / gr_scc_pagepool_total_pages_byte_granularity_v();
	}

	if (size == g->ops.gr.init.pagepool_default_size(g)) {
		size = gr_scc_pagepool_total_pages_hwmax_v();
	}

	nvgpu_log_info(g, "pagepool buffer addr : 0x%016llx, size : %lu",
		addr, size);

	pp_addr = nvgpu_safe_cast_u64_to_u32(addr);
	pp_size = nvgpu_safe_cast_u64_to_u32(size);
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_scc_pagepool_base_r(),
		gr_scc_pagepool_base_addr_39_8_f(pp_addr), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_scc_pagepool_r(),
		gr_scc_pagepool_total_pages_f(pp_size) |
		gr_scc_pagepool_valid_true_f(), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_gcc_pagepool_base_r(),
		gr_gpcs_gcc_pagepool_base_addr_39_8_f(pp_addr), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_gcc_pagepool_r(),
		gr_gpcs_gcc_pagepool_total_pages_f(pp_size), patch);
}

void gb10b_gr_init_gpc_mmu(struct gk20a *g)
{
	u32 temp;

	nvgpu_log_info(g, "initialize gpc mmu");

	temp = g->ops.fb.mmu_ctrl(g);
	temp &= gr_gpcs_pri_mmu_ctrl_vm_pg_size_m() |
		gr_gpcs_pri_mmu_ctrl_use_pdb_big_page_size_m() |
		gr_gpcs_pri_mmu_ctrl_comp_fault_m() |
		gr_gpcs_pri_mmu_ctrl_miss_gran_m() |
		gr_gpcs_pri_mmu_ctrl_cache_mode_m() |
		gr_gpcs_pri_mmu_ctrl_atomic_capability_mode_m()|
		gr_gpcs_pri_mmu_ctrl_atomic_capability_sys_ncoh_mode_m();
	nvgpu_writel(g, gr_gpcs_pri_mmu_ctrl_r(), temp);
	nvgpu_writel(g, gr_gpcs_pri_mmu_pm_unit_mask_r(), 0);
	nvgpu_writel(g, gr_gpcs_pri_mmu_pm_req_mask_r(), 0);

	nvgpu_writel(g, gr_gpcs_pri_mmu_debug_ctrl_r(),
			g->ops.fb.mmu_debug_ctrl(g));
	nvgpu_writel(g, gr_gpcs_pri_mmu_debug_wr_r(),
			g->ops.fb.mmu_debug_wr(g));
	nvgpu_writel(g, gr_gpcs_pri_mmu_debug_rd_r(),
			g->ops.fb.mmu_debug_rd(g));
	nvgpu_writel(g, gr_gpcs_mmu_num_active_ltcs_r(),
				g->ops.fb.get_num_active_ltcs(g));
}

#ifdef CONFIG_NVGPU_GFXP
void gb10b_gr_init_commit_cbes_reserve(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, bool patch)
{
	u32 cbes_reserve = gr_gpcs_swdx_beta_cb_ctrl_cbes_reserve_gfxp_v();

	nvgpu_gr_ctx_patch_write(g, gr_ctx,
		gr_gpcs_swdx_beta_cb_ctrl_r(),
		gr_gpcs_swdx_beta_cb_ctrl_cbes_reserve_f(cbes_reserve),
		patch);
	nvgpu_gr_ctx_patch_write(g, gr_ctx,
		gr_gpcs_ppcs_cbm_beta_cb_ctrl_r(),
		gr_gpcs_ppcs_cbm_beta_cb_ctrl_cbes_reserve_f(cbes_reserve),
		patch);
}
#endif

static void __gb10b_gr_init_commit_global_attrib_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, struct nvgpu_gr_ctx_mappings *mappings,
	u32 tpc_count, u32 max_tpc, u64 addr, bool patch)
{
	u32 cb_addr;

	(void)tpc_count;
	(void)max_tpc;
	(void)mappings;

	addr = addr >> gr_gpcs_setup_attrib_cb_base_addr_39_12_align_bits_v();

	nvgpu_log_info(g, "attrib cb addr : 0x%016llx", addr);

	cb_addr = nvgpu_safe_cast_u64_to_u32(addr);
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_setup_attrib_cb_base_r(),
		gr_gpcs_setup_attrib_cb_base_addr_39_12_f(cb_addr) |
		gr_gpcs_setup_attrib_cb_base_valid_true_f(), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_tpcs_pe_pin_cb_global_base_addr_r(),
		gr_gpcs_tpcs_pe_pin_cb_global_base_addr_v_f(cb_addr) |
		gr_gpcs_tpcs_pe_pin_cb_global_base_addr_valid_true_f(), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_tpcs_mpc_vtg_cb_global_base_addr_r(),
		gr_gpcs_tpcs_mpc_vtg_cb_global_base_addr_v_f(cb_addr) |
		gr_gpcs_tpcs_mpc_vtg_cb_global_base_addr_valid_true_f(), patch);
}

void gb10b_gr_init_commit_global_attrib_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, struct nvgpu_gr_ctx_mappings *mappings,
	u32 tpc_count, u32 max_tpc, u64 addr, bool patch)
{
	u32 attrBufferSize;
	u32 cb_addr;

	__gb10b_gr_init_commit_global_attrib_cb(g, gr_ctx, mappings, tpc_count,
		max_tpc, addr, patch);

	addr = addr >> gr_gpcs_setup_attrib_cb_base_addr_39_12_align_bits_v();

#ifdef CONFIG_NVGPU_GFXP
	if (nvgpu_gr_ctx_mappings_get_ctx_va(mappings, NVGPU_GR_CTX_PREEMPT_CTXSW) != 0ULL) {
		attrBufferSize = nvgpu_safe_cast_u64_to_u32(
			nvgpu_gr_ctx_get_ctx_mem(gr_ctx, NVGPU_GR_CTX_BETACB_CTXSW)->size);
	} else {
#endif
		attrBufferSize = g->ops.gr.init.get_global_attr_cb_size(g,
			tpc_count, max_tpc);
#ifdef CONFIG_NVGPU_GFXP
	}
#endif

	attrBufferSize /= gr_gpcs_tpcs_tex_rm_cb_1_size_div_128b_granularity_f();

	cb_addr = nvgpu_safe_cast_u64_to_u32(addr);
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_tpcs_mpc_vtg_cb_global_base_addr_r(),
		gr_gpcs_tpcs_mpc_vtg_cb_global_base_addr_v_f(cb_addr) |
		gr_gpcs_tpcs_mpc_vtg_cb_global_base_addr_valid_true_f(), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_tpcs_tex_rm_cb_0_r(),
		gr_gpcs_tpcs_tex_rm_cb_0_base_addr_43_12_f(cb_addr), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_tpcs_tex_rm_cb_1_r(),
		gr_gpcs_tpcs_tex_rm_cb_1_size_div_128b_f(attrBufferSize) |
		gr_gpcs_tpcs_tex_rm_cb_1_valid_true_f(), patch);
}

void gb10b_gr_init_commit_global_bundle_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, u64 addr, u32 size, bool patch)
{
	u32 data;
	u32 cb_addr;
	u32 bundle_cb_token_limit = g->ops.gr.init.get_bundle_cb_token_limit(g);

	addr = addr >> gr_scc_bundle_cb_base_addr_39_8_align_bits_v();

	nvgpu_log_info(g, "bundle cb addr : 0x%016llx, size : %u",
		addr, size);

	cb_addr = nvgpu_safe_cast_u64_to_u32(addr);
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_scc_bundle_cb_base_r(),
		gr_scc_bundle_cb_base_addr_39_8_f(cb_addr), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_scc_bundle_cb_size_r(),
		gr_scc_bundle_cb_size_div_256b_f(size) |
		gr_scc_bundle_cb_size_valid_true_f(), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_swdx_bundle_cb_base_r(),
		gr_gpcs_swdx_bundle_cb_base_addr_39_8_f(cb_addr), patch);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_swdx_bundle_cb_size_r(),
		gr_gpcs_swdx_bundle_cb_size_div_256b_f(size) |
		gr_gpcs_swdx_bundle_cb_size_valid_true_f(), patch);

	/* data for state_limit */
	data = nvgpu_safe_mult_u32(
			g->ops.gr.init.get_bundle_cb_default_size(g),
			gr_scc_bundle_cb_size_div_256b_byte_granularity_v()) /
		gr_pd_ab_dist_cfg2_state_limit_scc_bundle_granularity_v();

	data = min_t(u32, data, g->ops.gr.init.get_min_gpm_fifo_depth(g));

	nvgpu_log_info(g, "bundle cb token limit : %d, state limit : %d",
		bundle_cb_token_limit, data);

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_pd_ab_dist_cfg2_r(),
		gr_pd_ab_dist_cfg2_token_limit_f(bundle_cb_token_limit) |
		gr_pd_ab_dist_cfg2_state_limit_f(data), patch);
}

void gb10b_gr_init_commit_global_cb_manager(struct gk20a *g,
	struct nvgpu_gr_config *config, struct nvgpu_gr_ctx *gr_ctx, bool patch)
{
	u32 attrib_offset_in_chunk = 0;
	u32 alpha_offset_in_chunk = 0;
	u32 pd_ab_max_output;
	u32 gpc_index, ppc_index;
	u32 temp, temp2;
	u32 cbm_cfg_size_beta, cbm_cfg_size_alpha, cbm_cfg_size_steadystate;
	u32 attrib_size_in_chunk, cb_attrib_cache_size_init;
	u32 attrib_cb_default_size = g->ops.gr.init.get_attrib_cb_default_size(g);
	u32 alpha_cb_default_size = g->ops.gr.init.get_alpha_cb_default_size(g);
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 ppc_in_gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_PPC_IN_GPC_STRIDE);
	u32 num_pes_per_gpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_PES_PER_GPC);
	u32 attrib_cb_size;
	u32 alpha_cb_size;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr, " ");
	if ((nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG))
			&& (config != NULL)
			&& (!(config->is_gfx_capable))) {
		return;
	}

	attrib_cb_size = g->ops.gr.init.get_attrib_cb_size(g,
			 nvgpu_gr_config_get_tpc_count(config));
	alpha_cb_size = g->ops.gr.init.get_alpha_cb_size(g,
			  nvgpu_gr_config_get_tpc_count(config));


#ifdef CONFIG_NVGPU_GFXP
	if (nvgpu_gr_ctx_get_graphics_preemption_mode(gr_ctx)
			== NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP) {
		attrib_size_in_chunk =
			g->ops.gr.init.get_attrib_cb_gfxp_size(g);
		cb_attrib_cache_size_init =
			g->ops.gr.init.get_attrib_cb_gfxp_default_size(g);
	} else {
#endif
		attrib_size_in_chunk = attrib_cb_size;
		cb_attrib_cache_size_init = attrib_cb_default_size;
#ifdef CONFIG_NVGPU_GFXP
	}
#endif
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_ds_tga_constraintlogic_beta_r(),
		attrib_cb_default_size, patch);
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_ds_tga_constraintlogic_alpha_r(),
		alpha_cb_default_size, patch);

	pd_ab_max_output = nvgpu_safe_mult_u32(alpha_cb_default_size,
		gr_gpc0_ppc0_cbm_beta_cb_size_v_granularity_v()) /
		gr_pd_ab_dist_cfg1_max_output_granularity_v();

	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_pd_ab_dist_cfg1_r(),
		gr_pd_ab_dist_cfg1_max_output_f(pd_ab_max_output) |
		gr_pd_ab_dist_cfg1_max_batches_init_f(), patch);

	attrib_offset_in_chunk = nvgpu_safe_add_u32(alpha_offset_in_chunk,
				   nvgpu_safe_mult_u32(
					nvgpu_gr_config_get_tpc_count(config),
					alpha_cb_size));

	for (gpc_index = 0;
	     gpc_index < nvgpu_gr_config_get_gpc_count(config);
	     gpc_index++) {
		temp = nvgpu_safe_mult_u32(gpc_stride, gpc_index);
		temp2 = nvgpu_safe_mult_u32(num_pes_per_gpc, gpc_index);
		for (ppc_index = 0;
		     ppc_index < nvgpu_gr_config_get_gpc_ppc_count(config, gpc_index);
		     ppc_index++) {
			u32 pes_tpc_count =
				nvgpu_gr_config_get_pes_tpc_count(config,
							gpc_index, ppc_index);
			u32 ppc_posn = nvgpu_safe_mult_u32(ppc_in_gpc_stride,
							ppc_index);
			u32 sum_temp_pcc = nvgpu_safe_add_u32(temp, ppc_posn);

			cbm_cfg_size_beta =
				nvgpu_safe_mult_u32(
						cb_attrib_cache_size_init,
						pes_tpc_count);
			cbm_cfg_size_alpha =
				nvgpu_safe_mult_u32(alpha_cb_default_size,
						pes_tpc_count);
			cbm_cfg_size_steadystate =
				nvgpu_safe_mult_u32(attrib_cb_default_size,
						pes_tpc_count);

			nvgpu_gr_ctx_patch_write(g, gr_ctx,
				nvgpu_safe_add_u32(
					gr_gpc0_ppc0_cbm_beta_cb_size_r(),
					sum_temp_pcc),
				cbm_cfg_size_beta, patch);

			nvgpu_gr_ctx_patch_write(g, gr_ctx,
				nvgpu_safe_add_u32(
					gr_gpc0_ppc0_cbm_beta_cb_offset_r(),
					sum_temp_pcc),
				attrib_offset_in_chunk, patch);

			nvgpu_gr_ctx_patch_write(g, gr_ctx,
			     nvgpu_safe_add_u32(
			       gr_gpc0_ppc0_cbm_beta_steady_state_cb_size_r(),
			       sum_temp_pcc),
			       cbm_cfg_size_steadystate, patch);

			attrib_offset_in_chunk = nvgpu_safe_add_u32(
				attrib_offset_in_chunk,
				nvgpu_safe_mult_u32(attrib_size_in_chunk,
							pes_tpc_count));

			nvgpu_gr_ctx_patch_write(g, gr_ctx,
				nvgpu_safe_add_u32(
					gr_gpc0_ppc0_cbm_alpha_cb_size_r(),
					sum_temp_pcc),
				cbm_cfg_size_alpha, patch);

			nvgpu_gr_ctx_patch_write(g, gr_ctx,
				nvgpu_safe_add_u32(
					gr_gpc0_ppc0_cbm_alpha_cb_offset_r(),
					sum_temp_pcc),
				alpha_offset_in_chunk, patch);

			alpha_offset_in_chunk = nvgpu_safe_add_u32(
				alpha_offset_in_chunk,
				nvgpu_safe_mult_u32(alpha_cb_size,
							pes_tpc_count));

			nvgpu_gr_ctx_patch_write(g, gr_ctx,
				gr_gpcs_swdx_tc_beta_cb_size_r(
					nvgpu_safe_add_u32(ppc_index, temp2)),
				gr_gpcs_swdx_tc_beta_cb_size_v_f(cbm_cfg_size_steadystate),
				patch);
		}
	}
}

void gb10b_gr_init_commit_global_timeslice(struct gk20a *g)
{
	u32 pd_ab_dist_cfg0 = 0U;
	u32 pe_vaf;
	u32 pe_vsc_vpc;

	nvgpu_log_fn(g, " ");

	pe_vaf = nvgpu_readl(g, gr_gpcs_tpcs_pe_vaf_r());
	pe_vsc_vpc = nvgpu_readl(g, gr_gpcs_tpcs_pes_vsc_vpc_r());

	pe_vaf = gr_gpcs_tpcs_pe_vaf_fast_mode_switch_true_f() | pe_vaf;
	pe_vsc_vpc = gr_gpcs_tpcs_pes_vsc_vpc_fast_mode_switch_true_f() |
		     pe_vsc_vpc;

	nvgpu_gr_ctx_patch_write(g, NULL, gr_gpcs_tpcs_pe_vaf_r(), pe_vaf,
		false);
	nvgpu_gr_ctx_patch_write(g, NULL, gr_gpcs_tpcs_pes_vsc_vpc_r(),
		pe_vsc_vpc, false);

	pd_ab_dist_cfg0 = nvgpu_readl(g, gr_pd_ab_dist_cfg0_r());
	pd_ab_dist_cfg0 = pd_ab_dist_cfg0 |
		gr_pd_ab_dist_cfg0_timeslice_enable_en_f();
	nvgpu_gr_ctx_patch_write(g, NULL, gr_pd_ab_dist_cfg0_r(),
		pd_ab_dist_cfg0, false);
}

void gb10b_gr_init_capture_gfx_regs(struct gk20a *g, struct nvgpu_gr_obj_ctx_gfx_regs *gfx_regs)
{
	gfx_regs->reg_sm_disp_ctrl =
			nvgpu_readl(g, gr_gpcs_tpcs_sm_disp_ctrl_r());
	gfx_regs->reg_gpcs_setup_debug =
			nvgpu_readl(g, gr_pri_gpcs_setup_debug_r());
	gfx_regs->reg_tex_lod_dbg =
			nvgpu_readl(g, gr_pri_gpcs_tpcs_tex_lod_dbg_r());
	gfx_regs->reg_hww_warp_esr_report_mask =
			nvgpu_readl(g, gr_gpcs_tpcs_sms_hww_warp_esr_report_mask_r());
}

u32 gb10b_gr_init_get_min_gpm_fifo_depth(struct gk20a *g)
{
	(void)g;
	return gr_pd_ab_dist_cfg2_state_limit_min_gpm_fifo_depths_v();
}

u32 gb10b_gr_init_get_bundle_cb_token_limit(struct gk20a *g)
{
	(void)g;
	return gr_pd_ab_dist_cfg2_token_limit_init_v();
}

void gb10b_gr_init_set_default_gfx_regs(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
		struct nvgpu_gr_obj_ctx_gfx_regs *gfx_regs)
{
	u32 reg_val;

	nvgpu_gr_ctx_patch_write_begin(g, gr_ctx, true);

	reg_val = set_field(gfx_regs->reg_sm_disp_ctrl,
			gr_gpcs_tpcs_sm_disp_ctrl_killed_ld_is_nop_m(),
			gr_gpcs_tpcs_sm_disp_ctrl_killed_ld_is_nop_disable_f());
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_tpcs_sm_disp_ctrl_r(),
		reg_val, true);

	reg_val = set_field(gfx_regs->reg_gpcs_setup_debug,
			gr_pri_gpcs_setup_debug_poly_offset_nan_is_zero_m(),
			gr_pri_gpcs_setup_debug_poly_offset_nan_is_zero_enable_f());
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_pri_gpcs_setup_debug_r(),
		reg_val, true);

	reg_val = set_field(gfx_regs->reg_tex_lod_dbg,
			gr_pri_gpcs_tpcs_tex_lod_dbg_cubeseam_aniso_m(),
			gr_pri_gpcs_tpcs_tex_lod_dbg_cubeseam_aniso_enable_f());
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_pri_gpcs_tpcs_tex_lod_dbg_r(),
		reg_val, true);

	reg_val = set_field(gfx_regs->reg_hww_warp_esr_report_mask,
			gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_oor_addr_m(),
			gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_oor_addr_no_report_f());
	reg_val = set_field(reg_val,
			gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_misaligned_addr_m(),
			gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_misaligned_addr_no_report_f());
	reg_val = set_field(reg_val,
			gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_invalid_const_addr_ldc_m(),
			gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_invalid_const_addr_ldc_no_report_f());
	reg_val = set_field(reg_val,
			gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_tex_format_m(),
			gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_tex_format_no_report_f());
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_tpcs_sms_hww_warp_esr_report_mask_r(),
		reg_val, true);

	nvgpu_gr_ctx_patch_write_end(g, gr_ctx, true);
}

static void gb10b_gr_init_check_gpcs_status(struct gk20a *g, bool assert_reset)
{
	u32 gpc, gpc_offset;
	u32 reg_offset;
	u32 reg_val;
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 cur_gr_instance_id = nvgpu_gr_get_cur_instance_id(g);
	u32 gpc_count = nvgpu_grmgr_get_gr_num_gpcs(g, cur_gr_instance_id);
	u32 expected_status = 0U;

	if (assert_reset) {
		expected_status = gr_gpc0_gpccs_engine_reset_ctl_status_asserted_v();
	} else {
		expected_status = gr_gpc0_gpccs_engine_reset_ctl_status_deasserted_v();
	}

	for (gpc = 0U; gpc < gpc_count; gpc++) {
		gpc_offset = nvgpu_safe_mult_u32(gpc_stride, gpc);
		reg_offset = nvgpu_safe_add_u32(gpc_offset,
						gr_gpc0_gpccs_engine_reset_ctl_r());
		reg_val = nvgpu_readl(g, reg_offset);
		if (gr_gpc0_gpccs_engine_reset_ctl_status_v(reg_val) != expected_status) {
			nvgpu_err(g, "timed out waiting for GPC%u reset %s", gpc,
					assert_reset ? "assert" : "deassert");
		}
	}
}

void gb10b_gr_init_set_gpcs_reset(struct gk20a *g, bool assert_reset)
{
	u32 reg_val = 0U;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (assert_reset) {
		reg_val = gr_gpcs_gpccs_engine_reset_ctl_reset_assert_f();
	} else {
		reg_val = gr_gpcs_gpccs_engine_reset_ctl_reset_deassert_f();
	}

	nvgpu_writel(g, gr_gpcs_gpccs_engine_reset_ctl_r(), reg_val);
	/* Read same register back to ensure hw propagation of write */
	reg_val = nvgpu_readl(g, gr_gpcs_gpccs_engine_reset_ctl_r());

	nvgpu_udelay(NVGPU_GR_ENGINE_RESET_DELAY_US);

	/* After toggling gpcs reset, check gpc status is as expected */
	gb10b_gr_init_check_gpcs_status(g, assert_reset);
}

void gb10b_gr_init_set_engine_reset(struct gk20a *g, bool assert_reset)
{
	u32 reg_val = 0U;
	u32 expected_status = 0U;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (assert_reset) {
		reg_val = gr_fecs_engine_reset_ctl_reset_assert_f();
		expected_status = gr_fecs_engine_reset_ctl_status_asserted_v();
	} else {
		reg_val = gr_fecs_engine_reset_ctl_reset_deassert_f();
		expected_status = gr_fecs_engine_reset_ctl_status_deasserted_v();
	}

	nvgpu_writel(g, gr_fecs_engine_reset_ctl_r(), reg_val);
	/* Read same register back to ensure hw propagation of write */
	reg_val = nvgpu_readl(g, gr_fecs_engine_reset_ctl_r());

	nvgpu_udelay(NVGPU_GR_ENGINE_RESET_DELAY_US);
	reg_val = nvgpu_readl(g, gr_fecs_engine_reset_ctl_r());

	if (gr_fecs_engine_reset_ctl_status_v(reg_val) != expected_status) {
		nvgpu_err(g, "timed out waiting for GR reset %s",
				assert_reset ? "assert" : "deassert");
	}
}

int gb10b_gr_init_fecs_pwr_blcg_force_on(struct gk20a *g, bool force_on)
{
	struct nvgpu_timeout timeout;
	int ret = 0;
	u32 reg_val;

#ifdef CONFIG_NVGPU_SIM
	if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)) {
		return 0;
	}
#endif

	/*
	 * PWR_BLCG is used to send a request to pmu to force the clocks to gr
	 * engine on. To send a request to force clocks on, set PWR_BLCG_DISABLE
	 * to TRUE and * set the PWR_BLCG_REQ to SEND using a pri write.
	 * PWR_BLCG_DISABLE_AUTO means force clocks off.
	 */
	if (force_on) {
		reg_val = gr_fecs_pwr_blcg_req_send_f() |
			  gr_fecs_pwr_blcg_disable_true_f();
	} else {
		reg_val = gr_fecs_pwr_blcg_req_send_f() |
			  gr_fecs_pwr_blcg_disable_auto_f();
	}

	nvgpu_timeout_init_retry(g, &timeout,
				NVGPU_FECS_PWR_MODE_TIMEOUT_MAX_US /
				NVGPU_FECS_PWR_MODE_TIMEOUT_DEFAULT_US);

	nvgpu_writel(g, gr_fecs_pwr_blcg_r(), reg_val);

	ret = -ETIMEDOUT;

	/*
	 * To verify the request has been completed by PMU, poll for
	 * PWR_BLCG_REQ == DONE.
	 */
	do {
		u32 req = gr_fecs_pwr_blcg_req_v(
				nvgpu_readl(g, gr_fecs_pwr_blcg_r()));
		if (req == gr_fecs_pwr_blcg_req_done_v()) {
			ret = 0;
			break;
		}

		nvgpu_udelay(NVGPU_FECS_PWR_MODE_TIMEOUT_DEFAULT_US);
	} while (nvgpu_timeout_expired_msg(&timeout,
				"timeout setting FE mode %u", force_on) == 0);

	return ret;
}
