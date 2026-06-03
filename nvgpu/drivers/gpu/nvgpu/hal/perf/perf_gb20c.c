// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/regops.h>
#include <hal/gr/gr/gr_gk20a.h>
#include <nvgpu/perfbuf.h>
#include <nvgpu/string.h>

#include "perf_gb20c.h"
#include "perf_gb10b.h"
#include <nvgpu/hw/gb20c/hw_perf_gb20c.h>
#include "common/gr/gr_config_priv.h"

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
static int gb20c_perf_set_gpc_ccuprof_registers(struct gk20a *g, u32 val,
					       u32 start_gpc, u32 num_gpc, u32 gpc_stride,
					       u32 *dg_map_mask, bool is_ctx_resident,
					       struct nvgpu_tsg *tsg)
{
	u32 dgmap_reg_count = perf_pmmsysrouter_user_dgmap_status_secure__size_1_v();
	u32 gpc_local_idx, dgmap_reg_idx;
	u32 dg_idx, reg_idx;
	u32 tpc_phy_idx, reg_offset;
	u32 gpc_reg_idx, gpc_log_idx;
	int ret = 0;
	(void) tsg;
	(void) is_ctx_resident;

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
				/* CCU prof registers are not context switched. */
				nvgpu_writel(g, reg_offset, val);
			}
		}
	}

	return ret;
}

/*
 * Program the secure config registers of the fbp ccuprof perfmons.
 * Offset of the registers are calculated based on the fbp stride and perfmon's
 * ccuprof relative index retrieved from the dg id(From dg_map_mask).
 */
static int gb20c_perf_program_fbp_ccuprof_registers(struct gk20a *g,
						   u32 val, u32 start_fbp, u32 num_fbp,
						   u32 fbp_stride, u32 *dg_map_mask,
						   bool is_ctx_resident, struct nvgpu_tsg *tsg)
{
	u32 dgmap_reg_count =
		perf_pmmsysrouter_user_dgmap_status_secure__size_1_v();
	u32 dg_idx = 0;
	u32 fbp_local;
	u32 reg_offset = 0;
	u32 index = 0;
	u32 chiplet_reg_index;
	u32 dg_id;
	int ret = 0;
	(void) tsg;
	(void) is_ctx_resident;

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
				/* CCU prof registers are not context switched. */
				nvgpu_writel(g, reg_offset, val);
			}
		}
	}

	return ret;
}

#define NUM_OF_CCUPROF_PER_TPC		1U
#define NUM_OF_CCUPROF_PER_FBP		1U

int gb20c_perf_set_secure_config_for_ccu_prof(struct gk20a *g, u32 gr_instance_id,
					     u32 pma_channel_id, bool dg_enable,
					     bool is_ctx_resident, struct nvgpu_tsg *tsg)
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

	if (is_ctx_resident == true) {
		/* Wait for Pmm routers to be idle before we program secure config for DGs */
		err = g->ops.perf.wait_for_idle_pmm_routers(g, gr_instance_id,
							   pma_channel_id);
		if (err)
			return err;
	}

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
	err = gb20c_perf_set_gpc_ccuprof_registers(g, config_val, start_gpc, num_gpc,
						  ccuprof_gpc_stride, gpc_dg_map_mask,
						  is_ctx_resident, tsg);
	if (err != 0) {
		nvgpu_err(g, "CCUPROF secure config prog for GPC failed - %d", err);
		goto label_fail;
	}

	/* FBP */
	gb10b_update_fbp_dg_map_status_mask(num_fbp,
					   perf_fbp_streaming_dg_id_ccuprofltc0ltsp0_v(),
					   NUM_OF_CCUPROF_PER_FBP,
					   fbp_dg_map_mask,
					   fbp_dg_map_mask_size);
	ccuprof_fbp_stride = perf_ccuprof_fbp1_secure_config_r(0U) -
			     perf_ccuprof_fbp0_secure_config_r(0U);
	err = gb20c_perf_program_fbp_ccuprof_registers(g, config_val, start_fbp, num_fbp,
						      ccuprof_fbp_stride, fbp_dg_map_mask,
						      is_ctx_resident, tsg);
	if (err != 0) {
		nvgpu_err(g, "CCUPROF secure config prog for FBP failed - %d", err);
		goto label_fail;
	}

	for (i = 0; i < num_fbp; i++) {
		nvgpu_log(g, gpu_dbg_prof,
			  "gr_instance_id: %u, fbp_dg_map_mask[%u]: 0x%X",
			  gr_instance_id, i * dgmap_status_reg_count,
			  fbp_dg_map_mask[i * dgmap_status_reg_count]);
	}

	if (is_ctx_resident == true) {
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
	}

