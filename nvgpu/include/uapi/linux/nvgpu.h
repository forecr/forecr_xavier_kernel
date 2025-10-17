/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _UAPI__LINUX_NVGPU_IOCTL_H
#define _UAPI__LINUX_NVGPU_IOCTL_H

#include "nvgpu-ctrl.h"
#include "nvgpu-event.h"
#include "nvgpu-as.h"

#include "nvgpu-nvs.h"

/**
 * @defgroup NVGPURM_LINUX_IOCTL NvGPU UAPI
 * @brief This is Linux NvGPU usermode API
 *
 * The NvGPU UAPI is accessed by opening a device node and invoking the
 * respective IOCTL commands on the opened connection. The ioctl() is a system
 * call initiated by the userspace and served by the nvgpu kernel driver. There
 * are other system calls like read/write/poll etc. available for specific nodes
 * but their usage is quite limited.
 *
 * The IOCTL interface is a message-passing interface. Every IOCTL has a
 * command argument, which multiplexes command id, direction, and size of
 * associated header structure. Certain IOCTL calls access data in additional buffers.
 * These buffers are generally referred to by pointer/size fields. There are some variations
 * in the way these additional buffers are passed. E.g. only address of virtual SM mapping
 * is passed in IOCTL @ref NVGPURM_INTF_VSMS_MAPPING, the size information is determined
 * using TPC and SM per TPC values. Individual IOCTL describes how the additional buffers
 * are passed.
 *
 * In the NvGPU UAPI, the general structure is as follows:
 *
 * <table><caption>General message passing format</caption>
 * <tr>
 *   <th>Header struct or additional buffer(s)</th>
 *   <th>REQUEST (usermode to RM in kernel)<br>@c _IOW and @c _IOWR directions</th>
 *   <th>RESPONSE (RM in kernel to usermode)<br>@c _IOR and @c _IOWR directions</th>
 * </tr>
 * <tr>
 *   <td>
 *     Header struct (Pointer passed as third argument of ioctl call, size is encoded in ioctl
 *     command)
 *   </td>
 *   <td>Fixed-size header: '[in]' fields</td>
 *   <td>Fixed-size header: '[out]' fields</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1 (optional. Pointer/size passed in header struct)</td>
 *   <td>Variable-length data</td>
 *   <td>Variable-length data</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 2 (optional. Pointer/size passed in header struct)</td>
 *   <td>Variable-length data</td>
 *   <td>Variable-length data</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer ... (optional. Pointer/size passed in header struct)</td>
 *   <td>Variable-length data</td>
 *   <td>Variable-length data</td>
 * </tr>
 * </table>
 *
 * See the Linux manual documentation on ioctl() for further information in
 * https://man7.org/linux/man-pages/man2/ioctl.2.html.
 * Note that IOCTL direction @c _IO does not pass a message in either
 * direction.
 *
 * The fields in the IOCTL command structures marked as reserved and
 * pad/padding are not used. Usermode should clear these fields when sending to
 * RM and ignore when receiving from RM.
 *
 * A range wherever mentioned denotes a valid range, and anything outside the
 * valid range is invalid. Common forms of specification are:
 *
 * -# Range specification: <tt>[a, b]</tt>. This means that @c value is valid
 *    when @c a &le; @c value &le; @c b.
 *
 * -# Bit mask specification: Bit mask of <tt>X</tt>. This means that @c value
 *    is valid when @c value BITWISE-OR @c X = @c X. This is often used for flag
 *    fields.
 *
 *    - Alternatively, flag fields may specify the prefix for valid flag
 *      constants. Multiple flags may be combined with BITWISE-OR.
 *
 * -# HW-dependent specification. For example, the set of valid values for
 *    @c #nvgpu_as_map_buffer_ex_args::compr_kind is HW-specific.
 *
 *    - Familiarity with the hardware is assumed.
 *
 * Several IOCTL commands send and receive additional buffers. Pointers of buffer are passed which
 * can be read and written to using copy_from_user() and copy_to_user() call by nvgpu-rm driver.
 * Most of these commands use an in/out length parameter in the argument header for the
 * variable-length message as follows
 *
 * - The [in] direction denotes the size of the buffer reserved by the caller. The nvgpu-rm kernel
 *   driver copies at most this many bytes to the buffer pointer, thus, avoiding overflowing the
 *   receive buffer.
 *
 * - The [out] direction denotes the full buffer size.
 *
 * For an example, see, *e.g.*, @ref NVGPURM_INTF_GET_ENGINE_INFO.
 *
 *
 * The per-device directory names for the supported igpu/dgpu devices are as follows:
 *
 * <table><caption>Per-device directory path</caption>
 * <tr>
 *   <th>Device node path</th>
 *   <th>Device type</th>
 *   <th>Remarks</th>
 * </tr>
 * <tr>
 *   <td><tt>/dev/nvgpu/igpu\%u/</tt></td>
 *   <td>iGPU (connected directly to the SoC)</td>
 *   <td>The igpu parameter is the iGPU device number.</td>
 * </tr>
 * <tr>
 *   <td><tt>/dev/nvgpu/igpu-xxxx:bb:dd.f/</tt></td>
 *   <td>iGPU (connected over internal PCIe bus)</td>
 *   <td>Here xxxx=domain, bb=bus, dd=device, f=function.</td>
 * </tr>
 * <tr>
 *   <td><tt>/dev/nvgpu/dgpu-xxxx:bb:dd.f/</tt></td>
 *   <td>dGPU</td>
 *   <td>Here xxxx=domain, bb=bus, dd=device, f=function.</td>
 * </tr>
 * <tr>
 *   <td><tt>/dev/nvgpu/igpu\%u/fgpu-\%u-\%u/</tt></td>
 *   <td>iGPU / MIG partition (connected directly to the SoC)</td>
 *   <td>The igpu parameter is the iGPU device number.
 *       The fgpu parameters denote the GPU instance
 *       (swizzle id) and GR/compute instance (sys pipe id).</td>
 * </tr>
 * <tr>
 *   <td><tt>/dev/nvgpu/dgpu-xxxx:bb:dd.f/fgpu-\%u-\%u/</tt></td>
 *   <td>dGPU / MIG partition</td>
 *   <td>Here xxxx=domain, bb=bus, dd=device, f=function.
 *       The fgpu parameters denote the GPU instance
 *       (swizzle id) and GR/compute instance (sys pipe id).</td>
 * </td>
 * <tr>
 *   <td><tt>/dev/nvgpu/igpu-xxxx:bb:dd.f/fgpu-\%u-\%u/</tt></td>
 *   <td>iGPU / MIG partition (connected over internal PCIe bus)</td>
 *   <td>Here xxxx=domain, bb=bus, dd=device, f=function.
 *       The fgpu parameters denote the GPU instance
 *       (swizzle id) and GR/compute instance (sys pipe id).</td>
 * </td>
 * </tr>
 * </table>
 *
 * The following device nodes are provided by nvgpu-rm kernel driver. The @c <gpu> is
 * determined as per the per-device directory table above.
 *
 * <table><caption>Device nodes</caption>
 * <tr>
 *   <th>Device node</th>
 *   <th>Functionality group</th>
 *   <th>Description</th>
 * </tr>
 * <tr>
 *   <td>@c /dev/nvgpu/<gpu>/ctrl</td>
 *   <td>@ref NVGPURM_LINUX_IOCTL_CTRL</td>
 *   <td>
 *     Device-specific (context independent) commands.
 *   </td>
 * </tr>
 * </table>
 */

/**
 * @defgroup NVGPURM_STD_IOCTL_CTRL NVGPU UAPI: Device control (safety subset)
 * @ingroup NVGPURM_STD_IOCTL
 * @brief Safety subset of the NvGPU resource manager usermode API:
 * device control functionality (@c /dev/nvgpu/<gpu>/ctrl).
 *
 * **About HW unit indexes and related terminology.** Certain multi-instance
 * units (namely, GPCs, TPCs, and SMs) can be indexed in multiple ways:
 *
 * <dl>
 *   <dt>Physical indexing</dt>
 *   <dd>Selects the unit by its physical location.</dd>
 *
 *   <dt>Logical indexing</dt>
 *   <dd>Selects the unit by its location after excluding disabled units.
 *       The indexes are contiguous from 0 to the number of enabled
 *       units - 1.</dd>
 *
 *   <dt>Local indexing (within MIG partition)</dt>
 *   <dd>Similar to logical indexing, but the set of enabled units are those
 *       within the MIG partition.</dd>
 *
 *   <dt>Local indexing (within GPC or TPC)</dt>
 *   <dd>Similar to logical indexing, but the set of enabled units are those
 *       within the parent unit.</dd>
 *
 *   <dt>Virtual indexing</dt>
 *   <dd>Units are selected through a mapping. Notably, virtual SMs are mapped
 *       to physical units, and different contexts may use different mappings
 *       for added resiliency.
 *       (See @ref NVGPURM_INTF_VSMS_MAPPING,
 *       @c #NVGPU_GPU_FLAGS_SUPPORT_SM_DIVERSITY.)
 * </dl>
 *
 * In addition, the GPU has logical copy engines which have a set of physical
 * copy engines associated for copy operations. @ref
 * NVGPURM_INTF_GET_ENGINE_INFO reports the available logical copy engine
 * instances. Logical-to-physical copy engine mapping is not reported by the
 * NVGPU UAPI.
 */

/**
 * @defgroup NVGPURM_LINUX_IOCTL_CHANNEL NVGPU UAPI: Channel
 * @ingroup NVGPURM_LINUX_IOCTL
 * @brief NvGPU resource manager usermode API: channel functionality.
 *
 * Channel is created using @ref NVGPURM_LINUX_IOCTL_CTRL ioctl @ref NVGPURM_INTF_OPEN_CHANNEL.
 * This ioctl returns file descriptor which can be used to issue channel specific ioctl.
 * The channel open sequence is as follows:
 *
 * <table><caption>Channel open sequence</caption>
 * <tr>
 *   <th>STEP</th>
 *   <th>Action</th>
 *   <th>Required</th>
 *   <th>Remarks</th>
 * </tr>
 * <tr>
 *   <td>1</td>
 *   <td>Channel creation: @ref NVGPURM_INTF_OPEN_CHANNEL</td>
 *   <td>Yes</td>
 *   <td>N/A</td>
 * </tr>
 * <tr>
 *   <td>2</td>
 *   <td>Bind to address space: @ref NVGPURM_INTF_AS_BIND_CHANNEL</td>
 *   <td>No</td>
 *   <td>N/A</td>
 * </tr>
 * <tr>
 *   <td>3</td>
 *   <td>Bind to TSG: @ref NVGPURM_INTF_TSG_BIND_CHANNEL_EX</td>
 *   <td>No</td>
 *   <td>N/A</td>
 * </tr>
 * <tr>
 *   <td>4</td>
 *   <td>Set channel watchdog: @ref NVGPURM_INTF_CHANNEL_WDT</td>
 *   <td>No</td>
 *   <td>Default is platform-specific.</td>
 * </tr>
 * <tr>
 *   <td>5</td>
 *   <td>Set channel object: @ref NVGPURM_INTF_ALLOC_OBJ_CTX</td>
 *   <td>Yes</td>
 *   <td>N/A</td>
 * </tr>
 * <tr>
 *   <td>6</td>
 *   <td>
 *     Allocate/set GPFIFO, USERD: @ref NVGPURM_INTF_ALLOC_SETUP_BIND /
 *     @ref NVGPURM_INTF_ALLOC_OBJ_CTX
 *   </td>
 *   <td>Yes</td>
 *   <td>N/A</td>
 * </tr>
 * <tr>
 *   <td>7</td>
 *   <td>Allocate/get channel syncpoint: @ref NVGPURM_INTF_CHANNEL_GET_SYNCPT</td>
 *   <td>No</td>
 *   <td>Channel syncpoint allocatable only when
 *     @c #NVGPU_CHANNEL_SETUP_BIND_FLAGS_DETERMINISTIC is specified.</td>
 * </tr>
 * <tr>
 *   <td>8</td>
 *   <td>Set error notifier: @ref NVGPURM_INTF_SET_ERR_NOTIF</td>
 *   <td>No</td>
 *   <td>Note: May be omitted in non-production use.</td>
 * </tr>
 * <tr>
 *   <td>9</td>
 *   <td>Set preemption modes: @ref NVGPURM_INTF_SET_PREEMPTION_MODE</td>
 *   <td>No</td>
 *   <td>Default is @c #NVGPU_GRAPHICS_PREEMPTION_MODE_WFI /
 *       @c #NVGPU_COMPUTE_PREEMPTION_MODE_WFI</td>
 * </tr>
 * </table>
 *
 * The channel close sequence is as follows:
 *
 * <table><caption>Channel close sequence</caption>
 * <tr>
 *   <th>STEP</th>
 *   <th>Action</th>
 *   <th>Required</th>
 *   <th>Remarks</th>
 * </tr>
 * <tr>
 *   <td>1</td>
 *   <td>Wait for idle</td>
 *   <td>Yes</td>
 *   <td>Yes</td>
 *   <td>The caller must ensure that the channel is idle. See, *e.g.*,
 *     https://p4viewer.nvidia.com/get//hw/doc/gpu/ampere_info/gen_manuals/ga10b/dev_pbdma.ref /
 *     NV_UDMA_WFI</td>
 * </tr>
 * <tr>
 *   <td>2</td>
 *   <td>Unbind from TSG: @ref NVGPURM_INTF_TSG_UNBIND_CHANNEL</td>
 *   <td>No</td>
 *   <td>Currently on QNX, unbinding is done when channel fd is close.</td>
 * </tr>
 * <tr>
 *   <td>3</td>
 *   <td>Close the channel fd (connection id).</td>
 *   <td>Yes</td>
 *   <td>N/A</td>
 * </tr>
 * </table>
 */

/**
 * @defgroup NVGPURM_LINUX_IOCTL_COMMON NVGPU UAPI: Common definitions
 * @ingroup NVGPURM_LINUX_IOCTL
 * @brief NvGPU resource manager usermode API: common definitions.
 */

/**
 * @defgroup NVGPURM_LINUX_IOCTL_TSG NVGPU UAPI: TSG
 * @ingroup NVGPURM_LINUX_IOCTL
 * @brief NvGPU resource manager usermode API: TSG (time-slice group) functionality.
 *
 * When @c #NVGPU_GPU_FLAGS_MULTI_PROCESS_TSG_SHARING is set, @ref NVGPURM_INTF_OPEN_TSG initializes
 * a new GPU TSG or reuses an already initialized GPU TSG. This choice is indicated by
 * @c #NVGPU_GPU_IOCTL_OPEN_TSG_FLAGS_SHARE.
 *
 * When @c #NVGPU_GPU_FLAGS_MULTI_PROCESS_TSG_SHARING is not set (legacy
 * behavior), the TSG sharing feature is not available and hence @ref NVGPURM_INTF_OPEN_TSG
 * shall initialize a new GPU TSG device if available.
 *
 * The TSG open sequence is as follows:
 *
 * <table><caption>GPU TSG open sequence</caption>
 * <tr>
 *   <th>STEP</th>
 *   <th>Action</th>
 *   <th>Required</th>
 *   <th>Remarks</th>
 * </tr>
 * <tr>
 *   <td>1</td>
 *   <td>Initialize the TSG: @ref NVGPURM_INTF_TSG_OPEN_TSG</td>
 *   <td>Yes</td>
 *   <td>Depending on @c #NVGPU_GPU_IOCTL_OPEN_TSG_FLAGS_SHARE, creates a new
 *       TSG or shares an existing TSG, returns a TSG fd</td>
 * </tr>
 * </table>
 *
 * The TSG close sequence is as follows:
 *
 * <table><caption>GPU TSG close sequence</caption>
 * <tr>
 *   <th>STEP</th>
 *   <th>Action</th>
 *   <th>Required</th>
 *   <th>Remarks</th>
 * </tr>
 * <tr>
 *   <td>1</td>
 *   <td>Close the TSG fd.</td>
 *   <td>Yes</td>
 *   <td>N/A</td>
 * </tr>
 * </table>
 */

/**
 * @defgroup NVGPURM_LINUX_IOCTL_AS NVGPU UAPI: Address Space
 * @ingroup NVGPURM_LINUX_IOCTL
 * @brief NvGPU resource manager usermode API:
 * Virtual address space functionality.
 */

#define NVGPU_TSG_IOCTL_MAGIC 'T'

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_BIND_CHANNEL_EX IOCTL: NVGPU_TSG_IOCTL_BIND_CHANNEL_EX
 * @brief Binds a channel to the TSG.
 *
 * This command binds a channel to the TSG of the node receiving the
 * command. One channel can be part of at most one TSG, but one TSG can have
 * multiple channels bound to it.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_tsg_bind_channel_ex_args</td>
 *   <td>@c #nvgpu_tsg_bind_channel_ex_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---     Flag @c #NVGPU_ALLOC_GPFIFO_EX_FLAGS_VPR_ENABLED is set
 *                     which is no longer supported.
 * - @c EINVAL ---  Input out of range
 * - @c EINVAL ---  GPU TSG is not initialized (see @ref NVGPURM_LINUX_IOCTL_TSG).
 * - @c EINVAL ---  If hardware does not support subcontexts.
 * - @c EINVAL ---  If VM pointer does not match for channel and subcontext.
 * - @c ENOSPC ---  If sync veid @c 0 is passed but TSG's sync_veid already set to true.
 * - @c ENODEV ---  Invalid @c #nvgpu_tsg_bind_channel_ex_args::channel_fd
 * - @c ENODEV ---  GPU not ready
 * - @c EPERM ---   Scheduler control is locked
 * - @c EFAULT ---  Copy from user space fails
 *
 * @remark In safety, each channel must be bound to a TSG before work can be
 * submitted to the channel.
 *
 * @sa @ref NVGPURM_INTF_TSG_UNBIND_CHANNEL
 */

/**
 * @ingroup NVGPURM_INTF_TSG_BIND_CHANNEL_EX
 * @brief Header struct for @ref NVGPURM_INTF_TSG_BIND_CHANNEL_EX.
 */
struct nvgpu_tsg_bind_channel_ex_args {
	/**
	 * @brief Channel fd to bind
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Channel fd to bind.
	 *
	 *     Range: Valid channel fd (see device node of @ref
	 *     NVGPURM_LINUX_IOCTL_CHANNEL).
	 *   </dd>
	 * </dl>
	 */
	__s32 channel_fd;

	/**
	 * @brief Subcontext id (VEID)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Subcontext id (VEID) of for the channel.
	 *
	 *     Range: Valid subcontext id from a previous call to
	 *     @ref NVGPURM_INTF_TSG_CREATE_SUBCONTEXT.
	 *   </dd>
	 * </dl>
	 */
	__u32 subcontext_id;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u8 reserved[16];
};

