/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMU_PG_SW_GA10B_H
#define NVGPU_PMU_PG_SW_GA10B_H

#include <nvgpu/types.h>
#include <nvgpu/pmu/pmuif/pg.h>

struct gk20a;
struct pmu_pg_stats_data;

/*!
 * @brief Sub Features in LPWR Architecture
 *
 * =================================================================================================================
 * Bit Number |    Feature Name            |     Description                                                       |
 * =================================================================================================================
 *     0      |  IDLE_SNAP_DBG             | Sends idle snap notification to RM                                    |
 *     3      |  IDLE_THRESHOLD_CHECK      | Check the minimum value of idle threshold before updating the HW      |
 *     7      |  LPWR_LP_IDLE_SNAP_NOTIFY  | LPWR_LP task Based notification sending of Idle Sanp to RM            |
 *
 * =================================================================================================================
 *
 */

#define NV_PMU_LPWR_FSM_FEATURE_ID_IDLE_SNAP_DBG                BIT32(0)
#define NV_PMU_LPWR_FSM_FEATURE_ID_IDLE_THRESHOLD_CHECK         BIT32(3)
#define NV_PMU_LPWR_FSM_FEATURE_ID_LPWR_LP_IDLE_SNAP_NOTIFY     BIT32(7)

#define NVGPU_PMU_GR_FEATURE_MASK_LOGIC_PG                BIT32(1)
#define NVGPU_PMU_GR_FEATURE_MASK_SRAM_RPG                BIT32(2)
#define NVGPU_PMU_GR_FEATURE_MASK_PRIV_RING               BIT32(5)
#define NVGPU_PMU_GR_FEATURE_MASK_SAVE_GLOBAL_STATE       BIT32(7)
#define NVGPU_PMU_GR_FEATURE_MASK_RESET                   BIT32(8)
#define NVGPU_PMU_GR_FEATURE_MASK_HW_SEQUENCE             BIT32(9)
#define NVGPU_PMU_GR_FEATURE_MASK_GSP                     BIT32(10)
#define NVGPU_PMU_GR_FEATURE_MASK_SEC2                    BIT32(11)
#define NVGPU_PMU_GR_FEATURE_MASK_ECC_SCRUB_SRAM          BIT32(12)
#define NVGPU_PMU_GR_FEATURE_MASK_XVE                     BIT32(15)
#define NVGPU_PMU_GR_FEATURE_MASK_FG_RPPG_EXCLUSION       BIT32(20)
#define NVGPU_PMU_GR_FEATURE_MASK_HOST_PREEMPTION         BIT32(21)
#define NVGPU_PMU_GR_FEATURE_MASK_CENTRAL_PRIV_BLOCKER    BIT32(23)
#define NVGPU_PMU_GR_FEATURE_MASK_PREEMPTION_HOLDOFF      BIT32(24)
#define NVGPU_PMU_GR_FEATURE_MASK_FLCG_GPCCLK             BIT32(25)

#define NVGPU_PMU_GR_FEATURE_MASK_DEFAULT_GA10B               \
(                                                                     \
		NVGPU_PMU_GR_FEATURE_MASK_LOGIC_PG                     |\
		NVGPU_PMU_GR_FEATURE_MASK_SRAM_RPG                     |\
		NVGPU_PMU_GR_FEATURE_MASK_PRIV_RING                    |\
		NVGPU_PMU_GR_FEATURE_MASK_SAVE_GLOBAL_STATE            |\
		NVGPU_PMU_GR_FEATURE_MASK_RESET                        |\
		NVGPU_PMU_GR_FEATURE_MASK_HW_SEQUENCE                  |\
		NVGPU_PMU_GR_FEATURE_MASK_ECC_SCRUB_SRAM               |\
		NVGPU_PMU_GR_FEATURE_MASK_HOST_PREEMPTION              |\
		NVGPU_PMU_GR_FEATURE_MASK_PREEMPTION_HOLDOFF            \
)

/*
* Brief Identifier for each Lpwr Group Ctrl ids
*/
enum
{
	NV_PMU_LPWR_GRP_CTRL_ID_GR = 0,
	NV_PMU_LPWR_GRP_CTRL_ID_MS,
	NV_PMU_LPWR_GRP_CTRL_ID_EI,
	NV_PMU_LPWR_GRP_CTRL_ID_VID,
	NV_PMU_LPWR_GRP_CTRL_ID__COUNT,
};