label_fail:
	nvgpu_kfree(g, fbp_dg_map_mask);
label_fbp_dg_map_mask:
	nvgpu_kfree(g, gpc_dg_map_mask);
label_gpc_dg_map_mask:
	return err;
}

u32 gb20c_perf_get_num_cwd_hems(struct gk20a *g)
{
	(void)g;

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

u32 gb20c_perf_get_ctx_switched_pma_channel_count(struct gk20a *g)
{
	return g->ops.perf.get_pma_channel_count(g);
}

u32 gb20c_perf_get_sysrouter_channel_stride(void)
{
	return perf_pmmsysrouter_channel_stride_v();
}

u32 gb20c_perf_get_pmasys_channel_stride(void)
{
	return perf_pmasys_channel_stride_v();
}

u32 gb20c_perf_get_pmasys_cmdslice_stride(void)
{
	return perf_pmasys_command_slice_stride_v();
}

u32 gb20c_perf_get_pmasys_fwd_channel_stride(void)
{
	return perf_pmasys_fwd_channel_stride_v();
}

// This is technically returning stride but all the older chips have wrongly
// named it offset.
u32 gb20c_perf_get_pmmfbprouter_per_chiplet_offset(void)
{
	return (perf_pmmfbprouter_extent_v() - perf_pmmfbprouter_base_v() + 0x1U);
}

void gb20c_perf_get_sys_perfmon_range(u32 *perfmon_start_idx,
					u32 *perfmon_end_idx)
{
	*perfmon_start_idx =  perf_sys_streaming_dg_id_cmc0_v();
	*perfmon_end_idx = perf_sys_streaming_dg_id_xbar0_v();
}

u32 gb20c_perf_get_sys_perfmon_stride(void)
{
	return perf_pmmsys_perdomain_offset_v();
}

void gb20c_perf_get_tpc_perfmon_range(u32 *perfmon_start_idx,
	u32 *perfmon_end_idx)
{
	*perfmon_start_idx =  perf_gpc_streaming_dg_id_gpctpca0_v();
	*perfmon_end_idx = perf_gpc_streaming_dg_id_gpctpcc5_v();
}

u32 gb20c_perf_get_max_num_gpc_perfmons(void)
{
	return perf_pmmgpc_control__size_1_v();
}

int gb20c_perf_supports_extended_b2cc(struct gk20a *g, struct nvgpu_tsg *tsg,
				     bool *is_ctx_resident)
{
	int ret;

	ret = nvgpu_gr_disable_ctxsw(g);
	if (ret != 0) {
		nvgpu_err(g, "Unable to stop GR ctxsw");
		return ret;
	}

	*is_ctx_resident = gk20a_is_tsg_ctx_resident(tsg);

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg, "is curr ctx resident = %d",
		*is_ctx_resident);

	return 0;
}

int gb20c_perf_write_hwpm_register(struct gk20a *g, u32 reg_offset, u32 val,
				struct nvgpu_tsg *tsg, bool is_ctx_resident, u32 mask)
{
	u32 num_wr_opr, num_rd_opr, num_opr;
	int err;
	struct nvgpu_dbg_reg_op ctx_ops = {
		.op = REGOP(WRITE_32),
		.type = REGOP(TYPE_GR_CTX),
		.offset = reg_offset,
		.value_lo = val,
		.and_n_mask_lo = mask,
	};

	num_wr_opr = 0x1U;
	num_rd_opr = 0x0U;
	num_opr = num_wr_opr + num_rd_opr;

	if (tsg != NULL) {
		err = gr_exec_ctx_ops(tsg, &ctx_ops, num_opr, num_wr_opr,
				     num_rd_opr, is_ctx_resident);
		if (err != 0) {
			nvgpu_err(g, "Write to 0x%08x failed ", reg_offset);
		}
	} else {
		nvgpu_assert(is_ctx_resident == true);
		nvgpu_writel(g, reg_offset, val);
		err = 0;
	}

