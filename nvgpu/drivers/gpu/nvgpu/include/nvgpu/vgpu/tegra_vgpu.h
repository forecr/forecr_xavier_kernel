/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef TEGRA_VGPU_H
#define TEGRA_VGPU_H

#include <nvgpu/ecc.h>	/* For NVGPU_ECC_STAT_NAME_MAX_SIZE */

#include <nvgpu/channel.h>

enum {
	TEGRA_VGPU_CMD_CONNECT = 0,
	TEGRA_VGPU_CMD_CACHE_MAINT = 28,
	TEGRA_VGPU_CMD_GET_ZCULL_INFO = 30,
	TEGRA_VGPU_CMD_ZBC_SET_TABLE = 31,
	TEGRA_VGPU_CMD_ZBC_QUERY_TABLE = 32,
	TEGRA_VGPU_CMD_SET_MMU_DEBUG_MODE = 35,
	TEGRA_VGPU_CMD_TSG_PREEMPT = 54,
	TEGRA_VGPU_CMD_TSG_FORCE_RESET = 57,
	TEGRA_VGPU_CMD_SET_POWERGATE = 60,
	TEGRA_VGPU_CMD_SET_GPU_CLK_RATE = 61,
	TEGRA_VGPU_CMD_GET_CONSTANTS = 62,
	TEGRA_VGPU_CMD_CHANNEL_CYCLESTATS_SNAPSHOT = 63,
	TEGRA_VGPU_CMD_GET_GPU_LOAD = 65,
	TEGRA_VGPU_CMD_GET_GPU_CLK_RATE = 69,
	TEGRA_VGPU_CMD_GET_GPU_FREQ_TABLE = 70,
	TEGRA_VGPU_CMD_CAP_GPU_CLK_RATE = 71,
	TEGRA_VGPU_CMD_PROF_MGT = 72,
	TEGRA_VGPU_CMD_PERFBUF_MGT = 73,
	TEGRA_VGPU_CMD_GET_VSMS_MAPPING = 76,
	TEGRA_VGPU_CMD_SUSPEND = 82,
	TEGRA_VGPU_CMD_RESUME = 83,
	TEGRA_VGPU_CMD_GET_ECC_INFO = 84,
	TEGRA_VGPU_CMD_GET_ECC_COUNTER_VALUE = 85,
	TEGRA_VGPU_CMD_GET_TPC_EXCEPTION_EN_STATUS = 87,
	TEGRA_VGPU_CMD_FB_SET_MMU_DEBUG_MODE = 88,
	TEGRA_VGPU_CMD_PERF_UPDATE_GET_PUT = 95,
	TEGRA_VGPU_CMD_VF_REG_OPS = 102,
	TEGRA_VGPU_CMD_VF_ENABLE_RUNLIST = 103,
	TEGRA_VGPU_CMD_VF_SET_MMU_NACK_PENDING = 104,
	TEGRA_VGPU_CMD_VF_SUSPEND_CONTEXTS = 105,
	TEGRA_VGPU_CMD_VF_CHANNEL_READ_STATE = 106,
	TEGRA_VGPU_CMD_VF_CHANNEL_ENABLE = 107,
	TEGRA_VGPU_CMD_VF_CHANNEL_DISABLE = 108,
	TEGRA_VGPU_CMD_VF_CHANNEL_CLEAR = 109,
	TEGRA_VGPU_CMD_VF_CHANNEL_FORCE_CTX_RELOAD = 110,
	TEGRA_VGPU_CMD_VF_GET_GOLDEN_IMAGE = 111,
	TEGRA_VGPU_CMD_VF_SUBMIT_RUNLIST = 112,
	TEGRA_VGPU_CMD_VF_CHANNEL_RESET_FAULTED = 113,
	TEGRA_VGPU_CMD_VF_HANDLE_FAULT = 114,
	TEGRA_VGPU_CMD_VF_PROF_RESET_PM_TRIGGER_MASK = 117,
	TEGRA_VGPU_CMD_VF_PROF_ENABLE_PM_TRIGGER = 118,
	TEGRA_VGPU_CMD_VF_PROF_BIND_UNBIND = 119,
	TEGRA_VGPU_CMD_VF_SET_HS_CREDITS = 120,
	TEGRA_VGPU_CMD_VF_PERF_INST_BLOCK_MGMT = 121,
	TEGRA_VGPU_CMD_VF_RESET_STREAMING = 122,
	TEGRA_VGPU_CMD_GET_PTO_COUNTER_FREQ = 123,
	TEGRA_VGPU_CMD_GET_GPU_TEMPERATURE = 124,
	TEGRA_VGPU_CMD_GET_GPU_VOLTAGE = 125,
	TEGRA_VGPU_CMD_GET_VF_POINTS_STATUS = 126,
	TEGRA_VGPU_CMD_GET_GPU_VOLT_RAIL_LIMITS = 127,
	TEGRA_VGPU_CMD_VF_GSP_VM_SCHED_GET_ENG_TIMESLICE_CFG = 128,
	TEGRA_VGPU_CMD_VF_GSP_VM_SCHED_GET_ENG_TIMESLICE = 129,
	TEGRA_VGPU_CMD_VF_GSP_VM_SCHED_SET_ENG_TIMESLICE = 130,
	TEGRA_VGPU_CMD_DUMMY = 0xffffffff,
};

