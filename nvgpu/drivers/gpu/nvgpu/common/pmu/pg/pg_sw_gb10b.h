/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_PG_SW_GB10B_H
#define NVGPU_PMU_PG_SW_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/pmu/pmuif/pg.h>

struct gk20a;

/*
* Mask of all GR sub-features except GPCCLK FLCG
*/
#define NVGPU_PMU_GR_FEATURE_MASK_DEFAULT_GB10B               \
(                                                                     \
        NVGPU_PMU_GR_FEATURE_MASK_LOGIC_PG                     |\
        NVGPU_PMU_GR_FEATURE_MASK_SRAM_RPG                     |\
        NVGPU_PMU_GR_FEATURE_MASK_PRIV_RING                    |\
        NVGPU_PMU_GR_FEATURE_MASK_SAVE_GLOBAL_STATE            |\
        NVGPU_PMU_GR_FEATURE_MASK_RESET                        |\
        NVGPU_PMU_GR_FEATURE_MASK_HW_SEQUENCE                  |\
        NVGPU_PMU_GR_FEATURE_MASK_GSP                          |\
        NVGPU_PMU_GR_FEATURE_MASK_SEC2                         |\
        NVGPU_PMU_GR_FEATURE_MASK_ECC_SCRUB_SRAM               |\
        NVGPU_PMU_GR_FEATURE_MASK_XVE                          |\
        NVGPU_PMU_GR_FEATURE_MASK_FG_RPPG_EXCLUSION            |\
        NVGPU_PMU_GR_FEATURE_MASK_HOST_PREEMPTION              |\
        NVGPU_PMU_GR_FEATURE_MASK_CENTRAL_PRIV_BLOCKER         |\
        NVGPU_PMU_GR_FEATURE_MASK_PREEMPTION_HOLDOFF            \
)

#define NV_PMU_SUB_FEATURE_SUPPORT_GPCCLK_FLCG_MASK(enable)   \
        (enable ? (NVGPU_PMU_GR_FEATURE_MASK_DEFAULT_GB10B |          \
                NVGPU_PMU_GR_FEATURE_MASK_FLCG_GPCCLK) :  \
                        NVGPU_PMU_GR_FEATURE_MASK_DEFAULT_GB10B)

/*!
 * @brief Sub Features in MS
 *
 * =================================================================================
 * Bit Number |    Feature Name      |     Description                             |
 * =================================================================================
 *     0      | CG                   | GPC/LTC/XBAR clock gating                   |
 *     6      | HSHUB                | NvLink Support in HSHUB                     |
 *     7      | SEC2                 | SEC2 RTOS support                           |
 *     8      | RPPG                 | MSCG coupled RPPG feature                   |
 *     15     | CG_STOP_CLOCK        | MSCG Clock Gating using Stop Clock Feature  |
 *     16     | GSP                  | GSP support                                 |
 *     17     | PERF                 | PERF Support                                |
 *     25     | CENTRAL_PRIV_BLOCKER | Centralized PRIV Blocker                    |
 *     27     | GSPLITE              | GSPLITE Support                             |
 */

#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_CG				BIT32(0)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_HSHUB			BIT32(6)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_SEC2				BIT32(7)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_RPPG				BIT32(8)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_CG_STOP_CLOCK		BIT32(15)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_GSP				BIT32(16)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_PERF				BIT32(17)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_CENTRAL_PRIV_BLOCKER		BIT32(25)
#define NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_GSPLITE			BIT32(27)

/*
 * Bug 4726545 From GH100 Peregrine has implemented a design where
 * FALCON_ENGCTL_SET_STALLREQ is extended to stall not only FBIF, but
 * also other peregrine initiator interface, including PRI, SECBUS, GIN, P2P.
 * But FALCON_ENGCTL_STALLREQ_MODE is not used by PRI, SECBUS, GIN, P2P interface.
 * This results in these interfaces are not recovered after STALLREQ clear.
 * RISCV can not access Global IO, SECHUB.
 * Thus, GSP is unable to process ACR commands after GR recovery (with MS_LTC enabled)
 * So remove GSP, SEC2 and GSPLITE from MS_LTC SFM to keep these blockers disabled for MS_LTC
 */