/*
* Defines the structure that holds data used to execute PRE_INIT RPC.
*/
struct pmu_rpc_struct_lpwr_loading_pre_init
{
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] Lpwr group data
	 */
	u32 grp_ctrl_mask[NV_PMU_LPWR_GRP_CTRL_ID__COUNT];
	/*
	 * [IN] Mask of NV_PMU_SUBFEATURE_ID_ARCH_xyz
	 */
	u32 arch_sf_support_mask;
	/*
	 * [IN] Base sampling period for centralised LPWR callback
	 */
	u16 base_period_ms;
	/*
	 * [IN] Indicates if it is a no pstate vbios
	 */
	bool b_no_pstate_vbios;
	/*
	 * [NONE] Must be last field in RPC structure.
	 */
	u32 scratch[1];
};

/*
* Defines the structure that holds data used to execute POST_INIT RPC.
*/
struct pmu_rpc_struct_lpwr_loading_post_init
{
	/*
	 * [IN/OUT] Must be first field in RPC structure
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * Voltage rail data in LPWR
	 */
	struct pmu_pg_volt_rail pg_volt_rail[PG_VOLT_RAIL_IDX_MAX];
	/*!
	 * [IN] Support for IST clock gating
	 */
	bool ist_cg_support;
	/*!
	 * [IN] Support for GPC_CG Feature
	 */
	bool gpc_cg_supported;
	/*!
	 * [IN] Support for NVD_CG Feature
	 */
	bool nvd_cg_supported;
	/*!
	 * [IN] Support for SYSCLK_SD Feature
	 */
	bool sys_clksd_supported;
	/*
	 * Must be last field in RPC structure.
	 */
	u32 scratch[1];
 };

struct pmu_rpc_struct_lpwr_loading_pg_ctrl_init
{
	/*[IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;
	/*
	* [OUT] stats dmem offset
	*/
	u32 stats_dmem_offset;
	/*
	* [OUT] Engines hold off Mask
	*/
	u32 eng_hold_off_Mask[2];
	/*
	* [OUT] HW FSM index
	*/
	u8 hw_eng_idx;
	/*
	* [OUT] Indicates if wakeup reason type is cumulative or normal
	*/
	bool b_cumulative_wakeup_mask;
	/*
	* [IN/OUT] Sub-feature support mask
	*/
	u32 support_mask;
	/*
	* [IN] Controller ID - NV_PMU_PG_ELPG_ENGINE_ID_xyz
	*/
	u32 ctrl_id;
	/*
	* [NONE] Must be last field in RPC structure.
	*/
	u32 scratch[1];
};

/*
* Defines the structure that holds data used to execute PG_CTRL_ALLOW RPC.
*/
struct pmu_rpc_struct_lpwr_pg_ctrl_allow
{
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] Controller ID - NV_PMU_PG_ELPG_ENGINE_ID_xyz
	 */
	u32 ctrl_id;
	/*
	 * [NONE] Must be last field in RPC structure.
	 */
	u32 scratch[1];
};

/*
* Defines the structure that holds data used to execute PG_CTRL_DISALLOW RPC.
*/
struct pmu_rpc_struct_lpwr_pg_ctrl_disallow
{
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] Controller ID - NV_PMU_PG_ELPG_ENGINE_ID_xyz
	 */
	u32 ctrl_id;
	/*
	 * [NONE] Must be last field in RPC structure.
	 */
	u32 scratch[1];
};

/*
* Brief Structure defining PG Ctrl thresholds
*/
struct pg_ctrl_threshold
{
	/*
	 *Idle threshold. HW FSM raises entry interrupt after expiration
	 *of idle threshold.
	 */
	u32 idle;
	/*
	 * Post power up threshold. This helps to avoid immediate entry
	 * after exit. PPU threshold is used for HOST based wake-up.
	 */
	u32 ppu;
	/* Minimum value of Idle threshold supported */
	u32 min_idle;
	/* Maximum value of Idle threshold supported */
	u32 max_idle;
};