struct nvgpu_tsg_bind_scheduling_domain_args {
	/* in: id of the domain this tsg will be bound to */
	__s32 domain_fd;
	/* Must be set to 0 */
	__s32 reserved0;
	/* Must be set to 0 */
	__u64 reserved[3];
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_UNBIND_CHANNEL IOCTL: NVGPU_TSG_IOCTL_UNBIND_CHANNEL
 * @brief Unbinds a channel from the TSG. See caveats.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c int -- channel fd</td>
 *   <td>@c int (unused)</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---  GPU TSG is not initialized (see @ref NVGPURM_LINUX_IOCTL_TSG).
 * - @c EFAULT ---  Copy from user space fails
 *
 * @note This IOCTL command has caveats:
 * -# In QNX, this does nothing. The channel is unbound from the TSG when the
 *    channel fd is closed. This command exists for compatibility with Linux. In
 *    Linux, closing the fd does not always close the underlying channel
 *    immediately due to reference counting.
 */

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_READ_SM_ERR_STATE IOCTL: NVGPU_TSG_IOCTL_READ_SINGLE_SM_ERROR_STATE
 * @brief Reads the error state of a single SM.
 *
 * This command retrieves the SM error state, which holds the error
 * information after an SM exception.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_tsg_read_single_sm_error_state_args</td>
 *   <td>@c #nvgpu_tsg_read_single_sm_error_state_args</td>
 * </tr>
 * <tr>
 *   <td>@c Additional buffer 1 (@c #nvgpu_tsg_sm_error_state_record::record_mem)</td>
 *   <td>(none)</td>
 *   <td>@c #nvgpu_tsg_sm_error_state_record</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL --- Input(s) out of range.
 * - @c EINVAL --- GPU TSG is not initialized (see @ref NVGPURM_LINUX_IOCTL_TSG).
 * - @c EIO ---    Failed to write the error state record to the output buffer.
 * - @c EFAULT --- Copy from user space fails
 *
 * @remark An error notifier buffer can be assigned to the channel to receive a
 * notification when an SM exception occurs. (See @ref
 * NVGPURM_INTF_SET_ERR_NOTIF.)
 */

/**
 * @ingroup NVGPURM_INTF_TSG_READ_SM_ERR_STATE
 * @brief SM error state record
 *
 * This struct describes the SM error record format.
 */
struct nvgpu_tsg_sm_error_state_record {
	/** @brief Global error status register. */
	__u32 global_esr;

	/** @brief Warp error status register. */
	__u32 warp_esr;

	/** @brief Warp error status register program counter. */
	__u64 warp_esr_pc;

	/** @brief Global error status register report Mask. */
	__u32 global_esr_report_mask;

	/** @brief Warp error status register report Mask. */
	__u32 warp_esr_report_mask;

	/** @brief CGA error status register. */
	__u32 cga_esr;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;

};

/**
 * @ingroup NVGPURM_INTF_TSG_READ_SM_ERR_STATE
 * @brief Header struct for #NVGPU_TSG_IOCTL_READ_SINGLE_SM_ERROR_STATE.
 */
struct nvgpu_tsg_read_single_sm_error_state_args {
	/**
	 * @brief SM identifier
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     SM identifier.
	 *
	 *     Range: [0, (Number of SMs - 1)] (see @ref NVGPURM_INTF_VSMS_MAPPING)
	 *   </dd>
	 * </dl>
	 *
	 */
	__u32 sm_id;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;

	/** @brief Pointer to output buffer.
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt>
	 *   <dd>
	 *     Pointer to output buffer which should be of atleast @c record_size.
	 *   </dd>
	 * </dl>
	 */
	__u64 record_mem;

	/**
	 * @brief Record size
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Size (in bytes) of the record to read. Should be
	 *     <tt>sizeof(struct nvgpu_tsg_sm_error_state_record)</tt>.
	 *     nvgpu-rm copies the record up to @c record_size in the output
	 *     buffer.
	 *
	 *     Range: (unspecified)
	 *   </dd>
	 * </dl>
	 */
	__u64 record_size;
};

/*
 * This struct helps to read SM error states for all the SMs
 */
struct nvgpu_tsg_read_all_sm_error_state_args {
	/*
	 * in: Number of SM error states to be returned. Must be equal to the number of SMs.
	 */
	__u32 num_sm;
	/*
	 * Padding to make KMD UAPI compatible with both 32-bit and 64-bit callers.
	 */
	__u32 reserved;
	/*
	 * out: This points to an array of nvgpu_tsg_read_single_sm_error_state_args.
	 */
	__u64 buffer_mem;
	/* in: size of the buffer to store error states */
	__u64 buffer_size;
};

/*
 * This struct is used to read and configure l2 max evict_last
 * setting.
 */
struct nvgpu_tsg_l2_max_ways_evict_last_args {
	/*
	 * Maximum number of ways in a l2 cache set that can be allocated
	 * with eviction_policy=EVICT_LAST
	 */
	__u32 max_ways;
	__u32 reserved;
};

/*
 * This struct contains the parameter for configuring L2 sector promotion.
 * It supports 3 valid options:-
 *  - PROMOTE_NONE(1): cache-miss doens't get promoted.
 *  - PROMOTE_64B(2): cache-miss gets promoted to 64 bytes if less than 64 bytes.
 *  - PROMOTE_128B(4): cache-miss gets promoted to 128 bytes if less than 128 bytes.
 */
#define NVGPU_GPU_IOCTL_TSG_L2_SECTOR_PROMOTE_FLAG_NONE		(1U << 0U)
#define NVGPU_GPU_IOCTL_TSG_L2_SECTOR_PROMOTE_FLAG_64B		(1U << 1U)
#define NVGPU_GPU_IOCTL_TSG_L2_SECTOR_PROMOTE_FLAG_128B		(1U << 2U)
struct nvgpu_tsg_set_l2_sector_promotion_args {
	/* Valid promotion flag */
	__u32 promotion_flag;
	__u32 reserved;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_CREATE_SUBCONTEXT IOCTL: NVGPU_TSG_IOCTL_CREATE_SUBCONTEXT
 * @brief Creates a subcontext within the TSG.
 *
 * This IOCTL creates a subcontext of specified type within the TSG and returns
 * VEID for it. The newly created subcontext is bound to the user-supplied
 * address space.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_tsg_create_subcontext_args</td>
 *   <td>@c #nvgpu_tsg_create_subcontext_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL --- Invalid parameters specified.
 * - @c EINVAL --- GPU TSG is not initialized (see @ref NVGPURM_LINUX_IOCTL_TSG).
 * - @c ENOSPC --- Subcontext of the specified type could not be allocated.
 * - @c EFAULT --- Copy from user space fails
 *
 * @pre @c #NVGPU_GPU_FLAGS_SUPPORT_TSG_SUBCONTEXTS
 */

/* Subcontext types */

/**
 * @ingroup NVGPURM_INTF_TSG_CREATE_SUBCONTEXT
 * @brief Synchronous subcontext.
 *
 * Subcontext of this type may hold the
 * graphics channel and multiple copy engine and compute channels.
 */
#define NVGPU_TSG_SUBCONTEXT_TYPE_SYNC               (0x0U)

/**
 * @ingroup NVGPURM_INTF_TSG_CREATE_SUBCONTEXT
 * @brief Asynchronous subcontext.
 *
 * Asynchronous subcontext is for compute
 * and copy engine channels only.
 */
#define NVGPU_TSG_SUBCONTEXT_TYPE_ASYNC              (0x1U)

/**
 * @ingroup NVGPURM_INTF_TSG_CREATE_SUBCONTEXT
 * @brief Header argument struct for @ref NVGPURM_INTF_TSG_CREATE_SUBCONTEXT.
 */
struct nvgpu_tsg_create_subcontext_args {
	/**
	 * @brief Subcontext type
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Subcontext type.
	 *
	 *     Range: One of @c #NVGPU_TSG_SUBCONTEXT_TYPE_SYNC,
	 *     @c #NVGPU_TSG_SUBCONTEXT_TYPE_ASYNC.
	 *   </dd>
	 * </dl>
	 */
	__u32 type;

	/**
	 * @brief Address space fd for the subcontext
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Address space file descriptor for the subcontext.
	 *
	 *     Range: Valid address space fd (see device node of @ref
	 *     NVGPURM_LINUX_IOCTL_AS)
	 *   </dd>
	 * </dl>
	 */
	__s32 as_fd;

	/**
	 * @brief Subcontext id (VEID).
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>
	 *     Subcontext identifier (equals to VEID in current HW design).
	 *   </dd>
	 * </dl>
	 *
	 * @remark This is a HW-specific value. For Volta+ chips (up to T264
	 * Legacy mode), the HW-specific range is [0..63]. Of these, VEID 0 is
	 * for the @c #NVGPU_TSG_SUBCONTEXT_TYPE_SYNC subcontext and the
	 * remaining numbers are for @c #NVGPU_TSG_SUBCONTEXT_TYPE_ASYNC
	 * subcontexts.
	 *
	 * @remark In case of SMC, a smaller number of VEIDs is supported per
	 * GPU instance. In this case, the reported VEID is relative to the SMC.
	 *
	 * @remark The caller should not assume any specific meaning for
	 * subcontext identifier values.
	 */
	__u32 veid;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_DELETE_SUBCONTEXT IOCTL: NVGPU_TSG_IOCTL_DELETE_SUBCONTEXT
 * @brief Deletes a subcontext within the TSG.
 *
 * This IOCTL deletes a previously created subcontext within the TSG. After a
 * successful call, the provided subcontext id (VEID) is freed.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_tsg_delete_subcontext_args</td>
 *   <td>(none)</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL --- Invalid parameters specified.
 * - @c EINVAL --- GPU TSG is not initialized (see @ref NVGPURM_LINUX_IOCTL_TSG).
 * - @c EFAULT --- Copy from user space fails
 */

/**
 * @ingroup NVGPURM_INTF_TSG_DELETE_SUBCONTEXT
 * @brief Header argument struct for @ref NVGPURM_INTF_TSG_DELETE_SUBCONTEXT.
 */
struct nvgpu_tsg_delete_subcontext_args {
	/**
	 * @brief Subcontext id (VEID)
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>
	 *     Identifier of the subcontext to close.
	 *
	 *     Range: Subcontext id (VEID) received from a previous call to
	 *     @ref NVGPURM_INTF_TSG_DELETE_SUBCONTEXT.
	 *   </dd>
	 * </dl>
	 */
	__u32 veid;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_GET_SHARE_TOKEN IOCTL: NVGPU_TSG_IOCTL_GET_SHARE_TOKEN
 * @brief Creates a single use TSG sharing token.
 *
 * This IOCTL creates a share token for the underlying GPU TSG. TSG sharing
 * allows multiple TSG fds to refer to the same underlying GPU TSG, even across
 * different processes.
 *
 * When shared, the underlying GPU TSG is reference-counted. The underlying GPU
 * TSG is closed when all TSG fds bound to it has been closed.
 *
 * When the underlying TSG is closed, all in-flight share tokens for the GPU TSG
 * are invalidated. A share token can also be explicitly invalidated by a call
 * to @ref NVGPURM_INTF_TSG_REVOKE_SHARE_TOKEN.
 *
 * Argument @c #nvgpu_tsg_get_share_token_args::target_device_instance_id
 * specifies the recipient of the share token. When the token is consumed by a
 * call to @ref NVGPURM_INTF_TSG_OPEN_TSG, the device instance id of the TSG
 * parent ctrl node must match. (See <tt>@ref
 * nvgpu_gpu_characteristics::device_instance_id</tt>.) This allows the share
 * token to be consumed only by a dedicated receiver for secure sharing.
 *
 * When @c #NVGPU_GPU_FLAGS_MULTI_PROCESS_TSG_SHARING is unset, this IOCTL
 * returns an error.
 *
 * TBD: TSG share tokens have an expiration time. This is controlled
 * by /sys/kernel/debug/<gpu>/tsg_share_token_timeout_ms. The default
 * timeout is 30000 ms on silicon platforms and 0 (= no timeout) on
 * pre-silicon platforms.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_tsg_get_share_token_args</td>
 *   <td>@c #nvgpu_tsg_get_share_token_args</td>
 * </tr>
 * </table>
 *
 * @pre @c #NVGPU_GPU_FLAGS_MULTI_PROCESS_TSG_SHARING
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c ENODEV ---  GPU not ready.
 * - @c ENOTTY ---  When @c #NVGPU_GPU_FLAGS_MULTI_PROCESS_TSG_SHARING is unset.
 * - @c EINVAL ---  Invalid @c #nvgpu_tsg_get_share_token_args::target_device_instance_id.
 * - @c EINVAL ---  Invalid @c #nvgpu_tsg_get_share_token_args::share_token.
 * - @c ENOSPC ---  All subcontexts are in-use.
 * - @c ENOMEM ---  Memory allocation for share token failed.
 * - @c EINVAL ---  GPU TSG is not initialized (see @ref NVGPURM_LINUX_IOCTL_TSG).
 * - @c EFAULT ---  Copy from user space fails
 *
 * **Example:** TSG sharing sequence
 *
 * Typical TSG sharing sequence is as follows.
 *
 * <table>
 * <tr>
 *   <th>STEP</th><th>PARTICIPANT</th><th>Operation</th>
 * </tr>
 * <tr>
 *   <td>@c 1</td>
 *   <td>Importer</td>
 *   <td>Send @c #nvgpu_gpu_characteristics::device_instance_id to
 *	 TSG exporter.</td>
 * </tr>
 * <tr>
 *   <td>@c 2</td>
 *   <td>Exporter</td>
 *   <td>Create a share token for Importer by calling @ref
 *	 NVGPURM_INTF_TSG_GET_SHARE_TOKEN.</td>
 * </tr>
 * <tr>
 *   <td>@c 3</td>
 *   <td>Exporter</td>
 *   <td>Sends share token to Importer (value copy).</td>
 * </tr>
 * <tr>
 *   <td>@c 4</td>
 *   <td>Importer</td>
 *   <td>Open a TSG using the share token. See the sequence in @ref
 *	 NVGPURM_INTF_TSG_OPEN_TSG.</td>
 * </tr>
 * <tr>
 *   <td>@c 5</td>
 *   <td>Importer</td>
 *   <td>Create subcontext in the shared TSG by calling @ref
 *	 NVGPURM_INTF_TSG_CREATE_SUBCONTEXT.</td>
 * </tr>
 * </table>
 *
 * @remark In case the sharing sequence fails after the share token has been
 * generated, it is recommended that the share token is revoked by a call to
 * @ref NVGPURM_INTF_TSG_REVOKE_SHARE_TOKEN.
 *
 * @remark Expiration of unconsumed TSG share tokens is not currently
 * implemented.
 *
 * @remark The share token is created by concatenating the 64-bit source device
 * instance identifier and a 64-bit number. This may change in a future release.
 */

/**
 * @ingroup NVGPURM_INTF_TSG_GET_SHARE_TOKEN
 * @brief Header argument struct for @ref NVGPURM_INTF_TSG_GET_SHARE_TOKEN.
 */
struct nvgpu_tsg_get_share_token_args {
	/**
	 * @brief Target device instance id.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     The target device instance id for this share token.
	 *
	 *     Range: Valid control device instance id
	 *     (see @c #nvgpu_gpu_characteristics::device_instance_id).
	 *   </dd>
	 * </dl>
	 */
	__u64 target_device_instance_id;

	/**
	 * @brief Share token (non-confidential).
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>
	 *     A single-use share token that references this TSG. The caller (exporter)
	 *     should send this share token to the importer by its own custom
	 *     protocol.
	 *     <p>
	 *     A share token is generally considered non-confidential, since it
	 *     can be used only by the dedicated recipient.
	 *     <p>
	 *     Range: (unspecified)
	 *   </dd>
	 * </dl>
	 */
	__u8 share_token[NVGPU_TSG_SHARE_TOKEN_SIZE];
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_REVOKE_SHARE_TOKEN IOCTL: NVGPU_TSG_IOCTL_REVOKE_SHARE_TOKEN
 * @brief Revokes a TSG share token.
 *
 * This IOCTL revokes a TSG share token. It is intended to be called when the
 * share token data exchange with the other endpoint (importer) is
 * unsuccessful. See @ref NVGPURM_INTF_TSG_GET_SHARE_TOKEN for details.
 *
 * When @c #NVGPU_GPU_FLAGS_MULTI_PROCESS_TSG_SHARING is unset, this IOCTL
 * returns an error.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_tsg_get_share_token_args</td>
 *   <td>(none)</td>
 * </tr>
 * </table>
 *
 * @pre @c #NVGPU_GPU_FLAGS_MULTI_PROCESS_TSG_SHARING
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c ENODEV ---  GPU not ready.
 * - @c ENOTTY ---  When @c #NVGPU_GPU_FLAGS_MULTI_PROCESS_TSG_SHARING is unset.
 * - @c EINVAL ---  Invalid @c #nvgpu_tsg_revoke_share_token_args::target_device_instance_id.
 * - @c EINVAL ---  Invalid @c #nvgpu_tsg_revoke_share_token_args::share_token.
 * - @c EINVAL ---  GPU TSG is not initialized (see @ref NVGPURM_LINUX_IOCTL_TSG).
 * - @c EFAULT ---  Copy from user space fails
 */

/**
 * @ingroup NVGPURM_INTF_TSG_REVOKE_SHARE_TOKEN
 * @brief Header argument struct for @ref NVGPURM_INTF_TSG_REVOKE_SHARE_TOKEN.
 */
struct nvgpu_tsg_revoke_share_token_args {
	/**
	 * @brief Target device instance id
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     The target/importer device instance id with which the GPU TSG is
	 *     shared with.
	 *
	 *     Range: Valid control device instance id
	 *     (<tt>@ref nvgpu_gpu_characteristics::device_instance_id</tt>)
	 *   </dd>
	 * </dl>
	 */
	__u64 target_device_instance_id;

	/**
	 * @brief Share token (non-confidential)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Revoke the share token in the scenario when token sharing
	 *     with other endpoint is not successful.
	 *
	 *     Range: Valid share token received from
	 *     <tt>@ref nvgpu_tsg_get_share_token_args::share_token</tt>.
	 *   </dd>
	 * </dl>
	 */
	__u8 share_token[NVGPU_TSG_SHARE_TOKEN_SIZE];
};

#define NVGPU_TSG_IOCTL_BIND_CHANNEL \
	_IOW(NVGPU_TSG_IOCTL_MAGIC, 1, int)
/**
 * @ingroup NVGPURM_INTF_TSG_UNBIND_CHANNEL
 * @brief Command code for @ref NVGPURM_INTF_TSG_UNBIND_CHANNEL.
 */
#define NVGPU_TSG_IOCTL_UNBIND_CHANNEL \
	_IOW(NVGPU_TSG_IOCTL_MAGIC, 2, int)

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_ENABLE IOCTL: NVGPU_IOCTL_TSG_ENABLE
 * @brief Enable the TSG in runlist.
 *
 * This command enables the TSG to be scheduled by the runlist. The
 * TSG can be previously disabled in the runlist, e.g.,:
 * - as a consequence of a trap/error; or
 * - explicitly with NVGPU_IOCTL_TSG_DISABLE
 *
 * By default, TSG is enabled.
 *
 * nvgpu module performs the following sequence to enable the TSG in the
 * runlist:
 * -# For all the channels bound to this TSG, enable each of the channel
 *    in the runlist.
 * -# kick the doorbell with the runlist and channel(last) identifier to
 *    kick-start processing of the channels again.
 *
 * When TSG Sharing is enabled and same TSG is shared by multiple processes,
 * then, the TSG is enabled only when all processes have issued TSG enable
 * (this ioctl) that did a TSG disable(NVGPU_IOCTL_TSG_DISABLE) or have
 * exited keeping the TSG disabled.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>(none)</td>
 *   <td>(none)</td>
 *   <td>(none)</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---    GPU TSG is not initialized (With TSG Sharing).
 * - @c EFAULT ---    Copy from user space fails
 */

/**
 * @ingroup NVGPURM_INTF_TSG_ENABLE
 * @brief Command code for @ref NVGPURM_INTF_TSG_ENABLE.
 */
#define NVGPU_IOCTL_TSG_ENABLE \
	_IO(NVGPU_TSG_IOCTL_MAGIC, 3)

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_DISABLE IOCTL: NVGPU_IOCTL_TSG_DISABLE
 * @brief Disable the TSG in runlist.
 *
 * On issuing this command, the TSG may not get disabled immediately in
 * the runlist if the scheduler is still executing the same runlist which
 * has this TSG as active or loaded. Issue @c #NVGPU_IOCTL_TSG_PREEMPT
 * immediately after disabling the TSG (this ioctl) to ensure TSG is not
 * active or running any further.
 *
 * nvgpu module performs the following sequence to disable the TSG in the
 * runlist:
 * -# For all the channels bound to this TSG, disable each of the channel
 *    in the runlist.
 *
 * When TSG Sharing is enabled and same TSG is shared by multiple processes,
 * then:
 * - TSG shall get disabled even when one of the processes disables it.
 * - TSG remains disabled unless all processes which disabled the TSG
 *    have issued TSG enable NVGPU_IOCTL_TSG_ENABLE or have closed their TSG
 *    handle.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>(none)</td>
 *   <td>(none)</td>
 *   <td>(none)</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---    GPU TSG is not initialized (With TSG Sharing).
 * - @c EFAULT ---    Copy from user space fails
 */

/**
 * @ingroup NVGPURM_INTF_TSG_DISABLE
 * @brief Command code for @ref NVGPURM_INTF_TSG_DISABLE.
 */
#define NVGPU_IOCTL_TSG_DISABLE \
	_IO(NVGPU_TSG_IOCTL_MAGIC, 4)

/**
 * @ingroup NVGPURM_LINUX_IOCTL_TSG
 * @defgroup NVGPURM_INTF_TSG_PREEMPT IOCTL: NVGPU_IOCTL_TSG_PREEMPT
 * @brief Preempts a TSG.
 *
 * This command requests a TSG preemption. Preemption interrupts the TSG
 * execution and prepares it for context switch-out. This is done by saving the
 * state and draining the GPU pipelines as necessary.
 *
 * The resource manager performs the following sequence to preempt the TSG:
 * -# Disable the TSG (if not already disabled).
 * -# Preempt the TSG.
 * -# Wait for the completion of TSG preemption.
 * -# Re-enable the TSG (if it was not disabled prior to the call).
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>(none)</td>
 *   <td>(none)</td>
 *   <td>(none)</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---    GPU TSG is not initialized (see @ref NVGPURM_LINUX_IOCTL_TSG).
 * - @c ETIMEDOUT --- Preemption did not complete within the
 *         preemption poll timeout.
 * - @c EFAULT ---    Copy from user space fails
 */

/**
 * @ingroup NVGPURM_INTF_TSG_PREEMPT
 * @brief Command code for @ref NVGPURM_INTF_TSG_PREEMPT.
 */
#define NVGPU_IOCTL_TSG_PREEMPT \
	_IO(NVGPU_TSG_IOCTL_MAGIC, 5)
#define NVGPU_IOCTL_TSG_EVENT_ID_CTRL \
	_IOWR(NVGPU_TSG_IOCTL_MAGIC, 7, struct nvgpu_event_id_ctrl_args)
#define NVGPU_IOCTL_TSG_SET_RUNLIST_INTERLEAVE \
	_IOW(NVGPU_TSG_IOCTL_MAGIC, 8, struct nvgpu_runlist_interleave_args)
#define NVGPU_IOCTL_TSG_SET_TIMESLICE \
	_IOW(NVGPU_TSG_IOCTL_MAGIC, 9, struct nvgpu_timeslice_args)
#define NVGPU_IOCTL_TSG_GET_TIMESLICE \
	_IOR(NVGPU_TSG_IOCTL_MAGIC, 10, struct nvgpu_timeslice_args)
/**
 * @ingroup NVGPURM_INTF_TSG_BIND_CHANNEL_EX
 *
 * @brief Command code for @ref NVGPURM_INTF_TSG_BIND_CHANNEL_EX.
 */
#define NVGPU_TSG_IOCTL_BIND_CHANNEL_EX \
	_IOWR(NVGPU_TSG_IOCTL_MAGIC, 11, struct nvgpu_tsg_bind_channel_ex_args)
/**
 * @ingroup NVGPURM_INTF_TSG_READ_SM_ERR_STATE
 *
 * @brief Command code for @ref NVGPURM_INTF_TSG_READ_SM_ERR_STATE.
 */
#define NVGPU_TSG_IOCTL_READ_SINGLE_SM_ERROR_STATE \
	_IOWR(NVGPU_TSG_IOCTL_MAGIC, 12, \
			struct nvgpu_tsg_read_single_sm_error_state_args)
/**
 * @ingroup NVGPURM_INTF_TSG_SET_L2_MAX_WAYS_EVICT_LAST
 * Configure L2 evict last policy.
 */
#define NVGPU_TSG_IOCTL_SET_L2_MAX_WAYS_EVICT_LAST \
	_IOW(NVGPU_TSG_IOCTL_MAGIC, 13, \
			struct nvgpu_tsg_l2_max_ways_evict_last_args)
/**
 * @ingroup NVGPURM_INTF_TSG_GET_L2_MAX_WAYS_EVICT_LAST
 * Read L2 evict last policy.
 */
#define NVGPU_TSG_IOCTL_GET_L2_MAX_WAYS_EVICT_LAST \
	_IOR(NVGPU_TSG_IOCTL_MAGIC, 14, \
			struct nvgpu_tsg_l2_max_ways_evict_last_args)
/**
 * @ingroup NVGPURM_INTF_TSG_SET_L2_SECTOR_PROMOTION
 * Configure L2 sector promotion policy.
 */
#define NVGPU_TSG_IOCTL_SET_L2_SECTOR_PROMOTION \
	_IOW(NVGPU_TSG_IOCTL_MAGIC, 15, \
			struct nvgpu_tsg_set_l2_sector_promotion_args)
#define NVGPU_TSG_IOCTL_BIND_SCHEDULING_DOMAIN \
	_IOW(NVGPU_TSG_IOCTL_MAGIC, 16, \
			struct nvgpu_tsg_bind_scheduling_domain_args)
#define NVGPU_TSG_IOCTL_READ_ALL_SM_ERROR_STATES \
	_IOWR(NVGPU_TSG_IOCTL_MAGIC, 17, \
			struct nvgpu_tsg_read_all_sm_error_state_args)
/**
 * @ingroup NVGPURM_INTF_TSG_CREATE_SUBCONTEXT
 *
 * @brief Command code for @ref NVGPURM_INTF_TSG_CREATE_SUBCONTEXT.
 */
#define NVGPU_TSG_IOCTL_CREATE_SUBCONTEXT \
	_IOWR(NVGPU_TSG_IOCTL_MAGIC, 18, \
			struct nvgpu_tsg_create_subcontext_args)
/**
 * @ingroup NVGPURM_INTF_TSG_DELETE_SUBCONTEXT
 *
 * @brief Command code for @ref NVGPURM_INTF_TSG_DELETE_SUBCONTEXT.
 */
#define NVGPU_TSG_IOCTL_DELETE_SUBCONTEXT \
	_IOW(NVGPU_TSG_IOCTL_MAGIC, 19, \
			struct nvgpu_tsg_delete_subcontext_args)
/**
 * @ingroup NVGPURM_INTF_TSG_GET_SHARE_TOKEN
 *
 * @brief Command code for @ref NVGPURM_INTF_TSG_GET_SHARE_TOKEN.
 */
#define NVGPU_TSG_IOCTL_GET_SHARE_TOKEN \
	_IOWR(NVGPU_TSG_IOCTL_MAGIC, 20, \
			struct nvgpu_tsg_get_share_token_args)
/**
 * @ingroup NVGPURM_INTF_TSG_REVOKE_SHARE_TOKEN
 *
 * @brief Command code for @ref NVGPURM_INTF_TSG_REVOKE_SHARE_TOKEN.
 */
#define NVGPU_TSG_IOCTL_REVOKE_SHARE_TOKEN \
	_IOW(NVGPU_TSG_IOCTL_MAGIC, 21, \
			struct nvgpu_tsg_revoke_share_token_args)
#define NVGPU_TSG_IOCTL_MAX_ARG_SIZE	\
		sizeof(struct nvgpu_tsg_bind_scheduling_domain_args)

#define NVGPU_TSG_IOCTL_LAST		\
	_IOC_NR(NVGPU_TSG_IOCTL_REVOKE_SHARE_TOKEN)

/*
 * /dev/nvhost-dbg-gpu device
 *
 * Opening a '/dev/nvhost-dbg-gpu' device node creates a new debugger
 * session.  nvgpu channels (for the same module) can then be bound to such a
 * session.
 *
 * One nvgpu channel can also be bound to multiple debug sessions
 *
 * As long as there is an open device file to the session, or any bound
 * nvgpu channels it will be valid.  Once all references to the session
 * are removed the session is deleted.
 *
 */

#define NVGPU_DBG_GPU_IOCTL_MAGIC 'D'

/*
 * Binding/attaching a debugger session to an nvgpu channel
 *
 * The 'channel_fd' given here is the fd used to allocate the
 * gpu channel context.
 *
 */
struct nvgpu_dbg_gpu_bind_channel_args {
	__u32 channel_fd; /* in */
	__u32 _pad0[1];
};

#define NVGPU_DBG_GPU_IOCTL_BIND_CHANNEL				\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 1, struct nvgpu_dbg_gpu_bind_channel_args)

/*
 * Register operations
 * All operations are targeted towards first channel
 * attached to debug session
 */
/* valid op values */
#define NVGPU_DBG_GPU_REG_OP_READ_32                             (0x00000000)
#define NVGPU_DBG_GPU_REG_OP_WRITE_32                            (0x00000001)
#define NVGPU_DBG_GPU_REG_OP_READ_64                             (0x00000002)
#define NVGPU_DBG_GPU_REG_OP_WRITE_64                            (0x00000003)
/* note: 8b ops are unsupported */
#define NVGPU_DBG_GPU_REG_OP_READ_08                             (0x00000004)
#define NVGPU_DBG_GPU_REG_OP_WRITE_08                            (0x00000005)

/* valid type values */
#define NVGPU_DBG_GPU_REG_OP_TYPE_GLOBAL                         (0x00000000)
#define NVGPU_DBG_GPU_REG_OP_TYPE_GR_CTX                         (0x00000001)
#define NVGPU_DBG_GPU_REG_OP_TYPE_GR_CTX_TPC                     (0x00000002)
#define NVGPU_DBG_GPU_REG_OP_TYPE_GR_CTX_SM                      (0x00000004)
#define NVGPU_DBG_GPU_REG_OP_TYPE_GR_CTX_CROP                    (0x00000008)
#define NVGPU_DBG_GPU_REG_OP_TYPE_GR_CTX_ZROP                    (0x00000010)
/*#define NVGPU_DBG_GPU_REG_OP_TYPE_FB                           (0x00000020)*/
#define NVGPU_DBG_GPU_REG_OP_TYPE_GR_CTX_QUAD                    (0x00000040)

/* valid status values */
#define NVGPU_DBG_GPU_REG_OP_STATUS_SUCCESS                      (0x00000000)
#define NVGPU_DBG_GPU_REG_OP_STATUS_INVALID_OP                   (0x00000001)
#define NVGPU_DBG_GPU_REG_OP_STATUS_INVALID_TYPE                 (0x00000002)
#define NVGPU_DBG_GPU_REG_OP_STATUS_INVALID_OFFSET               (0x00000004)
#define NVGPU_DBG_GPU_REG_OP_STATUS_UNSUPPORTED_OP               (0x00000008)
#define NVGPU_DBG_GPU_REG_OP_STATUS_INVALID_MASK                 (0x00000010)

struct nvgpu_dbg_gpu_reg_op {
	__u8    op;
	__u8    type;
	__u8    status;
	__u8    quad;
	__u32   group_mask;
	__u32   sub_group_mask;
	__u32   offset;
	__u32   value_lo;
	__u32   value_hi;
	__u32   and_n_mask_lo;
	__u32   and_n_mask_hi;
};

struct nvgpu_dbg_gpu_exec_reg_ops_args {
	__u64 ops; /* pointer to nvgpu_reg_op operations */
	__u32 num_ops;
	__u32 gr_ctx_resident;
};

#define NVGPU_DBG_GPU_IOCTL_REG_OPS					\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 2, struct nvgpu_dbg_gpu_exec_reg_ops_args)

/* Enable/disable/clear event notifications */
struct nvgpu_dbg_gpu_events_ctrl_args {
	__u32 cmd; /* in */
	__u32 _pad0[1];
};

/* valid event ctrl values */
#define NVGPU_DBG_GPU_EVENTS_CTRL_CMD_DISABLE                    (0x00000000)
#define NVGPU_DBG_GPU_EVENTS_CTRL_CMD_ENABLE                     (0x00000001)
#define NVGPU_DBG_GPU_EVENTS_CTRL_CMD_CLEAR                      (0x00000002)

#define NVGPU_DBG_GPU_IOCTL_EVENTS_CTRL				\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 3, struct nvgpu_dbg_gpu_events_ctrl_args)


/* Powergate/Unpowergate control */

#define NVGPU_DBG_GPU_POWERGATE_MODE_ENABLE                                 1
#define NVGPU_DBG_GPU_POWERGATE_MODE_DISABLE                                2

struct nvgpu_dbg_gpu_powergate_args {
	__u32 mode;
};

#define NVGPU_DBG_GPU_IOCTL_POWERGATE					\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 4, struct nvgpu_dbg_gpu_powergate_args)


/* SMPC Context Switch Mode */
#define NVGPU_DBG_GPU_SMPC_CTXSW_MODE_NO_CTXSW               (0x00000000)
#define NVGPU_DBG_GPU_SMPC_CTXSW_MODE_CTXSW                  (0x00000001)

struct nvgpu_dbg_gpu_smpc_ctxsw_mode_args {
	__u32 mode;
};

#define NVGPU_DBG_GPU_IOCTL_SMPC_CTXSW_MODE \
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 5, struct nvgpu_dbg_gpu_smpc_ctxsw_mode_args)

/* Suspend /Resume SM control */
#define NVGPU_DBG_GPU_SUSPEND_ALL_SMS 1
#define NVGPU_DBG_GPU_RESUME_ALL_SMS  2

struct nvgpu_dbg_gpu_suspend_resume_all_sms_args {
	__u32 mode;
};

#define NVGPU_DBG_GPU_IOCTL_SUSPEND_RESUME_ALL_SMS			\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 6, struct nvgpu_dbg_gpu_suspend_resume_all_sms_args)

struct nvgpu_dbg_gpu_perfbuf_map_args {
	__u32 dmabuf_fd;	/* in */
	__u32 reserved;
	__u64 mapping_size;	/* in, size of mapped buffer region */
	__u64 offset;		/* out, virtual address of the mapping */
};

struct nvgpu_dbg_gpu_perfbuf_unmap_args {
	__u64 offset;
};

#define NVGPU_DBG_GPU_IOCTL_PERFBUF_MAP \
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 7, struct nvgpu_dbg_gpu_perfbuf_map_args)
#define NVGPU_DBG_GPU_IOCTL_PERFBUF_UNMAP \
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 8, struct nvgpu_dbg_gpu_perfbuf_unmap_args)

/* Enable/disable PC Sampling */
struct nvgpu_dbg_gpu_pc_sampling_args {
	__u32 enable;
	__u32 _pad0[1];
};

#define NVGPU_DBG_GPU_IOCTL_PC_SAMPLING_DISABLE	0
#define NVGPU_DBG_GPU_IOCTL_PC_SAMPLING_ENABLE	1

#define NVGPU_DBG_GPU_IOCTL_PC_SAMPLING \
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC,  9, struct nvgpu_dbg_gpu_pc_sampling_args)

/* Enable/Disable timeouts */
#define NVGPU_DBG_GPU_IOCTL_TIMEOUT_ENABLE                                   1
#define NVGPU_DBG_GPU_IOCTL_TIMEOUT_DISABLE                                  0

struct nvgpu_dbg_gpu_timeout_args {
	__u32 enable;
	__u32 padding;
};

#define NVGPU_DBG_GPU_IOCTL_TIMEOUT \
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC, 10, struct nvgpu_dbg_gpu_timeout_args)

#define NVGPU_DBG_GPU_IOCTL_GET_TIMEOUT \
	_IOR(NVGPU_DBG_GPU_IOCTL_MAGIC, 11, struct nvgpu_dbg_gpu_timeout_args)


struct nvgpu_dbg_gpu_set_next_stop_trigger_type_args {
	__u32 broadcast;
	__u32 reserved;
};

#define NVGPU_DBG_GPU_IOCTL_SET_NEXT_STOP_TRIGGER_TYPE			\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 12, struct nvgpu_dbg_gpu_set_next_stop_trigger_type_args)


/* PM Context Switch Mode */
/*This mode says that the pms are not to be context switched. */
#define NVGPU_DBG_GPU_HWPM_CTXSW_MODE_NO_CTXSW               (0x00000000)
/* This mode says that the pms in Mode-B are to be context switched */
#define NVGPU_DBG_GPU_HWPM_CTXSW_MODE_CTXSW                  (0x00000001)
/* This mode says that the pms in Mode-E (stream out) are to be context switched. */
#define NVGPU_DBG_GPU_HWPM_CTXSW_MODE_STREAM_OUT_CTXSW       (0x00000002)

struct nvgpu_dbg_gpu_hwpm_ctxsw_mode_args {
	__u32 mode;
	__u32 reserved;
};

#define NVGPU_DBG_GPU_IOCTL_HWPM_CTXSW_MODE \
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 13, struct nvgpu_dbg_gpu_hwpm_ctxsw_mode_args)