#define NV_PMU_LPWR_FSM_CTRL_MS_LTC_FEATURE_MASK_ALL                       \
	(                                                                 \
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_CG                   | \
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_CG_STOP_CLOCK        | \
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_HSHUB                | \
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_PERF                 | \
		NV_PMU_LPWR_FSM_CTRL_FEATURE_ID_MS_CENTRAL_PRIV_BLOCKER   \
	)

/* Priv blocker Ids */
enum
{
	NV_PMU_PRIV_BLOCKER_ID_SEC2 = 0,
	NV_PMU_PRIV_BLOCKER_ID_GSP     ,
	NV_PMU_PRIV_BLOCKER_ID_XVE     ,
	NV_PMU_PRIV_BLOCKER_ID_CENTRAL ,
	NV_PMU_PRIV_BLOCKER_ID_MAX     ,
};

/*
* Defines the structure that holds data used to execute
* LPWR_LOADIN_PRIV_BLOCKER_INIT RPC.
*/
struct pmu_rpc_struct_lpwr_loading_priv_blocker_init
{
	/*
	 * [IN/OUT] Must be first field in RPC structure
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] Mask for the Priv blockers to be Initialized
	 */
	u32 blocker_support_mask;
	/*
	 * Must be last field in RPC structure.
	 */
	u32 scratch[1];
};

/*
 * Defines the structure that holds data used
 * to execute CG_ELCG_STAT_GET RPC.
 */
struct pmu_rpc_struct_lpwr_cg_elcg_stat_get
{
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] ELCG CtrlId
	 */
	u32 ctrl_id;
	/*
	 * [IN] Counter/resident time mode.
	 */
	u32 mode;
	/*
	 * [OUT] ELCG Entry Count/Resident Time
	 */
	u32 val;
	/*
	 * [OUT] ELCG delta sleep Time
	 */
	u32 delta_sleep_time_us;
	/*
	 * [OUT] Delta Wall Time
	 */
	u32 delta_wall_time_us;
	/*
	 * [NONE] Must be last field in RPC structure.
	 * Used as variable size scrach space on
	 * RM managed DMEM heap for this RPC.
	 */
	u32 scratch[1];
};

/*
 * Defines the structure that holds data used
 * to execute CG_ELCG_MON_START RPC.
 */
struct pmu_rpc_struct_lpwr_cg_elcg_mon_start
{
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] ELCG CtrlId
	 */
	u32 ctrl_id;
	/*
	 * [IN] Start/Stop ELCG Monitor Counter
	 */
	bool start;
	/*
	 * [NONE] Must be last field in RPC structure.
	 * Used as variable size scrach space on
	 * RM managed DMEM heap for this RPC.
	 */
	u32 scratch[1];
};

/*
 * Defines the structure that holds data used to
 * execute CG_ELCG_MON_CONFIG RPC.
 */
