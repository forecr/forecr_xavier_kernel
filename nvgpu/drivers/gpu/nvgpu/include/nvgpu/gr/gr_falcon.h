/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FALCON_H
#define NVGPU_GR_FALCON_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * common.gr.falcon unit interface
 */
struct gk20a;
struct nvgpu_gr_falcon;

#if defined(CONFIG_NVGPU_DEBUGGER) || defined(CONFIG_NVGPU_RECOVERY)
#define NVGPU_GR_FALCON_METHOD_CTXSW_STOP			0
#define NVGPU_GR_FALCON_METHOD_CTXSW_START			1
#endif

/** Falcon method to halt FE pipeline. */
#define NVGPU_GR_FALCON_METHOD_HALT_PIPELINE			2

#ifdef CONFIG_NVGPU_FECS_TRACE
#define NVGPU_GR_FALCON_METHOD_FECS_TRACE_FLUSH			3
#endif

/** Falcon method to query golden context image size. */
#define NVGPU_GR_FALCON_METHOD_CTXSW_DISCOVER_IMAGE_SIZE	4

#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
#define NVGPU_GR_FALCON_METHOD_CTXSW_DISCOVER_ZCULL_IMAGE_SIZE 	5
#endif

#if defined(CONFIG_NVGPU_DEBUGGER) || \
defined(CONFIG_NVGPU_CTXSW_FW_ERROR_CODE_TESTING)
#define NVGPU_GR_FALCON_METHOD_CTXSW_DISCOVER_PM_IMAGE_SIZE	6
#endif

#ifdef CONFIG_NVGPU_POWER_PG
#define NVGPU_GR_FALCON_METHOD_REGLIST_DISCOVER_IMAGE_SIZE	7
#define NVGPU_GR_FALCON_METHOD_REGLIST_BIND_INSTANCE		8
#define NVGPU_GR_FALCON_METHOD_REGLIST_SET_VIRTUAL_ADDRESS	9
#endif

/** Falcon method to bind the instance block. */
#define NVGPU_GR_FALCON_METHOD_ADDRESS_BIND_PTR			10
/** Falcon method to save golden context image. */
#define NVGPU_GR_FALCON_METHOD_GOLDEN_IMAGE_SAVE		11

#ifdef CONFIG_NVGPU_GRAPHICS
#define NVGPU_GR_FALCON_METHOD_PREEMPT_IMAGE_SIZE		12
#define NVGPU_GR_FALCON_METHOD_CONFIGURE_CTXSW_INTR		13
#endif

/** Falcon method to set watchdog timeout. */
#define NVGPU_GR_FALCON_METHOD_SET_WATCHDOG_TIMEOUT		14

#if defined(CONFIG_NVGPU_DEBUGGER) || defined(CONFIG_NVGPU_PROFILER)
#define NVGPU_GR_FALCON_METHOD_START_SMPC_GLOBAL_MODE		15
#define NVGPU_GR_FALCON_METHOD_STOP_SMPC_GLOBAL_MODE		16
#endif

#ifdef CONFIG_NVGPU_MIG
#define NVGPU_GR_FALCON_METHOD_SET_SMC_LTS_MASK_INDEX          17
#define NVGPU_GR_FALCON_METHOD_ASSIGN_SMC_LTS_MASK             18
#endif

#define NVGPU_GR_FALCON_METHOD_REGLIST_BIND_INSTANCE_HI         19
#define NVGPU_GR_FALCON_METHOD_REGLIST_SET_VIRTUAL_ADDRESS_HI   20
#define NVGPU_PRI_FECS_METHOD_PUSH_ADR_BUILD_PRI_ADDRESS_MAP    21

/** Sleep while waiting for Falcon ACK */
#define NVGPU_GR_FALCON_SUBMIT_METHOD_F_SLEEP		BIT32(0)

/** Falcon lock already held */
#define NVGPU_GR_FALCON_SUBMIT_METHOD_F_LOCKED		BIT32(1)

/** Falcon index of mailbox 0. */
#define NVGPU_GR_FALCON_FECS_CTXSW_MAILBOX0 0U
/** Falcon index of mailbox 1. */
#define NVGPU_GR_FALCON_FECS_CTXSW_MAILBOX1 1U
/** Falcon index of mailbox 2. */
#define NVGPU_GR_FALCON_FECS_CTXSW_MAILBOX2 2U
/** Falcon index of mailbox 4. */
#define NVGPU_GR_FALCON_FECS_CTXSW_MAILBOX4 4U
/** Falcon index of mailbox 6. */
#define NVGPU_GR_FALCON_FECS_CTXSW_MAILBOX6 6U
/** Falcon index of mailbox 7. */
#define NVGPU_GR_FALCON_FECS_CTXSW_MAILBOX7 7U

// 2-stage Boot-plugin mailbox signals
#define NV_UPROC_BOOT_PLUGIN_MAILBOX_STAGE1_COMPLETE    (0x1)
#define NV_UPROC_BOOT_PLUGIN_MAILBOX_FIRMWARE_LOADED    (0x2)
#define NV_UPROC_BOOT_PLUGIN_MAILBOX_TARGETMASK_LOWERED (0x3)