struct tegra_vgpu_connect_params {
	u64 handle;
};

enum {
	TEGRA_VGPU_L2_MAINT_FLUSH = 0,
	TEGRA_VGPU_L2_MAINT_FLUSH_INV,
	TEGRA_VGPU_FB_FLUSH,
};

struct tegra_vgpu_cache_maint_params {
	u8 op;
};

#ifdef CONFIG_NVGPU_GRAPHICS

struct tegra_vgpu_zcull_info_params {
	u32 width_align_pixels;
	u32 height_align_pixels;
	u32 pixel_squares_by_aliquots;
	u32 aliquot_total;
	u32 region_byte_multiplier;
	u32 region_header_size;
	u32 subregion_header_size;
	u32 subregion_width_align_pixels;
	u32 subregion_height_align_pixels;
	u32 subregion_count;
};

#define TEGRA_VGPU_ZBC_COLOR_VALUE_SIZE		4
#define TEGRA_VGPU_ZBC_TYPE_INVALID		0
#define TEGRA_VGPU_ZBC_TYPE_COLOR		1
#define TEGRA_VGPU_ZBC_TYPE_DEPTH		2
#define TEGRA_VGPU_ZBC_TYPE_STENCIL		3

struct tegra_vgpu_zbc_set_table_params {
	u32 color_ds[TEGRA_VGPU_ZBC_COLOR_VALUE_SIZE];
	u32 color_l2[TEGRA_VGPU_ZBC_COLOR_VALUE_SIZE];
	u32 depth;
	u32 stencil;
	u32 format;
	u32 type;     /* color, depth or stencil */
};

struct tegra_vgpu_zbc_query_table_params {
	u32 color_ds[TEGRA_VGPU_ZBC_COLOR_VALUE_SIZE];
	u32 color_l2[TEGRA_VGPU_ZBC_COLOR_VALUE_SIZE];
	u32 depth;
	u32 stencil;
	u32 ref_cnt;
	u32 format;
	u32 type;             /* color, depth or stencil */
	u32 index_size;       /* [out] size, [in] index */
};
#endif

struct tegra_vgpu_mmu_debug_mode {
	u32 enable;
};

struct tegra_vgpu_reg_op {
	u8 op;
	u8 type;
	u8 status;
	u8 quad;
	u32 group_mask;
	u32 sub_group_mask;
	u32 offset;
	u32 value_lo;
	u32 value_hi;
	u32 and_n_mask_lo;
	u32 and_n_mask_hi;
};

struct tegra_vgpu_reg_ops_params {
	u64 num_ops;
	u32 tsg_id;
	u32 flags;
};

struct tegra_vf_regop_offset {
	u16 regop_idx;
	u8 buffer_type;
	u8 num_offsets;
	struct {
		u32 offset;
		u32 offset_addr;
	} offsets[];
};

struct tegra_vgpu_ecc_info_params {
	u32 ecc_stats_count;
};

struct tegra_vgpu_ecc_info_entry {
	u32 ecc_id;
	char name[NVGPU_ECC_STAT_NAME_MAX_SIZE];
};

