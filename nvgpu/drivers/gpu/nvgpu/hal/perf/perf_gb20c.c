// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "perf_gb20c.h"
#include "perf_gb10b.h"
#include <nvgpu/hw/gb20c/hw_perf_gb20c.h>

/* From Gb10b only cblock count changes. Channel count per cblock does not change.*/
#define pmasys_cblock_instances_count					2U

#define PMA_CBLOCK_IDX(pma_channel_id)		((pma_channel_id) /	 \
			(g->ops.perf.get_pma_channels_per_cblock()))
#define PMA_CBLOCK_CH_IDX(pma_channel_id)	((pma_channel_id) %	 \
			(g->ops.perf.get_pma_channels_per_cblock()))

#define NV_PERF_SYS_PARTITION_ALL_DG	({\
	(1U << perf_sys_streaming_dg_id_cmc0_v())	|	\
	(1U << perf_sys_streaming_dg_id_nvdec0_v())	|	\
	(1U << perf_sys_streaming_dg_id_nvenc0_v())	|	\
	(1U << perf_sys_streaming_dg_id_ofa0_v())	|	\
	(1U << perf_sys_streaming_dg_id_pce0_v())	|	\
	(1U << perf_sys_streaming_dg_id_pcie0_v())	|	\
	(1U << perf_sys_streaming_dg_id_pwr0_v())	|	\
	(1U << perf_sys_streaming_dg_id_smca0_v())	|	\
	(1U << perf_sys_streaming_dg_id_smcb0_v())	|	\
	(1U << perf_sys_streaming_dg_id_smga0_v())	|	\
	(1U << perf_sys_streaming_dg_id_smgb0_v())	|	\
	(1U << perf_sys_streaming_dg_id_sys0_v())	|	\
	(1U << perf_sys_streaming_dg_id_sys1_v())	|	\
	(1U << perf_sys_streaming_dg_id_sys2_v())	|	\
	(1U << perf_sys_streaming_dg_id_xbar0_v());	\
})

u32 gb20c_perf_get_cblock_instance_count(void)
{
	return pmasys_cblock_instances_count;
}

u32 gb20c_perf_get_hwpm_fbp_fbpgs_ltcs_base_addr(void)
{
	return perf_pmmfbp_fbpgs_ltcs_base_v();
}

u32 gb20c_perf_get_gpc_tpc_start_dg_idx(void)
{
	return perf_gpc_streaming_dg_id_gpctpca0_v();
}

u32 gb20c_perf_get_gpc_tpc_end_dg_idx(void)
{
	return perf_gpc_streaming_dg_id_gpctpcc5_v();
}

void gb20c_perf_get_num_hwpm_perfmon(struct gk20a *g, u32 *num_sys_perfmon,
				     u32 *num_fbp_perfmon, u32 *num_gpc_perfmon)
{
	(void)g;

	/*
	 * Gb10b onwards, as any TPC can be floorswept with in GPC, perfmons are not
	 * continuous in the GPC range. Hence exact num of perfmon per across GPC chiplets
	 * are not same. So we will capture the existing perfmon masks per each gpc at bind
	 * time based on the floorsweeping mask. These variables are used to store the max
	 * permons per chiplet type.
	 */
	*num_sys_perfmon = perf_pmmsys_engine_sel__size_1_v();
	*num_fbp_perfmon = perf_pmmfbp_engine_sel__size_1_v();
	*num_gpc_perfmon = perf_pmmgpc_engine_sel__size_1_v();
}

void gb20c_update_sys_dg_map_status_mask(struct gk20a *g, u32 gr_instance_id, u32 *sys_dg_map_mask)
{
	(void)gr_instance_id;
	(void)g;

	/**
	 * Only index 0 needs to be updated because we have only 15 DG's
	 * 15 bits. In scenarios where we have more than 32 DG's,
	 * indices of this array need to be filled with appropriate bits.
	 */
	sys_dg_map_mask[0] = NV_PERF_SYS_PARTITION_ALL_DG;
}

bool gb20c_perf_is_perfmon_simulated(void)
{
	return true;
}

#define NUM_OF_DGS_PER_REGISTER		32U
/*
 * Program the secure config registers of the gpc/tpc ccuprof perfmons.
 * Offset of the registers are calculated based on the gpc stride and perfmon's
 * associated tpc physical index retrieved from the dg id(From dg_map_mask).
 */
