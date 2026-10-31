/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _UAPI__LINUX_NVGPU_CTRL_H__
#define _UAPI__LINUX_NVGPU_CTRL_H__

#include "nvgpu-uapi-common.h"

#define NVGPU_GPU_IOCTL_MAGIC 'G'

/* return zcull ctx size */
struct nvgpu_gpu_zcull_get_ctx_size_args {
	__u32 size;
};

/* return zcull info */
struct nvgpu_gpu_zcull_get_info_args {
	__u32 width_align_pixels;
	__u32 height_align_pixels;
	__u32 pixel_squares_by_aliquots;
	__u32 aliquot_total;
	__u32 region_byte_multiplier;
	__u32 region_header_size;
	__u32 subregion_header_size;
	__u32 subregion_width_align_pixels;
	__u32 subregion_height_align_pixels;
	__u32 subregion_count;
};

#define NVGPU_ZBC_COLOR_VALUE_SIZE	4
#define NVGPU_ZBC_TYPE_INVALID		0
#define NVGPU_ZBC_TYPE_COLOR		1
#define NVGPU_ZBC_TYPE_DEPTH		2
#define NVGPU_ZBC_TYPE_STENCIL		3

struct nvgpu_gpu_zbc_set_table_args {
	__u32 color_ds[NVGPU_ZBC_COLOR_VALUE_SIZE];
	__u32 color_l2[NVGPU_ZBC_COLOR_VALUE_SIZE];
	__u32 depth;
	__u32 stencil;
	__u32 format;
	__u32 type;	/* color, depth or stencil */
};

struct nvgpu_gpu_zbc_query_table_args {
	__u32 color_ds[NVGPU_ZBC_COLOR_VALUE_SIZE];
	__u32 color_l2[NVGPU_ZBC_COLOR_VALUE_SIZE];
	__u32 depth;
	__u32 stencil;
	__u32 ref_cnt;
	__u32 format;
	__u32 type;		/* color, depth or stencil */
	__u32 index_size;	/* [out] size, [in] index */
};


/* This contains the minimal set by which the userspace can
   determine all the properties of the GPU */
#define NVGPU_GPU_ARCH_GK100	0x000000E0
#define NVGPU_GPU_ARCH_GM200	0x00000120
#define NVGPU_GPU_ARCH_GP100	0x00000130
#define NVGPU_GPU_ARCH_GV110	0x00000150
#define NVGPU_GPU_ARCH_GV100	0x00000140

#define NVGPU_GPU_IMPL_GK20A	0x0000000A
#define NVGPU_GPU_IMPL_GM204	0x00000004
#define NVGPU_GPU_IMPL_GM206	0x00000006
#define NVGPU_GPU_IMPL_GM20B	0x0000000B
#define NVGPU_GPU_IMPL_GM20B_B	0x0000000E
#define NVGPU_GPU_IMPL_GP104	0x00000004
#define NVGPU_GPU_IMPL_GP106	0x00000006
#define NVGPU_GPU_IMPL_GP10B	0x0000000B
#define NVGPU_GPU_IMPL_GV11B	0x0000000B
#define NVGPU_GPU_IMPL_GV100	0x00000000

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 * @defgroup NVGPURM_INTF_GET_CHARACTERISTICS IOCTL: NVGPU_GPU_IOCTL_GET_CHARACTERISTICS
 * @brief Returns the GPU characteristics.
 *
 * This IOCTL retrieves the main hardware/software configuration of the
 * GPU. This includes the main hardware traits and the NvGPU resource manager
 * (nvgpu-rm) capabilities. The capabilities exposed by nvgpu-rm may be a subset
 * of the full capabilities supported by the GPU hardware device. In safety,
 * only the safety-critical capability set is supported. This is indicated by
 * flag @c #NVGPU_GPU_FLAGS_DRIVER_REDUCED_PROFILE.
 *
 * Additional configuration information is provided by:
 * - @ref NVGPURM_INTF_GET_ENGINE_INFO
 * - @ref NVGPURM_INTF_GET_TPC_MASKS
 * - @ref NVGPURM_INTF_VSMS_MAPPING
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_gpu_get_characteristics</td>
 *   <td>@c #nvgpu_gpu_get_characteristics</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1</td>
 *   <td>(none)</td>
 *   <td>@c #nvgpu_gpu_characteristics</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EIO ---    Writing @c #nvgpu_gpu_characteristics failed
 * - @c ENODEV --- GPU not ready
 * - @c EFAULT --- Copy from userspace fails.
 */

/**
 * @ingroup NVGPURM_INTF_GET_CHARACTERISTICS
 * @{
 */

/**
 * @brief iGPU bus type is unspecified or iGPU directly connected
 *
 * This is usually used only in simulation environments.
 */
#define NVGPU_GPU_BUS_TYPE_NONE	0U

/**
 * @brief iGPU bus type is AXI.
 */
#define NVGPU_GPU_BUS_TYPE_AXI	32U

/** @} */

/**
 * @ingroup NVGPURM_INTF_GET_CHARACTERISTICS
 * @{
 */

/**
 * @brief Tegra Host1x syncpoints are supported.
 */
#define NVGPU_GPU_FLAGS_HAS_SYNCPOINTS					NVGPU_BIT64(0)

/**
 * @brief Sparse GPU VA space allocation is supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_AS_ALLOC_SPACE_FLAGS_SPARSE
 */
#define NVGPU_GPU_FLAGS_SUPPORT_SPARSE_ALLOCS				NVGPU_BIT64(2)

/**
 * @brief sync fence FDs are available in, e.g., submit_gpfifo
 */
#define NVGPU_GPU_FLAGS_SUPPORT_SYNC_FENCE_FDS				NVGPU_BIT64(3)

/**
 * @brief Cycle stats is supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_DBG_GPU_IOCTL_CYCLE_STATS
 */
#define NVGPU_GPU_FLAGS_SUPPORT_CYCLE_STATS				NVGPU_BIT64(4)

/**
 * @brief Cycle stats snapshot is supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_DBG_GPU_IOCTL_CYCLE_STATS_SNAPSHOT
 */
#define NVGPU_GPU_FLAGS_SUPPORT_CYCLE_STATS_SNAPSHOT			NVGPU_BIT64(6)

/**
 * @brief TSGs are supported.
 *
 * @sa @ref NVGPURM_LINUX_IOCTL_TSG
 */
#define NVGPU_GPU_FLAGS_SUPPORT_TSG					NVGPU_BIT64(8)

/**
 * @brief Clock controls are supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_GPU_IOCTL_CLK_GET_VF_POINTS
 */
#define NVGPU_GPU_FLAGS_SUPPORT_CLOCK_CONTROLS				NVGPU_BIT64(9)

/**
 * @brief Sensor info 'voltage' is supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_GPU_IOCTL_GET_VOLTAGE
 */
#define NVGPU_GPU_FLAGS_SUPPORT_GET_VOLTAGE				NVGPU_BIT64(10)

/**
 * @brief Sensor info 'electric current' is supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_GPU_IOCTL_GET_CURRENT
 */
#define NVGPU_GPU_FLAGS_SUPPORT_GET_CURRENT				NVGPU_BIT64(11)

/**
 * @brief Sensor info 'power' is supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_GPU_IOCTL_GET_POWER
 */
#define NVGPU_GPU_FLAGS_SUPPORT_GET_POWER				NVGPU_BIT64(12)

/**
 * @brief Sensor info 'temperature' is supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_GPU_IOCTL_GET_TEMPERATURE
 */
#define NVGPU_GPU_FLAGS_SUPPORT_GET_TEMPERATURE				NVGPU_BIT64(13)

/**
 * @brief Thermal alerts are supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_GPU_IOCTL_SET_THERM_ALERT_LIMIT
 */
#define NVGPU_GPU_FLAGS_SUPPORT_SET_THERM_ALERT_LIMIT			NVGPU_BIT64(14)

/**
 * @brief Operating system events (event fds) are supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_GPU_IOCTL_GET_EVENT_FD
 */
#define NVGPU_GPU_FLAGS_SUPPORT_DEVICE_EVENTS				NVGPU_BIT64(15)

/**
 * @brief CTXSW firmware traces are supported or not.
 *
 * @remark Not supported in safety.
 */
#define NVGPU_GPU_FLAGS_SUPPORT_FECS_CTXSW_TRACE			NVGPU_BIT64(16)

/** @brief Deprecated. Do not use. */
#define NVGPU_GPU_FLAGS_SUPPORT_MAP_COMPBITS				NVGPU_BIT64(17)

/**
 * @brief Indicates support for deterministic KMD submits without job tracking.
 *
 * @sa @c #NVGPU_ALLOC_GPFIFO_EX_FLAGS_DETERMINISTIC
 */
#define NVGPU_GPU_FLAGS_SUPPORT_DETERMINISTIC_SUBMIT_NO_JOBTRACKING	NVGPU_BIT64(18)

/**
 * @brief Indicates support for deterministic KMD submits with job tracking.
 *
 * @sa @c #NVGPU_ALLOC_GPFIFO_EX_FLAGS_DETERMINISTIC
 */
#define NVGPU_GPU_FLAGS_SUPPORT_DETERMINISTIC_SUBMIT_FULL		NVGPU_BIT64(19)

/**
 * @brief IO coherency is supported.
 *
 * @sa @c #NVGPU_AS_MAP_BUFFER_FLAGS_SYSTEM_COHERENT
 */
#define NVGPU_GPU_FLAGS_SUPPORT_IO_COHERENCE				NVGPU_BIT64(20)

/**
 * @brief Reschedule runlist is supported.
 *
 * @sa @c #NVGPU_IOCTL_CHANNEL_RESCHEDULE_RUNLIST
 */
#define NVGPU_GPU_FLAGS_SUPPORT_RESCHEDULE_RUNLIST			NVGPU_BIT64(21)

/**
 * @brief TSG subcontexts are supported.
 *
 * @sa @ref NVGPURM_INTF_TSG_CREATE_SUBCONTEXT
 * @sa @ref NVGPURM_INTF_TSG_DELETE_SUBCONTEXT
 */
#define NVGPU_GPU_FLAGS_SUPPORT_TSG_SUBCONTEXTS				NVGPU_BIT64(22)

/**
 * @brief Setting channel(s) as deterministic is supported.
 *
 * @sa @c #NVGPU_GPU_IOCTL_SET_DETERMINISTIC_OPTS
 */
#define NVGPU_GPU_FLAGS_SUPPORT_DETERMINISTIC_OPTS			NVGPU_BIT64(24)

/**
 * @brief Simultaneous compute and graphics is supported.
 */
#define NVGPU_GPU_FLAGS_SUPPORT_SCG					NVGPU_BIT64(25)

/**
 * @brief Read-only syncpoint-semaphore shim is supported.
 *
 * @sa @ref NVGPURM_INTF_AS_GET_SYNCPT_RO_MAP
 */
#define NVGPU_GPU_FLAGS_SUPPORT_SYNCPOINT_ADDRESS			NVGPU_BIT64(26)

/**
 * @brief Video Protected Region (VPR) is supported.
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_ALLOC_GPFIFO_EX_FLAGS_VPR_ENABLED
 */
#define NVGPU_GPU_FLAGS_SUPPORT_VPR					NVGPU_BIT64(27)

/**
 * @brief Indicates support for allocating channel read-write syncpoints.
 *
 * @sa @ref NVGPURM_INTF_CHANNEL_GET_SYNCPT
 */
#define NVGPU_GPU_FLAGS_SUPPORT_USER_SYNCPOINT				NVGPU_BIT64(28)

/**
 * @brief Railgating (powering the GPU off completely) is supported and enabled.
 */
#define NVGPU_GPU_FLAGS_CAN_RAILGATE					NVGPU_BIT64(29)

/**
 * @brief Usermode submit channels are supported.
 *
 * @sa @c #NVGPU_ALLOC_GPFIFO_EX_FLAGS_USERMODE_SUPPORT
 */
#define NVGPU_GPU_FLAGS_SUPPORT_USERMODE_SUBMIT				NVGPU_BIT64(30)

/**
 * @brief Reduced profile is enabled.
 */
#define NVGPU_GPU_FLAGS_DRIVER_REDUCED_PROFILE				NVGPU_BIT64(31)

/**
 * @brief MMU debug mode is available
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_DBG_GPU_IOCTL_SET_CTX_MMU_DEBUG_MODE
 */
#define NVGPU_GPU_FLAGS_SUPPORT_SET_CTX_MMU_DEBUG_MODE			NVGPU_BIT64(32)

/**
 * @brief GPU context fault recovery is enabled.
 *
 * When this flag is present, GPU context fault recovery procedures are
 * enabled.
 *
 * When this flag is not present, a GPU context fault will trigger the
 * resource manager to enter *Quiesce state*. In this state, the GPU becomes
 * inoperable and all error notifiers are signaled with an error.
 *
 * @remark Context fault recovery is not enabled in safety
 *
 * @remark Context fault is triggered by, *e.g.*, illegal shader instruction,
 * illegal pushbuffer method, or access to an unmapped GPU virtual address.
 */
#define NVGPU_GPU_FLAGS_SUPPORT_FAULT_RECOVERY				NVGPU_BIT64(33)

/**
 * @brief Mapping modification is supported.
 *
 * @sa @ref NVGPURM_INTF_AS_MAPPING_MODIFY
 */
#define NVGPU_GPU_FLAGS_SUPPORT_MAPPING_MODIFY				NVGPU_BIT64(34)

/**
 * @brief Remap of mapping is supported.
 *
 * @sa @ref NVGPURM_INTF_AS_REMAP
 */
