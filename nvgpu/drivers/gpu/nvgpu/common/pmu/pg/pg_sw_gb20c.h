/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_PG_SW_GB20C_H
#define NVGPU_PMU_PG_SW_GB20C_H

#include <nvgpu/types.h>

struct gk20a;

/*!
 * @brief Sub Features in Video PG
 *
 * =============================================================================
 * Bit Number |    Feature Name      |     Description                         |
 * =============================================================================
 *     0      |  NVDEC0              | NVDEC0 Support                          |
 *     4      |  NVENC0              | NVENC0 Support                          |
 *     8      |  OFA0                | OFA0 Support                            |
 *     13     |  PRIV_BLOCKER        | PRIV Blocker Sequence                   |
 *     14     |  HOST_PREEMPTION     | Host Preemption Sequence                |
 *     15     |  PRIV_RING           | Block/Unblock priv ring                 |
 *     16     |  FGPG_EXCLUSION      | FGPG Mutual Exclusion                   |
 *     17     |  RPG                 | Video RPG Support                       |
 *     18     |  STALL_REQ           | Video Engine Stall Request              |
 *     19     |  RISCV_BS            | Video Engine Riscv Bootstrap Support    |
 *     20     |  LOGIC_PG            | Video Engine Logic Power Gating Support |
 *     21     |  HW_SEQUENCER        | Video Engine LPWR HW Sequencer Support  |
 *     22     |  RESET               | Video Engine PG Reset Sequence Support  |
 *     23     |  RISCV_BS_LPSEC      | Video Engine Bootstrapping via LPSEC    |
 *     24     |  NVDCLK_BYPASS       | NVDCLK Bypass                           |
 */

#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_NVDEC0				BIT32(0)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_NVENC0				BIT32(4)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_OFA0				BIT32(8)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_PRIV_BLOCKER		BIT32(13)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_HOST_PREEMPTION		BIT32(14)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_PRIV_RING			BIT32(15)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_FGPG_EXCLUSION		BIT32(16)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RPG				    BIT32(17)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_STALL_REQ			BIT32(18)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RISCV_BS			BIT32(19)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_LOGIC_PG			BIT32(20)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_HW_SEQUENCER		BIT32(21)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RESET				BIT32(22)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RISCV_BS_LPSEC		BIT32(23)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_NVDCLK_BYPASS		BIT32(24)

/*!
 * Mask for all Video NVDEC PG sub-features
 */
#define NV_PMU_LPWR_FSM_CTRL_VID_NVDEC_PG_FEATURE_MASK_ALL       \
	(                                                            \
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_NVDEC0              |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_PRIV_BLOCKER        |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_HOST_PREEMPTION	    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_PRIV_RING		    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_FGPG_EXCLUSION	    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RPG				    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_LOGIC_PG		    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RESET			    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_HW_SEQUENCER	    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RISCV_BS		    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_STALL_REQ		    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RISCV_BS_LPSEC	    |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_NVDCLK_BYPASS        \
	)

/*!
 * Mask for all Video NVENC PG sub-features
 */
#define NV_PMU_LPWR_FSM_CTRL_VID_NVENC_PG_FEATURE_MASK_ALL       \
	(                                                            \
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_NVENC0              |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_PRIV_BLOCKER        |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_HOST_PREEMPTION     |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_PRIV_RING           |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_FGPG_EXCLUSION      |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RPG                 |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_LOGIC_PG            |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RESET               |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_HW_SEQUENCER        |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RISCV_BS            |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_STALL_REQ           |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RISCV_BS_LPSEC      |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_NVDCLK_BYPASS        \
	)

/*!
 * Mask for all Video OFA PG sub-features
 */
#define NV_PMU_LPWR_FSM_CTRL_VID_OFA_PG_FEATURE_MASK_ALL         \
	(                                                            \
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_OFA0                |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_PRIV_BLOCKER        |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_HOST_PREEMPTION     |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_PRIV_RING           |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_FGPG_EXCLUSION      |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RPG                 |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_LOGIC_PG            |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RESET               |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_HW_SEQUENCER        |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RISCV_BS            |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_STALL_REQ           |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_RISCV_BS_LPSEC      |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_VID_NVDCLK_BYPASS        \
	)

