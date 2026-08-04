// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/class.h>
#include <nvgpu/nvgpu_gb10b_class.h>
#include <nvgpu/nvgpu_gb20c_class.h>

#include <nvgpu/hw/gb20c/hw_proj_gb20c.h>

#include "hal_gb20c_litter.h"

u32 gb20c_get_litter_value(struct gk20a *g, int value)
{
	u32 ret = 0;

	switch (value) {
	case GPU_LIT_NUM_GPCS:
		ret = proj_scal_litter_num_gpcs_v();
		break;
	case GPU_LIT_NUM_PES_PER_GPC:
		ret = proj_scal_litter_num_pes_per_gpc_v();
		break;
	case GPU_LIT_NUM_ZCULL_BANKS:
		ret = proj_scal_litter_num_zcull_banks_v();
		break;
	case GPU_LIT_NUM_TPC_PER_GPC:
		ret = proj_scal_litter_num_tpc_per_gpc_v();
		break;
	case GPU_LIT_NUM_SM_PER_TPC:
		ret = proj_scal_litter_num_sm_per_tpc_v();
		break;
	case GPU_LIT_NUM_FBPS:
		ret = proj_scal_litter_num_fbps_v();
		break;
	case GPU_LIT_GPC_BASE:
		ret = proj_gpc_base_v();
		break;
	case GPU_LIT_GPC_STRIDE:
		ret = proj_gpc_stride_v();
		break;
	case GPU_LIT_GPC_SHARED_BASE:
		ret = proj_gpc_shared_base_v();
		break;
	case GPU_LIT_GPC_ADDR_WIDTH:
		ret = proj_gpc_addr_width_v();
		break;
	case GPU_LIT_TPC_ADDR_WIDTH:
		ret = proj_tpc_addr_width_v();
		break;
	case GPU_LIT_TPC_IN_GPC_BASE:
		ret = proj_tpc_in_gpc_base_v();
		break;
	case GPU_LIT_TPC_IN_GPC_STRIDE:
		ret = proj_tpc_in_gpc_stride_v();
		break;
	case GPU_LIT_TPC_IN_GPC_SHARED_BASE:
		ret = proj_tpc_in_gpc_shared_base_v();
		break;
	case GPU_LIT_PPC_IN_GPC_BASE:
		ret = proj_ppc_in_gpc_base_v();
		break;
	case GPU_LIT_PPC_IN_GPC_SHARED_BASE:
		ret = proj_ppc_in_gpc_shared_base_v();
		break;
	case GPU_LIT_PPC_IN_GPC_STRIDE:
		ret = proj_ppc_in_gpc_stride_v();
		break;
	case GPU_LIT_ROP_BASE:
		ret = proj_rop_base_v();
		break;
	case GPU_LIT_ROP_STRIDE:
		ret = proj_rop_stride_v();
		break;
	case GPU_LIT_ROP_SHARED_BASE:
		ret = proj_rop_shared_base_v();
		break;
	case GPU_LIT_HOST_NUM_ENGINES:
		ret = proj_host_num_engines_v();
		break;
	case GPU_LIT_HOST_NUM_PBDMA:
		ret = proj_host_num_pbdma_v();
		break;
	case GPU_LIT_LTC_STRIDE:
		ret = proj_ltc_stride_v();
		break;
	case GPU_LIT_LTS_STRIDE:
		ret = proj_lts_stride_v();
		break;
	case GPU_LIT_SM_PRI_STRIDE:
		ret = proj_sm_stride_v();
		break;
	case GPU_LIT_SMPC_PRI_BASE:
		ret = proj_smpc_base_v();
		break;
	case GPU_LIT_SMPC_PRI_SHARED_BASE:
		ret = proj_smpc_shared_base_v();
		break;
	case GPU_LIT_SMPC_PRI_UNIQUE_BASE:
		ret = proj_smpc_unique_base_v();
		break;
	case GPU_LIT_SMPC_PRI_STRIDE:
		ret = proj_smpc_stride_v();
		break;
	case GPU_LIT_SM_UNIQUE_BASE:
		ret = proj_sm_unique_base_v();
		break;
	case GPU_LIT_SM_SHARED_BASE:
		ret = proj_sm_shared_base_v();
		break;
	case GPU_LIT_NUM_FBPAS:
		ret = proj_scal_litter_num_fbpas_v();
		break;
	/* Hardcode FBPA values other than NUM_FBPAS to 0. */
	case GPU_LIT_FBPA_STRIDE:
	case GPU_LIT_FBPA_BASE:
	case GPU_LIT_FBPA_SHARED_BASE:
		ret = 0;
		break;
#ifdef CONFIG_NVGPU_GRAPHICS
	/* Exposed to userspace only. */
	case GPU_LIT_TWOD_CLASS:
		ret = FERMI_TWOD_A;
		break;
	/* Exposed to userspace only. */
	case GPU_LIT_THREED_CLASS:
		ret = BLACKWELL_B;
		break;
#endif
	/* Exposed to userspace only. */
	case GPU_LIT_COMPUTE_CLASS:
		ret = BLACKWELL_COMPUTE_B;
		break;
	/* Exposed to userspace. Used by RAMFC for PBDMA signature. */
	case GPU_LIT_GPFIFO_CLASS:
		ret = BLACKWELL_CHANNEL_GPFIFO_B;
		break;
	/* Exposed to userspace only. */
	case GPU_LIT_I2M_CLASS:
		ret = BLACKWELL_INLINE_TO_MEMORY_A;
		break;
	/* Exposed to userspace. Used by CE engine. */
	case GPU_LIT_DMA_COPY_CLASS:
		ret = BLACKWELL_DMA_COPY_B;
		break;
	case GPU_LIT_NVENC_CLASS:
		ret = NVD1B7_VIDEO_ENCODER;
		break;
	case GPU_LIT_OFA_CLASS:
		ret = NVD1FA_VIDEO_OFA;
		break;
	case GPU_LIT_NVDEC_CLASS:
		ret = NVD1B0_VIDEO_DECODER;
		break;
	case GPU_LIT_GPC_PRIV_STRIDE:
		ret = proj_gpc_priv_stride_v();
		break;
	case GPU_LIT_NUM_LTC_LTS_SETS:
		ret = proj_scal_litter_num_ltc_lts_sets_v();
		break;
	case GPU_LIT_NUM_LTC_LTS_WAYS:
		ret = proj_scal_litter_num_ltc_lts_ways_v();
		break;
	/*
	 * The perfmon start, count for various chiplets are taken
	 * from the PM programming guide.
	 * TODO: Revisit the hard-coded numbers w.r.t. GB10B.
	 */
#ifdef CONFIG_NVGPU_DEBUGGER
	case GPU_LIT_PERFMON_PMMGPCTPCA_DOMAIN_START:
		ret = 2;
		break;
	case GPU_LIT_PERFMON_PMMGPCTPCB_DOMAIN_START:
		ret = 6;
		break;
	case GPU_LIT_PERFMON_PMMGPCTPC_DOMAIN_COUNT:
		ret = 4;
		break;
	case GPU_LIT_PERFMON_PMMFBP_LTC_DOMAIN_START:
		ret = 1;
		break;
	case GPU_LIT_PERFMON_PMMFBP_LTC_DOMAIN_COUNT:
		ret = 4;
		break;
	case GPU_LIT_PERFMON_PMMGPC_ROP_DOMAIN_START:
		ret = 14;
		break;
	case GPU_LIT_PERFMON_PMMGPC_ROP_DOMAIN_COUNT:
		ret = 2;
		break;
#endif
	case GPU_LIT_ROP_IN_GPC_BASE:
		ret = proj_rop_in_gpc_base_v();
		break;
	case GPU_LIT_ROP_IN_GPC_SHARED_BASE:
		ret = proj_rop_in_gpc_shared_base_v();
		break;
	case GPU_LIT_ROP_IN_GPC_PRI_SHARED_IDX:
		ret = proj_rop_in_gpc_pri_shared_index_v();
		break;
	case GPU_LIT_ROP_IN_GPC_STRIDE:
		ret = proj_rop_in_gpc_stride_v();
		break;
	case GPU_LIT_MAX_RUNLISTS_SUPPORTED:
		/*
		 * Per dev_runlist_expanded.ref, GB10B supports 12 runlists.
		 * TODO: Revisit the hard-coded numbers w.r.t. GB10B.
		 */
		ret = 12;
		break;
	case GPU_LIT_NUM_SINGLETON_GPCS:
		ret = proj_litter_num_singleton_gpcs_v();
		break;
	case GPU_LIT_NUM_CPC_PER_GPC:
		ret = proj_scal_litter_num_cpc_per_gpc_v();
		break;
	case GPU_LIT_LRC_STRIDE:
		ret = proj_lrc_stride_v();
		break;
	case GPU_LIT_LRCC_STRIDE:
		ret = proj_lrcc_stride_v();
		break;
	default:
		nvgpu_err(g, "Missing definition %d", value);
		BUG();
		break;
	}

	return ret;
}