#define NVGPU_GPU_FLAGS_SUPPORT_REMAP					NVGPU_BIT64(35)

/**
 * @brief Graphics compression is supported.
 *
 * @sa @ref NVGPURM_INTF_REGISTER_BUFFER
 * @sa @ref NVGPURM_INTF_AS_MAP_BUFFER_EX
 */
#define NVGPU_GPU_FLAGS_SUPPORT_COMPRESSION				NVGPU_BIT64(36)

/**
 * @brief SM tree traversal unit (TTU) is enabled
 *
 * @remark Not supported in safety.
 */
#define NVGPU_GPU_FLAGS_SUPPORT_SM_TTU					NVGPU_BIT64(37)

/**
 * @brief Post-L2 compression is supported.
 *
 * @sa @ref NVGPURM_INTF_REGISTER_BUFFER
 * @sa @ref NVGPURM_INTF_AS_MAP_BUFFER_EX
 */
#define NVGPU_GPU_FLAGS_SUPPORT_POST_L2_COMPRESSION			NVGPU_BIT64(38)

/**
 * @brief Indicates support for buffer mapping access flags other than the default.
 *
 * @sa @c #NVGPU_AS_MAP_BUFFER_ACCESS_READ_ONLY
 * @sa @c #NVGPU_AS_MAP_BUFFER_ACCESS_READ_WRITE
 */
#define NVGPU_GPU_FLAGS_SUPPORT_MAP_ACCESS_TYPE				NVGPU_BIT64(39)

/**
 * @brief Indicates support for the TWOD (2d) graphics class.
 *
 * @sa @ref NVGPURM_INTF_ALLOC_OBJ_CTX
 */
#define NVGPU_GPU_FLAGS_SUPPORT_2D					NVGPU_BIT64(40)

/**
 * @brief Indicates support for the THREED (3d) graphics class.
 *
 * @sa @ref NVGPURM_INTF_ALLOC_OBJ_CTX
 */
#define NVGPU_GPU_FLAGS_SUPPORT_3D					NVGPU_BIT64(41)

/**
 * @brief Indicates support for the COMPUTE class.
 *
 * @sa @ref NVGPURM_INTF_ALLOC_OBJ_CTX
 */
#define NVGPU_GPU_FLAGS_SUPPORT_COMPUTE					NVGPU_BIT64(42)

/**
 * @brief Indicates support for the I2M (inline-to-memory) class.
 *
 * @sa @ref NVGPURM_INTF_ALLOC_OBJ_CTX
 */
#define NVGPU_GPU_FLAGS_SUPPORT_I2M					NVGPU_BIT64(43)

/**
 * @brief Indicates support for Zero-Bandwidth Clear (ZBC).
 *
 * In safety, chip-specific static tables are used. This is indicated by @c
 * #NVGPU_GPU_FLAGS_DRIVER_REDUCED_PROFILE.
 *
 * @remark See https://nvbugs/3585766/53 for static table values for GA10B.
 */
#define NVGPU_GPU_FLAGS_SUPPORT_ZBC					NVGPU_BIT64(44)

/**
 * @brief Profiler V2 device objects are supported.
 *
 * @remark Not supported in safety
 *
 * @sa @c #NVGPU_PROFILER_IOCTL_RESERVE_PM_RESOURCE
 */
#define NVGPU_GPU_FLAGS_SUPPORT_PROFILER_V2_DEVICE			NVGPU_BIT64(46)

/**
 * @brief Profiler V2 context objects are supported.
 *
 * @remark Not supported in safety
 *
 * @sa @c #NVGPU_PROFILER_IOCTL_BIND_CONTEXT
 */
#define NVGPU_GPU_FLAGS_SUPPORT_PROFILER_V2_CONTEXT			NVGPU_BIT64(47)

/**
 * @brief Profiling SMPC in global mode is supported.
 *
 * @remark Not supported in safety
 *
 * @sa @c #NVGPU_DBG_GPU_IOCTL_SMPC_CTXSW_MODE
 */
#define NVGPU_GPU_FLAGS_SUPPORT_SMPC_GLOBAL_MODE			NVGPU_BIT64(48)

/**
 * @brief Retrieving contents of graphics context is supported.
 */
#define NVGPU_GPU_FLAGS_SUPPORT_GET_GR_CONTEXT				NVGPU_BIT64(49)

/**
 * @brief Additional buffer metadata association supported.
 *
 * @remark Not supported in safety
 *
 * @sa @c #nvgpu_gpu_register_buffer_args::metadata_size
 */
#define NVGPU_GPU_FLAGS_SUPPORT_BUFFER_METADATA				NVGPU_BIT64(50)

/**
 * @brief Indicates support for configuring L2_MAXEVICTLAST_WAYS.
 *
 * @remark Not supported in safety
 *
 * @sa @c #NVGPU_TSG_IOCTL_SET_L2_MAX_WAYS_EVICT_LAST
 */
#define NVGPU_GPU_FLAGS_L2_MAX_WAYS_EVICT_LAST_ENABLED			NVGPU_BIT64(51)

/**
 * @brief Indicates support for VIDMEM Access Bits (VAB).
 *
 * @remark Not supported in safety
 *
 * @sa @c #NVGPU_PROFILER_IOCTL_VAB_RESERVE
 */
#define NVGPU_GPU_FLAGS_SUPPORT_VAB					NVGPU_BIT64(52)

/**
 * @brief The NVS scheduler real-time control interface is supported.
 *
 * @sa @c #NVGPU_NVS_CTRL_FIFO_IOCTL_QUERY_SCHEDULER_CHARACTERISTICS
 */
#define NVGPU_GPU_FLAGS_SUPPORT_NVS					NVGPU_BIT64(53)

/**
 * @brief Virtual function is supported.
 */
#define NVGPU_GPU_FLAGS_IS_VIRTUAL_FUNCTION				NVGPU_BIT64(54)

/**
 * @brief Indicates support for implicit ERRBAR.
 *
 * @sa @c #NVGPU_DBG_GPU_IOCTL_SET_SCHED_EXIT_WAIT_FOR_ERRBAR
 */
#define NVGPU_GPU_FLAGS_SCHED_EXIT_WAIT_FOR_ERRBAR_SUPPORTED		NVGPU_BIT64(55)

/**
 * @brief Indicates support for multiprocess TSG sharing.
 *
 * @remark Multiprocess TSG sharing is not available in safety.
 *
 * @sa @ref NVGPURM_INTF_TSG_OPEN_TSG
 * @sa @ref NVGPURM_INTF_TSG_GET_SHARE_TOKEN
 * @sa @ref NVGPURM_INTF_TSG_REVOKE_SHARE_TOKEN
 */
#define NVGPU_GPU_FLAGS_MULTI_PROCESS_TSG_SHARING			NVGPU_BIT64(56)

/**
 * @brief Flag to indicate GPU MMIO support
 */
#define NVGPU_GPU_FLAGS_SUPPORT_GPU_MMIO	(1ULL << 57)

/**
 * @brief Flag to indicate KMD submit uses semaphore-based GP_GET tracking
 */
#define NVGPU_GPU_FLAGS_SUPPORT_SEMA_BASED_GP_GET_TRACKING	NVGPU_BIT64(58)

/**
 * @brief Flag to indicate device is virtualized GPU
 */
#define NVGPU_GPU_FLAGS_IS_VIRTUALIZATION (1ULL << 59)

/**
 * @brief Indicates that SM LRF ECC is enabled. See remarks.
 *
 * @remark ECC status of individual units is no longer reported. In addition to
 * SM LRF, this flag indicates that ECC is enabled in general. See the
 * chip-specific POR documents for the exact list of ECC-enabled GPU units.
 *
 * @remark ECC is always enabled in safety.
 */
#define NVGPU_GPU_FLAGS_ECC_ENABLED_SM_LRF				NVGPU_BIT64(60)

/**
 * @brief Indicates that SM SHM ECC is enabled. See remarks.
 *
 * @remark ECC status of individual units is no longer reported. In addition to
 * SM LRF, this flag indicates that ECC is enabled in general. See the
 * chip-specific POR documents for the exact list of ECC-enabled GPU units.
 *
 */
#define NVGPU_GPU_FLAGS_ECC_ENABLED_SM_SHM	(1ULL << 61)

/**
 * @brief Indicates that TEX ECC is enabled. See remarks.
 *
 * @remark ECC status of individual units is no longer reported. In addition to
 * SM LRF, this flag indicates that ECC is enabled in general. See the
 * chip-specific POR documents for the exact list of ECC-enabled GPU units.
 *
 */
#define NVGPU_GPU_FLAGS_ECC_ENABLED_TEX		(1ULL << 62)

/**
 * @brief Indicates that GPU L2 ECC is enabled. See remarks
 *
 * @remark ECC status of individual units is no longer reported. In addition to
 * L2, this flag indicates that ECC is enabled in general. See the
 * chip-specific POR documents for the exact list of ECC-enabled GPU units.
 *
 * @remark ECC is always enabled in safety.
 */
#define NVGPU_GPU_FLAGS_ECC_ENABLED_LTC					NVGPU_BIT64(63)

/**
 * @brief All ECC support flags
 */
#define NVGPU_GPU_FLAGS_ALL_ECC_ENABLED	\
				(NVGPU_GPU_FLAGS_ECC_ENABLED_SM_LRF |	\
				NVGPU_GPU_FLAGS_ECC_ENABLED_SM_SHM |	\
				NVGPU_GPU_FLAGS_ECC_ENABLED_TEX    |	\
				NVGPU_GPU_FLAGS_ECC_ENABLED_LTC)

/**
 * @brief Moniker for no NUMA info
 *
 * @sa @c nvgpu_gpu_characteristics::numa_domain_id
 */
#define NVGPU_GPU_CHARACTERISTICS_NO_NUMA_INFO (-1)

/** @} */

/**
 * @ingroup NVGPURM_INTF_GET_CHARACTERISTICS
 * @brief The main configuration data for the GPU hardware and the resource
 * manager.
 */
struct nvgpu_gpu_characteristics {

	/**
	 * @brief GPU chip ID architecture field (shifted left by 4)
	 *
	 * Bit mask range: @c 0x1F0
	 *
	 * @remark The chip ID architecture value for GA10B is @c 0x170U.
	 * @remark The chip ID architecture value for GB10B is @c 0x1A0U.
	 *
	 * @remark The GPU chip ID can be reconstructed by:
	 * @code
	 * gpuId = arch | impl;
	 * @endcode
	 */
	__u32 arch;

	/**
	 * @brief GPU chip ID implementation field
	 *
	 * Bit mask range: @c 0xF
	 *
	 * @remark The chip ID implementation value for GA10B is @c 0xB.
	 * @remark The chip ID implementation value for GB10B is @c 0xB.
	 *
	 * @remark The GPU chip ID can be reconstructed by:
	 * @code
	 * gpuId = arch | impl;
	 * @endcode
	 */
	__u32 impl;

	/**
	 * @brief GPU HW major revision field
	 *
	 * Bit mask range: @c 0xF
	 */
	__u32 rev;

	/**
	 * @brief Number of available GPC units.
	 *
	 * Range: [1, @c NV_SCAL_LITTER_NUM_GPCS]
	 *
	 * @sa HW litters for GA10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
	 *
	 * @sa HW litters for GB10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
	 */
	__u32 num_gpc;

	/**
	 * @brief NUMA domain of the GPU device
	 *
	 * Value @c #NVGPU_GPU_CHARACTERISTICS_NO_NUMA_INFO means no NUMA domain
	 * info.
	 */
	__s32 numa_domain_id;

	/**
	 * @brief GPU L2 cache size (bytes)
	 *
	 * Range is HW-specific.
	 *
	 * On Ampere+:
	 * - Range in general is <tt>[1 * L2_slice_size, N_max * L2_slice_bytes]</tt>
	 *   in increments of @c L2_slice_bytes.
	 * - @c L2_slice_bytes = @c NV_SCAL_LITTER_NUM_LTC_LTS_SETS *
	 *   @c NV_SCAL_LITTER_NUM_LTC_LTS_WAYS * @c NV_CHIP_LTC_LTS_BYTES_PER_LINE
	 * - @c N_max = @c NV_SCAL_LITTER_NUM_FBPS * @c
	 *   NV_SCAL_LITTER_NUM_LTC_PER_FBP * @c NV_SCAL_LITTER_NUM_SLICES_PER_LTC
	 *
	 * In QNX, L2 floorsweeping is currently supported at FBP level only. Thus,
	 * full configuration within enabled FBPs is always assumed.
	 *
	 * @remark The possible values for GA10B are: 2097152, 4194304
	 *
	 * @sa @c #fbp_en_mask.
	 *
	 * @sa HW litters for GA10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
	 *
	 * @sa HW litters for GB10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
	 */
	__u64 L2_cache_size;

	/**
	 * @brief GPU local memory size (dGPU only).
	 *
	 * @remark In safety, this value is currently always 0.
	 */
	__u64 on_board_video_memory_size;  /* bytes */

	/**
	 * @brief The number of TPC units per GPC before floorsweeping.
	 *
	 * Same as litter @c NV_SCAL_LITTER_NUM_TPC_PER_GPC.
	 *
	 * @sa HW litters for GA10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
	 *
	 * @sa HW litters for GB10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
	 */
	__u32 num_tpc_per_gpc;

	/**
	 * @brief GPU bus type (iGPUs only)
	 *
	 * For iGPU/silicon, the bus type is currently always @c
	 * #NVGPU_GPU_BUS_TYPE_AXI.
	 *
	 * For dGPUs, this field is ignored. The bus type is determined by the
	 * device node directory in the file system.
	 */
	__u32 bus_type;