struct nvgpu_dbg_gpu_sm_error_state_record {
	__u32 hww_global_esr;
	__u32 hww_warp_esr;
	__u64 hww_warp_esr_pc;
	__u32 hww_global_esr_report_mask;
	__u32 hww_warp_esr_report_mask;
	__u32 hww_cga_esr;
	__u32 padding;

	/*
	 * Notes
	 * - This struct can be safely appended with new fields. However, always
	 *   keep the structure size multiple of 8 and make sure that the binary
	 *   layout does not change between 32-bit and 64-bit architectures.
	 */
};

struct nvgpu_dbg_gpu_read_single_sm_error_state_args {
	__u32 sm_id;
	__u32 padding;
	__u64 sm_error_state_record_mem;
	__u64 sm_error_state_record_size;
};

#define NVGPU_DBG_GPU_IOCTL_READ_SINGLE_SM_ERROR_STATE			\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 14, struct nvgpu_dbg_gpu_read_single_sm_error_state_args)


struct nvgpu_dbg_gpu_clear_single_sm_error_state_args {
	__u32 sm_id;
	__u32 padding;
};

#define NVGPU_DBG_GPU_IOCTL_CLEAR_SINGLE_SM_ERROR_STATE			\
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC, 15, struct nvgpu_dbg_gpu_clear_single_sm_error_state_args)