struct tegra_vgpu_ecc_counter_params {
	u32 ecc_id;
	u32 value;
};

struct tegra_vgpu_tsg_params {
	u32 runlist_id;
	u32 tsg_id;
};

struct tegra_vgpu_read_ptimer_params {
	u64 time;
};

#define TEGRA_VGPU_GET_TIMESTAMPS_ZIPPER_MAX_COUNT      16
#define TEGRA_VGPU_GET_TIMESTAMPS_ZIPPER_SRC_ID_TSC     1
struct tegra_vgpu_get_timestamps_zipper_params {
	/* timestamp pairs */
	struct {
		/* gpu timestamp value */
		u64 cpu_timestamp;
		/* raw GPU counter (PTIMER) value */
		u64 gpu_timestamp;
	} samples[TEGRA_VGPU_GET_TIMESTAMPS_ZIPPER_MAX_COUNT];
	/* number of pairs to read */
	u32 count;
	/* cpu clock source id */
	u32 source_id;
};

#define TEGRA_VGPU_POWERGATE_MODE_ENABLE	1
#define TEGRA_VGPU_POWERGATE_MODE_DISABLE	2
struct tegra_vgpu_set_powergate_params {
	u32 mode;
};

struct tegra_vgpu_gpu_clk_rate_params {
	u64 rate; /* in Hz */
};

struct tegra_vgpu_set_sm_exception_type_mask_params {
	u64 handle;
	u32 mask;
};

/* TEGRA_VGPU_MAX_ENGINES must be equal or greater than num_engines */
#define TEGRA_VGPU_MAX_ENGINES			14
struct tegra_vgpu_engines_info {
	u32 num_engines;
	struct engineinfo {
		u32 engine_id;
		u32 intr_mask;
		u32 reset_mask;
		u32 runlist_id;
		u32 rleng_id;
		u32 pbdma_id;
		u32 inst_id;
		u32 pri_base;
		u32 engine_enum;
		u32 fault_id;
	} info[TEGRA_VGPU_MAX_ENGINES];
};

#define TEGRA_VGPU_MAX_GPC_COUNT 3U
#define TEGRA_VGPU_MAX_TPC_COUNT_PER_GPC 5U
#define TEGRA_VGPU_MAX_PES_COUNT_PER_GPC 3U
#define TEGRA_VGPU_L2_EN_MASK 32U
#define TEGRA_VGPU_MAX_FBP_COUNT 4U

struct tegra_vgpu_constants_params {
	u32 arch;
	u32 impl;
	u32 rev;
	u64 max_freq;
	u32 num_channels;
	u32 golden_ctx_size;
	u32 zcull_ctx_size;
	u32 l2_size;
	u32 ltc_count;
	u32 cacheline_size;
	u32 slices_per_ltc;
	u32 sm_arch_sm_version;
	u32 sm_arch_spa_version;
	u32 sm_arch_warp_count;
	u32 max_gpc_count;
	u32 gpc_count;
	u32 gpc_mask;
	u32 max_tpc_per_gpc_count;
	u32 num_fbps;
	u32 fbp_en_mask;
	u32 ltc_per_fbp;
	u32 max_lts_per_ltc;
	u8 gpc_tpc_count[TEGRA_VGPU_MAX_GPC_COUNT];
	/* mask bits should be equal or larger than
	 * TEGRA_VGPU_MAX_TPC_COUNT_PER_GPC
	 */
	u16 gpc_tpc_mask[TEGRA_VGPU_MAX_GPC_COUNT];
	u16 gpc_tpc_mask_physical[TEGRA_VGPU_MAX_GPC_COUNT];
	u16 gpc_ppc_count[TEGRA_VGPU_MAX_GPC_COUNT];
	u32 pes_tpc_count[TEGRA_VGPU_MAX_PES_COUNT_PER_GPC
			* TEGRA_VGPU_MAX_GPC_COUNT];
	u32 pes_tpc_mask[TEGRA_VGPU_MAX_PES_COUNT_PER_GPC
			* TEGRA_VGPU_MAX_GPC_COUNT];
	u32 gpc_pes_mask_physical[TEGRA_VGPU_MAX_GPC_COUNT];
	struct {
		/** Logical GPC Id which is used to access GPC registers. */
		u32 logical_id;
		/** Physical GPC Id. */
		u32 physical_id;
		/** GPC group Id. */
		u32 gpcgrp_id;
	} gpc[TEGRA_VGPU_MAX_GPC_COUNT];
	/* fbp physical id to logical id mapping */
	u32 fbp_physical_id_map[TEGRA_VGPU_MAX_FBP_COUNT];
	/* fbp logical id to local id mapping */
	u32 fbp_mappings[TEGRA_VGPU_MAX_FBP_COUNT];
	u32 hwpm_ctx_size;
	u8 can_set_clkrate;
	u8 support_sm_ttu;
	u32 preempt_ctx_size;
	u32 channel_base;
	struct tegra_vgpu_engines_info engines_info;
	u32 num_pce;
	u32 sm_per_tpc;
	u32 max_subctx_count;
	u32 l2_en_mask[TEGRA_VGPU_L2_EN_MASK];
	/** Max SM configuration count. */
	u32 max_sm_diversity_config_count;
	u64 per_device_identifier;
	u32 max_rop_per_gpc;
	u32 phy_gpc_rop_mask[TEGRA_VGPU_MAX_GPC_COUNT];
	bool can_control_gsp_vm_sched;
	u32 gsp_vm_sched_min_timeslice;
	u32 gsp_vm_sched_max_timeslice;
};