/*
* Defines the structure that holds data used to execute PG_CTRL_THRESHOLD_UPDATE RPC.
*/
struct pmu_rpc_struct_lpwr_pg_ctrl_threshold_update
{
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] Controller ID - NV_PMU_PG_ELPG_ENGINE_ID_xyz
	 */
	u32 ctrl_id;
	/*
	 * [IN] PgCtrl thresholds
	 */
	struct pg_ctrl_threshold threshold_cycles;
	/*
	 * [NONE] Must be last field in RPC structure.
	 */
	u32 scratch[1];
};

/*
* Defines the structure that holds data used to execute PG_CTRL_SFM_UPDATE RPC.
*/
struct pmu_rpc_struct_lpwr_pg_ctrl_sfm_update
{
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] Updated enabled mask - NV_PMU_PG_ELPG_ENGINE_ID_xyz
	*/
	u32 enabled_mask;
	/*
	 * [IN] Controller ID - NV_PMU_PG_ELPG_ENGINE_ID_xyz
	 */
	u8 ctrl_id;
	/*
	 * [NONE] Must be last field in RPC structure.
	 */
	u32 scratch[1];
};

/*
* Defines the structure that holds data used to execute PG_CTRL_BUF_LOAD RPC.
*/
struct pmu_rpc_struct_lpwr_loading_pg_ctrl_buf_load
{
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] DMA buffer descriptor
	 */
	struct flcn_mem_desc_v0 dma_desc;
	/*
	 * [IN] PgCtrl ID
	 */
	u8 ctrl_id;
	/*
	 * [IN] Engine Buffer Index
	 */
	u8 buf_idx;
	/*
	 * [NONE] Must be last field in RPC structure.
	 */
	u32 scratch[1];
};

/*
 * Brief Statistics structure for PG features
 */
struct pmu_pg_stats_v3
{
	/* Number of time PMU successfully engaged sleep state */
	u32 entry_count;
	/* Number of time PMU exit sleep state */
	u32 exit_count;
	/* Number of time PMU aborted in entry sequence */
	u32 abort_count;
	/* Number of time task thrashing/starvation detected by Task MGMT feature */
	u32 detection_count;
	/* Entry and exit latency of current sleep cycle */
	u32 entry_latency_us;
	u32 exit_latency_us;
	/* Rolling average entry and exit latencies */
	u32 entry_latency_avg_us;
	u32 exit_latency_avg_us;
	/* Max entry and exit latencies */
	u32 entry_latency_max_us;
	u32 exit_latency_max_us;
	/* Total time spent in sleep and non-sleep state */
	u32 total_sleep_time_us;
	u32 total_non_sleep_time_us;
	/* Wakeup Type - Saves events that caused a power-up. */
	u32 wake_up_events;
	/* Abort Reason - Saves reason that caused an abort */
	u32 abort_reason;
	u32 sw_disallow_reason_mask;
	u32 hw_disallow_reason_mask;
};

/*
 * Defines the structure that holds data used to execute PG_CTRL_STATS_GET RPC.
 */
struct pmu_rpc_struct_lpwr_pg_ctrl_stats_get {
	/*!
	 * Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*!
	 * PgCtrl statistics
	 */
	struct pmu_pg_stats_v3 stats;
	/*!
	 * Control ID
	 */
	u8 ctrl_id;
	/*!
	 * Must be last field in RPC structure.
	 * Used as variable size scrach space on
	 * RM managed DMEM heap for this RPC.
	 */
	u32 scratch[1];
};


/*
 * Defines the structure that holds data used to execute AP_INIT RPC.
 */
struct pmu_rpc_struct_lpwr_loading_ap_init {
	/*
	 * [IN/OUT] Must be first field in RPC structure
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [NONE] Must be last field in RPC structure.
	 * Used as variable size scrach space on RM managed DMEM heap
	 * for this RPC.
	 */
	u32 scratch[1];
};

/*
 * Defines the structure that holds data used to execute
 * AP_CTRL_INIT_AND_ENABLE RPC.
 */