/*
 * Unbinding/detaching a debugger session from a nvgpu channel
 *
 * The 'channel_fd' given here is the fd used to allocate the
 * gpu channel context.
 */
struct nvgpu_dbg_gpu_unbind_channel_args {
	__u32 channel_fd; /* in */
	__u32 _pad0[1];
};

#define NVGPU_DBG_GPU_IOCTL_UNBIND_CHANNEL				\
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC, 17, struct nvgpu_dbg_gpu_unbind_channel_args)


#define NVGPU_DBG_GPU_SUSPEND_ALL_CONTEXTS	1
#define NVGPU_DBG_GPU_RESUME_ALL_CONTEXTS	2

struct nvgpu_dbg_gpu_suspend_resume_contexts_args {
	__u32 action;
	__u32 is_resident_context;
	__s32 resident_context_fd;
	__u32 padding;
};

#define NVGPU_DBG_GPU_IOCTL_SUSPEND_RESUME_CONTEXTS			\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 18, struct nvgpu_dbg_gpu_suspend_resume_contexts_args)


#define NVGPU_DBG_GPU_IOCTL_ACCESS_FB_MEMORY_CMD_READ	1
#define NVGPU_DBG_GPU_IOCTL_ACCESS_FB_MEMORY_CMD_WRITE	2

struct nvgpu_dbg_gpu_access_fb_memory_args {
	__u32 cmd;       /* in: either read or write */

	__s32 dmabuf_fd; /* in: dmabuf fd of the buffer in FB */
	__u64 offset;    /* in: offset within buffer in FB, should be 4B aligned */

	__u64 buffer;    /* in/out: temp buffer to read/write from */
	__u64 size;      /* in: size of the buffer i.e. size of read/write in bytes, should be 4B aligned */
};

#define NVGPU_DBG_GPU_IOCTL_ACCESS_FB_MEMORY	\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 19, struct nvgpu_dbg_gpu_access_fb_memory_args)

struct nvgpu_dbg_gpu_profiler_obj_mgt_args {
	__u32 profiler_handle;
	__u32 reserved;
};

#define NVGPU_DBG_GPU_IOCTL_PROFILER_ALLOCATE	\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 20, struct nvgpu_dbg_gpu_profiler_obj_mgt_args)

#define NVGPU_DBG_GPU_IOCTL_PROFILER_FREE	\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 21, struct nvgpu_dbg_gpu_profiler_obj_mgt_args)

struct nvgpu_dbg_gpu_profiler_reserve_args {
	__u32 profiler_handle;
	__u32 acquire;
};

#define NVGPU_DBG_GPU_IOCTL_PROFILER_RESERVE			\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 22, struct nvgpu_dbg_gpu_profiler_reserve_args)

/*
 * This struct helps to set the exception mask. If mask is not set
 * or set to NVGPU_DBG_GPU_IOCTL_SET_SM_EXCEPTION_TYPE_MASK_NONE
 * then kernel code will follow recovery path on sm exception.
 * If mask is set to NVGPU_DBG_GPU_IOCTL_SET_SM_EXCEPTION_TYPE_MASK_FATAL, then
 * kernel code will skip recovery path on sm exception.
 */
struct nvgpu_dbg_gpu_set_sm_exception_type_mask_args {
#define NVGPU_DBG_GPU_IOCTL_SET_SM_EXCEPTION_TYPE_MASK_NONE	(0x0U)
#define NVGPU_DBG_GPU_IOCTL_SET_SM_EXCEPTION_TYPE_MASK_FATAL	(0x1U << 0U)
	/* exception type mask value */
	__u32 exception_type_mask;
	__u32 reserved;
};

#define NVGPU_DBG_GPU_IOCTL_SET_SM_EXCEPTION_TYPE_MASK \
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC, 23, \
			struct nvgpu_dbg_gpu_set_sm_exception_type_mask_args)

struct nvgpu_dbg_gpu_cycle_stats_args {
	__u32 dmabuf_fd;
	__u32 reserved;
};

#define NVGPU_DBG_GPU_IOCTL_CYCLE_STATS	\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 24, struct nvgpu_dbg_gpu_cycle_stats_args)

/* cycle stats snapshot buffer support for mode E */
struct nvgpu_dbg_gpu_cycle_stats_snapshot_args {
	__u32 cmd;		/* in: command to handle     */
	__u32 dmabuf_fd;	/* in: dma buffer handler    */
	__u32 extra;		/* in/out: extra payload e.g.*/
				/*    counter/start perfmon  */
	__u32 reserved;
};

/* valid commands to control cycle stats shared buffer */
#define NVGPU_DBG_GPU_IOCTL_CYCLE_STATS_SNAPSHOT_CMD_FLUSH   0
#define NVGPU_DBG_GPU_IOCTL_CYCLE_STATS_SNAPSHOT_CMD_ATTACH  1
#define NVGPU_DBG_GPU_IOCTL_CYCLE_STATS_SNAPSHOT_CMD_DETACH  2

#define NVGPU_DBG_GPU_IOCTL_CYCLE_STATS_SNAPSHOT	\
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 25, struct nvgpu_dbg_gpu_cycle_stats_snapshot_args)

/*
 * MMU Debug Mode
 *
 * This ioctl enables or disables MMU debug mode. Support for this ioctl is indicated by the
 * NVGPU_GPU_FLAGS_SUPPORT_SET_CTX_MMU_DEBUG_MODE flag.
 *
 * The mode is defined as an enum of the following values:
 *
 * NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_DISABLED: Disable the MMU debug mode.
 * NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_ENABLED: Enable the MMU debug mode.
 * NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_GCC_DISABLED: Disable the GCC MMU debug mode.
 * NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_GCC_ENABLED: Enable the GCC MMU debug mode.
 *
 * NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_GCC_ENABLED is supported only on specific
 * GPUs(GB20C and later).
 *
 * When GPU does not support GCC MMU debug mode (prior to GB20C),
 * NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_ENABLED enables MMU debug mode for all clients
 * (TEX, PE, RASTER, GPCCS, TPCCS).
 *
 * When GPU supports GCC MMU debug mode (GB20C and later),
 * NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_ENABLED enables MMU debug mode for all
 * clients except GCC, while NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_GCC_ENABLED enables
 * MMU debug mode specifically for GCC.
 *
 * Repeated Enable/Disable calls are ignored by Nvgpu.
 *
 * Returns -EINVAL if the mode is not supported.
 */
#define NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_DISABLED	0U
#define NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_ENABLED	1U
#define NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_GCC_DISABLED	2U
#define NVGPU_DBG_GPU_CTX_MMU_DEBUG_MODE_GCC_ENABLED	3U

struct nvgpu_dbg_gpu_set_ctx_mmu_debug_mode_args {
	__u32 mode;
	__u32 reserved;
};
#define NVGPU_DBG_GPU_IOCTL_SET_CTX_MMU_DEBUG_MODE	\
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC, 26, \
	struct nvgpu_dbg_gpu_set_ctx_mmu_debug_mode_args)

/* Get gr context size */
struct nvgpu_dbg_gpu_get_gr_context_size_args {
	__u32 size;
	__u32 reserved;
};

#define NVGPU_DBG_GPU_IOCTL_GET_GR_CONTEXT_SIZE \
	_IOR(NVGPU_DBG_GPU_IOCTL_MAGIC, 27, \
	struct nvgpu_dbg_gpu_get_gr_context_size_args)

/* Get gr context */
struct nvgpu_dbg_gpu_get_gr_context_args {
	__u64 buffer;    /* in/out: the output buffer containing contents of the gr context.
						buffer address is given by the user */
	__u32 size;      /* in: size of the context buffer */
	__u32 reserved;
};

#define NVGPU_DBG_GPU_IOCTL_GET_GR_CONTEXT \
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC, 28, \
	struct nvgpu_dbg_gpu_get_gr_context_args)

#define NVGPU_DBG_GPU_IOCTL_TSG_SET_TIMESLICE \
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC, 29, \
	struct nvgpu_timeslice_args)

#define NVGPU_DBG_GPU_IOCTL_TSG_GET_TIMESLICE \
	_IOR(NVGPU_DBG_GPU_IOCTL_MAGIC, 30, \
	struct nvgpu_timeslice_args)

struct nvgpu_dbg_gpu_get_mappings_entry {
	/* out: start of GPU VA for this mapping */
	__u64 gpu_va;
	/* out: size in bytes of this mapping */
	__u64 size;
};

struct nvgpu_dbg_gpu_get_mappings_args {
	/* in: lower VA range, inclusive */
	__u64 va_lo;
	/* in: upper VA range, exclusive */
	__u64 va_hi;
	/* in: Pointer to the struct nvgpu_dbg_gpu_get_mappings_entry. */
	__u64 ops_buffer;
	/*
	 * in: maximum number of the entries that ops_buffer may hold.
	 * out: number of entries written to ops_buffer.
	 * When ops_buffer is zero:
	 * out: number of mapping entries in range [va_lo, va_hi).
	 */
	__u32 count;
	/* out: Has more valid mappings in this range than count */
	__u8 has_more;
	__u8 reserved[3];
};

/* Maximum read/write ops supported in a single call */
#define NVGPU_DBG_GPU_IOCTL_ACCESS_GPUVA_CMD_READ 1U
#define NVGPU_DBG_GPU_IOCTL_ACCESS_GPUVA_CMD_WRITE 2U
struct nvgpu_dbg_gpu_va_access_entry {
	/* in: gpu_va address */
	__u64 gpu_va;
	/* in/out: Pointer to buffer through which data needs to be read/written */
	__u64 data;
	/* in: Access size in bytes */
	__u32 size;
	/* out: Whether the GpuVA is accessible */
	__u8 valid;
	__u8 reserved[3];
};

struct nvgpu_dbg_gpu_va_access_args {
	/* in/out: Pointer to the struct nvgpu_dbg_gpu_va_access_entry */
	__u64 ops_buf;
	/* in: Number of buffer ops */
	__u32 count;
	/* in: Access cmd Read/Write */
	__u8 cmd;
	__u8 reserved[3];
};

#define NVGPU_DBG_GPU_IOCTL_GET_MAPPINGS \
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 31, struct nvgpu_dbg_gpu_get_mappings_args)
#define NVGPU_DBG_GPU_IOCTL_ACCESS_GPU_VA \
	_IOWR(NVGPU_DBG_GPU_IOCTL_MAGIC, 32, struct nvgpu_dbg_gpu_va_access_args)

/* Implicit ERRBAR Mode */
#define NVGPU_DBG_GPU_SCHED_EXIT_WAIT_FOR_ERRBAR_DISABLED	0
#define NVGPU_DBG_GPU_SCHED_EXIT_WAIT_FOR_ERRBAR_ENABLED	1

struct nvgpu_sched_exit_wait_for_errbar_args {
	__u32 enable; /* enable 1, disable 0*/
};

#define NVGPU_DBG_GPU_IOCTL_SET_SCHED_EXIT_WAIT_FOR_ERRBAR \
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC, 33, \
	struct nvgpu_sched_exit_wait_for_errbar_args)

#define NVGPU_DBG_GPU_SET_SM_DBGR_CTRL_SINGLE_STEP_MODE_ENABLE	1
#define NVGPU_DBG_GPU_SET_SM_DBGR_CTRL_SINGLE_STEP_MODE_DISABLE	0

struct nvgpu_set_sm_dbgr_ctrl_single_step_mode_args {
	/* Valid SM ID */
	__u32 sm_id;
	__u32 enable; /* enable 1, disable 0*/
};

#define NVGPU_DBG_GPU_IOCTL_SET_SM_DBGR_CTRL_SINGLE_STEP_MODE \
	_IOW(NVGPU_DBG_GPU_IOCTL_MAGIC, 34, \
	struct nvgpu_set_sm_dbgr_ctrl_single_step_mode_args)

#define NVGPU_DBG_GPU_IOCTL_LAST		\
	_IOC_NR(NVGPU_DBG_GPU_IOCTL_SET_SM_DBGR_CTRL_SINGLE_STEP_MODE)

#define NVGPU_DBG_GPU_IOCTL_MAX_ARG_SIZE		\
	sizeof(struct nvgpu_dbg_gpu_access_fb_memory_args)


/*
 * /dev/nvhost-prof-dev-gpu and /dev/nvhost-prof-ctx-gpu devices
 *
 * Opening a '/dev/nvhost-prof-*' device node creates a way to
 * open and manage a profiler object.
 */

#define NVGPU_PROFILER_IOCTL_MAGIC 'P'

struct nvgpu_profiler_bind_context_args {
	__s32 tsg_fd;		/* in: TSG file descriptor */
	__u32 reserved;
};

#define NVGPU_PROFILER_PM_RESOURCE_ARG_HWPM_LEGACY	0U
#define NVGPU_PROFILER_PM_RESOURCE_ARG_SMPC		1U
#define NVGPU_PROFILER_PM_RESOURCE_ARG_PC_SAMPLER	2U
#define NVGPU_PROFILER_PM_RESOURCE_ARG_HES_CWD		3U
#define NVGPU_PROFILER_PM_RESOURCE_ARG_CCU_PROF		4U

struct nvgpu_profiler_reserve_pm_resource_args {
	__u32 resource; 	/* in: NVGPU_PROFILER_PM_RESOURCE_ARG_* resource to be reserved */

/* in: if ctxsw should be enabled for resource */
#define NVGPU_PROFILER_RESERVE_PM_RESOURCE_ARG_FLAG_CTXSW	(1 << 0)
	__u32 flags;

	__u32 reserved[2];
};

struct nvgpu_profiler_release_pm_resource_args {
	__u32 resource; 	/* in: NVGPU_PROFILER_PM_RESOURCE_ARG_* resource to be released */
	__u32 reserved;
};

struct nvgpu_profiler_alloc_pma_stream_args {
	__u64 pma_buffer_map_size;			/* in: PMA stream buffer size */
	__u64 pma_buffer_offset;			/* in: offset of PMA stream buffer */
	__u64 pma_buffer_va;				/* out: PMA stream buffer virtual address */
	__s32 pma_buffer_fd;				/* in: PMA stream buffer fd */

	__s32 pma_bytes_available_buffer_fd;		/* in: PMA available bytes buffer fd */

/* in: if ctxsw should be enabled for PMA channel */
#define NVGPU_PROFILER_ALLOC_PMA_STREAM_ARG_FLAG_CTXSW		(1 << 0)
	__u32 flags;

	__u32 pma_channel_id;				/* out: PMA hardware stream id */

	__u32 reserved[2];
};

struct nvgpu_profiler_free_pma_stream_args {
	__u32 pma_channel_id;				/* in: PMA hardware stream id */
	__u32 reserved[2];
};

struct nvgpu_profiler_pma_stream_update_get_put_args {
	__u64 bytes_consumed;		/* in: total bytes consumed by user since last update */
	__u64 bytes_available;		/* out: available bytes in PMA buffer for user to consume */

	__u64 put_ptr;			/* out: current PUT pointer to be returned */

/* in: if available bytes buffer should be updated */
#define NVGPU_PROFILER_PMA_STREAM_UPDATE_GET_PUT_ARG_FLAG_UPDATE_AVAILABLE_BYTES	(1 << 0)
/* in: if need to wait for available bytes buffer to get updated */
#define NVGPU_PROFILER_PMA_STREAM_UPDATE_GET_PUT_ARG_FLAG_WAIT_FOR_UPDATE		(1 << 1)
/* in: if current PUT pointer should be returned */
#define NVGPU_PROFILER_PMA_STREAM_UPDATE_GET_PUT_ARG_FLAG_RETURN_PUT_PTR		(1 << 2)
/* out: if PMA stream buffer overflow was triggered */
#define NVGPU_PROFILER_PMA_STREAM_UPDATE_GET_PUT_ARG_FLAG_OVERFLOW_TRIGGERED		(1 << 3)
	__u32 flags;

	__u32 pma_channel_id;			/* in: PMA channel index */

	__u32 reserved[2];
};