enum {
	TEGRA_VGPU_CYCLE_STATS_SNAPSHOT_CMD_FLUSH = 0,
	TEGRA_VGPU_CYCLE_STATS_SNAPSHOT_CMD_ATTACH = 1,
	TEGRA_VGPU_CYCLE_STATS_SNAPSHOT_CMD_DETACH = 2,
};

struct tegra_vgpu_channel_cyclestats_snapshot_params {
	u64 handle;
	u32 perfmon_start;
	u32 perfmon_count;
	u32 buf_info; /* client->srvr: get ptr; srvr->client: num pending */
	u8 subcmd;
	u8 hw_overflow;
};

struct tegra_vgpu_gpu_load_params {
	u32 load;
	u32 perfmon_class;
};

enum {
	TEGRA_VGPU_PROF_PM_RESERVATION_ACQUIRE = 0,
	TEGRA_VGPU_PROF_PM_RESERVATION_RELEASE,
};

struct tegra_vgpu_prof_mgt_params {
	u32 mode;
	u32 reservation_id;
	u32 pm_resource;
	u32 scope;
	u32 pma_channel_id;
};

struct tegra_vgpu_perfbuf_mgt_params {
	u64 vm_handle;
	u64 offset;
	u32 size;
	u32 pma_channel_id;
};

#define TEGRA_VGPU_GPU_FREQ_TABLE_SIZE		25

struct tegra_vgpu_get_gpu_freq_table_params {
	u32 num_freqs;
};

struct tegra_vgpu_vsms_mapping_params {
	u32 num_sm;
};

struct tegra_vgpu_vsms_mapping_entry {
	u32 gpc_index;
	u32 tpc_index;
	u32 sm_index;
	u32 global_tpc_index;
	u32 virtual_gpc_index;
};

struct tegra_vgpu_get_tpc_exception_en_status_params {
	u64 tpc_exception_en_sm_mask;
};

struct tegra_vgpu_fb_set_mmu_debug_mode_params {
	u8 enable;
};

enum {
	TEGRA_VGPU_PROF_BIND_HWPM = 0,
	TEGRA_VGPU_PROF_BIND_STREAMOUT = 2,
	TEGRA_VGPU_PROF_UNBIND_HWPM_STREAMOUT = 3,
	TEGRA_VGPU_PROF_BIND_SMPC = 4,
	TEGRA_VGPU_PROF_UNBIND_SMPC = 5,
	TEGRA_VGPU_PROF_BIND_HES_CWD = 6,
	TEGRA_VGPU_PROF_UNBIND_CCUPROF = 7,
};

struct tegra_vgpu_prof_bind_unbind_params {
	u32 subcmd;
	u8 is_ctxsw;
	u8 smpc_reserved;
	u32 tsg_id;
	u32 pma_buffer_size;
	u64 pma_buffer_va;
	u64 pma_bytes_available_buffer_va;
	u32 reservation_id;
	u32 gr_instance_id;
	u32 mode;
	u32 pma_channel_id;
	bool is_ctx_resident;
};