struct pmu_rpc_struct_lpwr_loading_ap_ctrl_init_and_enable {
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [OUT] Address of the dmem for stats data
	 */
	u32 stats_dmem_offset;
	/*
	 * [IN] Minimum idle threshold in Us
	 */
	u32 min_idle_threshold_us;
	/*
	 * [IN] Maximum idle threshold in Us
	 */
	u32 max_idle_threshold_us;
	/*
	 * [IN] Break-even resident time for one cycle of parent feature
	 */
	u16 breakeven_resident_time_us;
	/*
	 * [IN] Maximum number of allowed power feature cycles per sample
	 */
	u16 max_cycles_per_sample;
	/*
	 * [IN] Minimum targeted residency
	 */
	u8 min_residency;
	/*
	 * [IN] AP_CTRL index
	 */
	u8 ctrl_id;
	/*
	 * [IN] Base multipler for centralised LPWR callback
	 */
	u8 base_multiplier;
	/*
	 * [IN] NV_TRUE if ApCtrl requires SW Histograms
	 */
	bool sw_hist_enabled;
	/*
	 * [NONE] Must be last field in RPC structure.
	 * Used as variable size scrach space on RM managed DMEM heap
	 * for this RPC.
	 */
	u32 scratch[1];
};

/*
 * Defines the structure that holds data used to execute AP_CTRL_ENABLE RPC.
 */
struct pmu_rpc_struct_lpwr_ap_ctrl_enable {
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] AP_CTRL index
	 */
	u8 ctrl_id;
	/*
	 * [NONE] Must be last field in RPC structure.
	 * Used as variable size scrach space on RM managed DMEM heap
	 * for this RPC.
	 */
	u32 scratch[1];
};

/*
 * Defines the structure that holds data used to execute
 * AP_CTRL_ENABLE RPC.
 */
struct pmu_rpc_struct_lpwr_ap_ctrl_disable {
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] AP_CTRL index
	 */
	u8 ctrl_id;
	/*
	 * [NONE] Must be last field in RPC structure.
	 * Used as variable size scrach space on RM managed DMEM heap
	 * for this RPC.
	 */
	u32 scratch[1];
};

void nvgpu_ga10b_pg_sw_init(struct gk20a *g, struct nvgpu_pmu_pg *pg);
u32 ga10b_pmu_pg_engines_list(struct gk20a *g);
int ga10b_pmu_pg_pre_init(struct gk20a *g, struct nvgpu_pmu *pmu);
int ga10b_pmu_pg_init(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int ga10b_pmu_pg_allow(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int ga10b_pmu_pg_disallow(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int ga10b_pmu_pg_threshold_update(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 pg_engine_id);
int ga10b_pmu_pg_sfm_update(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 pg_engine_id);
int ga10b_pmu_pg_post_init(struct gk20a *g, struct nvgpu_pmu *pmu);
int ga10b_pmu_pg_init_send(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 pg_engine_id);
void ga10b_pg_rpc_handler(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct nv_pmu_rpc_header *rpc, struct rpc_handler_payload *rpc_payload);
void ga10b_pg_loading_rpc_handler(struct gk20a *g,
	struct nv_pmu_rpc_header *rpc, struct rpc_handler_payload *rpc_payload);
int ga10b_pmu_pg_handle_async_cmd_resp(struct gk20a *g, u32 ctrl_id,
		u32 msg_id);
int ga10b_pmu_pg_process_pg_event(struct gk20a *g, void *pmu_rpc);
int ga10b_pmu_pg_aelpg_init(struct gk20a *g);
int ga10b_pmu_pg_aelpg_init_and_enable(struct gk20a *g, u8 ctrl_id);
int ga10b_pmu_pg_aelpg_disable(struct gk20a *g, u8 ctrl_id);
int ga10b_pmu_pg_aelpg_enable(struct gk20a *g, u8 ctrl_id);
int ga10b_pmu_elpg_statistics(struct gk20a *g, u32 pg_engine_id,
		struct pmu_pg_stats_data *pg_stat_data);
int ga10b_pmu_pg_handle_idle_snap_rpc(struct gk20a *g,
		struct pmu_nv_rpc_struct_lpwr_pg_idle_snap *idle_snap_rpc);
#endif /* NVGPU_PMU_PG_SW_GA10B_H */