	return err;
}

int gb20c_perf_read_hwpm_register(struct gk20a *g, u32 reg_offset, u32 *val,
				 struct nvgpu_tsg *tsg, bool is_ctx_resident, u32 mask)
{
	int ret;
	u32 num_wr_opr, num_rd_opr, num_opr;
	struct nvgpu_dbg_reg_op ctx_ops = {
		.op = REGOP(READ_32),
		.type = REGOP(TYPE_GR_CTX),
		.offset = reg_offset,
		.and_n_mask_lo = mask,
	};

	num_wr_opr = 0x0U;
	num_rd_opr = 0x1U;
	num_opr = num_wr_opr + num_rd_opr;

	if (tsg != NULL) {
		ret = gr_exec_ctx_ops(tsg, &ctx_ops, num_opr, num_wr_opr,
				     num_rd_opr, is_ctx_resident);
		if (ret != 0) {
			nvgpu_err(g, " Read from 0x%08x failed with error - %d ",
				 reg_offset, ret);
		}
		*val = ctx_ops.value_lo;
	} else {
		nvgpu_assert(is_ctx_resident == true);
		*val = nvgpu_readl(g, reg_offset);
		ret = 0;
	}

	return ret;
}

int gb20c_perf_reset_pm_trigger_masks(struct gk20a *g, u32 pma_channel_id,
				     u32 gr_instance_id, u32 reservation_id,
				     struct nvgpu_tsg *tsg, bool is_ctx_resident)
{
	u32 reg_offset = perf_pmasys_command_slice_trigger_mask_secure0_r(pma_channel_id);
	int err;
	u32 val = 0x0U;

	(void) gr_instance_id;
	(void) reservation_id;

	err = g->ops.perf.write_hwpm_register(g, reg_offset, val, tsg,
			is_ctx_resident, U32_MAX);
	if (err != 0) {
		nvgpu_err(g, " PM trigger mask reset failed ");
	}

	return err;
}

static int gb20c_perf_enable_pm_hes_trigger(struct gk20a *g, u32 pma_channel_id,
					   bool event, struct nvgpu_tsg *tsg,
					   bool is_ctx_resident)
{
	u32 reg_offset = perf_pmasys_command_slice_trigger_mask_secure0_r(pma_channel_id);
	u32 reg_val;
	int err;

	err = g->ops.perf.read_hwpm_register(g, reg_offset, &reg_val, tsg,
			is_ctx_resident, U32_MAX);
	if (err != 0) {
		nvgpu_err(g, "Reading PM trigger reg failed - %d", err);
		goto label_fail;
	}

	if (event) {
		reg_val |= BIT(perf_pmasys_engine_index_grevents_v());
	} else {
		reg_val |= BIT(perf_pmasys_engine_index_gr_v());
	}

	err = g->ops.perf.write_hwpm_register(g, reg_offset, reg_val, tsg, is_ctx_resident,
				event ? BIT(perf_pmasys_engine_index_grevents_v()) :
				BIT(perf_pmasys_engine_index_gr_v()));
	if (err != 0) {
		nvgpu_err(g, " Setting PM trigger reg failed with err - %d", err);
	}

label_fail:
	return err;
}

int gb20c_perf_enable_pm_trigger(struct gk20a *g, u32 gr_instance_id, u32 pma_channel_id,
				u32 reservation_id, struct nvgpu_tsg *tsg, bool is_ctx_resident)
{
	(void) reservation_id;
	(void) gr_instance_id;

	return gb20c_perf_enable_pm_hes_trigger(g, pma_channel_id, false, tsg, is_ctx_resident);
}

int gb20c_perf_enable_hes_event_trigger(struct gk20a *g, u32 gr_instance_id, u32 pma_channel_id,
				       struct nvgpu_tsg *tsg, bool is_ctx_resident)
{
	(void) gr_instance_id;

	return gb20c_perf_enable_pm_hes_trigger(g, pma_channel_id, true, tsg, is_ctx_resident);
}