struct tegra_vgpu_perf_update_get_put_params {
	u64 bytes_consumed;
	u64 put_ptr;
	u32 pma_channel_id;
	u32 tsg_id;
	u8 update_available_bytes;
	u8 overflowed;
	bool is_ctx_resident;
};

struct tegra_vgpu_vf_enable_runlist_params {
	u32 virt_runlist_id;
};

struct tegra_vgpu_vf_mmu_nack_pending_params {
	u32 gr_instance_id;
	u32 gr_ctx_id;
	bool handled;
};

struct tegra_vgpu_vf_suspend_contexts {
	u32 num_tsgs;
	u32 resident_tsgid;
};

struct tegra_vgpu_vf_ch_state_params {
	u16 virt_runlist_id;
	u16 virt_chid;
	struct nvgpu_channel_hw_state state;
};

struct tegra_vgpu_vf_ch_params {
	u16 virt_runlist_id;
	u16 virt_chid;
};

struct tegra_vgpu_vf_golden_image_params {
	u32 size;
};

struct tegra_vgpu_vf_runlist_params {
	u16 virt_runlist_id;
	u64 iova;
	u32 length;
	u32 offset;
};

struct tegra_vgpu_vf_fault_params {
	u32 virt_runlist_id;
	u32 tsgid;
};

struct tegra_vgpu_vf_reset_pm_trigger_mask {
	u32 pma_channel_id;
	u32 reservation_id;
	u32 gr_instance_id;
};

struct tegra_vgpu_vf_enable_pm_trigger {
	u32 gr_instance_id;
	u32 pma_channel_id;
	u32 reservation_id;
};

struct tegra_vgpu_vf_set_hs_credits {
	u32 gr_instance_id;
	u32 pma_channel_id;
	u32 chiplet_type;
	u32 chiplet_local_index;
	u32 num_credits;
};

enum {
	TEGRA_VGPU_VF_PROF_PERF_INST_BLOCK_INIT = 0,
	TEGRA_VGPU_VF_PROF_PERF_INST_BLOCK_DEINIT,
};

struct tegra_vgpu_vf_perf_inst_block {
	u32 iova;
	u32 pma_channel_id;
	u32 mode;
	u32 aperture;
};

struct tegra_vgpu_vf_reset_streaming {
	u32 pma_channel_id;
};

enum {
	TEGRA_VGPU_CLK_GPC = 0,
	TEGRA_VGPU_CLK_SYS = 1,
	TEGRA_VGPU_CLK_NVD = 2,
	TEGRA_VGPU_CLK_UPROC = 3,
	TEGRA_VGPU_CLK_ALL = 4,
};

struct tegra_vgpu_get_pto_counter_freq_params {
	u32 counter_type;
	u64 frequency_hz;
};

enum {
	TEGRA_VGPU_GPU_TEMPERATURE_TYPE_MAX = 0,
	TEGRA_VGPU_GPU_TEMPERATURE_TYPE_AVG = 1,
	TEGRA_VGPU_GPU_TEMPERATURE_TYPE_MIN = 2,
};

struct tegra_vgpu_gpu_temperature_params {
	u32 type;
	s32 temp_raw;
};

enum {
	TEGRA_VGPU_GPU_VOLTAGE_TYPE_GPM = 0,
	TEGRA_VGPU_GPU_VOLTAGE_TYPE_GPC = 1,
};

struct tegra_vgpu_gpu_voltage_params {
	u32 type;
	u32 voltage;
};

struct tegra_vgpu_gpu_volt_rail_limits_params {
	u8 type;
	u32 v_min;
	u32 v_max;
};

struct tegra_vgpu_get_vf_points_status_params {
	u32 num_vf_points;
	u8 type;
};

struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params {
	u32 vmid;
	u32 engine_type;
	u32 instance;
	u32 timeslice;
};

#define TEGRA_VGPU_NUM_VM_MAX          16
struct tegra_vgpu_vf_gsp_vm_sched_config {
	u32 min_timeslice;
	u32 max_timeslice;
	u32 count;
	struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params eng_params[TEGRA_VGPU_MAX_ENGINES * TEGRA_VGPU_NUM_VM_MAX];
};