// pre determined maximum size of second stage boot-plugin
#define NV_UPROC_BOOT_PLUGIN_STAGE_TWO_BYTE_SIZE_LIMIT (512)

/**
 * Description of one ucode segment.
 */
struct nvgpu_ctxsw_ucode_segment {
	/** Offset of segment in the ucode. */
	u32 offset;
	/** Size of segment in the ucode. */
	u32 size;
};

/**
 * Description of ucode layout and boot/code/data segments of ucode.
 */
struct nvgpu_ctxsw_ucode_segments {
	/** Falcon boot vector. */
	u32 boot_entry;
	/** IMEM offset. */
	u32 boot_imem_offset;
	/** Checksum of boot image. */
	u32 boot_signature;
	/** Boot segment of ucode. */
	struct nvgpu_ctxsw_ucode_segment boot;
	/** Code segment of ucode. */
	struct nvgpu_ctxsw_ucode_segment code;
	/** Data segment of ucode. */
	struct nvgpu_ctxsw_ucode_segment data;
};

/**
 * Details needed to handle FECS interrupts.
 */
struct nvgpu_fecs_host_intr_status {
	/**
	 * Write this value to clear HOST_INT0 context switch error interrupt.
	 */
	u32 ctxsw_intr0;
	/**
	 * Write this value to clear HOST_INT1 context save completion
	 * interrupt.
	 */
	u32 ctxsw_intr1;
	/** This value is used for "warnings" during ctxsw transaction. */
	u32 ctxsw_intr2;
	/** This flag is set for fault raised during ctxsw transaction. */
	bool fault_during_ctxsw_active;
	/** This flag is set for unhandled firmware method. */
	bool unimp_fw_method_active;
	/** This flag is set if falcon watchdog expires. */
	bool watchdog_active;
	/**
	 * This flag is set when a parity error is
	 * detected when FECS receives a poison bit.
	 */
	bool poisoned_read;
	/**
	 * This flag is set when a context load is
	 * requested after a halt.
	 */
	bool ctx_load_after_halt;
};

/**
 * Book keeping for ECC errors originating from FECS.
 */
struct nvgpu_fecs_ecc_status {
	/** This flag is set if IMEM corrected error is hit. */
	bool imem_corrected_err;
	/** This flag is set if IMEM uncorrected error is hit. */
	bool imem_uncorrected_err;
	/** This flag is set if DMEM corrected error is hit. */
	bool dmem_corrected_err;
	/** This flag is set if DMEM uncorrected error is hit. */
	bool dmem_uncorrected_err;
	/** Address of memory where ECC error occurred. */
	u32  ecc_addr;
	/** Number of corrected ECC errors. */
	u32  corrected_delta;
	/** Number of uncorrected ECC errors. */
	u32  uncorrected_delta;
};

/**
 * @brief Initialize GR falcon structure.
 *
 * @param g [in]		Pointer to GPU driver struct.
 *
 * This function allocates memory for #nvgpu_gr_falcon structure and
 * initializes all mutexes in this structure.
 *
 * @return pointer to #nvgpu_gr_falcon struct in case of success,
 *         NULL in case of failure.
 */
struct nvgpu_gr_falcon *nvgpu_gr_falcon_init_support(struct gk20a *g);

/**
 * @brief Free GR falcon structure.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param falcon [in]		Pointer to GR falcon struct.
 *
 * This function will free memory allocated for #nvgpu_gr_falcon
 * structure.
 */
void nvgpu_gr_falcon_remove_support(struct gk20a *g,
		struct nvgpu_gr_falcon *falcon);

/**
 * @brief Load and boot CTXSW ucodes.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param falcon [in]		Pointer to GR falcon struct.
 *
 * This function will load FECS and GPCCS ucodes and bootstrap them
 * on falcon microcontrollers. This function will also make sure that
 * falcon microcontrollers are ready for further processing by waiting
 * on correct mailbox status.
 *
 * In case of secure boot, this function will internally call
 * #nvgpu_gr_falcon_load_secure_ctxsw_ucode() to boot ucodes.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ENOENT if ucodes cannot be found on filesystem.
 * @retval -ENOMEM if ucode surface memory allocation fails.
 * @retval -ETIMEDOUT if communication with falcon timed out.
 * @retval -ETIMEDOUT if method opcode submission returns
 *         ucode wait status of \a WAIT_UCODE_TIMEOUT.
 * @retval -EINVAL if method opcode submission returns
 *         ucode wait status of \a WAIT_UCODE_ERROR.
 */
int nvgpu_gr_falcon_init_ctxsw(struct gk20a *g, struct nvgpu_gr_falcon *falcon);