int gb20c_perf_enable_pma_trigger(struct gk20a *g, u32 pma_channel_id, struct nvgpu_tsg *tsg,
				 bool is_ctx_resident)
{
	u32 reg_offset = perf_pmasys_command_slice_trigger_mask_secure0_r(pma_channel_id);
	u32 reg_val;
	int err;

	err = g->ops.perf.read_hwpm_register(g, reg_offset, &reg_val, tsg,
					    is_ctx_resident, U32_MAX);
	if (err != 0) {
		nvgpu_err(g, " Reading PMA trigger reg failed with err - %d", err);
		goto label_fail;
	}

	reg_val |= BIT(perf_pmasys_engine_index_pma_v());

	err = g->ops.perf.write_hwpm_register(g, reg_offset, reg_val, tsg,
					       is_ctx_resident,
					       BIT(perf_pmasys_engine_index_pma_v()));
	if (err != 0) {
		nvgpu_err(g, " Writing to PMA trigger reg failed with err - %d", err);
	}

label_fail:
	return err;
}

int gb20c_perf_update_get_put(struct gk20a *g, u32 pma_channel_id,
				u64 bytes_consumed, bool update_available_bytes, u64 *put_ptr,
				bool *overflowed, struct nvgpu_tsg *tsg, bool is_ctx_resident,
				u32 *ctx_bytes_available)
{
	u32 cblock_idx = PMA_CBLOCK_IDX(pma_channel_id);
	u32 channel_idx = PMA_CBLOCK_CH_IDX(pma_channel_id);
	u32 val, reg_offset, mem_bytes, temp_put_data;
	int err = 0;

	nvgpu_assert(pma_channel_id < g->ops.perf.get_pma_channel_count(g));

	nvgpu_log(g, gpu_dbg_prof, "is_ctx_resident - %u", is_ctx_resident);

	if (is_ctx_resident == true) {
		if (bytes_consumed != 0) {
			nvgpu_writel(g,
				perf_pmasys_channel_mem_bump_r(cblock_idx, channel_idx),
				(u32)bytes_consumed);
		}
		if (update_available_bytes == true) {
			val = nvgpu_readl(g,
				perf_pmasys_channel_control_user_r(cblock_idx, channel_idx));
			val = set_field(val,
				perf_pmasys_channel_control_user_update_bytes_m(),
				perf_pmasys_channel_control_user_update_bytes_doit_f());
			nvgpu_writel(g,
				perf_pmasys_channel_control_user_r(cblock_idx, channel_idx), val);
		}
	} else {
		/* Get mem bytes */
		reg_offset = perf_pmasys_channel_mem_bytes_r(cblock_idx, channel_idx);
		err = g->ops.perf.read_hwpm_register(g, reg_offset, &mem_bytes, tsg,
						    is_ctx_resident, U32_MAX);
		if (err != 0) {
			nvgpu_err(g, "Reading PMA SYS channel mem bytes failed with err - %d", err);
			goto label_fail;
		}
		nvgpu_log(g, gpu_dbg_prof, "Channel Mem Bytes - 0x%x", mem_bytes);
		/* Update mem bytes if required. */
		if (bytes_consumed != 0) {
			mem_bytes -= bytes_consumed;
			reg_offset = perf_pmasys_channel_mem_bytes_r(cblock_idx, channel_idx);
			err = g->ops.perf.write_hwpm_register(g, reg_offset, mem_bytes, tsg,
							       is_ctx_resident, U32_MAX);
			if (err != 0) {
				nvgpu_err(g, "Updating stream mem bytes failed with err - %d", err);
				goto label_fail;
			}
		}
		if (update_available_bytes == true) {
			*ctx_bytes_available = mem_bytes;
		}
	}

	if (put_ptr) {
		reg_offset = perf_pmasys_channel_mem_head_r(cblock_idx, channel_idx);
		err = g->ops.perf.read_hwpm_register(g, reg_offset, &temp_put_data, tsg,
						    is_ctx_resident, U32_MAX);
		if (err != 0) {
			nvgpu_err(g,
			"Reading from PMA SYS channel mem head reg failed with err - %d", err);
			goto label_fail;
		}
		*put_ptr = (u64)temp_put_data;
		nvgpu_log(g, gpu_dbg_prof, "Channel Mem Head - 0x%llx", *put_ptr);
	}

	if (overflowed) {
		err = g->ops.perf.get_membuf_overflow_status(g, pma_channel_id, overflowed,
							    tsg, is_ctx_resident);
		if (err != 0) {
			nvgpu_err(g, "Getting overflow status failed with err - %d", err);
		}
	}

label_fail:
	return err;
}