static void gb20c_perf_set_gpc_ccuprof_registers(struct gk20a *g, u32 val,
						u32 start_gpc, u32 num_gpc,
						u32 gpc_stride, u32 *dg_map_mask)
{
	u32 dgmap_reg_count = perf_pmmsysrouter_user_dgmap_status_secure__size_1_v();
	u32 gpc_local_idx;
	u32 dgmap_reg_idx;
	u32 dg_idx;
	u32 reg_idx;
	u32 tpc_phy_idx;
	u32 reg_offset;
	u32 gpc_reg_idx;
	u32 gpc_log_idx;

	for (gpc_local_idx = 0U; gpc_local_idx < num_gpc; gpc_local_idx++) {
		dgmap_reg_idx = nvgpu_safe_mult_u32(gpc_local_idx, dgmap_reg_count);
		for (reg_idx = dgmap_reg_idx;
			reg_idx < nvgpu_safe_add_u32(dgmap_reg_idx, dgmap_reg_count); reg_idx++) {
			if (dg_map_mask[reg_idx] == 0U) {
				continue;
			}
			gpc_reg_idx = nvgpu_safe_sub_u32(reg_idx, dgmap_reg_idx);
			for_each_set_bit(dg_idx,
					(const unsigned long *)&dg_map_mask[reg_idx],
					BITS_PER_BYTE * sizeof(dg_map_mask[reg_idx])) {
				dg_idx = nvgpu_safe_add_u32(dg_idx,
					nvgpu_safe_mult_u32(gpc_reg_idx, NUM_OF_DGS_PER_REGISTER));
				tpc_phy_idx = nvgpu_safe_sub_u32(dg_idx,
					perf_gpc_streaming_dg_id_ccuproftpc0_v());
				reg_offset = perf_ccuprof_gpc0_secure_config_r(tpc_phy_idx);
				gpc_log_idx = nvgpu_safe_add_u32(gpc_local_idx, start_gpc);
				reg_offset = nvgpu_safe_add_u32(reg_offset,
						nvgpu_safe_mult_u32(gpc_log_idx, gpc_stride));
				nvgpu_log(g, gpu_dbg_prof,
					 "gpc_idx: %u, dg_idx: %u, reg_offset: 0x%08x: val: 0x%08x",
					 gpc_local_idx, dg_idx, reg_offset, val);
				nvgpu_writel(g, reg_offset, val);
			}
		}
	}
}

/*
 * Program the secure config registers of the fbp ccuprof perfmons.
 * Offset of the registers are calculated based on the fbp stride and perfmon's
 * ccuprof relative index retrieved from the dg id(From dg_map_mask).
 */
static void gb20c_perf_program_fbp_ccuprof_registers(struct gk20a *g,
		u32 val, u32 start_fbp, u32 num_fbp,
		u32 fbp_stride, u32 *dg_map_mask)
{
	u32 dgmap_reg_count =
		perf_pmmsysrouter_user_dgmap_status_secure__size_1_v();
	u32 dg_idx = 0;
	u32 fbp_local;
	u32 reg_offset = 0;
	u32 index = 0;
	u32 chiplet_reg_index;
	u32 dg_id;

	for (fbp_local = 0; fbp_local < num_fbp; fbp_local++) {
		chiplet_reg_index = nvgpu_safe_mult_u32(fbp_local,
						       dgmap_reg_count);

		/* Loop through the dgmap_reg_count of dg_map_mask registers for each fpb chiplet*/
		for (index = chiplet_reg_index;
		     index < nvgpu_safe_add_u32(chiplet_reg_index,
						       dgmap_reg_count);
		     index++) {
			if (dg_map_mask[index] == 0U)
				continue;
			/* programming for each ccuprof perfmon marked in dg_map_mask[index] */
			for_each_set_bit(dg_idx,
					(const unsigned long *)&dg_map_mask[index],
					BITS_PER_BYTE * sizeof(dg_map_mask[index])) {
				dg_id = nvgpu_safe_add_u32(dg_idx,
					nvgpu_safe_mult_u32(
						nvgpu_safe_sub_u32(index,
							chiplet_reg_index),
						NUM_OF_DGS_PER_REGISTER));

				dg_id = nvgpu_safe_sub_u32(dg_id,
					perf_fbp_streaming_dg_id_ccuprofltc0ltsp0_v());

				reg_offset = perf_ccuprof_fbp0_secure_config_r(dg_id);

				reg_offset = nvgpu_safe_add_u32(reg_offset,
						nvgpu_safe_mult_u32(nvgpu_safe_add_u32(fbp_local,
									start_fbp),
							  fbp_stride));

				nvgpu_log(g, gpu_dbg_prof,
					 "fbp_idx: %u, dg_idx: %u, reg_offset: 0x%08x: val: 0x%08x",
					 fbp_local, dg_idx, reg_offset, val);
				nvgpu_writel(g, reg_offset, val);
			}
		}
	}
}