/*
 * HS credit programming:
 *
 * As part of each successful bind with pma channel, NVGPU will create the
 * HS credit pool, indexing it with pma_channel_index.
 * Each pool will have one or more of following chiplet types within it.
 * CHIPLET_TYPE_GPC, CHIPLET_TYPE_FBP and CHIPLET_TYPE_SYS
 *
 * Each chiplet type will have n number of the chiplets. Here n is number of
 * chiplets from those the perfmons are bound to the pma_channel.
 *
 * Based on the connectivity of the chiplet routers to the pma engine,
 * NVGPU allocates num of credits per chiplet(router). So based on this
 * allocation each chiplet group gets the max total_num_credits.
 * Max total_num_credits of a chiplet type can't be exceeded at any point
 * in time by userspace through NVGPU_PROFILER_IOCTL_SET_PMA_STREAM_HS_CREDIT.
 * Nvgpu mandates it.
 *
 * Ioctl NVGPU_PROFILER_IOCTL_GET_PMA_STREAM_POOLS_INFO provides the userspace
 * with array of struct nvgpu_profiler_channel_pool_info, where each element
 * provides details of each chiplet group like, chiplet_type, num_chiplets,
 * total_num_credits, and pool_index etc.
 *
 * Ioctl NVGPU_PROFILER_IOCTL_GET_PMA_STREAM_HS_CREDIT enables userspace to
 * gather the num_credits for the list of chiplets pointed by array of
 * struct nvgpu_profiler_channel_get_credit_buf.
 * Nvgpu validate if such chiplet exist in the pool. If any get_credit request
 * fails nvgpu update the get_credit_response of that request with corresponding
 * error code.
 *
 * Ioctl NVGPU_PROFILER_IOCTL_SET_PMA_STREAM_HS_CREDIT enables userspace to
 * alter the num_credits for a list of chiplets through array of
 * struct nvgpu_profiler_channel_set_credit_buf.
 * Nvgpu validate the existence of the chiplet with in the pool and also
 * validates that if all requested alteration of credits are executed, total
 * credits of chiplet groups shouldn't exceed their max total_num_credits
 * allocated at bind. For failing set request corresponding set_credit_response
 * will be set with error code.
 */

/* Macro to represent the GPC chiplet types */
#define NVGPU_PROFILER_CHANNEL_CHIPLET_TYPE_GPC		0
/* Macro to represent the FBP chiplet types */
#define NVGPU_PROFILER_CHANNEL_CHIPLET_TYPE_FBP		1
/* Macro to represent the sys chiplet types */
#define NVGPU_PROFILER_CHANNEL_CHIPLET_TYPE_SYS		2


struct nvgpu_profiler_channel_pool_info {

	/* [out] Type of chiplet. */
	__u8 chiplet_type;

	/* [out] Number of chiplets available per chiplet type */
	__u32 num_chiplets;

	/*
	 * [out] Max number of credits that can be allocated to the chiplets of
	 * a chiplet_type
	 */
	__u16 total_num_credits;

	/* [out] Pool index. Same as pma_channel_idx */
	__u8 pool_index;
};

struct nvgpu_profiler_channel_get_pools_info_args {
	/* [in] Userspace addr for array of struct nvgpu_profiler_channel_pool_info.
	 * if the val is NULL, NVGPU will only return number of pool entries
	 * in num_pool_entries.
	 */
	__u64 nvgpu_profiler_channel_pool_info_addr;

	/* [in] Number of pool entries nvgpu_profiler_channel_pool_info_addr
	 * can accommodate.
	 * [out] When nvgpu_profiler_channel_pool_info_addr is non-NULL, the
	 * number of entries written to the buffer. Otherwise the number of
	 * entries available for reading.
	 */
	__u32 num_pool_entries;
};

#define NVGPU_PROFILER_CHANNEL_GET_CREDIT_SUCCESS			0
#define NVGPU_PROFILER_CHANNEL_GET_CREDIT_INVALID_CREDITS		1
#define NVGPU_PROFILER_CHANNEL_GET_CREDIT_INVALID_CHIPLET_INDEX		2
#define NVGPU_PROFILER_CHANNEL_GET_CREDIT_INVALID_CHIPLET_TYPE		3

struct nvgpu_profiler_channel_get_credit_buf {

	/* [in] Type of the chiplet.*/
	__u8 chiplet_type;

	/* [in] Local Index of the chiplet within chiplet_type. */
	__u32 chiplet_index;

	/* [out] Num of credits allocated to the chiplet designated by
	 * (chiplet_type, chiplet_index). Filled only when get_credit_response
	 * is NVGPU_PROFILER_CHANNEL_GET_CREDIT_SUCCESS.
	 */
	__u32 num_credits;

	/* [out] Request status. if NVGPU_PROFILER_CHANNEL_GET_CREDIT_SUCCESS
	 * is not set then get request is not successful.
	 */
	__u8 get_credit_response;
};

#define NVGPU_PROFILER_CHANNEL_SET_CREDIT_SUCCESS			0
#define NVGPU_PROFILER_CHANNEL_SET_CREDIT_INVALID_CREDITS		1
#define NVGPU_PROFILER_CHANNEL_SET_CREDIT_INVALID_CHIPLET_INDEX		2
#define NVGPU_PROFILER_CHANNEL_SET_CREDIT_INVALID_CHIPLET_TYPE		3

struct nvgpu_profiler_channel_set_credit_buf {

	/* [in] Type of the chiplet */
	__u8 chiplet_type;

	/* [in] Local index of the chiplet within chiplet_type. */
	__u32 chiplet_index;

	/* [in] New number of credits requested for the chiplet designated by
	 * (chiplet_type, chiplet_index)
	 */
	__u32 num_credits;

	/* [out] Request status. if NVGPU_PROFILER_CHANNEL_SET_CREDIT_SUCCESS
	 * is not set then set request is not successful.
	 */
	__u8 set_credit_response;
};

struct nvgpu_profiler_channel_get_hs_credit_args {

	/* [in] Index of the pma stream for which credits are requested */
	__u32 pma_channel_id;

	/* [in, out] An array of struct nvgpu_profiler_channel_get_credit_buf.
	 * The size of array is specified by the credit_buf_count.
	 */
	__u64 get_credit_buf_addr;

	/* [in, out] Count of struct nvgpu_profiler_channel_get_credit_buf that
	 * get_credit_buf_addr holds.
	 */
	__u32 credit_buf_count;
};

struct nvgpu_profiler_channel_set_hs_credit_args {

	/* [in] Index of the pma stream, to which credit set is requested */
	__u32 pma_channel_id;

	/* [in, out] An array of struct nvgpu_profiler_channel_set_credit_buf.
	 * The size of array is specified by the credit_buf_count.
	 */
	__u64 set_credit_buf_addr;

	/* [in, out] Count of struct nvgpu_profiler_channel_set_credit_buf that
	 * set_credit_buf_addr is holding.
	 */
	__u32 credit_buf_count;
};

/*
 * MODE_ALL_OR_NONE
 * Reg_ops execution will bail out if any of the reg_op is not valid
 * or if there is any other error such as failure to access context image.
 * Subsequent reg_ops will not be executed and nvgpu_profiler_reg_op.status
 * will not be populated for them.
 * IOCTL will always return error for all of the errors.
 */
#define NVGPU_PROFILER_EXEC_REG_OPS_ARG_MODE_ALL_OR_NONE	0U
/*
 * MODE_CONTINUE_ON_ERROR
 * This mode allows continuing reg_ops execution even if some of the
 * reg_ops are not valid. Invalid reg_ops will be skipped and valid
 * ones will be executed.
 * IOCTL will return error only if there is some other severe failure
 * such as failure to access context image.
 * If any of the reg_op is invalid, or if didn't pass, it will be
 * reported via NVGPU_PROFILER_EXEC_REG_OPS_ARG_FLAG_ALL_PASSED flag.
 * IOCTL will return success in such cases.
 */
#define NVGPU_PROFILER_EXEC_REG_OPS_ARG_MODE_CONTINUE_ON_ERROR	1U

struct nvgpu_profiler_reg_op {
	__u8    op;		/* Operation in the form NVGPU_DBG_GPU_REG_OP_READ/WRITE_* */
	__u8    status;		/* Status in the form NVGPU_DBG_GPU_REG_OP_STATUS_* */
	__u32   offset;
	__u64   value;
	__u64   and_n_mask;
};

struct nvgpu_profiler_exec_reg_ops_args {
	__u32 mode;		/* in: operation mode NVGPU_PROFILER_EXEC_REG_OPS_ARG_MODE_* */

	__u32 count;		/* in: number of reg_ops operations,
				 *     upper limit nvgpu_gpu_characteristics.reg_ops_limit
				 */

	__u64 ops;		/* in/out: pointer to actual operations nvgpu_profiler_reg_op */

/* out: if all reg_ops passed, valid only for MODE_CONTINUE_ON_ERROR */
#define NVGPU_PROFILER_EXEC_REG_OPS_ARG_FLAG_ALL_PASSED		(1 << 0)
/* out: if the operations were performed directly on HW or in context image */
#define NVGPU_PROFILER_EXEC_REG_OPS_ARG_FLAG_DIRECT_OPS		(1 << 1)
	__u32 flags;

	__u32 reserved[3];
};

struct nvgpu_profiler_vab_range_checker {

	/*
	 * in: starting physical address. Must be aligned by
	 *     1 << (granularity_shift + bitmask_size_shift) where
	 *     bitmask_size_shift is a HW specific constant.
	 */
	__u64 start_phys_addr;

	/* in: log2 of coverage granularity per bit */
	__u8  granularity_shift;

	__u8  reserved[7];
};

/* Range checkers track all accesses (read and write) */
#define NVGPU_PROFILER_VAB_RANGE_CHECKER_MODE_ACCESS  1U
/* Range checkers track writes (writes and read-modify-writes) */
#define NVGPU_PROFILER_VAB_RANGE_CHECKER_MODE_DIRTY   2U

struct nvgpu_profiler_vab_reserve_args {

	/* in: range checker mode */
	__u8 vab_mode;

	__u8 reserved[3];

	/* in: number of range checkers, must match with the HW */
	__u32 num_range_checkers;

	/*
	 * in: range checker parameters. Pointer to array of
	 *     nvgpu_profiler_vab_range_checker elements
	 */
	__u64 range_checkers_ptr;
};

struct nvgpu_profiler_vab_flush_state_args {
	__u64 buffer_ptr;	/* in: usermode pointer to receive the
				 * VAB state buffer */
	__u64 buffer_size;	/* in: VAB buffer size. Must match
				 * with the hardware VAB state size */
};

#define NVGPU_PROFILER_IOCTL_BIND_CONTEXT \
	_IOW(NVGPU_PROFILER_IOCTL_MAGIC, 1, struct nvgpu_profiler_bind_context_args)
#define NVGPU_PROFILER_IOCTL_RESERVE_PM_RESOURCE \
	_IOW(NVGPU_PROFILER_IOCTL_MAGIC, 2, struct nvgpu_profiler_reserve_pm_resource_args)
#define NVGPU_PROFILER_IOCTL_RELEASE_PM_RESOURCE \
	_IOW(NVGPU_PROFILER_IOCTL_MAGIC, 3, struct nvgpu_profiler_release_pm_resource_args)
#define NVGPU_PROFILER_IOCTL_ALLOC_PMA_STREAM \
	_IOWR(NVGPU_PROFILER_IOCTL_MAGIC, 4, struct nvgpu_profiler_alloc_pma_stream_args)
#define NVGPU_PROFILER_IOCTL_FREE_PMA_STREAM \
	_IOW(NVGPU_PROFILER_IOCTL_MAGIC, 5, struct nvgpu_profiler_free_pma_stream_args)
#define NVGPU_PROFILER_IOCTL_BIND_PM_RESOURCES \
	_IO(NVGPU_PROFILER_IOCTL_MAGIC, 6)
#define NVGPU_PROFILER_IOCTL_UNBIND_PM_RESOURCES \
	_IO(NVGPU_PROFILER_IOCTL_MAGIC, 7)
#define NVGPU_PROFILER_IOCTL_PMA_STREAM_UPDATE_GET_PUT \
	_IOWR(NVGPU_PROFILER_IOCTL_MAGIC, 8, struct nvgpu_profiler_pma_stream_update_get_put_args)
#define NVGPU_PROFILER_IOCTL_EXEC_REG_OPS \
	_IOWR(NVGPU_PROFILER_IOCTL_MAGIC, 9, struct nvgpu_profiler_exec_reg_ops_args)
#define NVGPU_PROFILER_IOCTL_UNBIND_CONTEXT \
	_IO(NVGPU_PROFILER_IOCTL_MAGIC, 10)
#define NVGPU_PROFILER_IOCTL_VAB_RESERVE \
	_IOW(NVGPU_PROFILER_IOCTL_MAGIC, 11, struct nvgpu_profiler_vab_reserve_args)
#define NVGPU_PROFILER_IOCTL_VAB_RELEASE \
	_IO(NVGPU_PROFILER_IOCTL_MAGIC, 12)
#define NVGPU_PROFILER_IOCTL_VAB_FLUSH_STATE \
	_IOW(NVGPU_PROFILER_IOCTL_MAGIC, 13, struct nvgpu_profiler_vab_flush_state_args)
#define NVGPU_PROFILER_IOCTL_GET_PMA_STREAM_POOLS_INFO \
	_IOWR(NVGPU_PROFILER_IOCTL_MAGIC, 14, struct nvgpu_profiler_channel_get_pools_info_args)
#define NVGPU_PROFILER_IOCTL_GET_PMA_STREAM_HS_CREDIT \
	_IOWR(NVGPU_PROFILER_IOCTL_MAGIC, 15, struct nvgpu_profiler_channel_get_hs_credit_args)
#define NVGPU_PROFILER_IOCTL_SET_PMA_STREAM_HS_CREDIT \
	_IOWR(NVGPU_PROFILER_IOCTL_MAGIC, 16, struct nvgpu_profiler_channel_set_hs_credit_args)
#define NVGPU_PROFILER_IOCTL_MAX_ARG_SIZE	\
		sizeof(struct nvgpu_profiler_alloc_pma_stream_args)
#define NVGPU_PROFILER_IOCTL_LAST		\
	_IOC_NR(NVGPU_PROFILER_IOCTL_SET_PMA_STREAM_HS_CREDIT)


/*
 * /dev/nvhost-gpu device
 */

#define NVGPU_IOCTL_MAGIC 'H'
#define NVGPU_NO_TIMEOUT ((__u32)~0U)
#define NVGPU_TIMEOUT_FLAG_DISABLE_DUMP		0

/* this is also the hardware memory format */
struct nvgpu_gpfifo {
	__u32 entry0; /* first word of gpfifo entry */
	__u32 entry1; /* second word of gpfifo entry */
};

struct nvgpu_get_param_args {
	__u32 value;
};

struct nvgpu_channel_open_args {
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

struct nvgpu_set_nvmap_fd_args {
	__u32 fd;
};

/**
 * @ingroup NVGPURM_INTF_ALLOC_OBJ_CTX
 * @{
 */

/**
 * @brief Specifies that graphics preemption is used on this channel.
 *
 * @deprecated Should not be used. Use @ref NVGPURM_INTF_SET_PREEMPTION_MODE.
 */
#define NVGPU_ALLOC_OBJ_FLAGS_GFXP		NVGPU_BIT32(1)

/**
 * @brief Specifies that compute preemption is used on this channel.
 *
 * @deprecated Should not be used. Use @ref NVGPURM_INTF_SET_PREEMPTION_MODE.
 */
#define NVGPU_ALLOC_OBJ_FLAGS_CILP		NVGPU_BIT32(2)

/**
 * @brief Specifies that VPR is enabled on this channel.
 */
#define NVGPU_ALLOC_OBJ_FLAGS_VPR_ENABLED	NVGPU_BIT32(3)

/**
 * @}
 */

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CHANNEL
 *
 * @defgroup NVGPURM_INTF_ALLOC_OBJ_CTX IOCTL: NVGPU_IOCTL_CHANNEL_ALLOC_OBJ_CTX
 * @brief Allocates the channel object and related context buffers.
 *
 * This IOCTL allocates the channel object and related context buffers such as
 * the patch buffer and various alpha/beta circular buffers. In addition, the
 * golden context image is loaded. This is the environment required to make the
 * channel operational to drive the associated engines. The engines required are
 * denoted by @c #nvgpu_alloc_obj_ctx_args::class_num.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_alloc_obj_ctx_args</td>
 *   <td>@c #nvgpu_alloc_obj_ctx_args</td>
 * </tr>
 * </table>
 *
 * @remark On first invocation of this IOCTL, the golden context image is
 * initialized.
 *
 * @remark On first invocation per TSG, GR context, patch context, and global
 * context buffer mappings are created and golden context image is loaded. On
 * successive invocations they are reused.
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---  Invalid @c #nvgpu_alloc_obj_ctx_args::class_num value
 * - @c ENOMEM ---  Memory allocation failed for a context image.
 * - @c ENODEV ---  GPU not ready
 * - @c EFAULT ---  Copy from user space fails
 */
/**
 * @ingroup NVGPURM_INTF_ALLOC_OBJ_CTX
 * @brief Header struct for @ref NVGPURM_INTF_ALLOC_OBJ_CTX.
 */
struct nvgpu_alloc_obj_ctx_args {
	/**
	 * @brief Class number specifying the required engines for the channel.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Class number</dd>
	 * </dl>
	 *
	 * <table><caption>Engines per class number</caption>
	 *   <tr>
	 *     <th>Class type</th>
	 *     <th>Class number pattern (HEX)</th>
	 *     <th>Engines (class number patterns, HEX)</th>
	 *     <th>Remarks</th>
	 *   </tr>
	 *   <tr>
	 *     <td>Graphics</td>
	 *     <td>@c xx97</td>
	 *     <td>
	 *       Compute (@c xxC0)<br>
	 *       Copy (@c xxB5)<br>
	 *       Graphics 2D (@c xx2D)<br>
	 *       Graphics 3D (@c xx97)<br>
	 *       Inline-to-memory (@c xx40)
	 *     </td>
	 *     <td>
	 *       Channel context for running graphics workloads.
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td>Compute</td>
	 *     <td>@c xxC0</td>
	 *     <td>
	 *       Compute (@c xxC0)<br>
	 *       Copy (@c xxB5)
	 *     </td>
	 *     <td>
	 *       Channel context for running compute workloads.
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td>Copy</td>
	 *     <td>@c xxB5</td>
	 *     <td>
	 *       Copy (@c xxB5)
	 *     </td>
	 *     <td>
	 *       Context for driving the copy engine.
	 *     </td>
	 *   </tr>
	 * </table>
	 *
	 * Range: The class numbers are HW-specific.
	 */
	__u32 class_num;

	/**
	 * @brief Context object flags (@c NVGPU_ALLOC_OBJ_FLAGS_*).
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Allocation flags.</dd>
	 * </dl>
	 */
	__u32 flags;