	/**
	 * @brief Default big page size (in bytes) or 0 if big pages are not
	 * supported.
	 *
	 * @remark Instead of relying in this field, usermode should use @c
	 * #available_big_page_sizes and specify the desired big page size in @c
	 * #nvgpu_alloc_as_args::big_page_size when creating an address space.
	 */
	__u32 big_page_size;

	/**
	 * @brief Default huge page size (in bytes) or 0 if huge pages are not
	 * supported.
	 *
	 * @remark For Blackwell, huge page size is 2097152.
	 */
	__u32 huge_page_size;

	/**
	 * @brief Compression page size (in bytes) or 0 if compression is not
	 * supported.
	 *
	 * Compression page size denotes the coverage of a compression tag
	 * line. When mapping, compressible data should be considered as if using
	 * the compression page size, regardless of the actual GMMU page size. This
	 * applies to alignments of mapping addresses (GPU VA), mapping sizes, and
	 * buffer offsets. Violating the alignment may cause compression-related
	 * corruption.
	 *
	 * @remark Compression page size is 128 KB for Maxwell 2 and 64 KB for Pascal+, 2 MB
	 * for GB10B.
	 */
	__u32 compression_page_size;

	/**
	 * @brief Log<sub>2</sub> of the last level PDE size (in bytes).
	 *
	 * Last-level PDE size specifies the granularity for mixing small and big
	 * pages for fixed-address mappings. That is, the last level PDE may only
	 * contain mappings of the same page size.
	 *
	 * For GA10B and GB10B the value is 21, denoting the last level PDE size of 2097152
	 * bytes.
	 */
	__u32 pde_coverage_bit_count;

	/**
	 * @brief Bit mask of available big page sizes (in bytes) or 0 if no big
	 * pages are available.
	 *
	 * When mask bit @c N is set, then big page size 2<sup>@c N </sup> is
	 * supported for @c #nvgpu_alloc_as_args::big_page_size.
	 *
	 * For example, value @c 0x30000 (= <tt>0x20000 | 0x10000</tt>) denotes that
	 * big page sizes 131072 (@c 0x20000) and 65536 (@c 0x10000) are available.
	 *
	 * @remark Since Pascal, the only available big page size is 65536.
	 *
	 * @remark Big pages are not available for GA10B in safety.
	 */
	__u32 available_big_page_sizes;

	/**
	 * @brief Device and driver capability flags. The flags use prefix @c
	 * NVGPU_GPU_FLAGS_SUPPORT_.
	 */
	__u64 flags;

	/**
	 * @brief TWOD (2d) class number.
	 */
	__u32 twod_class;

	/**
	 * @brief THREED (3d) class number.
	 */
	__u32 threed_class;

	/**
	 * @brief COMPUTE class number.
	 */
	__u32 compute_class;

	/**
	 * @brief GPFIFO class number
	 */
	__u32 gpfifo_class;

	/**
	 * @brief I2M (inline-to-memory) class number
	 */
	__u32 inline_to_memory_class;

	/**
	 * @brief DMA copy engine class.
	 */
	__u32 dma_copy_class;

	/**
	 * @brief Bit mask of enabled GPC units (physical identifiers).
	 *
	 * @remark To be revised for MIG.
	 */
	__u32 gpc_mask; /* enabled GPCs */

	/**
	 * @brief SM core version (aka marketing version)
	 *
	 * @remark GA10B SM core version is 0x807
	 * @remark GB10B SM core version is 0xA01
	 */
	__u32 sm_arch_sm_version;

	/**
	 * @brief SM instruction set version.
	 *
	 * @remark GA10B SM SPA version is 0x806
	 * @remark GB10B SM core version is 0xA01
	 */
	__u32 sm_arch_spa_version;

	/**
	 * @brief Warp count per TPC.
	 *
	 * @note This field is imprecisely named for historical reasons. The naming
	 * dates back to when there was a single SM per TPC. Starting from Volta,
	 * there are 2 SMs per TPC.
	 *
	 * @remark GA10B and GB10B warp count per TPC is 96. Warp count per SM is 48.
	 */
	__u32 sm_arch_warp_count;

	/**
	 * @brief Last IOCTL command number for @ref NVGPURM_LINUX_IOCTL_CTRL
	 *
	 * @remark -1 if not supported
	 */
	__s16 gpu_ioctl_nr_last;
	/**
	 * @brief Last IOCTL command number for @ref NVGPURM_LINUX_IOCTL_TSG
	 *
	 * @remark -1 if not supported
	 */
	__s16 tsg_ioctl_nr_last;
	/**
	 * @brief Last IOCTL command number for @ref NVGPURM_LINUX_IOCTL_DBG
	 *
	 * @remark -1 if not supported
	 */
	__s16 dbg_gpu_ioctl_nr_last;
	/**
	 * @brief Last IOCTL command number for @ref NVGPURM_LINUX_IOCTL_CHANNEL
	 *
	 * @remark -1 if not supported
	 */
	__s16 ioctl_channel_nr_last;
	/**
	 * @brief Last IOCTL command number for @ref NVGPURM_LINUX_IOCTL_AS
	 *
	 * @remark -1 if not supported
	 */
	__s16 as_ioctl_nr_last;

	/**
	 * @brief Log<sub>2</sub> of the usable GPU VA size including nvgpu-rm
	 * private region.
	 *
	 * @remark In QNX safety, this value is 40 (0x28) denoting the
	 * 2<sup>40</sup> usable limit, including the nvgpu-rm private region.
	 *
	 * @sa @c #nvgpu_alloc_as_args::va_range_end
	 */
	__u8 gpu_va_bit_count;

	/**
	 * @brief Reserved
	 */
	__u8 reserved;

	/**
	 * @brief The number of FBP units before floorsweeping.
	 *
	 * Value sourced from litter @c NV_SCAL_LITTER_NUM_FBPS.
	 *
	 * @sa HW litters for GA10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
	 *
	 * @sa HW litters for GB10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
	 */
	__u32 max_fbps_count;

	/**
	 * @brief Bit mask of enabled FBP units (physical identifiers).
	 */
	__u32 fbp_en_mask;

	/**
	 * @brief Bit mask of enabled SoC EMC memory channel groups. (iGPU only)
	 *
	 * The iGPU shares the memory bus with SoC. Therefore, the iGPU memory bus
	 * width is determined by the SoC bus width.
	 *
	 * @remark On T234/GA10B, the memory bus width is:
	 * <tt>64 * popcount(#emc_en_mask)</tt> (in bits).
	 *
	 * @sa https://nvtegra/home/tegra_manuals/html/manuals/t234/arfuse.html
	 */
	__u32 emc_en_mask;

	/**
	 * @brief The number of LTC units per FBP before floorsweeping.
	 *
	 * Same as litter @c NV_SCAL_LITTER_NUM_LTC_PER_FBP.
	 *
	 * @sa HW litters for GA10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
	 *
	 * @sa HW litters for GB10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
	 */
	__u32 max_ltc_per_fbp;

	/**
	 * @brief The number of LTS units per LTC before floorsweeping.
	 *
	 * Same as litter @c NV_SCAL_LITTER_NUM_LTC_PER_FBP.
	 *
	 * @sa HW litters for GA10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
	 *
	 * @sa HW litters for GB10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
	 */
	__u32 max_lts_per_ltc;

	/**
	 * @brief The number of TEX units per TPC before floorsweeping.
	 *
	 * Same as chip constant @c NV_CHIP_TEX_PER_TPC.
	 */
	__u32 max_tex_per_tpc;

	/**
	 * @brief The number of GPC units before floorsweeping.
	 *
	 * Same as litter @c NV_SCAL_LITTER_NUM_GPCS.
	 *
	 * @sa HW litters for GA10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
	 *
	 * @sa HW litters for GB10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
	 */
	__u32 max_gpc_count;

	/** @brief Unused (deprecated) */
	__u32 rop_l2_en_mask_DEPRECATED[2];

	/**
	 * @brief Short name of the GPU chip (only use for internal tests)
	 */
	__u8 chipname[8];

	/** @brief Unused (deprecated) */
	__u64 gr_compbit_store_base_hw;

	/** @brief Unused (deprecated) */
	__u32 gr_gobs_per_comptagline_per_slice;

	/**
	 * @brief Number of enabled LTC units
	 *
	 * @sa @c #L2_cache_size
	 */
	__u32 num_ltc;

	/**
	 * @brief The number of LTS units per LTC before floorsweeping
	 *
	 * Same as litter @c NV_SCAL_LITTER_NUM_SLICES_PER_LTC.
	 *
	 * @sa HW litters for GA10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
	 *
	 * @sa HW litters for GB10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
	 */
	__u32 lts_per_ltc;

	/**
	 * @brief Unused (deprecated)
	 */
	__u32 cbc_cache_line_size;

	/**
	 * @brief Unused (deprecated)
	 */
	__u32 cbc_comptags_per_line;

	/**
	 * @brief Upper limit for the sum of nvgpu_as_map_buffer_batch_args::unmaps
	 * and nvgpu_as_map_buffer_batch_args::maps per single call.
	 *
	 * @remark @c NVGPU_AS_IOCTL_MAP_BUFFER_BATCH is not supported in safety
	 */
	__u32 map_buffer_batch_limit;

	/**
	 * @brief Maximum GPU frequency (in Hz)
	 */
	__u64 max_freq;

	/**
	 * @brief Supported graphics preemption modes (bit mask)
	 *
	 * Possible mask bits:
	 * - @c #NVGPU_GRAPHICS_PREEMPTION_MODE_WFI
	 * - @c #NVGPU_GRAPHICS_PREEMPTION_MODE_GFXP (not in safety)
	 */
	__u32 graphics_preemption_mode_flags;

	/**
	 * @brief Supported compute preemption modes (bit mask)
	 *
	 * Possible mask bits:
	 * - @c #NVGPU_COMPUTE_PREEMPTION_MODE_WFI
	 * - @c #NVGPU_COMPUTE_PREEMPTION_MODE_CTA
	 * - @c #NVGPU_COMPUTE_PREEMPTION_MODE_CILP (not in safety)
	 */
	__u32 compute_preemption_mode_flags;

	/**
	 * @brief Default graphics preemption mode
	 *
	 * @sa @ref NVGPURM_INTF_SET_PREEMPTION_MODE
	 */
	__u32 default_graphics_preempt_mode;

	/**
	 * @brief Default compute preemption mode
	 *
	 * @sa @ref NVGPURM_INTF_SET_PREEMPTION_MODE
	 */
	__u32 default_compute_preempt_mode;

	/**
	 * @brief On-board memory size (in bytes). Only applicable to dGPUs.
	 */
	__u64 local_video_memory_size;

    /* These are meaningful only for PCI devices */

	/**
	 * @brief PCI vendor id (only applicable to dGPUs/iGPUs connected to pci bus)
	 *
	 * @sa NVGPURM_LINUX_IOCTL_CTRL / Device type
	 */
	__u16 pci_vendor_id;

	/**
	 * @brief PCI device id (only applicable to dGPUs/iGPUs connected to pci bus)
	 *
	 * @sa NVGPURM_LINUX_IOCTL_CTRL / Device type
	 */
	__u16 pci_device_id;

	/**
	 * @brief PCI subsystem vendor id (only applicable to dGPUs/iGPUs connected to pci bus)
	 *
	 * @sa NVGPURM_LINUX_IOCTL_CTRL / Device type
	 */
	__u16 pci_subsystem_vendor_id;

	/**
	 * @brief PCI subsystem device id (only applicable to dGPUs/iGPUs connected to pci bus)
	 *
	 * @sa NVGPURM_LINUX_IOCTL_CTRL / Device type
	 */
	__u16 pci_subsystem_device_id;

	/**
	 * @brief PCI class (only applicable to dGPUs/iGPUs connected to pci bus)
	 *
	 * @sa NVGPURM_LINUX_IOCTL_CTRL / Device type
	 */
	__u16 pci_class;

	/**
	 * @brief PCI revision (only applicable to dGPUs/iGPUs connected to pci bus)
	 *
	 * @sa NVGPURM_LINUX_IOCTL_CTRL / Device type
	 */
	__u8  pci_revision;

	/**
	 * @brief VBIOS OEM version (dGPU only).
	 */
	__u8  vbios_oem_version;

	/**
	 * @brief VBIOS version (dGPU only).
	 */
	__u32 vbios_version;

	/* NVGPU_DBG_GPU_IOCTL_REG_OPS: the upper limit for the number
	 * of regops */
	__u32 reg_ops_limit;
	__u32 reserved1;

	__s16 event_ioctl_nr_last;
	__u16 pad;

	__u32 max_css_buffer_size;

	__s16 ctxsw_ioctl_nr_last;
	__s16 prof_ioctl_nr_last;
	__s16 nvs_ioctl_nr_last;
	__u8 reserved2[2];

	__u32 max_ctxsw_ring_buffer_size;
	__u32 reserved3;

	/**
	 * @brief Per-device identifier (PDI).
	 *
	 * Per-device identifier is a unique identifier for a physical device. MIG
	 * partitions of the same physical device share the PDI. Virtualized
	 * environments may hide the physical identifier by replacing it with a
	 * virtualized one.
	 *
	 * @remark On automotive platforms, PDI is guaranteed to be unique across
	 * all GPUs attached to the system for interoperability purposes. However,
	 * the identifiers may be non-unique between different systems to hide
	 * personally identifiable information (PII).
	 */
	__u64 per_device_identifier;

	/**
	 * @brief Pixel Processing clusters per GPC.
	 *
	 * Same as litter @c NV_SCAL_LITTER_NUM_PES_PER_GPC.
	 *
	 * @sa HW litters for GA10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
	 *
	 * @sa HW litters for GB10B are found in
	 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
	 */
	__u32 num_ppc_per_gpc;