struct pmu_rpc_struct_lpwr_cg_elcg_mon_config
{
	/*
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*
	 * [IN] Counter/resident time mode.
	 */
	 u32 mode;
	/*
	 * [NONE] Must be last field in RPC structure.
	 * Used as variable size scrach space on
	 * RM managed DMEM heap for this RPC.
	 */
	u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to
 * execute SEQ_FGPG_INIT RPC.
 */
struct pmu_rpc_struct_lpwr_loading_seq_fgpg_init
{
	/*!
	* [IN/OUT] Must be first field in RPC structure.
	*/
	struct nv_pmu_rpc_header hdr;
	/*!
	* [IN] Thermal Monitor Phys Index to be used
	*/
	u8 phys_idx;
	/*!
	* [IN] Support for FGPG Feature
	*/
	bool fgpg_supported;
	/*!
	* [OUT] Support Mask for FGPG feature
	*/
	u32 support_mask;
	/*!
	* [NONE] Must be last field in RPC structure.
	* Used as variable size scrach space on RM managed
	* DMEM heap for this RPC.
	*/
	u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to execute
 * SEQ_FGPG_STATE_CHANGE RPC.
 */
struct pmu_rpc_struct_lpwr_seq_fgpg_state_change
{
	/*!
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*!
	 * [IN] FGPG ctrlId
	 */
	u8 ctrl_id;
	/*!
	 * [IN] Enable/disable FGPG
	 */
	bool enable;
	/*!
	 * [NONE] Must be last field in RPC structure.
	 * Used as variable size scrach space on RM managed
	 * DMEM heap for this RPC.
	 */
	u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to execute
 * SEQ_FGPG_THRESHOLD_UPDATE RPC.
 */
struct pmu_rpc_struct_lpwr_seq_fgpg_threshold_update
{
	/*!
	* [IN/OUT] Must be first field in RPC structure.
	*/
	struct nv_pmu_rpc_header hdr;
	/*!
	* [IN] FGPG entry Threshold
	*/
	u32 entry_threshold_us;
	/*!
	* [IN] FGPG Min Resident Time
	*/
	u32 min_resident_time_us;
	/*!
	* [IN] FGPG ctrlId
	*/
	u8 ctrl_id;
	/*!
	* [NONE] Must be last field in RPC structure.
	* Used as variable size scrach space on RM managed
	* DMEM heap for this RPC.
	*/
	u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to execute CG_FLCG_INIT RPC.
 */
struct pmu_rpc_struct_lpwr_loading_cg_flcg_init
{
	/*!
	* [IN/OUT] Must be first field in RPC structure.
	*/
	struct nv_pmu_rpc_header hdr;
	/*!
	* [IN] FLCG Suppport mask.
	*/
	u32 support_mask;
	/*!
	* [NONE] Must be last field in RPC structure.
	* Used as variable size scrach space on RM managed DMEM heap for this RPC.
	*/
	u32 scratch[1];
};

/*
 * Defines the structure that holds data used to execute
 * CG_FLCG_CTRL_ENABLE RPC.
 */
struct pmu_rpc_struct_lpwr_cg_flcg_ctrl_enable
{
	/*!
	* [IN/OUT] Must be first field in RPC structure.
	*/
	struct nv_pmu_rpc_header hdr;
	/*!
	* [IN] FLCG CtrlId
	*/
	u8 ctrl_id;
	/*!
	* [IN] Enable/disable FLCG Control.
	*/
	bool enable;
	/*!
	* [NONE] Must be last field in RPC structure.
	* Used as variable size scrach space on RM managed DMEM heap for this RPC.
	*/
	u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to execute CG_FLCG_CTRL_THRESHOLD_UPDATE RPC.
 */
struct pmu_rpc_struct_lpwr_cg_flcg_ctrl_threshold_update
{
	/*!
	* [IN/OUT] Must be first field in RPC structure.
	*/
	struct nv_pmu_rpc_header hdr;
	/*!
	* [IN] FLCG ctrl_id
	*/
	u8 ctrl_id;
	/*!
	* [IN] Threshold Type.
	*/
	u16 threshold_type;
	/*!
	* [IN] Threshold in Us.
	*/
	u16 threshold_ns;
	/*!
	* [NONE] Must be last field in RPC structure.
	* Used as variable size scrach space on RM managed DMEM heap for this RPC.
	*/
	u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to execute MON_START RPC.
 */
struct pmu_rpc_struct_lpwr_lp_mon_start
{
	/*!
	* [IN/OUT] Must be first field in RPC structure.
	*/
	struct nv_pmu_rpc_header hdr;
	/*!
	* [IN] Index of LPWR_MON we want to start/stop.
	*/
	u32 lpwr_mon_index;
	/*!
	* [IN] Start/Stop LPWR_Monitor Counter.
	*/
	bool start;
	/*!
	* [NONE] Must be last field in RPC structure.
	* Used as variable size scrach space on RM managed DMEM heap for this RPC.
	*/
	u32 scratch[1];
};


/*!
 * Defines the structure that holds data used to execute MON_STAT_GET RPC.
 */
struct pmu_rpc_struct_lpwr_lp_mon_stat_get
{
	/*!
	* [IN/OUT] Must be first field in RPC structure.
	*/
	struct nv_pmu_rpc_header hdr;
	/*!
	* [IN] Index of LPWR_MON we want to get stat.
	*/
	u32 lpwr_mon_index;
	/*!
	* [OUT] Feature Entry Count.
	*/
	u32 entry_count;
	/*!
	* [OUT] Feature delta sleep Time.
	*/
	u32 delta_sleep_time_tick;
	/*!
	* [OUT] Delta Wall Time
	*/
	u32 delta_wall_time_tick;
	/*!
	* [NONE] Must be last field in RPC structure.
	* Used as variable size scrach space on RM managed DMEM heap for this RPC.
	*/
	u32 scratch[1];
};

/*
 * Defines the structure that holds data used to execute
 * CG_ELCG_STATE_CHANGE RPC.
 */
struct pmu_rpc_struct_lpwr_cg_elcg_state_change {
	/*!
	 * [IN/OUT] Must be first field in RPC structure.
	 */
	struct nv_pmu_rpc_header hdr;
	/*!
	 * [IN] List of enable requests
	 */
	bool enable_req[CG_ELCG_CTRL_ENGINE_COUNT];
	/*!
	 * [IN] List of disable requests
	 */
	bool disable_req[CG_ELCG_CTRL_ENGINE_COUNT];
	/*!
	 * [NONE] Must be last field in RPC structure.
	 * Used as variable size scrach space on RM managed DMEM heap for this RPC.
	 */
	u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to execute CSD_INIT RPC.
 */
struct pmu_rpc_struct_lpwr_csd_init {
    /*!
     * [IN/OUT] Must be first field in RPC structure.
     */
    struct nv_pmu_rpc_header hdr;
    /*!
     * [IN] LPWR_CSD Support mask.
     */
    u32 support_mask;
    /*!
     * [NONE] Must be last field in RPC structure.
     * Used as variable size scrach space on RM managed DMEM heap for this RPC.
     */
    u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to execute CSD_CTRL_ENABLE RPC.
 */
struct pmu_rpc_struct_lpwr_csd_ctrl_enable {
    /*!
     * [IN/OUT] Must be first field in RPC structure.
     */
    struct nv_pmu_rpc_header hdr;
    /*!
     * [IN] LPWR_CSD CtrlId
     */
    u8 ctrl_id;
    /*!
     * [IN] Enable/disable LPWR_CSD Control.
     */
    bool enable;
    /*!
     * [NONE] Must be last field in RPC structure.
     * Used as variable size scrach space on RM managed DMEM heap for this RPC.
     */
    u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to execute
 * CSD_CTRL_THRESHOLD_UPDATE RPC.
 */
struct pmu_rpc_struct_lpwr_csd_ctrl_threshold_update {
    /*!
     * [IN/OUT] Must be first field in RPC structure.
     */
    struct nv_pmu_rpc_header hdr;
    /*!
     * [IN] LPWR_CSD CtrlId
     */
    u8 ctrl_id;
    /*!
     * [IN] Threshold in Us.
     */
    u32 threshold_us;
    /*!
     * [NONE] Must be last field in RPC structure.
     * Used as variable size scrach space on RM managed DMEM heap for this RPC.
     */
    u32 scratch[1];
};

/*!
 * Defines the structure that holds data used to execute CSD_STAT_GET RPC.
 */
struct pmu_rpc_struct_lpwr_lp_csd_stat_get {
    /*!
     * [IN/OUT] Must be first field in RPC structure.
     */
    struct nv_pmu_rpc_header hdr;
    /*!
     * [IN] LPWR_CSD CtrlId
     */
    u8 ctrl_id;
    /*!
     * [OUT] LPWR_CSD Entry Count.
     */
    u32 entry_count;
    /*!
     * [OUT] LPWR_CSD delta sleep Time
     */
    u32 delta_sleep_time_us;
    /*!
     * [OUT] Delta Wall Time
     */
    u32 delta_wall_time_us;
    /*!
     * [NONE] Must be last field in RPC structure.
     * Used as variable size scrach space on RM managed DMEM heap for this RPC.
     */
    u32 scratch[1];
};

void nvgpu_gb10b_pg_sw_init(struct gk20a *g, struct nvgpu_pmu_pg *pg);
u32 gb10b_pmu_get_pg_buf_gpu_va(struct gk20a *g, struct nvgpu_pmu *pmu,
		u32 buf_id);
u32 gb10b_pmu_get_pg_buf_gpu_va_hi(struct gk20a *g, struct nvgpu_pmu *pmu,
		u32 buf_id);
u32 gb10b_pmu_pg_engines_list(struct gk20a *g);
int gb10b_pmu_pg_pre_init(struct gk20a *g, struct nvgpu_pmu *pmu);
int gb10b_pmu_pg_init(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int gb10b_pmu_pg_threshold_update(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 pg_engine_id);
int gb10b_pmu_pg_sfm_update(struct gk20a *g, struct nvgpu_pmu *pmu,
    u8 pg_engine_id, bool enable);
int gb10b_pmu_pg_post_init(struct gk20a *g, struct nvgpu_pmu *pmu);
int gb10b_pmu_pg_init_send(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 pg_engine_id);
int gb10b_pmu_pg_fgpg_init(struct gk20a *g, struct nvgpu_pmu *pmu);
int gb10b_pmu_pg_fgpg_state_change(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 fgpg_ctrl_id, bool enable);
int gb10b_pmu_pg_fgpg_threshold_update(struct gk20a *g,
			struct nvgpu_pmu *pmu, struct nvgpu_pmu_fgpg_ctrl *fgpg,
			u8 ctrl_id);
int gb10b_pmu_pg_priv_blocker_init(struct gk20a *g,	struct nvgpu_pmu *pmu);
int gb10b_pmu_pg_cg_elcg_get_entry_count(struct gk20a *g,
	       struct nvgpu_pmu *pmu, u32 ctrl_id, u32 mode, u32 *entry_count);
int gb10b_pmu_pg_cg_elcg_get_residency(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 ctrl_id, u32 mode, u32 *delta_sleep_time_us,
	u32 *delta_wall_time_us);
int gb10b_pmu_pg_cg_elcg_mon_start(struct gk20a *g,
		struct nvgpu_pmu *pmu, u32 ctrl_id, bool enable);
int gb10b_pmu_pg_cg_elcg_mon_config(struct gk20a *g,
		struct nvgpu_pmu *pmu, u32 mode);
int gb10b_pmu_pg_cg_flcg_init(struct gk20a *g,
		struct nvgpu_pmu *pmu, u32 support_mask);
int gb10b_pmu_pg_cg_flcg_ctrl_enable(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 ctrl_id, bool enable);
int gb10b_pmu_pg_cg_flcg_ctrl_threshold_update(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 ctrl_id, u16 threshold_type,
		u16 threshold_ns);
int gb10b_pmu_pg_lpwr_lp_mon_start(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 lpwr_mon_index, bool enable);
int gb10b_pmu_pg_lpwr_lp_get_residency(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 lpwr_mon_index, u32 *delta_sleep_time_tick,
	u32 *delta_wall_time_tick);
int gb10b_pmu_pg_cg_csd_init(struct gk20a *g, struct nvgpu_pmu *pmu);
int gb10b_pmu_pg_cg_csd_ctrl_enable(struct gk20a *g,
		struct nvgpu_pmu *pmu, bool enable);
int gb10b_pmu_pg_cg_csd_ctrl_threshold_update(struct gk20a *g,
			struct nvgpu_pmu *pmu, u32 threshold_us);
int gb10b_pmu_pg_cg_csd_get_entry_count(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 *entry_count);
int gb10b_pmu_pg_cg_elcg_state_change(struct gk20a *g,
		struct nvgpu_pmu *pmu, u32 elcg_engine_status[]);
void gb10b_pmu_pg_lpwr_lp_rpc_handler(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct nv_pmu_rpc_header *rpc, struct rpc_handler_payload *rpc_payload);
#endif /* NVGPU_PMU_PG_SW_GB10B_H */