	/** @brief Unused */
	__u64 obj_id;
};


/**
 * @ingroup NVGPURM_LINUX_IOCTL_CHANNEL
 * @defgroup NVGPURM_INTF_ALLOC_GPFIFO_EX IOCTL: NVGPU_IOCTL_CHANNEL_ALLOC_GPFIFO_EX
 * @brief Allocates the control buffers for the channel (GP FIFO, USERD)
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_alloc_gpfifo_ex_args</td>
 *   <td>@c #nvgpu_alloc_gpfifo_ex_args</td>
 * </tr>
 * </table>
 *
 * @remark This command is deprecated. Use @ref NVGPURM_INTF_ALLOC_SETUP_BIND instead.
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---     Flag @c #NVGPU_ALLOC_GPFIFO_EX_FLAGS_VPR_ENABLED is set
 *                     which is no longer supported.
 * - @c EINVAL ---     @c #nvgpu_alloc_gpfifo_ex_args::num_entries not in the
 *                    valid range.
 * - @c EINVAL ---     Channel not bound to an address space (@ref
 *                    NVGPURM_INTF_AS_BIND_CHANNEL)
 * - @c EINVAL ---     Channel not bound to a TSG (@ref
 *                    NVGPURM_INTF_TSG_BIND_CHANNEL_EX)
 * - @c EEXIST ---     GP FIFO already allocated.
 * - @c EINVAL ---     Flag @c #NVGPU_ALLOC_GPFIFO_EX_FLAGS_DETERMINISTIC is set and watchdog
 *                     is also enabled.
 * - @c E2BIG ---      No space available to append the channel to runlist.
 * - @c ETIMEDOUT ---  Runlist update timed out.
 * - @c ENODEV ---     GPU is not ready.
 * - @c EFAULT ---     Copy from user space fails
 */

/**
 * @ingroup NVGPURM_INTF_ALLOC_GPFIFO_EX
 * @{
 */

/**
 * @brief Channel is used in VPR context. This directs the KMD/RM to allocate
 * certain channel-specific buffers from VPR.
 *
 * @pre @c #NVGPU_GPU_FLAGS_SUPPORT_VPR
 *
 * @remark Not supported in safety.
 * @deprecated Use @ref NVGPURM_INTF_ALLOC_OBJ_CTX with @ref NVGPU_ALLOC_OBJ_FLAGS_VPR_ENABLED
 */
#define NVGPU_ALLOC_GPFIFO_EX_FLAGS_VPR_ENABLED		NVGPU_BIT32(0)

/**
 * @brief Requests deterministic behavior in the submit path.
 *
 * This flag makes the resource manager preallocate certain buffers in favor of
 * per-kickoff dynamic memory allocation, and thus, eliminating memory
 * allocation and other timing-wise non-deterministic operations from the submit
 * path. The use of this option prevents the use of sync fds (Linux only).
 *
 * Deterministic channels support user-managed syncpoints. (See @ref
 * NVGPURM_INTF_CHANNEL_GET_SYNCPT.)
 *
 * A deterministic channel also obtains a power reference, preventing the GPU
 * from railgating.
 *
 * @pre @c #NVGPU_GPU_FLAGS_SUPPORT_DETERMINISTIC_SUBMIT_FULL OR @c
 * #NVGPU_GPU_FLAGS_SUPPORT_DETERMINISTIC_SUBMIT_NO_JOBTRACKING.
 *
 * @remark VPR resize may still trigger a GPU reset. If this is not acceptable,
 * the user has to disable VPR resizing.
 */
#define NVGPU_ALLOC_GPFIFO_EX_FLAGS_DETERMINISTIC	NVGPU_BIT32(1)

/**
 * @brief Enable replayable gmmu faults for this channel
 *
 * This feature is currently for hardware verification purposes only.
 *
 * @remark GPU mapping must be created with flag @c #NVGPU_AS_MAP_BUFFER_FLAGS_UNMAPPED_PTE to
 * use this flag.
 */
#define NVGPU_ALLOC_GPFIFO_EX_FLAGS_REPLAYABLE_FAULTS_ENABLE	NVGPU_BIT32(2)

/** @} */

/**
 * @ingroup NVGPURM_INTF_ALLOC_GPFIFO_EX
 * @brief Header struct for @ref NVGPURM_INTF_ALLOC_GPFIFO_EX
 */
struct nvgpu_alloc_gpfifo_ex_args {
	/**
	 * @brief Size of the GP FIFO in entries.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>GP FIFO size.</dd>
	 * </dl>
	 */
	__u32 num_entries;

	/**
	 * @brief Maximum number of in-flight submits (deterministic KMD/RM submit
	 * channels only)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Maximum number of in-flight submits. This parameter is used for
	 *     allocating the method buffer and other structures for bookkeeping of in-flight submits.</dd>
	 * </dl>
	 */
	__u32 num_inflight_jobs;

	/**
	 * @brief GP FIFO allocation flags (@c NVGPU_ALLOC_GPFIFO_EX_FLAGS_*).
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Allocation flags.</dd>
	 * </dl>
	 */
	__u32 flags;

	/**
	 * @brief Hardware channel id.
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>Hardware channel id.</dd>
	 * </dl>
	 *
	 * The hardware channel id can be used to identify the channel in the nvgpu
	 * system log fault dumps.
	 */
	__u32 hw_channel_id;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved[4];
};

/**
 * @ingroup NVGPURM_INTF_ALLOC_SETUP_BIND
 * @{
 */

/**
 * @brief Channel is used in VPR context. This directs the KMD/RM to allocate
 * certain channel-specific buffers from VPR.
 *
 * @pre @c #NVGPU_GPU_FLAGS_SUPPORT_VPR
 *
 * @deprecated Use @ref NVGPURM_INTF_ALLOC_OBJ_CTX with @ref NVGPU_ALLOC_OBJ_FLAGS_VPR_ENABLED
 */
#define NVGPU_CHANNEL_SETUP_BIND_FLAGS_VPR_ENABLED		NVGPU_BIT32(0)

/**
 * @brief Requests deterministic behavior in the submit path.
 *
 * This flag makes the resource manager preallocate certain buffers in favor of
 * per-kickoff dynamic memory allocation, and thus, eliminating memory
 * allocation and other timing-wise non-deterministic operations from the submit
 * path. The use of this option prevents the use of sync fds (Linux only).
 *
 * Deterministic channels support user-managed syncpoints. (See @ref
 * NVGPURM_INTF_CHANNEL_GET_SYNCPT.)
 *
 * A deterministic channel also obtains a power reference, preventing the GPU
 * from railgating.
 *
 * @pre @c #NVGPU_GPU_FLAGS_SUPPORT_DETERMINISTIC_SUBMIT_FULL OR @c
 * #NVGPU_GPU_FLAGS_SUPPORT_DETERMINISTIC_SUBMIT_NO_JOBTRACKING.
 *
 * @remark VPR resize may still trigger a GPU reset. If this is not acceptable,
 * the user has to disable VPR resizing.
 */
#define NVGPU_CHANNEL_SETUP_BIND_FLAGS_DETERMINISTIC		NVGPU_BIT32(1)

/**
 * @brief Enable replayable gmmu faults for this channel
 *
 * This feature is currently for hardware verification purposes only.
 *
 * @remark GPU mapping must be created with flag @c #NVGPU_AS_MAP_BUFFER_FLAGS_UNMAPPED_PTE to
 * use this flag.
 */
#define NVGPU_CHANNEL_SETUP_BIND_FLAGS_REPLAYABLE_FAULTS_ENABLE	NVGPU_BIT32(2)

/**
 * @brief Use usermode submit procedure.
 *
 * When this flag is specified, usermode submit procedure is used by the
 * channel instead of @c #NVGPU_IOCTL_CHANNEL_SUBMIT_GPFIFO.
 *
 * Usually, also @c #NVGPU_CHANNEL_SETUP_BIND_FLAGS_DETERMINISTIC is set.
 *
 * @pre @c #NVGPU_GPU_FLAGS_SUPPORT_USERMODE_SUBMIT
 */
#define NVGPU_CHANNEL_SETUP_BIND_FLAGS_USERMODE_SUPPORT		NVGPU_BIT32(3)

/**
 * @brief Map usermode resources to GPU and return GPU VAs, work submit token and the
 * hardware channel ID to the userspace.
 */
#define NVGPU_CHANNEL_SETUP_BIND_FLAGS_USERMODE_GPU_MAP_RESOURCES_SUPPORT	NVGPU_BIT32(4)

/** @} */

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CHANNEL
 * @defgroup NVGPURM_INTF_ALLOC_SETUP_BIND IOCTL: NVGPU_IOCTL_CHANNEL_SETUP_BIND
 * @brief Allocates or assigns the control buffers for the channel (GP FIFO,
 * USERD)
 *
 * The control buffers are used to control the channel execution. GP FIFO
 * contains the entries to pushbuffer fragments, and USERD contains the GP_PUT
 * field to denote the most recent submitted GP FIFO entry. USERD is
 * user-accessible for UMD-submit channels only. On KMD/RM-submit channels, new
 * work is submitted by using @c #NVGPU_IOCTL_CHANNEL_SUBMIT_GPFIFO in which
 * KMD/RM updates the USERD.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_channel_setup_bind_args</td>
 *   <td>@c #nvgpu_channel_setup_bind_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---     Flag @c #NVGPU_CHANNEL_SETUP_BIND_FLAGS_VPR_ENABLED is set
 *                     which is no longer supported.
 * - @c EINVAL ---     @c #nvgpu_channel_setup_bind_args::num_gpfifo_entries not in the
 *                    valid range.
 * - @c EINVAL ---     Channel not bound to an address space (@ref
 *                    NVGPURM_INTF_AS_BIND_CHANNEL)
 * - @c EINVAL ---     Channel not bound to a TSG (@ref
 *                    NVGPURM_INTF_TSG_BIND_CHANNEL_EX)
 * - @c EINVAL ---     Flag @c NVGPU_CHANNEL_SETUP_BIND_FLAGS_DETERMINISTIC is set and watchdog
 *                     is also enabled.
 * - @c EINVAL ---     Flag @c #NVGPU_CHANNEL_SETUP_BIND_FLAGS_DETERMINISTIC is not set but
 *                     flag @c #NVGPU_CHANNEL_SETUP_BIND_FLAGS_USERMODE_SUPPORT is set.
 * - @c EEXIST ---     GP FIFO already allocated.
 * - @c E2BIG ---      No space available to append the channel to runlist.
 * - @c ETIMEDOUT ---  Runlist update timed out.
 * - @c ENODEV ---     GPU is not ready.
 * - @c EFAULT ---     Copy from user space fails
 */

/**
 * @ingroup NVGPURM_INTF_ALLOC_SETUP_BIND
 * @brief Header struct for @ref NVGPURM_INTF_ALLOC_SETUP_BIND.
 */
struct nvgpu_channel_setup_bind_args {
	/**
	 * @brief Size of the GP FIFO in entries.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>GP FIFO size.</dd>
	 * </dl>
	 *
	 * Range:
	 * -# Always power of two (HW restriction)
	 * -# When @c #NVGPU_CHANNEL_SETUP_BIND_FLAGS_USERMODE_SUPPORT is:
	 *    -# specified: [2, 2^28] (UMD-submit channels)
	 *    -# not specified: [4, 2^28] (KMD/RM-submit channels)
	 *
	 * @remark For KMD/RM-submit channels, extra entries are consumed per
	 * kickoff when pre/post-sync fences are specified:
	 * - One extra entry for a pre-sync fence
	 * - One extra entry for a post-sync fence.
	 * The caller is expected to keep track of GP FIFO usage.
	 *
	 * @remark The minimum usable GP FIFO size is 2 entries (HW
	 * restriction). For KMD/RM-submit channels, the minimum is 4 entries due to
	 * implicit entries by pre/post sync fences.
	 *
	 * @remark A common way to detect when GP FIFO entries are executed is to
	 * use a job tracking semaphore and make the GPU increment it. When the
	 * increment is observed, all previous methods on the channel have been sent
	 * to the engines.
	 */
	__u32 num_gpfifo_entries;

	/**
	 * @brief Maximum number of in-flight submits (deterministic KMD/RM submit
	 * channels only)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Maximum number of in-flight submits. This parameter is used for
	 *     allocating the method buffer and other structures for bookkeeping of in-flight submits.</dd>
	 * </dl>
	 *
	 * Range:
	 * <table>
	 * <tr>
	 *   <th>@c NVGPU_CHANNEL_SETUP_BIND_FLAGS_USERMODE_SUPPORT</th>
	 *   <th>@c NVGPU_CHANNEL_SETUP_BIND_FLAGS_DETERMINISTIC</th>
	 *   <th>Range</th>
	 * </tr>
	 * <tr>
	 *   <td>Unset</td>
	 *   <td>Unset</td>
	 *   <td>Ignored (dynamic job tracking)</td>
	 * </tr>
	 * <tr>
	 *   <td>Unset</td>
	 *   <td>Set</td>
	 *   <td>[1, @c #num_entries]</td>
	 * </tr>
	 * <tr>
	 *   <td>Set</td>
	 *   <td>Unset</td>
	 *   <td>Ignored (KMD/RM job tracking not used)</td>
	 * </tr>
	 * <tr>
	 *   <td>Set</td>
	 *   <td>Set</td>
	 *   <td>Ignored (KMD/RM job tracking not used)</td>
	 * </tr>
	 * </table>
	 */
	__u32 num_inflight_jobs;

	/**
	 * @brief GP FIFO allocation flags (@c NVGPU_CHANNEL_SETUP_BIND_FLAGS_*).
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Allocation flags.</dd>
	 * </dl>
	 */
	__u32 flags;

	/**
	 * @brief dma-buf fd of the USERD memory buffer (UMD submit channels only)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd> dma-buf fd of the userd memory buffer. The buffer must be big enough to contain
	 *     the RAM USERD buffer (HW-specific).</dd>
	 * </dl>
	 *
	 * When @c #NVGPU_CHANNEL_SETUP_BIND_FLAGS_USERMODE_SUPPORT is not specified,
	 * this field is ignored.
	 *
	 * @sa https://p4viewer.nvidia.com/get///hw/doc/gpu/ampere_info/gen_manuals/ga10b/dev_ram.ref /
	 * RAMUSERD
	 *
	 * @sa https://p4viewer.nvidia.com/get///hw/doc/gpu/blackwell_info/gen_manuals/gb10b/dev_ram.ref /
	 * RAMUSERD
	 */
	__s32 userd_dmabuf_fd;

	/**
	 * @brief dma-buf fd of the GPFIFO memory buffer (UMD submit channels only)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd> dma-buf fd of the userd memory buffer. The buffer must be big enough to contain
	 *     the GPFIFO buffer (HW-specific).</dd>
	 * </dl>
	 *
	 * When @c #NVGPU_CHANNEL_SETUP_BIND_FLAGS_USERMODE_SUPPORT is not specified,
	 * this field is ignored.
	 *
	 * @sa https://p4viewer.nvidia.com/get///hw/doc/gpu/ampere_info/gen_manuals/ga10b/dev_esched_pbdma.ref /
	 * Sec 2 - GPENTRY DATA FORMAT.
	 *
	 * @sa https://p4viewer.nvidia.com/get///hw/doc/gpu/blackwell_info/gen_manuals/gb10b/dev_esched_pbdma.ref /
	 * Sec 2 - GPENTRY DATA FORMAT.
	 */
	__s32 gpfifo_dmabuf_fd;

	/**
	 * @brief Work submit token
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>Work submit token to be used in UMD submit procedure.</dd>
	 * </dl>
	 *
	 * The work submit token is written to the DOORBELL register to signal the
	 * GPU that the new GP FIFO entries have been submitted to the channel.
	 */
	__u32 work_submit_token;

	/**
	 * @brief USERD buffer offset (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Starting offset of the USERD buffer to be used</dd>
	 * </dl>
	 *
	 * @remark Not yet supported in the driver.
	 */
	__u64 userd_dmabuf_offset;

	/**
	 * @brief GPFIFO buffer offset (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Starting offset of the GPFIFO buffer to be used</dd>
	 * </dl>
	 *
	 * @remark Not yet supported in the driver.
	 */
	__u64 gpfifo_dmabuf_offset;

	/**
	 * @brief GPU virtual address of GPFIFO buffer mapping
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>GPU virtual address of GPFIFO buffer mapping</dd>
	 * </dl>
	 */
	__u64 gpfifo_gpu_va;

	/**
	 * @brief GPU virtual address of USERD buffer mapping
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>GPU virtual address of USERD buffer mapping</dd>
	 * </dl>
	 */
	__u64 userd_gpu_va;

	/**
	 * @brief GPU virtual address for usermode region
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>GPU virtual address for usermode region</dd>
	 * </dl>
	 *
	 * The usermode region is a set of registers intended to be directly accessed by
	 * usermode. The provided functionality includes:
	 * - The GPU 64-bit nanosecond time (NV_USERMODE_TIME_0, NV_USERMODE_TIME_1)
	 * - Doorbell register to trigger the GPU to read the channel-specific
	 *   RAMUSERD/GP_PUT field as part of the usermode kickoff procedure.
	 *
	 * On AArch64, the provided MMIO region uses the Device-nGnRE memory type.
	 */
	__u64 usermode_mmio_gpu_va;