	/**
	 * @brief Maximum number of subcontexts per TSG.
	 *
	 * @sa @ref NVGPURM_INTF_TSG_CREATE_SUBCONTEXT
	 */
	__u32 max_veid_count_per_tsg;

	/**
	 * @brief Sub-partition per FBPA (only applicable to dGPUs)
	 *
	 * @sa NVGPURM_LINUX_IOCTL_CTRL / Device type
	 */
	__u32 num_sub_partition_per_fbpa;

	/**
	 * @brief MIG GPU instance id
	 *
	 * The GPU instance specifies the GPU resource partitioning identifier in
	 * fractional GPU configuration.
	 *
	 * This is the swizzle id on Ampere+ MIG devices, which specifies the
	 * resources (GPC/LTC) for the GPU instance.
	 *
	 * @remark The value is @c 0 for a non-MIG device (legacy mode)
	 */
	__u32 gpu_instance_id;

	/**
	 * @brief MIG GR/compute instance id within @c #gpu_instance_id
	 *
	 * On Ampere+, this is the MIG GR/compute SYS pipe id.
	 *
	 * @remark The value is @c 0 for a non-MIG device (legacy mode)
	 */
	__u32 gr_instance_id;

	/**
	 * @brief Maximum GP FIFO size (in entries)
	 *
	 * @sa @c #nvgpu_alloc_gpfifo_ex_args::num_entries
	 */
	__u32 max_gpfifo_entries;

	/**
	 * @brief Maximum timeslice value allowed for @c
	 * #NVGPU_DBG_GPU_IOCTL_TSG_SET_TIMESLICE.
	 */
	__u32 max_dbg_tsg_timeslice;
	/**
	 * @brief Reserved
	 */
	__u32 reserved5;

	/**
	 * @brief Unique identifier for an opened GPU device control node.
	 *
	 * This is unique within the lifetime of the underlying nvgpu
	 * resource manager process.
	 *
	 * @sa @ref NVGPURM_INTF_TSG_GET_SHARE_TOKEN
	 */
	__u64 device_instance_id;

	/* Notes:
	   - This struct can be safely appended with new fields. However, always
	     keep the structure size multiple of 8 and make sure that the binary
	     layout does not change between 32-bit and 64-bit architectures.
	   - If the last field is reserved/padding, it is not
	     generally safe to repurpose the field in future revisions.
	*/
};

/**
 * @ingroup NVGPURM_INTF_GET_CHARACTERISTICS
 * @brief Header struct for #NVGPU_GPU_IOCTL_GET_CHARACTERISTICS.
 */
struct nvgpu_gpu_get_characteristics {
	/**
	 * @brief The characteristics buffer size (in bytes, <tt>sizeof
	 * #nvgpu_gpu_characteristics</tt>).
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Size of the buffer reserved by the caller.
	 *   </dd>
	 *   <dt>[out]</dt>
	 *   <dd>
	 *     Full buffer size as known by the resource manager. This
	 *     equals to <tt>sizeof #nvgpu_gpu_characteristics</tt>.
	 *   </dd>
	 * </dl>
	 *
	 * @remark This field is for Linux compatibility.
	 */
	__u64 gpu_characteristics_buf_size;

	/**
	 * @brief address of nvgpu_gpu_characteristics buffer
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Filled with field values by exactly MIN(buf_size_in, buf_size_out) bytes.
	 *     Ignored, if @c gpu_characteristics_buf_size is zero.
	 *   </dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u64 gpu_characteristics_buf_addr;
};

#define NVGPU_GPU_COMPBITS_NONE		0
#define NVGPU_GPU_COMPBITS_GPU		(1 << 0)
#define NVGPU_GPU_COMPBITS_CDEH		(1 << 1)
#define NVGPU_GPU_COMPBITS_CDEV		(1 << 2)

struct nvgpu_gpu_prepare_compressible_read_args {
	__u32 handle;			/* in, dmabuf fd */
	union {
		__u32 request_compbits;	/* in */
		__u32 valid_compbits;	/* out */
	};
	__u64 offset;			/* in, within handle */
	__u64 compbits_hoffset;		/* in, within handle */
	__u64 compbits_voffset;		/* in, within handle */
	__u32 width;			/* in, in pixels */
	__u32 height;			/* in, in pixels */
	__u32 block_height_log2;	/* in */
	__u32 submit_flags;		/* in (NVGPU_SUBMIT_GPFIFO_FLAGS_) */
	union {
		struct {
			__u32 syncpt_id;
			__u32 syncpt_value;
		};
		__s32 fd;
	} fence;			/* in/out */
	__u32 zbc_color;		/* out */
	__u32 reserved;		/* must be zero */
	__u64 scatterbuffer_offset;	/* in, within handle */
	__u32 reserved2[2];		/* must be zero */
};