/*!
 * @brief Sub Features in GR-RG
 *
 * =====================================================================================
 * Bit Number |    Feature Name           |     Description                             |
 * =====================================================================================
 *     0      |  DEEP_OFF                 | Put VR in Deep Off mode                     |
 *     1      |  LOGIC_PG                 | Power-gate/ungate GR's LOGIC                |
 *     2      |  SRAM_RPG                 | Power-gate/ungate GR's SRAM                 |
 *     3      |  SRAM_RPPG                | Put SRAM in Retention mode                  |
 *     4      |  PSI                      | ELPG-GR coupled PSI                         |
 *     5      |  PRIV_RING                | Block/Unblock GPC priv ring                 |
 *     6      |  UNBIND                   | UnBind/Bind Graphics engine                 |
 *     7      |  SAVE_GLOBAL_STATE        | Save/Restore global state                   |
 *     8      |  RESET                    | Assert Engine Reset                         |
 *     9      |  HW_SEQUENCE              | HW power-down/up sequence                   |
 *     10     |  GSP                      | GSP support                                 |
 *     11     |  SEC2                     | SEC2 support                                |
 *     12     |  ECC_SCRUB_SRAM           | SRAM ECC Scrubbing support                  |
 *     13     |  RG_GPCCS_BS              | GPCCS bootstrapping as part of GPC-RG seq   |
 *     14     |  RG_PERF_CHANGE_SEQ       | Perf Sequencer used in GPC-RG seq           |
 *     15     |  XVE                      | XVE Priv Blocker                            |
 *     16     |  RG_GPCCS_BS_PMU          | GPCCS bootstrapping directly from PMU       |
 *     17     |  RG_TS_PD                 | TS_PD_READY pre-condition and control       |
 *     18     |  PERF_SEMAPHORE           | Take Perf Semaphore                         |
 *     19     |  RG_AUTO_WAKEUP           | Auto wakeup functionality                   |
 *     20     |  FGPG_EXCLUSION           | GR-FGPG Mutual Exclusion with GR Feature    |
 *     21     |  HOST_PREEMPTION          | Host Preemption Sequence                    |
 *     22     |  CLOCK_SLOWDOWN           | GPC clock Slowdown                          |
 *     23     |  CENTRAL_PRIV_BLOCKER     | Centralized PRIV Blocker                    |
 *     24     |  PREEMPTION_HOLDOFF       | Execute Pre-emption and Holdoff sequence    |
 *     25     |  FLCG_GPCCLK              | Gate/UnGate GPC CLk using FLCG              |
 *     26     |  CLK                      | Execute the CLK Init/De-init code           |
 *     27     |  RAM_REPAIR               | Execute the Ram Repair code                 |
 *     28     |  NVVDD_NEG_OFFSET         | Noise unaware NVVDD -Ve Vmin offset support |
 *     29     |  LPSEC_SAVE_RESTORE_STATE | Save/Restore the PL3 register via LPSEC     |
 *     30     |  DTD_PG_EXCLUSION         | DTD-PG Mutual Exclusion with GR Feature     |
 *     31     |  SHALLOW_OFF              | Put VR in Shallow Off mode                  |
 */

#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_DEEP_OFF				      BIT32(0)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_PRIV_RING				  BIT32(5)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_UNBIND					  BIT32(6)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_SAVE_GLOBAL_STATE		  BIT32(7)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RESET					  BIT32(8)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_HW_SEQUENCE			      BIT32(9)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_GSP					      BIT32(10)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_GPCCS_BS				  BIT32(13)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_PERF_CHANGE_SEQ		  BIT32(14)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_XVE						  BIT32(15)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_GPCCS_BS_PMU			  BIT32(16)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_TS_PD					  BIT32(17)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_PERF_SEMAPHORE			  BIT32(18)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_AUTO_WAKEUP			  BIT32(19)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_HOST_PREEMPTION			  BIT32(21)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_CENTRAL_PRIV_BLOCKER		  BIT32(23)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_PREEMPTION_HOLDOFF		  BIT32(24)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_CLK						  BIT32(26)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RAM_REPAIR				  BIT32(27)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_LPSEC_SAVE_RESTORE_STATE	  BIT32(29)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_DTD_PG_EXCLUSION			  BIT32(30)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_SHALLOW_OFF				  BIT32(31)

/*!
 * @brief Mask for all GR-RG sub-features
 *
 *  NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_AUTO_WAKEUP is not included in
 *  this mask because GPC-RG is only applicable to Sentry mode, where
 *  RG_AUTO_WAKEUP is not expected
 *
 */
#define NV_PMU_LPWR_FSM_CTRL_GR_RG_FEATURE_MASK_ALL                    \
	(                                                                  \
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_HOST_PREEMPTION            |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_PRIV_RING                  |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_UNBIND                     |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_SAVE_GLOBAL_STATE          |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RESET                      |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_HW_SEQUENCE                |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_GSP                        |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_GPCCS_BS                |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_PERF_CHANGE_SEQ         |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_XVE                        |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_GPCCS_BS_PMU            |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RG_TS_PD                   |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_PERF_SEMAPHORE             |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_CENTRAL_PRIV_BLOCKER       |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_PREEMPTION_HOLDOFF         |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_CLK                        |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_RAM_REPAIR                 |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_LPSEC_SAVE_RESTORE_STATE   |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_DTD_PG_EXCLUSION           |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_DEEP_OFF                   |\
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_GR_SHALLOW_OFF                 \
	)

u32 gb20c_pmu_pg_engines_list(struct gk20a *g);
int gb20c_pmu_pg_pre_init(struct gk20a *g, struct nvgpu_pmu *pmu);
int gb20c_pmu_pg_init(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int gb20c_pmu_pg_threshold_update(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 pg_engine_id);
int gb20c_pmu_pg_sfm_update(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id, bool enable);
void nvgpu_gb20c_pg_sw_init(struct gk20a *g, struct nvgpu_pmu_pg *pg);
int gb20c_pmu_pg_fgpg_init(struct gk20a *g, struct nvgpu_pmu *pmu);
int gb20c_pmu_pg_cg_elcg_state_change(struct gk20a *g,
		struct nvgpu_pmu *pmu, u32 elcg_engine_status[]);

#endif /* NVGPU_PMU_PG_SW_GB20C_H */