	/**
	 * @brief Hardware channel id.
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>Hardware channel id.</dd>
	 * </dl>
	 *
	 * The hardware channel id can be used to identify the channel in the nvgpu
	 * system log fault dumps.
	 */
	__u32 hw_channel_id;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved[8];
};

struct nvgpu_fence {
	__u32 id;        /* syncpoint id or sync fence fd */
	__u32 value;     /* syncpoint value (discarded when using sync fence) */
};

/* insert a wait on the fence before submitting gpfifo */
#define NVGPU_SUBMIT_GPFIFO_FLAGS_FENCE_WAIT	(1 << 0)
/* insert a fence update after submitting gpfifo and
   return the new fence for others to wait on */
#define NVGPU_SUBMIT_GPFIFO_FLAGS_FENCE_GET	(1 << 1)
/* choose between different gpfifo entry formats */
#define NVGPU_SUBMIT_GPFIFO_FLAGS_HW_FORMAT	(1 << 2)
/* interpret fence as a sync fence fd instead of raw syncpoint fence */
#define NVGPU_SUBMIT_GPFIFO_FLAGS_SYNC_FENCE	(1 << 3)
/* suppress WFI before fence trigger */
#define NVGPU_SUBMIT_GPFIFO_FLAGS_SUPPRESS_WFI	(1 << 4)
/* skip buffer refcounting during submit */
#define NVGPU_SUBMIT_GPFIFO_FLAGS_SKIP_BUFFER_REFCOUNTING	(1 << 5)

struct nvgpu_submit_gpfifo_args {
	__u64 gpfifo;
	__u32 num_entries;
	__u32 flags;
	struct nvgpu_fence fence;
};

struct nvgpu_wait_args {
#define NVGPU_WAIT_TYPE_NOTIFIER	0x0
#define NVGPU_WAIT_TYPE_SEMAPHORE	0x1
	__u32 type;
	__u32 timeout;
	union {
		struct {
			/* handle and offset for notifier memory */
			__u32 dmabuf_fd;
			__u32 offset;
			__u32 padding1;
			__u32 padding2;
		} notifier;
		struct {
			/* handle and offset for semaphore memory */
			__u32 dmabuf_fd;
			__u32 offset;
			/* semaphore payload to wait for */
			__u32 payload;
			__u32 padding;
		} semaphore;
	} condition; /* determined by type field */
};

struct nvgpu_set_timeout_args {
	__u32 timeout;
};

struct nvgpu_set_timeout_ex_args {
	__u32 timeout;
	__u32 flags;
};

#define NVGPU_ZCULL_MODE_GLOBAL		0
#define NVGPU_ZCULL_MODE_NO_CTXSW		1
#define NVGPU_ZCULL_MODE_SEPARATE_BUFFER	2
#define NVGPU_ZCULL_MODE_PART_OF_REGULAR_BUF	3

struct nvgpu_zcull_bind_args {
	__u64 gpu_va;
	__u32 mode;
	__u32 padding;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CHANNEL
 * @defgroup NVGPURM_INTF_SET_ERR_NOTIF IOCTL: NVGPU_IOCTL_CHANNEL_SET_ERROR_NOTIFIER
 * @brief Sets the error notifier buffer for the channel.
 *
 * Error notifier is a buffer that receives information from the NvGPU KMD/RM
 * when the channel encounters an asynchronous error. This could be, for
 * example, a GMMU fault due to shader access to an invalid GPU virtual memory
 * address. Error notifier is triggered also when the surrounding context
 * encounters an error that renders the channel inoperable. The notification
 * struct layout is defined by @c #nvgpu_notification.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_set_error_notifier</td>
 *   <td>@c #nvgpu_set_error_notifier</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---  Invalid @c #nvgpu_set_error_notifier::mem_import_id
 * - @c ENOMEM ---  Memory allocation failed
 * - @c EINVAL ---  @c #nvgpu_set_error_notifier::offset out of bounds
 * - @c ENODEV ---  GPU not ready
 * - @c EFAULT ---  Copy from user space fails
 *
 * @remark When waiting for a GPU task to complete, the error notifier is used
 * typically as follows:
 * -# Check task completion semaphore
 * -# If task completed, GO TO STEP 6
 * -# Read error notifier (@c #nvgpu_notification::status)
 * -# If error notifier indicates no error, then GO TO STEP 1
 * -# GO TO STEP 7
 * -# Read error notifier (@c #nvgpu_notification::status)
 * -# Depending on @c #nvgpu_notification::status, task COMPLETED or FAILED.
 *
 * @remark In case sync fds or syncpoints are used, then STEPs 1--5 are replaced with
 * sync fd wait or syncpoint wait.
 *
 * @remark Instead of @c #nvgpu_notification, the caller should use @c
 * NvRmGpuNotification provided by %nvrm_gpu when cross-OS compatibility is
 * required. Both definitions are equivalent.
 */

/**
 * @ingroup NVGPURM_INTF_SET_ERR_NOTIF
 * @brief Header struct for @ref NVGPURM_INTF_SET_ERR_NOTIF
 */
struct nvgpu_set_error_notifier {
	/**
	 * @brief Offset of the error notifier within the memory buffer (in bytes).
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Error notifier offset</dd>
	 * </dl>
	 *
	 * Range: [0, @c Buffer_Size - <tt>sizeof(#nvgpu_notification)</tt>]<br>
	 * where @c Buffer_Size is the size of the error notifier buffer (@c
	 * #mem).
	 *
	 * @remark The same underlying memory buffer can be used for multiple
	 * channel error notifiers.
	 */
	__u64 offset;
	/**
	 * @brief Size of the error notifier struct. Must equal to
	 * <tt>sizeof(#nvgpu_notification)</tt>.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd><tt>sizeof(#nvgpu_notification)</tt></dd>
	 * </dl>
	 *
	 * Range: <tt>sizeof(#nvgpu_notification)</tt>
	 *
	 * @remark This field is not checked by KMD/RM. The field is intended to
	 * allow future extensibility of the error notifier.
	 */
	__u64 size;
	/**
	 * @brief dma-buf file descriptor of the error notifier buffer
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>dma-buf fd of error notfier buffer</dd>
	 * </dl>
	 */
	__u32 mem;
	/**
	 * @brief Padding
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 padding;
};

/**
 * @ingroup NVGPURM_INTF_SET_ERR_NOTIF
 * @{
 */

/** @brief Fifo idle timeout error. */
#define	NVGPU_CHANNEL_FIFO_ERROR_IDLE_TIMEOUT		8U

/** @brief Gr sw method error. */
#define	NVGPU_CHANNEL_GR_ERROR_SW_METHOD		12U

/** @brief Gr sw notify error. */
#define	NVGPU_CHANNEL_GR_ERROR_SW_NOTIFY		13U

/** @brief Gr sw exception error. */
#define	NVGPU_CHANNEL_GR_EXCEPTION			13U

/** @brief Gr semaphore timeout error. */
#define	NVGPU_CHANNEL_GR_SEMAPHORE_TIMEOUT		24U

/** @brief Gr illegal notify error. */
#define	NVGPU_CHANNEL_GR_ILLEGAL_NOTIFY			25U

/** @brief Fifo mmu fault error. */
#define	NVGPU_CHANNEL_FIFO_ERROR_MMU_ERR_FLT		31U

/** @brief Pbdma error. */
#define	NVGPU_CHANNEL_PBDMA_ERROR			32U

/** @brief Fecs firmware method error. */
#define NVGPU_CHANNEL_FECS_ERR_UNIMP_FIRMWARE_METHOD	37U

/** @brief Reset channel verification error. */
#define	NVGPU_CHANNEL_RESETCHANNEL_VERIF_ERROR		43U

/** @brief Pbdma pushbuffer crc mismatch error. */
#define	NVGPU_CHANNEL_PBDMA_PUSHBUFFER_CRC_MISMATCH	80U

/** @brief Unused (deprecated). */
#define	NVGPU_CHANNEL_SUBMIT_TIMEOUT			1U

/** @} */

/**
 * @ingroup NVGPURM_INTF_SET_ERR_NOTIF
 *
 * @brief Error notification structure.
 *
 * @sa @c NvRmGpuNotification
 */
struct nvgpu_notification {

	/**
	 * @brief Nanoseconds since Jan. 1, 1970 (system clock CLOCK_REALTIME)
	 *
	 * @note This definition may be incorrect for NvGPU resource manager.
	 */
    struct {
	__u32 nanoseconds[2];
    } time_stamp;

	/**
	 * @brief Error code. See @c NVGPU_CHANNEL_* error definitions.
	 */
	__u32 info32;

	/** @brief Filled with channel id. */
	__u16 info16;

	/**
	 * @brief Error indicator.
	 *
	 * NvGPU resource manager sets the status to @c 0xFFFF to indicate that an
	 * error has occurred.
	 */
	__u16 status;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CHANNEL
 * @defgroup NVGPURM_INTF_CHANNEL_WDT IOCTL: NVGPU_IOCTL_CHANNEL_WDT
 * @brief Controls the watchdog for the channel forward progress.
 *
 * When enabled and the channel is active, the channel watchdog samples the
 * channel state periodically and checks whether the channel is making forward
 * progress. Forward progress here means that channel methods get executed. But
 * note that the execution of long-running compute kernels and graphics shaders
 * is not accounted for forward progress, and thus, they may trip the watchdog.
 *
 * When the watchdog trips, the channel is faulted and the error notifier (if
 * configured) will be set.
 *
 * The channel watchdog is mainly intended for detecting stuck fence waits where
 * the releaser has faulted and is no longer able to release the fence.
 *
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_channel_wdt_args</td>
 *   <td>@c #nvgpu_channel_wdt_args</td>
 * </tr>
 * </table>
 *
 * @remark The channel watchdog is not supported in safety. For compatibility
 * reasons, calling this IOCTL is allowed with parameters to disable the watchdog.
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---  Channel is deterministic but @c #nvgpu_channel_wdt_args::wdt_status is not set
 *                  to @c #NVGPU_IOCTL_CHANNEL_DISABLE_WDT.
 * - @c EINVAL ---  @c #nvgpu_channel_wdt_args::wdt_status is not set to
 *                  @c #NVGPU_IOCTL_CHANNEL_DISABLE_WDT 0r @c #NVGPU_IOCTL_CHANNEL_ENABLE_WDT.
 * - @c ENODEV ---  GPU not ready.
 * - @c EFAULT ---  Copy from user space fails
 */

/**
 * @ingroup NVGPURM_INTF_CHANNEL_WDT
 * @brief Header struct for @ref NVGPURM_INTF_CHANNEL_WDT.
 */
struct nvgpu_channel_wdt_args {

	/**
	 * @brief Controls the watchdog state
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Sets the watchdog control state. In safety, the value must be @c
	 *     #NVGPU_IOCTL_CHANNEL_DISABLE_WDT.</dd>
	 * </dl>
	 *
	 * Range: A combination of @c #NVGPU_IOCTL_CHANNEL_DISABLE_WDT,
	 * @c #NVGPU_IOCTL_CHANNEL_ENABLE_WDT,
	 * @c #NVGPU_IOCTL_CHANNEL_WDT_FLAG_SET_TIMEOUT,
	 * @c #NVGPU_IOCTL_CHANNEL_WDT_FLAG_DISABLE_DUMP. See individual flags for
	 * details and restrictions.
	 */
	__u32 wdt_status;

	/**
	 * @brief New watchdog timeout (ms) when
	 * @c #NVGPU_IOCTL_CHANNEL_WDT_FLAG_SET_TIMEOUT is set. Otherwise, ignored.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>New watchdog timeout.</dd>
	 * </dl>
	 *
	 * When set, the channel is allowed to make no forward progress for at least
	 * the timeout period.
	 *
	 * Range:
	 * - When @c #NVGPU_IOCTL_CHANNEL_WDT_FLAG_SET_TIMEOUT is specified: [1,
	 *   0xFFFFFFFFU]
	 * - Otherwise: ignored
	 */
	__u32 timeout_ms;
};

/**
 * @ingroup NVGPURM_INTF_CHANNEL_WDT
 *
 * @{
 */

/**
 * @brief Disables the channel watchdog. This flag is mutually exclusive with
 * @c #NVGPU_IOCTL_CHANNEL_ENABLE_WDT.
 */
#define NVGPU_IOCTL_CHANNEL_DISABLE_WDT			NVGPU_BIT32(0)

/**
 * @brief Enables the channel watchdog. This flag is mutually exclusive with
 * @c #NVGPU_IOCTL_CHANNEL_DISABLE_WDT.
 *
 * @remark Not supported in safety.
 */
#define NVGPU_IOCTL_CHANNEL_ENABLE_WDT			NVGPU_BIT32(1)

/**
 * @brief Sets the watchdog timeout. Use @c #nvgpu_channel_wdt_args::timeout_ms
 * to set the value.
 *
 * @remark Not supported in safety.
 */
#define NVGPU_IOCTL_CHANNEL_WDT_FLAG_SET_TIMEOUT	NVGPU_BIT32(2)

/**
 * @brief Disables the channel watchdog dump to system log when the watchdog
 * triggers.
 *
 * @remark Not supported in safety.
 */
#define NVGPU_IOCTL_CHANNEL_WDT_FLAG_DISABLE_DUMP	NVGPU_BIT32(3)

/** @} */

/*
 * Interleaving channels in a runlist is an approach to improve
 * GPU scheduling by allowing certain channels to appear multiple
 * times on the runlist. The number of times a channel appears is
 * governed by the following levels:
 *
 * low (L)   : appears once
 * medium (M): if L, appears L times
 *             else, appears once
 * high (H)  : if L, appears (M + 1) x L times
 *             else if M, appears M times
 *             else, appears once
 */
struct nvgpu_runlist_interleave_args {
	__u32 level;
	__u32 reserved;
};
#define NVGPU_RUNLIST_INTERLEAVE_LEVEL_LOW	0
#define NVGPU_RUNLIST_INTERLEAVE_LEVEL_MEDIUM	1
#define NVGPU_RUNLIST_INTERLEAVE_LEVEL_HIGH	2
#define NVGPU_RUNLIST_INTERLEAVE_NUM_LEVELS	3

/* controls how long a channel occupies an engine uninterrupted */
struct nvgpu_timeslice_args {
	__u32 timeslice_us;
	__u32 reserved;
};

struct nvgpu_event_id_ctrl_args {
	__u32 cmd; /* in */
	__u32 event_id; /* in */
	__s32 event_fd; /* out */
	__u32 padding;
};
#define NVGPU_IOCTL_CHANNEL_EVENT_ID_BPT_INT		0
#define NVGPU_IOCTL_CHANNEL_EVENT_ID_BPT_PAUSE		1
#define NVGPU_IOCTL_CHANNEL_EVENT_ID_BLOCKING_SYNC	2
#define NVGPU_IOCTL_CHANNEL_EVENT_ID_CILP_PREEMPTION_STARTED	3
#define NVGPU_IOCTL_CHANNEL_EVENT_ID_CILP_PREEMPTION_COMPLETE	4
#define NVGPU_IOCTL_CHANNEL_EVENT_ID_GR_SEMAPHORE_WRITE_AWAKEN	5
#define NVGPU_IOCTL_CHANNEL_EVENT_ID_MAX		6

#define NVGPU_IOCTL_CHANNEL_EVENT_ID_CMD_ENABLE		1

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CHANNEL
 * @defgroup NVGPURM_INTF_SET_PREEMPTION_MODE IOCTL: NVGPU_IOCTL_CHANNEL_SET_PREEMPTION_MODE
 * @brief Sets the channel preemption modes.
 *
 * The channel preemption modes specify the context that is required to be saved
 * on preemption, and consequently, the amount of engine pipeline draining
 * needed until the channel can be preempted. Essentially, the more fine-grain the
 * preemption (less drainage), the more context needs to be saved.
 *
 * The default preemption mode is #NVGPU_COMPUTE_PREEMPTION_MODE_WFI /
 * #NVGPU_GRAPHICS_PREEMPTION_MODE_WFI.
 *
 * Preemption mode can be safely set before any work has been submitted to the
 * channel. In case work has already been submitted, the following procedure is
 * used:
 *
 * -# Disable the TSG of the channel (@c #NVGPU_IOCTL_TSG_DISABLE)
 * -# Preempt the TSG (@c NVGPURM_INTF_TSG_PREEMPT)
 * -# Set preemption modes (this IOCTL)
 * -# Enable the TSG (@c #NVGPU_IOCTL_TSG_ENABLE)
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_preemption_mode_args</td>
 *   <td>(none)</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---    Setting preemption mode is not supported
 * - @c EINVAL ---    Invalid preemption mode(s)
 * - @c ENODEV ---    GPU not ready
 * - @c EFAULT ---    Copy from user space fails
 *
 * @pre <tt>(nvgpu_gpu_characteristics::graphics_preemption_mode_flags |
 *      nvgpu_gpu_characteristics::compute_preemption_mode_flags) &ne; 0</tt>
 */

/**
 * @ingroup NVGPURM_INTF_SET_PREEMPTION_MODE
 * @{
 */

/** @brief Unused (deprecated). */
#define NVGPU_ALLOC_OBJ_FLAGS_LOCKBOOST_ZERO   (1 << 0)

/**
 * @brief Graphics preemption mode: WFI
 */
#define NVGPU_GRAPHICS_PREEMPTION_MODE_WFI	NVGPU_BIT32(0)

/**
 * @brief Graphics preemption mode: GFXP
 *
 * @remark Not supported in safety
 */
#define NVGPU_GRAPHICS_PREEMPTION_MODE_GFXP	NVGPU_BIT32(1)

/**
 * @brief Compute preemption mode: WFI
 */
#define NVGPU_COMPUTE_PREEMPTION_MODE_WFI	NVGPU_BIT32(0)

/**
 * @brief Compute preemption mode: CTA
 */
#define NVGPU_COMPUTE_PREEMPTION_MODE_CTA	NVGPU_BIT32(1)

/**
 * @brief Compute preemption mode: CILP
 *
 * @remark Not supported in safety
 */
#define NVGPU_COMPUTE_PREEMPTION_MODE_CILP	NVGPU_BIT32(2)

/** @} */

/**
 * @ingroup NVGPURM_INTF_SET_PREEMPTION_MODE
 * @brief Header struct for @ref NVGPURM_INTF_SET_PREEMPTION_MODE.
 */
struct nvgpu_preemption_mode_args {

	/**
	 * @brief When non-zero, new graphics preemption mode for the channel.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Graphics preemption mode. 0 or one of @c NVGPU_GRAPHICS_PREEMPTION_MODE_*.
	 *   </dd>
	 * </dl>
	 *
	 * Range: Bit mask @c
	 * #nvgpu_gpu_characteristics::graphics_preemption_mode_flags indicates the
	 * supported graphics preemption modes.
	 */
	__u32 graphics_preempt_mode; /* in */

	/**
	 * @brief When non-zero, new compute preemption mode
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Compute preemption mode. 0 or one of @c NVGPU_COMPUTE_PREEMPTION_MODE_*.
	 *   </dd>
	 * </dl>
	 *
	 * Range: Bit mask @c
	 * #nvgpu_gpu_characteristics::compute_preemption_mode_flags indicates the
	 * supported compute preemption modes.
	 */
	__u32 compute_preempt_mode;
};

struct nvgpu_boosted_ctx_args {
#define NVGPU_BOOSTED_CTX_MODE_NORMAL			(0U)
#define NVGPU_BOOSTED_CTX_MODE_BOOSTED_EXECUTION	(1U)
	__u32 boost;
	__u32 padding;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_CHANNEL
 * @defgroup NVGPURM_INTF_CHANNEL_GET_SYNCPT IOCTL: NVGPU_IOCTL_CHANNEL_GET_USER_SYNCPOINT
 * @brief Allocates and/or queries the channel read-write syncpoint.
 *
 * This IOCTL allocates the channel syncpoint if not already allocated, and
 * returns the syncpoint ID, syncpoint allocation-time initial value, and the
 * GPU VA of the syncpoint for read-write access.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>(none)</td>
 *   <td>@c #nvgpu_get_user_syncpoint_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---  Channel syncpoints not supported.
 * - @c ENOMEM ---  Syncpoint allocation failed.
 * - @c ENODEV ---  GPU not ready.
 * - @c EFAULT ---  Copy from user space fails
 *
 * @pre @c #NVGPU_CHANNEL_SETUP_BIND_FLAGS_DETERMINISTIC specified for the channel
 */

/**
 * @ingroup NVGPURM_INTF_CHANNEL_GET_SYNCPT
 * @brief Header struct for @ref NVGPURM_INTF_CHANNEL_GET_SYNCPT
 */
struct nvgpu_get_user_syncpoint_args {