struct tegra_vgpu_vf_gsp_vm_sched_config_params {
	u32 min_timeslice;
	u32 max_timeslice;
	u32 timeslice_params_count;
};

struct tegra_vgpu_cmd_msg {
	u32 cmd;
	int ret;
	u64 handle;
	union {
		struct tegra_vgpu_connect_params connect;
		struct tegra_vgpu_cache_maint_params cache_maint;
#ifdef CONFIG_NVGPU_GRAPHICS
		struct tegra_vgpu_zcull_info_params zcull_info;
		struct tegra_vgpu_zbc_set_table_params zbc_set_table;
		struct tegra_vgpu_zbc_query_table_params zbc_query_table;
#endif
		struct tegra_vgpu_mmu_debug_mode mmu_debug_mode;
		struct tegra_vgpu_reg_ops_params reg_ops;
		struct tegra_vgpu_tsg_params tsg;
		struct tegra_vgpu_read_ptimer_params read_ptimer;
		struct tegra_vgpu_set_powergate_params set_powergate;
		struct tegra_vgpu_gpu_clk_rate_params gpu_clk_rate;
		struct tegra_vgpu_channel_cyclestats_snapshot_params cyclestats_snapshot;
		struct tegra_vgpu_gpu_load_params gpu_load;
		struct tegra_vgpu_prof_mgt_params prof_management;
		struct tegra_vgpu_perfbuf_mgt_params perfbuf_management;
		struct tegra_vgpu_get_timestamps_zipper_params get_timestamps_zipper;
		struct tegra_vgpu_get_gpu_freq_table_params get_gpu_freq_table;
		struct tegra_vgpu_vsms_mapping_params vsms_mapping;
		struct tegra_vgpu_ecc_info_params ecc_info;
		struct tegra_vgpu_ecc_counter_params ecc_counter;
		struct tegra_vgpu_set_sm_exception_type_mask_params set_sm_exception_mask;
		struct tegra_vgpu_get_tpc_exception_en_status_params get_tpc_exception_status;
		struct tegra_vgpu_fb_set_mmu_debug_mode_params fb_set_mmu_debug_mode;
		struct tegra_vgpu_prof_bind_unbind_params prof_bind_unbind;
		struct tegra_vgpu_perf_update_get_put_params perf_updat_get_put;
		struct tegra_vgpu_vf_enable_runlist_params vf_enable_runlist;
		struct tegra_vgpu_vf_mmu_nack_pending_params vf_mmu_nack_pending;
		struct tegra_vgpu_vf_suspend_contexts vf_suspend_contexts;
		struct tegra_vgpu_vf_ch_state_params vf_ch_state;
		struct tegra_vgpu_vf_ch_params vf_ch;
		struct tegra_vgpu_vf_golden_image_params vf_golden_image;
		struct tegra_vgpu_vf_runlist_params vf_runlist;
		struct tegra_vgpu_vf_fault_params vf_fault;
		struct tegra_vgpu_vf_reset_pm_trigger_mask reset_pm_trigger_mask;
		struct tegra_vgpu_vf_enable_pm_trigger enable_pm_trigger;
		struct tegra_vgpu_vf_set_hs_credits set_hs_credit;
		struct tegra_vgpu_vf_perf_inst_block inst_block_mgmt;
		struct tegra_vgpu_vf_reset_streaming reset_streaming;
		struct tegra_vgpu_get_pto_counter_freq_params get_pto_counter_freq;
		struct tegra_vgpu_gpu_temperature_params gpu_temperature;
		struct tegra_vgpu_gpu_voltage_params gpu_voltage;
		struct tegra_vgpu_gpu_volt_rail_limits_params gpu_volt_rail_limits;
		struct tegra_vgpu_get_vf_points_status_params get_vf_points_status;
		struct tegra_vgpu_vf_gsp_vm_sched_config_params gsp_vm_sched_config_params;
		struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params gsp_vm_sched_timeslice;
		char padding[184];
	} params;
};

_Static_assert(sizeof(struct tegra_vgpu_cmd_msg) <= 512U,
	"size of tegra_vgpu_cmd_msg greater than ivc frame");