struct nvgpu_gpu_mark_compressible_write_args {
	__u32 handle;			/* in, dmabuf fd */
	__u32 valid_compbits;		/* in */
	__u64 offset;			/* in, within handle */
	__u32 zbc_color;		/* in */
	__u32 reserved[3];		/* must be zero */
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 * @defgroup NVGPURM_INTF_ALLOC_AS IOCTL: NVGPU_GPU_IOCTL_ALLOC_AS
 * @brief Initializes an address space.
 *
 * This IOCTL is used to configure and initialize an address space. This IOCTL
 * internally opens an address space node and returns the file descriptor to
 * the caller @c #nvgpu_alloc_as_args::as_fd. On this file descriptor
 * regular ioctls of @ref NVGPURM_LINUX_IOCTL_AS group are issued.
 *
 * After initialization, the address space layout is as follows:
 *
 * <table><caption>GPU VA space layout</caption>
 *   <tr>
 *     <th>Start address (inclusive)</th>
 *     <th>End address (exclusive)</th>
 *     <th>Description</th>
 *   </tr>
 *   <tr>
 *     <td>2<sup>49</sup></td><td>2<sup>64</sup></td>
 *     <td>Reserved (out of HW range)</td>
 *   </tr>
 *   <tr>
 *     <td>2<sup>40</sup></td><td>2<sup>49</sup></td>
 *     <td>Reserved (high 2). On Tegra, GPU addresses from 2<sup>40</sup> and
 *         above are not supported. This is because of the following reasons:
 *         -# The kernel private region must reside under the 2<sup>40</sup>
 *            limit due to hardware constraints: HW address fields for buffers
 *            mapped in the private region have not been extended for the full
 *            2<sup>49</sup> address range.
 *         -# Address spaces with a gap for the private region are not
 *            implemented.
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>@c #nvgpu_alloc_as_args::va_range_end + 2<sup>32</sup></td><td>
 *          2<sup>40</sup></td>
 *     <td>Reserved (high 1). May be zero size.</td>
 *   </tr>
 *   <tr>
 *     <td>@c #nvgpu_alloc_as_args::va_range_end</td><td>
 *         @c #nvgpu_alloc_as_args::va_range_end + 2<sup>32</sup></td>
 *     <td>Kernel private region for channel context buffer mappings, syncpoint
 *         shim mappings, etc</td>
 *   </tr>
 *   <tr>
 *     <td>@c #nvgpu_alloc_as_args::va_range_start</td><td>
 *         @c #nvgpu_alloc_as_args::va_range_end</td>
 *     <td>User region for @ref NVGPURM_INTF_AS_MAP_BUFFER_EX,
 *         @ref NVGPURM_INTF_AS_ALLOC_SPACE</td>
 *   </tr>
 *   <tr>
 *     <td>0</td><td>@c #nvgpu_alloc_as_args::va_range_start</td>
 *     <td>Reserved (low). Minimum size: lowest-level PDE size. (See @c
 *         #nvgpu_gpu_characteristics::pde_coverage_bit_count)</td>
 *   </tr>
 * </table>
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_alloc_as_args</td>
 *   <td>(none)</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c ENOMEM ---  Insufficient memory
 * - @c EINVAL ---  Invalid argument
 * - @c ENODEV ---  GPU not ready
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @remark Historically, NvGPU KMD/RM used to split the address space in two:
 * the lower part used small pages and the upper part used big pages. However,
 * the modern implementation supports only unified address spaces, meaning
 * that there is no split between page sizes.
 */


/**
 * @ingroup NVGPURM_INTF_ALLOC_AS
 * @brief Specifies unified address space. That is, there is no small/big page
 * split.
 */
#define NVGPU_GPU_IOCTL_ALLOC_AS_FLAGS_UNIFIED_VA          (1U << 1)

/**
 * @ingroup NVGPURM_INTF_ALLOC_AS
 * @brief Header struct for @ref NVGPURM_INTF_ALLOC_AS
 */
struct nvgpu_alloc_as_args {
	/**
	 * @brief Big page size for the address space or 0 if big pages are not
	 * used.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Big page size</dd>
	 * </dl>
	 *
	 * Range: 0 or a big page size from @c
	 * #nvgpu_gpu_characteristics::available_big_page_sizes.
	 */
	__u32 big_page_size;

	/**
	 * @brief File descriptor of address space
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Ignore</dd>
	 *   <dt>[out]</dt><dd>Address space file descriptor</dd>
	 * </dl>
	 */
	__s32 as_fd;

	/**
	 * @brief Address space configuration flags
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Address space configuration flags. Must include @c
	 *     #NVGPU_GPU_IOCTL_ALLOC_AS_FLAGS_UNIFIED_VA</dd>
	 * </dl>
	 *
	 * Range: Combination of @c NVGPU_AS_ALLOC_AS_FLAGS_*. Must include @c
	 * #NVGPU_GPU_IOCTL_ALLOC_AS_FLAGS_UNIFIED_VA.
	 */
	__u32 flags;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;

	/**
	 * @brief Start address for the user region (inclusive).
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>User region start address</dd>
	 * </dl>
	 *
	 * Range:
	 * - Minimum value: lowest-level PDE size (See @c
	 *   #nvgpu_gpu_characteristics::pde_coverage_bit_count)
	 * - Multiple of lowest-level PDE size
	 * - Less than @c #va_range_end.
	 */
	__u64 va_range_start;

	/**
	 * @brief End address for the user region (exclusive).
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>User region end address</dd>
	 * </dl>
	 *
	 * Range:
	 * - Maximum value: (2<sup>40</sup> - 2<sup>32</sup>)
	 * - Multiple of lowest-level PDE size (See @c
	 *   #nvgpu_gpu_characteristics::pde_coverage_bit_count)
	 * - Greater than @c #va_range_start.
	 */
	__u64 va_range_end;

	/**
	 * @brief Small/big page split (legacy, unsupported). Must be 0 for no
	 * split.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Split address. Unsupported.</dd>
	 * </dl>
	 *
	 * Range: 0
	 */
	__u64 va_range_split;

	/**
	 * @brief Padding
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 padding[6];
};

/*
 * Size of share token.
 *
 * With multi-process TSG sharing, this token is created by
 * concatenating: 64b source device instance identifier and
 * 64b token that identifies the TSG sharing between the
 * source and target device instances.
 */
#define NVGPU_TSG_SHARE_TOKEN_SIZE		((size_t)16U)

/*
 * NVGPU_GPU_IOCTL_OPEN_TSG - create/share TSG ioctl.
 *
 * This IOCTL allocates one of the available TSG for user when called
 * without share token specified. When called with share token specified,
 * fd is created for already allocated TSG for sharing the TSG under
 * different device/CTRL object hierarchies in different processes.
 *
 * Source device is specified in the arguments and target device is
 * implied from the caller. Share token is unique for a TSG.
 *
 * When the TSG is successfully created first time or is opened with share
 * token, the device instance id associated with the CTRL fd will be added
 * to the TSG private data structure as authorized device instance ids.
 * This is used for a security check when creating a TSG share token with
 * nvgpu_tsg_get_share_token.
 *
 * return 0 on success, -1 on error.
 * retval EINVAL if invalid parameters are specified (if TSG_FLAGS_SHARE
 *               is set but source_device_instance_id and/or share token
 *               are zero or TSG_FLAGS_SHARE is not set but other
 *               arguments are non-zero).
 * retval EINVAL if share token doesn't exist or is expired.
 */

/*
 * Specify that the newly created TSG fd will map to existing hardware
 * TSG resources.
 */
#define NVGPU_GPU_IOCTL_OPEN_TSG_FLAGS_SHARE	((__u32)1U << 0U)

/* Arguments for NVGPU_GPU_IOCTL_OPEN_TSG */
struct nvgpu_gpu_open_tsg_args {
	__u32 tsg_fd;		/* out: tsg fd */
	__u32 flags;		/* in: NVGPU_GPU_IOCTL_OPEN_TSG_FLAGS_* */

	/*
	 * in: share token obtained from
	 * NVGPU_TSG_IOCTL_GET_SHARE_TOKEN. Ignored when
	 * NVGPU_GPU_IOCTL_OPEN_TSG_FLAGS_SHARE
	 * is unset.
	 */
	__u8 share_token[NVGPU_TSG_SHARE_TOKEN_SIZE];
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 * @defgroup NVGPURM_INTF_GET_TPC_MASKS IOCTL: NVGPU_GPU_IOCTL_GET_TPC_MASKS
 * @brief Retrieves the TPC enable mask vector.
 *
 * This call retrieves the TPC enable mask vector (physical identifiers).
 * Every vector element contains
 * a 32-bit TPC enable mask for the respective GPC. In case the GPC is disabled, the
 * respective TPC enable mask is 0.
 *
 * The caller should reserve a buffer big enough to receive TPC enable masks for
 * all enabled GPCs. (See "IOCTL message structure" below and
 * @c #nvgpu_gpu_characteristics::gpc_mask.)
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_gpu_get_tpc_masks_args</td>
 *   <td>@c #nvgpu_gpu_get_tpc_masks_args</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1</td>
 *   <td>(none)</td>
 *   <td>TPC enable mask vector of type <tt>uint32_t[N]</tt> where<br>
 *       @c N =
 *       <tt>highest_set_bit(#nvgpu_gpu_characteristics::gpc_mask) + 1</tt>
 *   </td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EIO ---     Writing TPC enable masks buffer failed.
 * - @c ENODEV ---  GPU not ready.
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @remark To be revised for MIG.
 */

/**
 * @ingroup NVGPURM_INTF_GET_TPC_MASKS
 * @brief Header struct for @ref NVGPURM_INTF_GET_TPC_MASKS.
 */
struct nvgpu_gpu_get_tpc_masks_args {
	/**
	 * @brief TPC enable mask buffer size (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Size of the receive buffer allocated by the caller.</dd>
	 *   <dt>[out]</dt>
	 *   <dd>Size of the kernel buffer.</dd>
	 * </dl>
	 */
	__u32 mask_buf_size;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;

	/**
	 * @brief address of TPC mask buffer
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     It will receive one 32-bit TPC mask per GPC or 0 if GPC is not enabled or not
	 *     present. This parameter is ignored if @c mask_buf_size is 0.
	 *   </dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u64 mask_buf_addr;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 * @defgroup NVGPURM_INTF_GET_PPC_MASKS IOCTL: NVGPU_GPU_IOCTL_GET_PPC_MASKS
 * @brief Retrieves the PPC enable mask vector.
 *
 * This call retrieves the PPC enable mask vector (physical identifiers).
 * Every vector element contains
 * a 32-bit PPC enable mask for the respective GPC. In case the GPC is disabled, the
 * respective PPC enable mask is 0.
 *
 * The caller should reserve a buffer big enough to receive PPC enable masks for
 * all enabled GPCs. (See "IOCTL message structure" below and
 * @c #nvgpu_gpu_characteristics::gpc_mask.)
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_gpu_get_ppc_masks_args</td>
 *   <td>@c #nvgpu_gpu_get_ppc_masks_args</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1</td>
 *   <td>(none)</td>
 *   <td>PPC enable mask vector of type <tt>uint32_t[N]</tt> where<br>
 *       @c N =
 *       <tt>highest_set_bit(#nvgpu_gpu_characteristics::gpc_mask) + 1</tt>
 *   </td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EIO ---     Writing PPC enable masks buffer failed.
 * - @c ENODEV ---  GPU not ready.
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @remark To be revised for MIG.
 */

/**
 * @ingroup NVGPURM_INTF_GET_PPC_MASKS
 * @brief Header struct for @ref NVGPURM_INTF_GET_PPC_MASKS.
 */
struct nvgpu_gpu_get_ppc_masks_args {
	/**
	 * @brief PPC enable mask buffer size (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Size of the receive buffer allocated by the caller.</dd>
	 *   <dt>[out]</dt>
	 *   <dd>Size of the kernel buffer.</dd>
	 * </dl>
	 */
	__u32 mask_buf_size;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;

	/**
	 * @brief address of PPC mask buffer
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     It will receive one 32-bit PPC mask per GPC or 0 if GPC is not enabled or not
	 *     present. This parameter is ignored if @c mask_buf_size is 0.
	 *   </dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u64 mask_buf_addr;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 * @defgroup NVGPURM_INTF_GET_ROP_MASKS IOCTL: NVGPU_GPU_IOCTL_GET_ROP_MASKS
 * @brief Retrieves the ROP enable mask vector.
 *
 * This call retrieves the ROP enable mask vector (physical identifiers).
 * Every vector element contains
 * a 32-bit ROP enable mask for the respective GPC. In case the GPC is disabled, the
 * respective ROP enable mask is 0.
 *
 * The caller should reserve a buffer big enough to receive ROP enable masks for
 * all enabled GPCs. (See "IOCTL message structure" below and
 * @c #nvgpu_gpu_characteristics::gpc_mask.)
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_gpu_get_rop_masks_args</td>
 *   <td>@c #nvgpu_gpu_get_rop_masks_args</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1</td>
 *   <td>(none)</td>
 *   <td>ROP enable mask vector of type <tt>uint32_t[N]</tt> where<br>
 *       @c N =
 *       <tt>highest_set_bit(#nvgpu_gpu_characteristics::gpc_mask) + 1</tt>
 *   </td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EIO ---     Writing ROP enable masks buffer failed.
 * - @c ENODEV ---  GPU not ready.
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @remark To be revised for MIG.
 */

/**
 * @ingroup NVGPURM_INTF_GET_ROP_MASKS
 * @brief Header struct for @ref NVGPURM_INTF_GET_ROP_MASKS.
 */
struct nvgpu_gpu_get_rop_masks_args {
	/**
	 * @brief ROP enable mask buffer size (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Size of the receive buffer allocated by the caller.</dd>
	 *   <dt>[out]</dt>
	 *   <dd>Size of the kernel buffer.</dd>
	 * </dl>
	 */
	__u32 mask_buf_size;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;

	/**
	 * @brief address of ROP mask buffer
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     It will receive one 32-bit ROP mask per GPC or 0 if GPC is not enabled or not
	 *     present. This parameter is ignored if @c mask_buf_size is 0.
	 *   </dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u64 mask_buf_addr;
};

struct nvgpu_gpu_get_gpc_physical_map_args {
	/* [in] GPC logical-map-buffer size. It must be
	 * sizeof(__u32) * popcnt(gpc_mask)
	 */
	__u32 map_buf_size;
	__u32 reserved;

	/* [out] pointer to array of u32 entries.
	 * For each entry, index=local gpc index and value=physical gpc index.
	 */
	__u64 physical_gpc_buf_addr;
};

struct nvgpu_gpu_get_gpc_logical_map_args {
	/* [in] GPC logical-map-buffer size. It must be
	 * sizeof(__u32) * popcnt(gpc_mask)
	 */
	__u32 map_buf_size;
	__u32 reserved;

	/* [out] pointer to array of u32 entries.
	 * For each entry, index=local gpc index and value=logical gpc index.
	 */
	__u64 logical_gpc_buf_addr;
};

struct nvgpu_gpu_get_fbp_physical_map_args {
	/* [in] FBP logical-map-buffer size. It must be
	 * sizeof(__u32) * popcnt(fbp_mask)
	 */
	__u32 map_buf_size;
	__u32 reserved;

	/* [out] pointer to array of u32 entries.
	 * For each entry, index=local fbp index and value=physical fbp index.
	 */
	__u64 physical_fbp_buf_addr;
};

struct nvgpu_gpu_get_fbp_logical_map_args {
	/* [in] FBP logical-map-buffer size. It must be
	 * sizeof(__u32) * popcnt(fbp_mask)
	 */
	__u32 map_buf_size;
	__u32 reserved;

	/* [out] pointer to array of u32 entries.
	 * For each entry, index=local fbp index and value=logical fbp index.
	 */
	__u64 logical_fbp_buf_addr;
};

struct nvgpu_gpu_open_channel_args {
	union {
		__s32 channel_fd; /* deprecated: use out.channel_fd instead */
		struct {
			 /* runlist_id is the runlist for the
			  * channel. Basically, the runlist specifies the target
			  * engine(s) for which the channel is
			  * opened. Runlist_id -1 is synonym for the primary
			  * graphics runlist. */
			__s32 runlist_id;
		} in;
		struct {
			__s32 channel_fd;
		} out;
	};
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 * @defgroup NVGPURM_INTF_FLUSH_L2 IOCTL: NVGPU_GPU_IOCTL_FLUSH_L2

 * @brief Performs synchronous GPU L2 cache maintenance and/or frame buffer
 * flush.
 *
 * This command is used to perform L2 cache maintenance and to flush pending
 * frame buffer writes to system memory.
 *
 * When @c #nvgpu_gpu_l2_fb_args::l2_flush is set, the following sequence is
 * executed:
 *
 * -# Flush all pending writes to memory (SysMemBar flush)
 * -# Write-back L2 dirty lines
 * -# If @c #nvgpu_gpu_l2_fb_args::l2_invalidate is set:
 *    -# Invalidate clean L2 lines
 * -# Invalidate TLB or flush all pending writes to memory depending on internal
 *    conditions
 *
 * Additionally, when @c #nvgpu_gpu_l2_fb_args::fb_flush is set:
 * -# Flush all pending writes to memory (SysMemBar flush)
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_gpu_l2_fb_args</td>
 *   <td>@c #nvgpu_gpu_l2_fb_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c ETIMEDOUT ---  L2 maintenance operation timed out.
 * - @c ETIMEDOUT ---  FB tlb invalidate timed out.
 * - @c EBUSY ---      FB flush timed out.
 * - @c ENODEV ---     GPU not ready.
 * - @c EFAULT ---     Copy from userspace fails.
 *
 * @note This IOCTL command has caveats:
 * -# Setting @c #nvgpu_gpu_l2_fb_args::l2_invalidate without @c
 *    #nvgpu_gpu_l2_fb_args::l2_flush has no effect. This behavior may be
 *    unexpected.
 * -# Setting both @c #nvgpu_gpu_l2_fb_args::l2_flush and @c
 *    #nvgpu_gpu_l2_fb_args::fb_flush may result in redundant FB flush
 *    operations.
 * -# It is not clear why TLB invalidate is performed by this IOCTL command.
 *
 * @sa https://p4viewer.nvidia.com/get//hw/doc/gpu/ampere_info/gen_manuals/ga10b/dev_flush.ref
 * @sa https://p4viewer.nvidia.com/get//hw/doc/gpu/blackwell_info/gen_manuals/gb10b/dev_flush.ref
 */

/**
 * @ingroup NVGPURM_INTF_FLUSH_L2
 * @brief Header argument struct for @ref NVGPURM_INTF_FLUSH_L2.
 */
struct nvgpu_gpu_l2_fb_args {
	/**
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Request L2 dirty lines to be written back.</dd>
	 * </dl>
	 *
	 * Range: [0, 1]
	 */
	__u32 l2_flush:1;

	/**
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Request L2 invalidate clean lines.</dd>
	 * </dl>
	 *
	 * Range: [0, 1]
	 *
	 * @note This field has no effect when @c #l2_flush is not set.
	 */
	__u32 l2_invalidate:1;

	/**
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Flush pending frame buffer writes (SysMemBar flush)</dd>
	 * </dl>
	 *
	 * Range: [0, 1]
	 */
	__u32 fb_flush:1;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;
} __packed;

struct nvgpu_gpu_mmu_debug_mode_args {
	__u32 state;
	__u32 reserved;
};

struct nvgpu_gpu_sm_debug_mode_args {
	int channel_fd;
	__u32 enable;
	__u64 sms;
};

struct warpstate {
	__u64 valid_warps[2];
	__u64 trapped_warps[2];
	__u64 paused_warps[2];
};

struct nvgpu_gpu_wait_pause_args {
	__u64 pwarpstate;
};

struct nvgpu_gpu_tpc_exception_en_status_args {
	__u64 tpc_exception_en_sm_mask;
};

struct nvgpu_gpu_num_vsms {
	__u32 num_vsms;
	__u32 reserved;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 * @defgroup NVGPURM_INTF_VSMS_MAPPING IOCTL: NVGPU_GPU_IOCTL_VSMS_MAPPING
 * @brief Retrieves virtual SM-to-GPC/TPC mappings for the given configuration.
 *
 * This IOCTL retrieves the virtual SM mapping information. That is, the
 * GPC/TPC logical indexes for each SM. Note that starting from Volta, there are
 * more than one SM per TPC. See the chip-specific HW litter @c
 * NV_SCAL_LITTER_NUM_SM_PER_TPC.
 *
 * The number of SM units is determined as follows:
 *
 * -# Compute the number of enabled TPC units by retrieving the TPC masks (see
 *    @ref NVGPURM_INTF_GET_TPC_MASKS) and counting the 1 bits in the masks
 *    (popcount).
 * -# @c Number_of_SMs = @c Number_of_TPCs * @c NV_SCAL_LITTER_NUM_SM_PER_TPC
 *
 * See @c #nvgpu_gpu_vsms_mapping_entry for valid (gpc, tpc) indexes.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_gpu_vsms_mapping</td>
 *   <td>@c #nvgpu_gpu_vsms_mapping</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1</td>
 *   <td>(none)</td>
 *   <td><tt>#nvgpu_gpu_vsms_mapping_entry[Number_of_SMs]</tt></td>
 *   </td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---     Input parameter out of range.
 * - @c ENOMEM ---     Memory allocation for VSMS mapping buffer failed.
 * - @c EOVERFLOW ---  GPC/TPC index is greater than 255. (Internal error)
 * - @c EIO ---        Writing to VSMS mapping buffer failed.
 * - @c ENODEV ---     GPU not ready.
 * - @c EFAULT ---     Copy from userspace fails.
 *
 * @sa HW litters for GA10B are found in
 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/ampere/ga10b/hwproject.h
 *
 * @sa HW litters for GB10B are found in
 * https://p4viewer.nvidia.com/get///sw/dev/gpu_drv/chips_a/drivers/common/inc/hwref/blackwell/gb10b/hwproject.h
 */

/**
 * @ingroup NVGPURM_INTF_VSMS_MAPPING
 * @brief GPC/TPC indexes of a virtual SM
 */
struct nvgpu_gpu_vsms_mapping_entry {

	/**
	 * @brief Logical GPC index post floorsweeping
	 *
	 * Range: [0, popcount(@c nvgpu_gpu_characteristics::gpc_mask) - 1]
	 *
	 * @remark To be revised for MIG.
	 *
	 * @sa @c #NVGPU_GPU_IOCTL_GET_GPC_LOCAL_TO_PHYSICAL_MAP
	 */
	__u8 gpc_logical_index;

	/**
	 * @brief Virtual GPC index
	 *
	 * @remark To be revised.
	 */
	__u8 gpc_virtual_index;

	/**
	 * @brief Local TPC index within the GPC
	 *
	 * Range: [0, num_enabled_tpcs_within_gpc - 1]
	 *
	 * @remark Number of enabled TPCs within a GPC can be determined as
	 * follows:
	 * -# Map logical GPC index to physical GPC index
	 *    (@c #NVGPU_GPU_IOCTL_GET_GPC_LOCAL_TO_PHYSICAL_MAP)
	 * -# Obtain TPC masks (@ref NVGPURM_INTF_GET_TPC_MASKS)
	 * -# Number of enabled TPCs is
	 *    <tt>popcount(tpc_masks[physical_gpc_index])</tt>
	 *
	 * @remark To be revised for MIG.
	 *
	 * @sa @c #NVGPU_GPU_IOCTL_GET_GPC_LOCAL_TO_PHYSICAL_MAP
	 */
	__u8 tpc_local_logical_index;

	/**
	 * @brief Global logical index of the TPC.
	 *
	 * Range: [0, num_enabled_tpcs - 1]
	 *
	 * @remark The number of enabled TPCs can be obtained by computing the
	 * combined population count of all TPC masks.
	 * See @ref NVGPURM_INTF_GET_TPC_MASKS.
	 *
	 * @remark To be revised for MIG.
	 */
	__u8 tpc_global_logical_index;

	/**
	 * @brief Local SM index within TPC.
	 *
	 * Range: [0, @c NV_SCAL_LITTER_NUM_SM_PER_TPC - 1]
	 *
	 * See @ref NVGPURM_INTF_VSMS_MAPPING for the litter.
	 *
	 * @remark To be revised for MIG.
	 */
	__u8 sm_local_id;

	/**
	 * @brief Migratable TPC index
	 *
	 * @remark To be revised.
	 */
	__u8 tpc_migratable_index;
};

/**
 * @ingroup NVGPURM_INTF_VSMS_MAPPING
 * @brief Header argument struct for @ref NVGPURM_INTF_VSMS_MAPPING.
 */
struct nvgpu_gpu_vsms_mapping {
	/**
	 * @brief address of vsms mapping buffer
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     The buffer size must be sufficient to hold array of vsms mapping entries.
	 *   </dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u64 vsms_map_buf_addr;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 * @defgroup NVGPURM_INTF_GET_BUFFER_INFO IOCTL: NVGPU_GPU_IOCTL_GET_BUFFER_INFO
 * @brief Returns information about a buffer.
 *
 * The information includes buffer registration status, compression tags
 * allocation status, size of the buffer, and the metadata blob.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_gpu_get_buffer_info_args</td>
 *   <td>@c #nvgpu_gpu_get_buffer_info_args</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1</td>
 *   <td>(none)</td>
 *   <td>When @c #nvgpu_gpu_get_buffer_info_args::metadata_size:
 *       - > 0: Buffer to receive the metadata blob.
 *       - = 0: (none)
 *   </td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---     Invalid parameters.
 * - @c EIO ---        Writing the metadata blob to output buffer failed.
 * - @c EFAULT ---     Copy from userspace fails.
 *
 * @sa @ref NVGPURM_INTF_REGISTER_BUFFER
 */

/**
 * @ingroup NVGPURM_INTF_GET_BUFFER_INFO
 * @{
 */

/**
 * @brief Buffer has been previously registered with
 * @ref NVGPURM_INTF_REGISTER_BUFFER.
 */
#define NVGPU_GPU_BUFFER_INFO_FLAGS_METADATA_REGISTERED		(1ULL << 0)

/**
 * @brief Compression tags are allocated for the buffer.
 */
#define NVGPU_GPU_BUFFER_INFO_FLAGS_COMPTAGS_ALLOCATED		(1ULL << 1)

/**
 * @brief Buffer may be re-registered with
 * @ref NVGPURM_INTF_REGISTER_BUFFER.
 */
#define NVGPU_GPU_BUFFER_INFO_FLAGS_MUTABLE_METADATA		(1ULL << 2)

/** @} */


/**
 * @ingroup NVGPURM_INTF_GET_BUFFER_INFO
 * @brief Header struct for @ref NVGPURM_INTF_GET_BUFFER_INFO.
 */
struct nvgpu_gpu_get_buffer_info_args {
	union {
		/**
		 * @brief Input arguments
		 */
		struct {
			/**
			 * @brief dma-buf file descriptor of the buffer
			 */
			__s32 dmabuf_fd;

			/**
			 * @brief Size of the metadata blob buffer reserved by
			 * the caller (in bytes).
			 */
			__u32 metadata_size;

			/**
			 * @brief Pointer to buffer for receiving metadata.
			 *
			 * <dl>
			 *   <dt>[in]</dt>
			 *   <dd>Address of metadata blob buffer.</dd>
			 * </dl>
			 */
			__u64 metadata_addr;
		} in;

		/**
		 * @brief Output arguments
		 */
		struct {

			/**
			 * @brief Buffer information flags. (See @c
			 * NVGPU_GPU_BUFFER_INFO_FLAGS_*)
			 */
			__u64 flags;

			/**
			 * @brief Size of the metadata blob or 0 if none
			 * associated.
			 */
			__u32 metadata_size;

			/**
			 * @brief Reserved
			 *
			 * <dl>
			 *   <dt>[in]</dt><dd>Zeroed</dd>
			 *   <dt>[out]</dt><dd>Ignore</dd>
			 * </dl>
			 */
			__u32 reserved;

			/** @brief Buffer size in bytes. */
			__u64 size;
		} out;
	};
};

#define NVGPU_GPU_GET_CPU_TIME_CORRELATION_INFO_MAX_COUNT		16
#define NVGPU_GPU_GET_CPU_TIME_CORRELATION_INFO_SRC_ID_TSC		1
#define NVGPU_GPU_GET_CPU_TIME_CORRELATION_INFO_SRC_ID_OSTIME	2

struct nvgpu_gpu_get_cpu_time_correlation_sample {
	/* gpu timestamp value */
	__u64 cpu_timestamp;
	/* raw GPU counter (PTIMER) value */
	__u64 gpu_timestamp;
};

struct nvgpu_gpu_get_cpu_time_correlation_info_args {
	/* timestamp pairs */
	struct nvgpu_gpu_get_cpu_time_correlation_sample samples[
		NVGPU_GPU_GET_CPU_TIME_CORRELATION_INFO_MAX_COUNT];
	/* number of pairs to read */
	__u32 count;
	/* cpu clock source id */
	__u32 source_id;
};

/**
 * @defgroup NVGPURM_INTF_GET_GPU_TIME Get gpu timestamp.
 *
 * Gpu timestamp value is read from ptimer registers(two 32 bit registers). The
 * counter is in nanoseconds. The wraparound of lesser significant 32 bit
 * register happens roughly every 4 seconds and is taken care of while reading
 * the timestamp.
 */

/**
 * This struct holds the gpu timestamp.
 * @ingroup NVGPURM_INTF_GET_GPU_TIME
 */
struct nvgpu_gpu_get_gpu_time_args {
	/** [out] raw GPU counter (PTIMER) value in nanoseconds. */
	__u64 gpu_timestamp;

	/** @brief Reserved */
	__u64 reserved;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 *
 * @defgroup NVGPURM_INTF_GET_ENGINE_INFO IOCTL: NVGPU_GPU_IOCTL_GET_ENGINE_INFO
 * @brief Retrieves the list of GPU engines with associated information. That is:
 * engine type, instance number, and runlist id.
 *
 * To retrieve the engine info list, the default call pattern is as follows:
 * -# Call this IOCTL with
 *    @c #nvgpu_gpu_get_engine_info_args::engine_info_buf_size = 0.
 * -# Allocate a buffer of size @c
 *    #nvgpu_gpu_get_engine_info_args::engine_info_buf_size as reported by the
 *    previous call.
 * -# Call this IOCTL again with the size of the allocated buffer.
 *
 * STEP 1 may be omitted by preallocating a big enough buffer.
 *
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_gpu_get_engine_info_args</td>
 *   <td>@c #nvgpu_gpu_get_engine_info_args</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1</td>
 *   <td>(none)</td>
 *   <td><tt>#nvgpu_gpu_get_engine_info_item[N]</tt>. See @c
 *       #nvgpu_gpu_get_engine_info_args::engine_info_buf_size
 *       for the number of elements (@c N).</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---     Engine type enumeration is not mapped. (Internal error)
 * - @c EOVERFLOW ---  Runlist id overflow. (Internal error)
 * - @c EIO ---        Writing engine info to output buffer failed.
 * - @c ENODEV ---     GPU not ready.
 * - @c EFAULT ---     Copy from userspace fails.
 *
 * @sa https://p4viewer.nvidia.com/get//hw/doc/gpu/ampere_info/gen_manuals/ga10b/dev_top.ref
 * @sa https://p4viewer.nvidia.com/get//hw/doc/gpu/blackwell_info/gen_manuals/gb10b/dev_top.ref
 */

/**
 * @ingroup NVGPURM_INTF_GET_ENGINE_INFO
 * @{
 */

/** @brief Engine type: Graphics */
#define NVGPU_GPU_ENGINE_ID_GR		0U

/** @brief Engine type: Graphics copy engine (logical) */
#define NVGPU_GPU_ENGINE_ID_GR_COPY	1U

/** @brief Engine type: Asynchronous copy engine (logical) */
#define NVGPU_GPU_ENGINE_ID_ASYNC_COPY	2U

/** @brief Engine type: NVENC */
#define NVGPU_GPU_ENGINE_ID_NVENC 5

/** @brief Engine type: OFA */
#define NVGPU_GPU_ENGINE_ID_OFA 6

/** @brief Engine type: NVDEC */
#define NVGPU_GPU_ENGINE_ID_NVDEC 7

/** @brief Engine type: NVJPG */
#define NVGPU_GPU_ENGINE_ID_NVJPG 8

/**
 * @}
 */

/**
 * @ingroup NVGPURM_INTF_GET_ENGINE_INFO
 * @brief Engine information list item
 */
struct nvgpu_gpu_get_engine_info_item {
	/**
	 * @brief Engine type identifier
	 *
	 * Range: #NVGPU_GPU_ENGINE_ID_GR, #NVGPU_GPU_ENGINE_ID_GR_COPY,
	 * #NVGPU_GPU_ENGINE_ID_ASYNC_COPY.
	 */
	__u32 engine_id;

	/**
	 * @brief Engine instance number
	 *
	 * Range: HW-specific
	 *
	 * @remark The instance number is used to distinguish the instances of
	 * multi-instance engines. For example, GA10B has 4 logical copy engines.
	 */
	__u32 engine_instance;

	/**
	 * @brief Runlist identifier for the engine.
	 *
	 * The runlist identifier is used to open a channel for sending methods to
	 * the engine.
	 *
	 * Range:
	 * - &ge; 0: HW-specific runlist identifier
	 * - -1: Channels not supported for the engine
	 *
	 * @sa @ref NVGPURM_INTF_SET_RUNLIST_ID
	 */
	__s32 runlist_id;

	/**
	 * @brief Reserved
	 *
	 * The value of this field should be ignored.
	 */
	__u32 reserved;
};

/**
 * @ingroup NVGPURM_INTF_GET_ENGINE_INFO
 * @brief Header struct for @ref NVGPURM_INTF_GET_ENGINE_INFO
 */
struct nvgpu_gpu_get_engine_info_args {

	/**
	 * @brief Buffer size (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Buffer size reserved by the caller. May be 0 to query the engine
	 *     info list size.</dd>
	 *
	 *   <dt>[out]</dt>
	 *   <dd>The size of the engine info list. The number of elements in the
	 *       list is:<br>
	 *       @c N = @c #nvgpu_gpu_get_engine_info_args::engine_info_buf_size /
	 *       <tt>sizeof(#nvgpu_gpu_get_engine_info_item)</tt>.
	 *   </dd>
	 * </dl>
	 */
	__u32 engine_info_buf_size;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;

	/**
	 * @brief address of engine info buffer
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     The buffer size must be sufficient to hold array of vsms mapping entries.
	 *   </dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u64 engine_info_buf_addr;
};

#define NVGPU_GPU_ALLOC_VIDMEM_FLAG_CONTIGUOUS		(1U << 0)

/* CPU access and coherency flags (3 bits). Use CPU access with care,
 * BAR resources are scarce. */
#define NVGPU_GPU_ALLOC_VIDMEM_FLAG_CPU_NOT_MAPPABLE	(0U << 1)
#define NVGPU_GPU_ALLOC_VIDMEM_FLAG_CPU_WRITE_COMBINE	(1U << 1)
#define NVGPU_GPU_ALLOC_VIDMEM_FLAG_CPU_CACHED		(2U << 1)
#define NVGPU_GPU_ALLOC_VIDMEM_FLAG_CPU_MASK		(7U << 1)

#define NVGPU_GPU_ALLOC_VIDMEM_FLAG_VPR			(1U << 4)

/* Allocation of device-specific local video memory. Returns dmabuf fd
 * on success. */
struct nvgpu_gpu_alloc_vidmem_args {
	union {
		struct {
			/* Size for allocation. Must be a multiple of
			 * small page size. */
			__u64 size;

			/* NVGPU_GPU_ALLOC_VIDMEM_FLAG_* */
			__u32 flags;

			/* Informational mem tag for resource usage
			 * tracking. */
			__u16 memtag;

			__u16 reserved0;

			/* GPU-visible physical memory alignment in
			 * bytes.
			 *
			 * Alignment must be a power of two. Minimum
			 * alignment is the small page size, which 0
			 * also denotes.
			 *
			 * For contiguous and non-contiguous
			 * allocations, the start address of the
			 * physical memory allocation will be aligned
			 * by this value.
			 *
			 * For non-contiguous allocations, memory is
			 * internally allocated in round_up(size /
			 * alignment) contiguous blocks. The start
			 * address of each block is aligned by the
			 * alignment value. If the size is not a
			 * multiple of alignment (which is ok), the
			 * last allocation block size is (size %
			 * alignment).
			 *
			 * By specifying the big page size here and
			 * allocation size that is a multiple of big
			 * pages, it will be guaranteed that the
			 * allocated buffer is big page size mappable.
			 */
			__u32 alignment;

			__u32 reserved1[3];
		} in;

		struct {
			__s32 dmabuf_fd;
		} out;
	};
};

/* Memory clock */
#define NVGPU_GPU_CLK_DOMAIN_MCLK                                (0)
/* Main graphics core clock */
#define NVGPU_GPU_CLK_DOMAIN_GPCCLK	                         (1)

struct nvgpu_gpu_clk_range {

	/* Flags (not currently used) */
	__u32 flags;

	/* NVGPU_GPU_CLK_DOMAIN_* */
	__u32 clk_domain;
	__u64 min_hz;
	__u64 max_hz;
};

/* Request on specific clock domains */
#define NVGPU_GPU_CLK_FLAG_SPECIFIC_DOMAINS		(1UL << 0)

struct nvgpu_gpu_clk_range_args {

	/* Flags. If NVGPU_GPU_CLK_FLAG_SPECIFIC_DOMAINS the request will
	   apply only to domains specified in clock entries. In this case
	   caller must set clock domain in each entry. Otherwise, the
	   ioctl will return all clock domains.
	*/
	__u32 flags;

	__u16 pad0;

	/* in/out: Number of entries in clk_range_entries buffer. If zero,
	   NVGPU_GPU_IOCTL_CLK_GET_RANGE will return 0 and
	   num_entries will be set to number of clock domains.
	 */
	__u16 num_entries;

	/* in: Pointer to clock range entries in the caller's address space.
	   size must be >= max_entries * sizeof(struct nvgpu_gpu_clk_range)
	 */
	__u64 clk_range_entries;
};

struct nvgpu_gpu_clk_vf_point {
	__u64 freq_hz;
};

struct nvgpu_gpu_clk_vf_points_args {

	/* in: Flags (not currently used) */
	__u32 flags;

	/* in: NVGPU_GPU_CLK_DOMAIN_* */
	__u32 clk_domain;

	/* in/out: max number of nvgpu_gpu_clk_vf_point entries in
	   clk_vf_point_entries.  If max_entries is zero,
	   NVGPU_GPU_IOCTL_CLK_GET_VF_POINTS will return 0 and max_entries will
	   be set to the max number of VF entries for this clock domain. If
	   there are more entries than max_entries, then ioctl will return
	   -EINVAL.
	*/
	__u16 max_entries;

	/* out: Number of nvgpu_gpu_clk_vf_point entries returned in
	   clk_vf_point_entries. Number of entries might vary depending on
	   thermal conditions.
	*/
	__u16 num_entries;

	__u32 reserved;

	/* in: Pointer to clock VF point entries in the caller's address space.
	   size must be >= max_entries * sizeof(struct nvgpu_gpu_clk_vf_point).
	 */
	__u64 clk_vf_point_entries;
};

/* Target clock requested by application*/
#define NVGPU_GPU_CLK_TYPE_TARGET	1
/* Actual clock frequency for the domain.
   May deviate from desired target frequency due to PLL constraints. */
#define NVGPU_GPU_CLK_TYPE_ACTUAL	2
/* Effective clock, measured from hardware */
#define NVGPU_GPU_CLK_TYPE_EFFECTIVE	3

struct nvgpu_gpu_clk_info {

	/* Flags (not currently used) */
	__u16 flags;

	/* in: When NVGPU_GPU_CLK_FLAG_SPECIFIC_DOMAINS set, indicates
	   the type of clock info to be returned for this entry. It is
	   allowed to have several entries with different clock types in
	   the same request (for instance query both target and actual
	   clocks for a given clock domain). This field is ignored for a
	   SET operation. */
	__u16 clk_type;

	/* NVGPU_GPU_CLK_DOMAIN_xxx */
	__u32 clk_domain;

	__u64 freq_hz;
};

struct nvgpu_gpu_clk_get_info_args {

	/* Flags. If NVGPU_GPU_CLK_FLAG_SPECIFIC_DOMAINS the request will
	   apply only to domains specified in clock entries. In this case
	   caller must set clock domain in each entry. Otherwise, the
	   ioctl will return all clock domains.
	*/
	__u32 flags;

	/* in: indicates which type of clock info to be returned (see
	   NVGPU_GPU_CLK_TYPE_xxx). If NVGPU_GPU_CLK_FLAG_SPECIFIC_DOMAINS
	   is defined, clk_type is specified in each clock info entry instead.
	 */
	__u16 clk_type;

	/* in/out: Number of clock info entries contained in clk_info_entries.
	   If zero, NVGPU_GPU_IOCTL_CLK_GET_INFO will return 0 and
	   num_entries will be set to number of clock domains. Also,
	   last_req_nr will be updated, which allows checking if a given
	   request has completed. If there are more entries than max_entries,
	   then ioctl will return -EINVAL.
	 */
	__u16 num_entries;

	/* in: Pointer to nvgpu_gpu_clk_info entries in the caller's address
	   space. Buffer size must be at least:
		num_entries * sizeof(struct nvgpu_gpu_clk_info)
	   If NVGPU_GPU_CLK_FLAG_SPECIFIC_DOMAINS is set, caller should set
	   clk_domain to be queried in  each entry. With this flag,
	   clk_info_entries passed to an NVGPU_GPU_IOCTL_CLK_SET_INFO,
	   can be re-used on completion for a NVGPU_GPU_IOCTL_CLK_GET_INFO.
	   This allows checking actual_mhz.
	 */
	__u64 clk_info_entries;

};

struct nvgpu_gpu_clk_set_info_args {

	/* in: Flags (not currently used). */
	__u32 flags;

	__u16 pad0;

	/* Number of clock info entries contained in clk_info_entries.
	   Must be > 0.
	 */
	__u16 num_entries;

	/* Pointer to clock info entries in the caller's address space. Buffer
	   size must be at least
		num_entries * sizeof(struct nvgpu_gpu_clk_info)
	 */
	__u64 clk_info_entries;

	/* out: File descriptor for request completion. Application can poll
	   this file descriptor to determine when the request has completed.
	   The fd must be closed afterwards.
	 */
	__s32 completion_fd;
};

struct nvgpu_gpu_get_event_fd_args {

	/* in: Flags (not currently used). */
	__u32 flags;

	/* out: File descriptor for events, e.g. clock update.
	 * On successful polling of this event_fd, application is
	 * expected to read status (nvgpu_gpu_event_info),
	 * which provides detailed event information
	 * For a poll operation, alarms will be reported with POLLPRI,
	 * and GPU shutdown will be reported with POLLHUP.
	 */
	__s32 event_fd;
};

struct nvgpu_gpu_get_memory_state_args {
	/*
	 * Current free space for this device; may change even when any
	 * kernel-managed metadata (e.g., page tables or channels) is allocated
	 * or freed. For an idle gpu, an allocation of this size would succeed.
	 */
	__u64 total_free_bytes;

	/* For future use; must be set to 0. */
	__u64 reserved[4];
};

struct nvgpu_gpu_get_fbp_l2_masks_args {
	/* [in]  L2 mask buffer size reserved by userspace. Should be
		 at least sizeof(__u32) * fls(fbp_en_mask) to receive LTC
		 mask for each FBP.
	   [out] full kernel buffer size
	*/
	__u32 mask_buf_size;
	__u32 reserved;

	/* [in]  pointer to L2 mask buffer. It will receive one
		 32-bit L2 mask per FBP or 0 if FBP is not enabled or
		 not present. This parameter is ignored if
		 mask_buf_size is 0. */
	__u64 mask_buf_addr;
};

#define NVGPU_GPU_VOLTAGE_CORE		1
#define NVGPU_GPU_VOLTAGE_SRAM		2
#define NVGPU_GPU_VOLTAGE_BUS		3	/* input to regulator */

struct nvgpu_gpu_get_voltage_args {
	__u64 reserved;
	__u32 which;		/* in: NVGPU_GPU_VOLTAGE_* */
	__u32 voltage;		/* uV */
};

struct nvgpu_gpu_get_current_args {
	__u32 reserved[3];
	__u32 currnt;		/* mA */
};

struct nvgpu_gpu_get_power_args {
	__u32 reserved[3];
	__u32 power;		/* mW */
};

struct nvgpu_gpu_get_temperature_args {
	__u32 reserved[3];
	/* Temperature in signed fixed point format SFXP24.8
	 *    Celsius = temp_f24_8 / 256.
	 */
	__s32 temp_f24_8;
};

struct nvgpu_gpu_set_therm_alert_limit_args {
	__u32 reserved[3];
	/* Temperature in signed fixed point format SFXP24.8
	 *    Celsius = temp_f24_8 / 256.
	 */
	__s32 temp_f24_8;
};

/*
 * Adjust options of deterministic channels in channel batches.
 *
 * This supports only one option currently: relax railgate blocking by
 * "disabling" the channel.
 *
 * Open deterministic channels do not allow the GPU to railgate by default. It
 * may be preferable to hold preopened channel contexts open and idle and still
 * railgate the GPU, taking the channels back into use dynamically in userspace
 * as an optimization. This ioctl allows to drop or reacquire the requirement
 * to hold GPU power on for individual channels. If allow_railgate is set on a
 * channel, no work can be submitted to it.
 *
 * num_channels is updated to signify how many channels were updated
 * successfully. It can be used to test which was the first update to fail.
 */
struct nvgpu_gpu_set_deterministic_opts_args {
	__u32 num_channels; /* in/out */
/*
 * Set or unset the railgating reference held by deterministic channels. If
 * the channel status is already the same as the flag, this is a no-op. Both
 * of these flags cannot be set at the same time. If none are set, the state
 * is left as is.
 */
#define NVGPU_GPU_SET_DETERMINISTIC_OPTS_FLAGS_ALLOW_RAILGATING    (1 << 0)
#define NVGPU_GPU_SET_DETERMINISTIC_OPTS_FLAGS_DISALLOW_RAILGATING (1 << 1)
	__u32 flags;        /* in */
	/*
	 * This is a pointer to an array of size num_channels.
	 *
	 * The channels have to be valid fds and be previously set as
	 * deterministic.
	 */
	__u64 channels; /* in */
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CTRL
 * @defgroup NVGPURM_INTF_REGISTER_BUFFER IOCTL: NVGPU_GPU_IOCTL_REGISTER_BUFFER
 * @brief Registers a buffer to the resource manager. This includes allocating
 * compression tags (optional) and associating the buffer with a metadata blob
 * (optional).
 *
 * This IOCTL triggers the NvGPU resource manager to register a buffer and
 * allocate additional resources required for mapping. In particular,
 * compression tags (comptags) are required to be allocated before mapping.
 *
 * For interoperability purposes, a small metadata blob can be associated with
 * the buffer. The blob is completely opaque to the NvGPU resource manager. It
 * is intended to contain information about the buffer that is needed by various
 * interoperability participants, possibly running in different processes. Such
 * information could be the GMMU memory kind of the buffer, GPU L2 cacheability,
 * and similar. Metadata blob can be queried with @ref
 * NVGPURM_INTF_GET_BUFFER_INFO. Metadata encoding and decoding is done by
 * libnvrm_gpu.so. Metadata blobs are not currently supported in safety.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_gpu_register_buffer_args</td>
 *   <td>@c #nvgpu_gpu_register_buffer_args</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1</td>
 *   <td>When @c #nvgpu_gpu_register_buffer_args::metadata_size:
 *       - > 0: Metadata blob (binary data, opaque to NvGPU RM)
 *       - = 0: (none)</td>
 *   <td>(none)</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---     Metadata blob specified (@c
 *                    #nvgpu_gpu_register_buffer_args::metadata_size &ne; 0) but
 *                    metadata not supported (see @c
 *                    #NVGPU_GPU_FLAGS_SUPPORT_BUFFER_METADATA)
 * - @c EINVAL ---     Compression required (@c
 *                    #NVGPU_GPU_COMPTAGS_ALLOC_REQUIRED) but compression not
 *                    supported (see @c #NVGPU_GPU_FLAGS_SUPPORT_COMPRESSION)
 * - @c ENOMEM ---     Insufficient memory to register the buffer.
 * - @c EFAULT ---     Metadata blob copy failed.
 * - @c EFAULT ---     Copy from userspace fails.
 */

/**
 * @ingroup NVGPURM_INTF_REGISTER_BUFFER
 * @{
 */

/**
 * @brief No comptags needed
 */
#define NVGPU_GPU_COMPTAGS_ALLOC_NONE			0U

/**
 * @brief Comptags preferred but it is allowed to fallback to no comptags.
 */
#define NVGPU_GPU_COMPTAGS_ALLOC_REQUESTED		1U

/**
 * @brief Comptags required and fallback is not allowed. In case comptag
 * allocation fails, the IOCTL fails.
 *
 * @pre <tt>nvgpu_gpu_characteristics::compression_page_size > 0</tt>
 */
#define NVGPU_GPU_COMPTAGS_ALLOC_REQUIRED		2U

/**
 * @brief Comptags successfully allocated for the buffer (**[out]** flag).
 */
#define NVGPU_GPU_REGISTER_BUFFER_FLAGS_COMPTAGS_ALLOCATED     (1U << 0)

/**
 * @brief Specifies that the buffer registration is mutable (**[in]** flag).
 * That is, buffer registration is allowed to be modified by calling this
 * IOCTL again with @c #NVGPU_GPU_REGISTER_BUFFER_FLAGS_MODIFY.
 *
 * Mutable registration is intended for private buffers where the physical
 * memory allocation may be recycled for another purpose.
 *
 * Buffers intended to be shared with other contexts (interoperability) should
 * be specified without this flag. This prevents accidental changes to the
 * attributes, and thus, creating inconsistency.
 */
#define NVGPU_GPU_REGISTER_BUFFER_FLAGS_MUTABLE			(1U << 1)

/**
 * @brief Re-register the buffer (**[in]** flag). When this flag is set, the
 * buffer compression tags allocation state, metadata binary blob, and other
 * registration attributes are re-defined.
 *
 * This flag may be set only when the buffer was previously registered as
 * mutable. This flag is ignored when the buffer is registered for the
 * first time.
 *
 * @remark If the buffer previously had comptags and the re-registration also
 * specifies comptags, the associated comptags are not cleared.
 */
#define NVGPU_GPU_REGISTER_BUFFER_FLAGS_MODIFY			(1U << 2)

/** @brief Maximum size of the user-supplied buffer metadata blob (in bytes) */
#define NVGPU_GPU_REGISTER_BUFFER_METADATA_MAX_SIZE	256U

/**
 * @}
 */

/**
 * @ingroup NVGPURM_INTF_REGISTER_BUFFER
 * @brief Header struct for @ref NVGPURM_INTF_REGISTER_BUFFER
 */
struct nvgpu_gpu_register_buffer_args {
	/**
	 * @brief dma-buf file descriptor of the buffer
	 */
	__s32 dmabuf_fd;

	/**
	 * @brief Compression tags allocation control.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Compression tag allocation control specifies whether
	 *     compression tags are not needed, preferred, or required.
	 *   </dd>
	 * </dl>
	 *
	 * Range: @c #NVGPU_GPU_COMPTAGS_ALLOC_NONE,
	 * @c #NVGPU_GPU_COMPTAGS_ALLOC_REQUESTED, @c
	 * #NVGPU_GPU_COMPTAGS_ALLOC_REQUIRED.
	 */
	__u8 comptags_alloc_control;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u8 reserved0;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u16 reserved1;

	/**
	 * @brief Pointer to buffer metadata.
	 *
	 * This is a binary blob populated by nvrm_gpu that will be associated
	 * with the dmabuf.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Address of metadata blob buffer.</dd>
	 * </dl>
	 */
	__u64 metadata_addr;

	/**
	 * @brief Buffer metadata size (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Size of the metadata blob.</dd>
	 * </dl>
	 *
	 * Range: When @c #NVGPU_GPU_FLAGS_SUPPORT_BUFFER_METADATA is:
	 * - supported: [0, @c #NVGPU_GPU_REGISTER_BUFFER_METADATA_MAX_SIZE]
	 * - not supported: 0
	 */
	__u32 metadata_size;

	/**
	 * @brief Registration flags.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Input flags</dd>
	 *   <dt>[out]</dt>
	 *   <dd>Output flags</dd>
	 * </dl>
	 *
	 * Range (bit mask): Flags with prefix @c
	 * NVGPU_GPU_REGISTER_BUFFER_FLAGS_*.
	 */
	__u32 flags;
};

#define NVGPU_GPU_IOCTL_ZCULL_GET_CTX_SIZE \
	_IOR(NVGPU_GPU_IOCTL_MAGIC, 1, struct nvgpu_gpu_zcull_get_ctx_size_args)
#define NVGPU_GPU_IOCTL_ZCULL_GET_INFO \
	_IOR(NVGPU_GPU_IOCTL_MAGIC, 2, struct nvgpu_gpu_zcull_get_info_args)
#define NVGPU_GPU_IOCTL_ZBC_SET_TABLE	\
	_IOW(NVGPU_GPU_IOCTL_MAGIC, 3, struct nvgpu_gpu_zbc_set_table_args)
#define NVGPU_GPU_IOCTL_ZBC_QUERY_TABLE	\
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 4, struct nvgpu_gpu_zbc_query_table_args)
/**
 * @ingroup NVGPURM_INTF_GET_CHARACTERISTICS
 * @brief Command code for @ref NVGPURM_INTF_GET_CHARACTERISTICS.
 */
#define NVGPU_GPU_IOCTL_GET_CHARACTERISTICS   \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 5, struct nvgpu_gpu_get_characteristics)
#define NVGPU_GPU_IOCTL_PREPARE_COMPRESSIBLE_READ \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 6, struct nvgpu_gpu_prepare_compressible_read_args)
#define NVGPU_GPU_IOCTL_MARK_COMPRESSIBLE_WRITE \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 7, struct nvgpu_gpu_mark_compressible_write_args)
/**
 * @ingroup NVGPURM_INTF_ALLOC_AS
 * @brief Command code for @ref NVGPURM_INTF_ALLOC_AS
 */
#define NVGPU_GPU_IOCTL_ALLOC_AS \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 8, struct nvgpu_alloc_as_args)
#define NVGPU_GPU_IOCTL_OPEN_TSG \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 9, struct nvgpu_gpu_open_tsg_args)
/**
 * @ingroup NVGPURM_INTF_GET_TPC_MASKS
 * @brief Command code for @ref NVGPURM_INTF_GET_TPC_MASKS.
 */
#define NVGPU_GPU_IOCTL_GET_TPC_MASKS \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 10, struct nvgpu_gpu_get_tpc_masks_args)
#define NVGPU_GPU_IOCTL_OPEN_CHANNEL \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 11, struct nvgpu_gpu_open_channel_args)
/**
 * @ingroup NVGPURM_INTF_FLUSH_L2
 * @brief Command code for @ref NVGPURM_INTF_FLUSH_L2.
 */
#define NVGPU_GPU_IOCTL_FLUSH_L2 \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 12, struct nvgpu_gpu_l2_fb_args)
#define NVGPU_GPU_IOCTL_SET_MMUDEBUG_MODE \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 14, struct nvgpu_gpu_mmu_debug_mode_args)
#define NVGPU_GPU_IOCTL_SET_SM_DEBUG_MODE \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 15, struct nvgpu_gpu_sm_debug_mode_args)
#define NVGPU_GPU_IOCTL_WAIT_FOR_PAUSE \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 16, struct nvgpu_gpu_wait_pause_args)
#define NVGPU_GPU_IOCTL_GET_TPC_EXCEPTION_EN_STATUS \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 17, struct nvgpu_gpu_tpc_exception_en_status_args)
#define NVGPU_GPU_IOCTL_NUM_VSMS \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 18, struct nvgpu_gpu_num_vsms)
/**
 * @ingroup NVGPURM_INTF_VSMS_MAPPING
 * @brief Command code for @ref NVGPURM_INTF_VSMS_MAPPING.
 */
#define NVGPU_GPU_IOCTL_VSMS_MAPPING \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 19, struct nvgpu_gpu_vsms_mapping)
#define NVGPU_GPU_IOCTL_RESUME_FROM_PAUSE \
	_IO(NVGPU_GPU_IOCTL_MAGIC, 21)
#define NVGPU_GPU_IOCTL_TRIGGER_SUSPEND \
	_IO(NVGPU_GPU_IOCTL_MAGIC, 22)
#define NVGPU_GPU_IOCTL_CLEAR_SM_ERRORS \
	_IO(NVGPU_GPU_IOCTL_MAGIC, 23)
#define NVGPU_GPU_IOCTL_GET_CPU_TIME_CORRELATION_INFO \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 24, \
			struct nvgpu_gpu_get_cpu_time_correlation_info_args)
/**
 * @ingroup NVGPURM_INTF_GET_GPU_TIME
 * @brief Command code for @ref NVGPURM_INTF_GET_GPU_TIME.
 */
#define NVGPU_GPU_IOCTL_GET_GPU_TIME \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 25, \
			struct nvgpu_gpu_get_gpu_time_args)
/**
 * @ingroup NVGPURM_INTF_GET_ENGINE_INFO
 * @brief Command code for @ref NVGPURM_INTF_GET_ENGINE_INFO.
 */
#define NVGPU_GPU_IOCTL_GET_ENGINE_INFO \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 26, \
			struct nvgpu_gpu_get_engine_info_args)
#define NVGPU_GPU_IOCTL_ALLOC_VIDMEM \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 27, \
			struct nvgpu_gpu_alloc_vidmem_args)
#define NVGPU_GPU_IOCTL_CLK_GET_RANGE \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 28, struct nvgpu_gpu_clk_range_args)
#define NVGPU_GPU_IOCTL_CLK_GET_VF_POINTS \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 29, struct nvgpu_gpu_clk_vf_points_args)
#define NVGPU_GPU_IOCTL_CLK_GET_INFO \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 30, struct nvgpu_gpu_clk_get_info_args)
#define NVGPU_GPU_IOCTL_CLK_SET_INFO \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 31, struct nvgpu_gpu_clk_set_info_args)
#define NVGPU_GPU_IOCTL_GET_EVENT_FD \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 32, struct nvgpu_gpu_get_event_fd_args)
#define NVGPU_GPU_IOCTL_GET_MEMORY_STATE \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 33, \
			struct nvgpu_gpu_get_memory_state_args)
#define NVGPU_GPU_IOCTL_GET_VOLTAGE \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 34, struct nvgpu_gpu_get_voltage_args)
#define NVGPU_GPU_IOCTL_GET_CURRENT \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 35, struct nvgpu_gpu_get_current_args)
#define NVGPU_GPU_IOCTL_GET_POWER \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 36, struct nvgpu_gpu_get_power_args)
#define NVGPU_GPU_IOCTL_GET_TEMPERATURE \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 37, struct nvgpu_gpu_get_temperature_args)
#define NVGPU_GPU_IOCTL_GET_FBP_L2_MASKS \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 38, struct nvgpu_gpu_get_fbp_l2_masks_args)
#define NVGPU_GPU_IOCTL_SET_THERM_ALERT_LIMIT \
		_IOWR(NVGPU_GPU_IOCTL_MAGIC, 39, \
			struct nvgpu_gpu_set_therm_alert_limit_args)
#define NVGPU_GPU_IOCTL_SET_DETERMINISTIC_OPTS \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 40, \
			struct nvgpu_gpu_set_deterministic_opts_args)
/**
 * @ingroup NVGPURM_INTF_REGISTER_BUFFER
 * @brief Command code for @ref NVGPURM_INTF_REGISTER_BUFFER.
 */
#define NVGPU_GPU_IOCTL_REGISTER_BUFFER	\
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 41, struct nvgpu_gpu_register_buffer_args)
/**
 * @ingroup NVGPURM_INTF_GET_BUFFER_INFO
 * @brief Command code for @ref NVGPURM_INTF_GET_BUFFER_INFO.
 */
#define NVGPU_GPU_IOCTL_GET_BUFFER_INFO	\
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 42, struct nvgpu_gpu_get_buffer_info_args)
/**
 * @ingroup NVGPURM_INTF_GET_GPC_LOCAL_TO_PHYSICAL_MAP
 * Get GPC Local-to-Physical map devctl command.
 */
#define NVGPU_GPU_IOCTL_GET_GPC_LOCAL_TO_PHYSICAL_MAP\
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 43, struct nvgpu_gpu_get_gpc_physical_map_args)
/**
 * @ingroup NVGPURM_INTF_GET_GPC_LOCAL_TO_LOGICAL_MAP
 * Get GPC Local-to-Logical map devctl command.
 */
#define NVGPU_GPU_IOCTL_GET_GPC_LOCAL_TO_LOGICAL_MAP\
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 44, struct nvgpu_gpu_get_gpc_logical_map_args)
/**
 * @ingroup NVGPURM_INTF_GET_PPC_MASKS
 * @brief Command code for @ref NVGPURM_INTF_GET_PPC_MASKS.
 */
#define NVGPU_GPU_IOCTL_GET_PPC_MASKS \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 45, struct nvgpu_gpu_get_ppc_masks_args)
/**
 * @ingroup NVGPURM_INTF_GET_ROP_MASKS
 * @brief Command code for @ref NVGPURM_INTF_GET_ROP_MASKS.
 */
#define NVGPU_GPU_IOCTL_GET_ROP_MASKS \
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 46, struct nvgpu_gpu_get_rop_masks_args)
#define NVGPU_GPU_IOCTL_GET_FBP_LOCAL_TO_PHYSICAL_MAP\
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 47, struct nvgpu_gpu_get_fbp_physical_map_args)
#define NVGPU_GPU_IOCTL_GET_FBP_LOCAL_TO_LOGICAL_MAP\
	_IOWR(NVGPU_GPU_IOCTL_MAGIC, 48, struct nvgpu_gpu_get_fbp_logical_map_args)
#define NVGPU_GPU_IOCTL_LAST		\
	_IOC_NR(NVGPU_GPU_IOCTL_GET_FBP_LOCAL_TO_LOGICAL_MAP)
#define NVGPU_GPU_IOCTL_MAX_ARG_SIZE	\
	sizeof(struct nvgpu_gpu_get_cpu_time_correlation_info_args)

#endif /* _UAPI__LINUX_NVGPU_CTRL_H__ */