int gb20c_perf_get_membuf_overflow_status(struct gk20a *g, u32 pma_channel_id,
					bool *overflowed, struct nvgpu_tsg *tsg,
					bool is_ctx_resident)
{
	u32 cblock_idx = PMA_CBLOCK_IDX(pma_channel_id);
	u32 channel_idx = PMA_CBLOCK_CH_IDX(pma_channel_id);
	const u32 state = perf_pmasys_channel_status_membuf_status_overflowed_f();
	const u32 stream_en = perf_pmasys_channel_config_user_stream_enable_f();
	const u32 keep_latest = perf_pmasys_channel_config_user_keep_latest_enable_f();
	bool is_stream_enabled, is_keep_latest_enabled;
	u32 reg_offset, reg_val, mem_bytes;
	int err = 0;

	nvgpu_assert(pma_channel_id < g->ops.perf.get_pma_channel_count(g));

	if (is_ctx_resident == true) {
		reg_offset = perf_pmasys_channel_status_r(cblock_idx, channel_idx);
		reg_val = nvgpu_readl(g, reg_offset);
		*overflowed = (state == (reg_val & state));
	} else {
		/* Get mem bytes */
		reg_offset = perf_pmasys_channel_mem_bytes_r(cblock_idx, channel_idx);
		err = g->ops.perf.read_hwpm_register(g, reg_offset, &mem_bytes, tsg,
						    is_ctx_resident, U32_MAX);
		if (err != 0) {
			nvgpu_err(g,
			 "Reading PMA SYS channel mem bytes failed with err - %d", err);
			return err;
		}
		reg_offset = perf_pmasys_channel_config_user_r(cblock_idx, channel_idx);
		err = g->ops.perf.read_hwpm_register(g, reg_offset, &reg_val, tsg,
						    is_ctx_resident, U32_MAX);
		if (err != 0) {
			nvgpu_err(g,
			 "Reading PMA SYS channel config user failed with err - %d", err);
			return err;
		}
		is_stream_enabled =
			(stream_en == (reg_val & perf_pmasys_channel_config_user_stream_m()));
		is_keep_latest_enabled =
			(keep_latest == (reg_val & perf_pmasys_channel_config_user_keep_latest_m()));
		/*
		 * Check - 2 channels in a cblock share the perfbuf.
		 * Should prof.pma buffer size be used instead
		 */
		*overflowed =
			(mem_bytes >= (PERFBUF_PMA_BUF_MAX_SIZE/2U)) &&
			(!is_keep_latest_enabled) && (is_stream_enabled);
	}

	return 0;
}

int gb20c_perf_disable_ccuprof(struct gk20a *g,
	u32 gr_instance_id, bool is_ctxsw, u32 reservation_id)
{
	u32 i;
	u32 reg_offset, val;
	struct nvgpu_gr_config *gr_config = nvgpu_gr_get_config_ptr(g);
	u32 max_tpc_per_gpc = gr_config->max_tpc_per_gpc_count;
	(void) gr_instance_id;
	(void) reservation_id;

	/* CCU prof is currently not context switched */
	if (!is_ctxsw) {
		/* GPCS - Disable all GPC CCUPROFs by clearing ULC_PM bit in shared
		 * data_config register
		 */
		for (i = 0U; i < max_tpc_per_gpc; i++) {
			reg_offset = perf_ccuprof_gpcs_data_config_r(i);
			val = nvgpu_readl(g, reg_offset);
			val = set_field(val,
				perf_ccuprof_gpcs_data_config_ulc_pm_m(),
				perf_ccuprof_gpcs_data_config_ulc_pm_disable_f());
			nvgpu_writel(g, reg_offset, val);
		}

		/* FBPS - Disable all FBP CCUPROFs by clearing ULC_PM bit in shared
		 * data_config register
		 */
		reg_offset = perf_ccuprof_fbps_data_config_r(0U);
		val = nvgpu_readl(g, reg_offset);
		val = set_field(val,
			perf_ccuprof_fbps_data_config_ulc_pm_m(),
			perf_ccuprof_fbps_data_config_ulc_pm_disable_f());
		nvgpu_writel(g, reg_offset, val);
	}

	return 0;
}