enum {
	TEGRA_VGPU_GR_INTR_NOTIFY = 0,
	TEGRA_VGPU_GR_INTR_SEMAPHORE = 9,
	TEGRA_VGPU_FIFO_INTR_CTXSW_TIMEOUT = 11,
	TEGRA_VGPU_GR_INTR_SM_EXCEPTION = 16,
};

struct tegra_vgpu_gr_intr_info {
	u32 type;
	u32 tsgid;
};

struct tegra_vgpu_fifo_intr_info {
	u32 type;
	u32 tsgid;
};

enum {
	TEGRA_VGPU_FECS_TRACE_DATA_UPDATE = 0
};

#define TEGRA_VGPU_CHANNEL_EVENT_ID_MAX 6
struct tegra_vgpu_channel_event_info {
	u32 event_id;
	u32 tsgid;
};

struct tegra_vgpu_sm_esr_info {
	u32 tsg_id;
	u32 sm_id;
	u32 hww_global_esr;
	u32 hww_warp_esr;
	u64 hww_warp_esr_pc;
	u32 hww_global_esr_report_mask;
	u32 hww_warp_esr_report_mask;
	u32 hww_cga_esr;
};

struct tegra_vgpu_semaphore_wakeup {
	u32 post_events;
};

struct tegra_vgpu_channel_cleanup {
	u32 chid;
};

struct tegra_vgpu_channel_set_error_notifier {
	u32 chid;
	u32 error;
};

struct tegra_vgpu_tsg_set_error_notifier {
	u32 tsgid;
	u32 error;
};

struct tegra_vf_tsg_set_unserviceable {
	u32 tsgid;
};

struct tegra_vf_runlist_disabled {
	u32 runlist_id;
	u32 tsgid;
};

struct tegra_vf_cilp_save_completed {
	u32 tsgid;
};

enum {

	TEGRA_VGPU_INTR_GR = 0,
	TEGRA_VGPU_INTR_FIFO = 1,
};

enum {
	TEGRA_VGPU_EVENT_INTR = 0,
	TEGRA_VGPU_EVENT_ABORT = 1,
	TEGRA_VGPU_EVENT_FECS_TRACE = 2,
	TEGRA_VGPU_EVENT_CHANNEL = 3,
	TEGRA_VGPU_EVENT_SM_ESR = 4,
	TEGRA_VGPU_EVENT_SEMAPHORE_WAKEUP = 5,
	TEGRA_VGPU_EVENT_CHANNEL_CLEANUP = 6,
	TEGRA_VGPU_EVENT_SET_ERROR_NOTIFIER = 7,
	TEGRA_VGPU_EVENT_TSG_SET_ERROR_NOTIFIER = 8,
	TEGRA_VF_EVENT_TSG_SET_UNSERVICEABLE = 9,
	TEGRA_VF_EVENT_RUNLIST_DISABLED_FOR_RC = 10,
	TEGRA_VF_EVENT_RUNLIST_DISABLED_FOR_CILP = 11,
	TEGRA_VF_EVENT_CILP_SAVE_COMPLETED = 12,
};

struct tegra_vgpu_intr_msg {
	unsigned int event;
	u32 unit;
	union {
		struct tegra_vgpu_gr_intr_info gr_intr;
		struct tegra_vgpu_fifo_intr_info fifo_intr;
		struct tegra_vgpu_channel_event_info channel_event;
		struct tegra_vgpu_sm_esr_info sm_esr;
		struct tegra_vgpu_semaphore_wakeup sem_wakeup;
		struct tegra_vgpu_channel_cleanup ch_cleanup;
		struct tegra_vgpu_channel_set_error_notifier set_error_notifier;
		struct tegra_vgpu_tsg_set_error_notifier tsg_set_error_notifier;
		struct tegra_vf_tsg_set_unserviceable tsg_set_unserviceable;
		struct tegra_vf_runlist_disabled runlist_disabled;
		struct tegra_vf_cilp_save_completed cilp_save_completed;
		char padding[32];
	} info;
};

_Static_assert(sizeof(struct tegra_vgpu_intr_msg) <= 64U,
	"size of tegra_vgpu_intr_msg greater than ivc frame");

#endif