#define NUM_OF_CCUPROF_PER_TPC		1U
#define NUM_OF_CCUPROF_PER_FBP		1U

int gb20c_perf_set_secure_config_for_ccu_prof(struct gk20a *g, u32 gr_instance_id,
					     u32 pma_channel_id, bool dg_enable)
{
	u32 cblock_idx = PMA_CBLOCK_IDX(pma_channel_id);
	u32 channel_idx = PMA_CBLOCK_CH_IDX(pma_channel_id);
	int err;
	u32 *gpc_dg_map_mask;
	u32 *fbp_dg_map_mask;
	u32 gpc_dg_map_mask_size;
	u32 fbp_dg_map_mask_size;
	u32 start_gpc;
	u32 start_fbp;
	u32 num_gpc = nvgpu_grmgr_get_gr_num_gpcs(g, gr_instance_id);
	u32 num_fbp = nvgpu_grmgr_get_gr_num_fbps(g, gr_instance_id);
	u32 config_val = 0U;
	u32 i;
	u32 ccuprof_gpc_stride;
	u32 ccuprof_fbp_stride;
	u32 dgmap_status_reg_count =
		perf_pmmsysrouter_user_dgmap_status_secure__size_1_v();

	/* Get logical GPC ID for the 1st local GPC in that MIG instance */
	start_gpc = nvgpu_grmgr_get_gr_gpc_logical_id(g, gr_instance_id, 0x0U);
	/* Get logical FBP index for the 1st local FBP chiplet in that MIG instance */
	start_fbp = nvgpu_grmgr_get_fbp_logical_id(g, gr_instance_id, 0x0U);

	nvgpu_log(g, gpu_dbg_prof,
		  "num_gpc: %u, start_gpc: %u, num_fbp: %u, start_fbp: %u",
		  num_gpc, start_gpc, num_fbp, start_fbp);

	/* Wait for Pmm routers to be idle before we program secure config for DGs */
	err = g->ops.perf.wait_for_idle_pmm_routers(g, gr_instance_id,
						   pma_channel_id);
	if (err)
		return err;

	gpc_dg_map_mask_size = dgmap_status_reg_count * num_gpc;
	gpc_dg_map_mask = nvgpu_kzalloc(g, sizeof(u32) * gpc_dg_map_mask_size);
	if (!gpc_dg_map_mask) {
		err = -ENOMEM;
		goto label_gpc_dg_map_mask;
	}

	fbp_dg_map_mask_size = dgmap_status_reg_count * num_fbp;
	fbp_dg_map_mask = nvgpu_kzalloc(g, sizeof(u32) * fbp_dg_map_mask_size);
	if (!fbp_dg_map_mask) {
		err = -ENOMEM;
		goto label_fbp_dg_map_mask;
	}

	if (dg_enable) {
		config_val = set_field(config_val,
				      perf_pmmsys_secure_config_cblock_id_m(),
				      perf_pmmsys_secure_config_cblock_id_f(cblock_idx));
		config_val = set_field(config_val,
				      perf_pmmsys_secure_config_channel_id_m(),
				      perf_pmmsys_secure_config_channel_id_f(channel_idx));
		config_val = set_field(config_val,
				      perf_pmmsys_secure_config_mapped_m(),
				      perf_pmmsys_secure_config_mapped_true_f());
		config_val = set_field(config_val,
				      perf_pmmsys_secure_config_cmd_slice_id_m(),
				      perf_pmmsys_secure_config_cmd_slice_id_f(pma_channel_id));
		config_val = set_field(config_val,
				      perf_pmmsys_secure_config_command_pkt_decoder_m(),
				      perf_pmmsys_secure_config_command_pkt_decoder_enable_f());
		config_val = set_field(config_val,
				      perf_pmmsys_secure_config_use_prog_dg_idx_m(),
				      perf_pmmsys_secure_config_use_prog_dg_idx_false_f());
		config_val = set_field(config_val,
				      perf_pmmsys_secure_config_dg_idx_m(),
				      perf_pmmsys_secure_config_dg_idx_f(0U));
		config_val = set_field(config_val,
				      perf_pmmsys_secure_config_ignore_cmd_pkt_reset_m(),
				      perf_pmmsys_secure_config_ignore_cmd_pkt_reset_false_f());
	} else {
		config_val = set_field(config_val,
				      perf_pmmsys_secure_config_mapped_m(),
				      perf_pmmsys_secure_config_mapped_false_f());
	}

	/* GPC */
	ccuprof_gpc_stride = perf_ccuprof_gpc1_secure_config_r(0U) -
			     perf_ccuprof_gpc0_secure_config_r(0U);
	g->ops.perf.update_cau_dg_map_status_mask(g, gr_instance_id, num_gpc,
						 perf_gpc_streaming_dg_id_ccuproftpc0_v(),
						 NUM_OF_CCUPROF_PER_TPC,
						 gpc_dg_map_mask,
						 gpc_dg_map_mask_size);
	gb20c_perf_set_gpc_ccuprof_registers(g, config_val,
					    start_gpc, num_gpc,
					    ccuprof_gpc_stride,
					    gpc_dg_map_mask);

	/* FBP */
	gb10b_update_fbp_dg_map_status_mask(num_fbp,
					   perf_fbp_streaming_dg_id_ccuprofltc0ltsp0_v(),
					   NUM_OF_CCUPROF_PER_FBP,
					   fbp_dg_map_mask,
					   fbp_dg_map_mask_size);
	ccuprof_fbp_stride = perf_ccuprof_fbp1_secure_config_r(0U) -
			     perf_ccuprof_fbp0_secure_config_r(0U);
	gb20c_perf_program_fbp_ccuprof_registers(g,
			config_val, start_fbp, num_fbp,
			ccuprof_fbp_stride, fbp_dg_map_mask);

	for (i = 0; i < num_fbp; i++) {
		nvgpu_log(g, gpu_dbg_prof,
			  "gr_instance_id: %u, fbp_dg_map_mask[%u]: 0x%X",
			  gr_instance_id, i * dgmap_status_reg_count,
			  fbp_dg_map_mask[i * dgmap_status_reg_count]);
	}

	/* GPC */
	err = poll_dgmap_status_gpc_perfmons_mapped(g, gpc_dg_map_mask,
						   gpc_dg_map_mask_size,
						   dg_enable, gr_instance_id);
	if (err)
		goto label_fail;

	/* FBP */
	err = poll_dgmap_status_fbp_perfmons_mapped(g, fbp_dg_map_mask,
						   fbp_dg_map_mask_size,
						   dg_enable, gr_instance_id);

label_fail:
	nvgpu_kfree(g, fbp_dg_map_mask);
label_fbp_dg_map_mask:
	nvgpu_kfree(g, gpc_dg_map_mask);
label_gpc_dg_map_mask:
	return err;
}

u32 gb20c_perf_get_num_cwd_hems(void)
{
	return perf_hem_sys0_secure_config__size_1_v();
}

void gb20c_perf_update_sys_hem_cwd_dg_map_mask(struct gk20a *g, u32 gr_instance_id,
					     u32 *sys_dg_map_mask,
					     u32 *start_hem_idx, u32 *num_hems)
{
	(void)g;
	(void)gr_instance_id;

	sys_dg_map_mask[0] = 1U << perf_sys_streaming_dg_id_hemcwd0_v();
	*num_hems = 1U;
	*start_hem_idx = 0U;
}

u32 gb20c_perf_get_gpc_tpc0_cau0_dg_idx(void)
{
	return perf_gpc_streaming_dg_id_tpc0cau0_v();
}