/**
 * @brief Initialize context state.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param falcon [in]		Pointer to GR falcon struct.
 *
 * This function will query golden context image size from FECS
 * microcontroller.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ETIMEDOUT if communication with falcon timed out.
 * @retval -ETIMEDOUT if method opcode submission returns
 *         ucode wait status of \a WAIT_UCODE_TIMEOUT.
 * @retval -EINVAL if method opcode submission returns
 *         ucode wait status of \a WAIT_UCODE_ERROR.
 */
int nvgpu_gr_falcon_init_ctx_state(struct gk20a *g,
		struct nvgpu_gr_falcon *falcon);

/**
 * @brief Initialize CTXSW ucodes.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param falcon [in]		Pointer to GR falcon struct.
 *
 * This function will read FECS and GPCCS ucodes from filesystem
 * and fill in details of boot, code, and data segments in
 * #nvgpu_ctxsw_ucode_segments struct.
 *
 * Both FECS and GPCCS ucode contents are copied in a ucode
 * surface memory buffer for local reference.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ENOMEM if context memory allocation fails.
 * @retval -ENOENT if ucodes cannot be found on filesystem.
 */
int nvgpu_gr_falcon_init_ctxsw_ucode(struct gk20a *g,
		struct nvgpu_gr_falcon *falcon);

/**
 * @brief Load and boot CTXSW ucodes in a secure method.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param falcon [in]		Pointer to GR falcon struct.
 *
 * This function will load FECS and GPCCS ucodes and bootstrap them
 * on falcon microcontrollers in a secure method. This function will
 * also make sure that falcon microcontrollers are ready for further
 * processing by waiting on correct mailbox status.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_gr_falcon_load_secure_ctxsw_ucode(struct gk20a *g,
		struct nvgpu_gr_falcon *falcon);

/**
 * @brief Get FECS ucode segments pointer.
 *
 * @param falcon [in]		Pointer to GR falcon struct.
 *
 * This function will return FECS ucode segment data structure pointer.
 * All the details of boot/code/data segments are stored in
 * #nvgpu_ctxsw_ucode_segments structure.
 *
 * @return Pointer to #nvgpu_ctxsw_ucode_segments struct.
 */
struct nvgpu_ctxsw_ucode_segments *nvgpu_gr_falcon_get_fecs_ucode_segments(
		struct nvgpu_gr_falcon *falcon);

/**
 * @brief Get GPCCS ucode segments pointer.
 *
 * @param falcon [in]		Pointer to GR falcon struct.
 *
 * This function will return GPCCS ucode segment data structure pointer.
 * All the details of boot/code/data segments are stored in
 * #nvgpu_ctxsw_ucode_segments structure.
 *
 * @return Pointer to #nvgpu_ctxsw_ucode_segments struct.
 */
struct nvgpu_ctxsw_ucode_segments *nvgpu_gr_falcon_get_gpccs_ucode_segments(
		struct nvgpu_gr_falcon *falcon);

/**
 * @brief Get CPU virtual address of ucode surface.
 *
 * @param falcon [in]		Pointer to GR falcon struct.
 *
 * This function returns CPU virtual address of ucode surface memory
 * buffer. This buffer is created while reading FECS and GPCCS ucodes
 * from filesystem. See #nvgpu_gr_falcon_init_ctxsw_ucode().
 *
 * @return CPU virtual address of ucode surface.
 */
void *nvgpu_gr_falcon_get_surface_desc_cpu_va(
		struct nvgpu_gr_falcon *falcon);

/**
 * @brief Get size of golden context image.
 *
 * @param falcon [in]		Pointer to GR falcon struct.
 *
 * This function returns size of golden context image read from FECS
 * microcontroller in #nvgpu_gr_falcon_init_ctx_state().
 *
 * @return Size of golden conext image.
 */
u32 nvgpu_gr_falcon_get_golden_image_size(struct nvgpu_gr_falcon *falcon);

#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
int nvgpu_gr_falcon_load_ctxsw_ucode(struct gk20a *g,
					struct nvgpu_gr_falcon *falcon);
#endif
#ifdef CONFIG_NVGPU_POWER_PG
int nvgpu_gr_falcon_bind_fecs_elpg(struct gk20a *g, u32 buf_id);
#endif
#ifdef CONFIG_NVGPU_ENGINE_RESET
struct nvgpu_mutex *nvgpu_gr_falcon_get_fecs_mutex(
		struct nvgpu_gr_falcon *falcon);
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
u32 nvgpu_gr_falcon_get_pm_ctxsw_image_size(struct nvgpu_gr_falcon *falcon);
#endif
#ifdef CONFIG_NVGPU_GFXP
u32 nvgpu_gr_falcon_get_preempt_image_size(struct nvgpu_gr_falcon *falcon);
#endif
#ifdef CONFIG_NVGPU_GRAPHICS
u32 nvgpu_gr_falcon_get_zcull_image_size(struct nvgpu_gr_falcon *falcon);
#endif
void nvgpu_gr_falcon_suspend(struct gk20a *g, struct nvgpu_gr_falcon *falcon);

#endif /* NVGPU_GR_FALCON_H */