	/**
	 * @brief GPU virtual address for the read-write syncpoint in syncpoint shim
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>GPU virtual address</dd>
	 * </dl>
	 *
	 * Range: &ge; @c #nvgpu_as_alloc_ex_args::va_range_end (NvGPU resource
	 * manager private region)
	 */
	__u64 gpu_va;

	/**
	 * @brief Channel syncpoint identifier
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>Syncpoint identifier</dd>
	 * </dl>
	 *
	 * Range: Platform-specific
	 *
	 * @sa @c NvRmHost1xSyncpointGetId()
	 */
	__u32 syncpoint_id;

	/**
	 * @brief Initial value of the syncpoint
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>Initial value</dd>
	 * </dl>
	 *
	 * Range: [0, 0xFFFFFFFF]
	 */
	__u32 syncpoint_max;
};

struct nvgpu_reschedule_runlist_args {
#define NVGPU_RESCHEDULE_RUNLIST_PREEMPT_NEXT           (1 << 0)
	__u32 flags;
};

#define NVGPU_IOCTL_CHANNEL_SET_NVMAP_FD	\
	_IOW(NVGPU_IOCTL_MAGIC, 5, struct nvgpu_set_nvmap_fd_args)
#define NVGPU_IOCTL_CHANNEL_SET_TIMEOUT	\
	_IOW(NVGPU_IOCTL_MAGIC, 11, struct nvgpu_set_timeout_args)
#define NVGPU_IOCTL_CHANNEL_GET_TIMEDOUT	\
	_IOR(NVGPU_IOCTL_MAGIC, 12, struct nvgpu_get_param_args)
#define NVGPU_IOCTL_CHANNEL_SET_TIMEOUT_EX	\
	_IOWR(NVGPU_IOCTL_MAGIC, 18, struct nvgpu_set_timeout_ex_args)
#define NVGPU_IOCTL_CHANNEL_WAIT		\
	_IOWR(NVGPU_IOCTL_MAGIC, 102, struct nvgpu_wait_args)
#define NVGPU_IOCTL_CHANNEL_SUBMIT_GPFIFO	\
	_IOWR(NVGPU_IOCTL_MAGIC, 107, struct nvgpu_submit_gpfifo_args)
/**
 * @ingroup NVGPURM_INTF_ALLOC_OBJ_CTX
 * @brief Command code for @ref NVGPURM_INTF_ALLOC_OBJ_CTX.
 */
#define NVGPU_IOCTL_CHANNEL_ALLOC_OBJ_CTX	\
	_IOWR(NVGPU_IOCTL_MAGIC, 108, struct nvgpu_alloc_obj_ctx_args)
#define NVGPU_IOCTL_CHANNEL_ZCULL_BIND		\
	_IOWR(NVGPU_IOCTL_MAGIC, 110, struct nvgpu_zcull_bind_args)
/**
 * @ingroup NVGPURM_INTF_SET_ERR_NOTIF
 * @brief Command code for @ref NVGPURM_INTF_SET_ERR_NOTIF.
 */
#define NVGPU_IOCTL_CHANNEL_SET_ERROR_NOTIFIER  \
	_IOWR(NVGPU_IOCTL_MAGIC, 111, struct nvgpu_set_error_notifier)
#define NVGPU_IOCTL_CHANNEL_OPEN	\
	_IOR(NVGPU_IOCTL_MAGIC,  112, struct nvgpu_channel_open_args)
#define NVGPU_IOCTL_CHANNEL_ENABLE	\
	_IO(NVGPU_IOCTL_MAGIC,  113)
#define NVGPU_IOCTL_CHANNEL_DISABLE	\
	_IO(NVGPU_IOCTL_MAGIC,  114)
#define NVGPU_IOCTL_CHANNEL_PREEMPT	\
	_IO(NVGPU_IOCTL_MAGIC,  115)
#define NVGPU_IOCTL_CHANNEL_FORCE_RESET	\
	_IO(NVGPU_IOCTL_MAGIC,  116)
#define NVGPU_IOCTL_CHANNEL_EVENT_ID_CTRL \
	_IOWR(NVGPU_IOCTL_MAGIC, 117, struct nvgpu_event_id_ctrl_args)
/**
 * @ingroup NVGPURM_INTF_CHANNEL_WDT
 * @brief Command code for @ref NVGPURM_INTF_CHANNEL_WDT.
 */
#define NVGPU_IOCTL_CHANNEL_WDT \
	_IOW(NVGPU_IOCTL_MAGIC, 119, struct nvgpu_channel_wdt_args)
#define NVGPU_IOCTL_CHANNEL_SET_RUNLIST_INTERLEAVE \
	_IOW(NVGPU_IOCTL_MAGIC, 120, struct nvgpu_runlist_interleave_args)
/**
 * @ingroup NVGPURM_INTF_SET_PREEMPTION_MODE
 * @brief Command code for @ref NVGPURM_INTF_SET_PREEMPTION_MODE.
 */
#define NVGPU_IOCTL_CHANNEL_SET_PREEMPTION_MODE \
	_IOW(NVGPU_IOCTL_MAGIC, 122, struct nvgpu_preemption_mode_args)
/**
 * @ingroup NVGPURM_INTF_ALLOC_GPFIFO_EX
 * @brief Command code for @ref NVGPURM_INTF_ALLOC_GPFIFO_EX
 */
#define NVGPU_IOCTL_CHANNEL_ALLOC_GPFIFO_EX	\
	_IOWR(NVGPU_IOCTL_MAGIC, 123, struct nvgpu_alloc_gpfifo_ex_args)
#define NVGPU_IOCTL_CHANNEL_SET_BOOSTED_CTX	\
	_IOW(NVGPU_IOCTL_MAGIC, 124, struct nvgpu_boosted_ctx_args)
/**
 * @ingroup NVGPURM_INTF_CHANNEL_GET_SYNCPT
 * @brief Command code for @ref NVGPURM_INTF_CHANNEL_GET_SYNCPT.
 */
#define NVGPU_IOCTL_CHANNEL_GET_USER_SYNCPOINT \
	_IOR(NVGPU_IOCTL_MAGIC, 126, struct nvgpu_get_user_syncpoint_args)
#define NVGPU_IOCTL_CHANNEL_RESCHEDULE_RUNLIST	\
	_IOW(NVGPU_IOCTL_MAGIC, 127, struct nvgpu_reschedule_runlist_args)
/**
 * @ingroup NVGPURM_INTF_ALLOC_SETUP_BIND
 * @brief Command code for @ref NVGPURM_INTF_ALLOC_SETUP_BIND.
 */
#define NVGPU_IOCTL_CHANNEL_SETUP_BIND	\
	_IOWR(NVGPU_IOCTL_MAGIC, 128, struct nvgpu_channel_setup_bind_args)

#define NVGPU_IOCTL_CHANNEL_LAST	\
	_IOC_NR(NVGPU_IOCTL_CHANNEL_SETUP_BIND)
#define NVGPU_IOCTL_CHANNEL_MAX_ARG_SIZE \
	sizeof(struct nvgpu_channel_setup_bind_args)

/*
 * /dev/nvhost-ctxsw-gpu device
 *
 * Opening a '/dev/nvhost-ctxsw-gpu' device node creates a way to trace
 * context switches on GR engine
 */

#define NVGPU_CTXSW_IOCTL_MAGIC 'C'

#define NVGPU_CTXSW_TAG_SOF			0x00
#define NVGPU_CTXSW_TAG_CTXSW_REQ_BY_HOST	0x01
#define NVGPU_CTXSW_TAG_FE_ACK			0x02
#define NVGPU_CTXSW_TAG_FE_ACK_WFI		0x0a
#define NVGPU_CTXSW_TAG_FE_ACK_GFXP		0x0b
#define NVGPU_CTXSW_TAG_FE_ACK_CTAP		0x0c
#define NVGPU_CTXSW_TAG_FE_ACK_CILP		0x0d
#define NVGPU_CTXSW_TAG_SAVE_END		0x03
#define NVGPU_CTXSW_TAG_RESTORE_START		0x04
#define NVGPU_CTXSW_TAG_CONTEXT_START		0x05
#define NVGPU_CTXSW_TAG_ENGINE_RESET		0xfe
#define NVGPU_CTXSW_TAG_INVALID_TIMESTAMP	0xff
#define NVGPU_CTXSW_TAG_LAST			\
	NVGPU_CTXSW_TAG_INVALID_TIMESTAMP

struct nvgpu_ctxsw_trace_entry {
	__u8 tag;
	__u8 vmid;
	__u16 seqno;		/* sequence number to detect drops */
	__u32 context_id;	/* context_id as allocated by FECS */
	__u64 pid;		/* 64-bit is max bits of different OS pid */
	__u64 timestamp;	/* 64-bit time */
};

#define NVGPU_CTXSW_RING_HEADER_MAGIC 0x7000fade
#define NVGPU_CTXSW_RING_HEADER_VERSION 0

struct nvgpu_ctxsw_ring_header {
	__u32 magic;
	__u32 version;
	__u32 num_ents;
	__u32 ent_size;
	volatile __u32 drop_count;	/* excluding filtered out events */
	volatile __u32 write_seqno;
	volatile __u32 write_idx;
	volatile __u32 read_idx;
};

struct nvgpu_ctxsw_ring_setup_args {
	__u32 size;	/* [in/out] size of ring buffer in bytes (including
			   header). will be rounded page size. this parameter
			   is updated with actual allocated size. */
};

#define NVGPU_CTXSW_FILTER_SIZE	(NVGPU_CTXSW_TAG_LAST + 1)
#define NVGPU_CTXSW_FILTER_SET(n, p) \
	((p)->tag_bits[(n) / 64] |=  (1 << ((n) & 63)))
#define NVGPU_CTXSW_FILTER_CLR(n, p) \
	((p)->tag_bits[(n) / 64] &= ~(1 << ((n) & 63)))
#define NVGPU_CTXSW_FILTER_ISSET(n, p) \
	((p)->tag_bits[(n) / 64] &   (1 << ((n) & 63)))
#define NVGPU_CTXSW_FILTER_CLR_ALL(p) \
	((void) memset((void *)(p), 0, sizeof(*(p))))
#define NVGPU_CTXSW_FILTER_SET_ALL(p) \
	((void) memset((void *)(p), ~0, sizeof(*(p))))

struct nvgpu_ctxsw_trace_filter {
	__u64 tag_bits[(NVGPU_CTXSW_FILTER_SIZE + 63) / 64];
};

struct nvgpu_ctxsw_trace_filter_args {
	struct nvgpu_ctxsw_trace_filter filter;
};

#define NVGPU_CTXSW_IOCTL_TRACE_ENABLE \
	_IO(NVGPU_CTXSW_IOCTL_MAGIC, 1)
#define NVGPU_CTXSW_IOCTL_TRACE_DISABLE \
	_IO(NVGPU_CTXSW_IOCTL_MAGIC, 2)
#define NVGPU_CTXSW_IOCTL_RING_SETUP \
	_IOWR(NVGPU_CTXSW_IOCTL_MAGIC, 3, struct nvgpu_ctxsw_ring_setup_args)
#define NVGPU_CTXSW_IOCTL_SET_FILTER \
	_IOW(NVGPU_CTXSW_IOCTL_MAGIC, 4, struct nvgpu_ctxsw_trace_filter_args)
#define NVGPU_CTXSW_IOCTL_GET_FILTER \
	_IOR(NVGPU_CTXSW_IOCTL_MAGIC, 5, struct nvgpu_ctxsw_trace_filter_args)
#define NVGPU_CTXSW_IOCTL_POLL \
	_IO(NVGPU_CTXSW_IOCTL_MAGIC, 6)

#define NVGPU_CTXSW_IOCTL_LAST            \
	_IOC_NR(NVGPU_CTXSW_IOCTL_POLL)

#define NVGPU_CTXSW_IOCTL_MAX_ARG_SIZE	\
	sizeof(struct nvgpu_ctxsw_trace_filter_args)

/*
 * /dev/nvhost-sched-gpu device
 *
 * Opening a '/dev/nvhost-sched-gpu' device node creates a way to control
 * GPU scheduling parameters.
 */

#define NVGPU_SCHED_IOCTL_MAGIC 'S'

/*
 * When the app manager receives a NVGPU_SCHED_STATUS_TSG_OPEN notification,
 * it is expected to query the list of recently opened TSGs using
 * NVGPU_SCHED_IOCTL_GET_RECENT_TSGS. The kernel driver maintains a bitmap
 * of recently opened TSGs. When the app manager queries the list, it
 * atomically clears the bitmap. This way, at each invocation of
 * NVGPU_SCHED_IOCTL_GET_RECENT_TSGS, app manager only receives the list of
 * TSGs that have been opened since last invocation.
 *
 * If the app manager needs to re-synchronize with the driver, it can use
 * NVGPU_SCHED_IOCTL_GET_TSGS to retrieve the complete list of TSGs. The
 * recent TSG bitmap will be cleared in that case too.
 */
struct nvgpu_sched_get_tsgs_args {
	/* in: size of buffer in bytes */
	/* out: actual size of size of TSG bitmap. if user-provided size is too
	 * small, ioctl will return -ENOSPC, and update this field, allowing
	 * application to discover required number of bytes and allocate
	 * a buffer accordingly.
	 */
	__u32 size;

	/* in: address of 64-bit aligned buffer */
	/* out: buffer contains a TSG bitmap.
	 * Bit #n will be set in the bitmap if TSG #n is present.
	 * When using NVGPU_SCHED_IOCTL_GET_RECENT_TSGS, the first time you use
	 * this command, it will return the opened TSGs and subsequent calls
	 * will only return the delta (ie. each invocation clears bitmap)
	 */
	__u64 buffer;
};

struct nvgpu_sched_get_tsgs_by_pid_args {
	/* in: process id for which we want to retrieve TSGs */
	__u64 pid;

	/* in: size of buffer in bytes */
	/* out: actual size of size of TSG bitmap. if user-provided size is too
	 * small, ioctl will return -ENOSPC, and update this field, allowing
	 * application to discover required number of bytes and allocate
	 * a buffer accordingly.
	 */
	__u32 size;

	/* in: address of 64-bit aligned buffer */
	/* out: buffer contains a TSG bitmap. */
	__u64 buffer;
};

struct nvgpu_sched_tsg_get_params_args {
	__u32 tsgid;		/* in: TSG identifier */
	__u32 timeslice;	/* out: timeslice in usecs */
	__u32 runlist_interleave;
	__u32 graphics_preempt_mode;
	__u32 compute_preempt_mode;
	__u64 pid;		/* out: process identifier of TSG owner */
};

struct nvgpu_sched_tsg_timeslice_args {
	__u32 tsgid;                    /* in: TSG identifier */
	__u32 timeslice;                /* in: timeslice in usecs */
};

struct nvgpu_sched_tsg_runlist_interleave_args {
	__u32 tsgid;			/* in: TSG identifier */

		/* in: see NVGPU_RUNLIST_INTERLEAVE_LEVEL_ */
	__u32 runlist_interleave;
};

struct nvgpu_sched_api_version_args {
	__u32 version;
};

struct nvgpu_sched_tsg_refcount_args {
	__u32 tsgid;                    /* in: TSG identifier */
};

#define NVGPU_SCHED_IOCTL_GET_TSGS					\
	_IOWR(NVGPU_SCHED_IOCTL_MAGIC, 1,				\
		struct nvgpu_sched_get_tsgs_args)
#define NVGPU_SCHED_IOCTL_GET_RECENT_TSGS				\
	_IOWR(NVGPU_SCHED_IOCTL_MAGIC, 2,				\
		struct nvgpu_sched_get_tsgs_args)
#define NVGPU_SCHED_IOCTL_GET_TSGS_BY_PID				\
	_IOWR(NVGPU_SCHED_IOCTL_MAGIC, 3,				\
		struct nvgpu_sched_get_tsgs_by_pid_args)
#define NVGPU_SCHED_IOCTL_TSG_GET_PARAMS				\
	_IOWR(NVGPU_SCHED_IOCTL_MAGIC, 4,				\
		struct nvgpu_sched_tsg_get_params_args)
#define NVGPU_SCHED_IOCTL_TSG_SET_TIMESLICE				\
	_IOW(NVGPU_SCHED_IOCTL_MAGIC, 5,				\
		struct nvgpu_sched_tsg_timeslice_args)
#define NVGPU_SCHED_IOCTL_TSG_SET_RUNLIST_INTERLEAVE			\
	_IOW(NVGPU_SCHED_IOCTL_MAGIC, 6,				\
		struct nvgpu_sched_tsg_runlist_interleave_args)
#define NVGPU_SCHED_IOCTL_LOCK_CONTROL					\
	_IO(NVGPU_SCHED_IOCTL_MAGIC, 7)
#define NVGPU_SCHED_IOCTL_UNLOCK_CONTROL				\
	_IO(NVGPU_SCHED_IOCTL_MAGIC, 8)
#define NVGPU_SCHED_IOCTL_GET_API_VERSION				\
	_IOR(NVGPU_SCHED_IOCTL_MAGIC, 9,				\
	    struct nvgpu_sched_api_version_args)
#define NVGPU_SCHED_IOCTL_GET_TSG					\
	_IOW(NVGPU_SCHED_IOCTL_MAGIC, 10,				\
		struct nvgpu_sched_tsg_refcount_args)
#define NVGPU_SCHED_IOCTL_PUT_TSG					\
	_IOW(NVGPU_SCHED_IOCTL_MAGIC, 11,				\
		struct nvgpu_sched_tsg_refcount_args)
#define NVGPU_SCHED_IOCTL_LAST						\
	_IOC_NR(NVGPU_SCHED_IOCTL_PUT_TSG)

#define NVGPU_SCHED_IOCTL_MAX_ARG_SIZE					\
	sizeof(struct nvgpu_sched_tsg_get_params_args)


#define NVGPU_SCHED_SET(n, bitmap)	\
	(((__u64 *)(bitmap))[(n) / 64] |=  (1ULL << (((__u64)n) & 63)))
#define NVGPU_SCHED_CLR(n, bitmap)	\
	(((__u64 *)(bitmap))[(n) / 64] &= ~(1ULL << (((__u64)n) & 63)))
#define NVGPU_SCHED_ISSET(n, bitmap)	\
	(((__u64 *)(bitmap))[(n) / 64] & (1ULL << (((__u64)n) & 63)))

#define NVGPU_SCHED_STATUS_TSG_OPEN	(1ULL << 0)

struct nvgpu_sched_event_arg {
	__u64 reserved;
	__u64 status;
};

#define NVGPU_SCHED_API_VERSION		1

#endif
