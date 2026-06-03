/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_BUS_H
#define NVGPU_GOPS_BUS_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * bus interface.
 * @page bus bus
 *
 * Overview
 * ========
 * The |nvgpu-rm| bus unit provides lot of utilities for s/w control of h/w.
 * The |nvgpu-rm| bus unit provides interfaces to enable and handle
 * interrupts for priv errors. Following host h/w errors are reported to upper
 * layer:
 * - GPU_HOST_PBUS_SQUASH_ERROR indicates priv transaction is killed since
 *  the target is in reset.
 * - GPU_HOST_PBUS_FECS_ERROR indicates fecs hit with a priv error.
 * - GPU_HOST_PBUS_TIMEOUT_ERROR indicates priv target failed to respond or
 *  any other priv timeout error.
 * The |nvgpu-rm| bus unit also provide interfaces for bar1 and bar2
 * binding, which are required for data sharing between h/w and s/w like
 * mmu fault buffers.
 *
 * Static Design
 * =============
 *
 * External APIs
 * -------------
 *   + gops_bus.init_hw
 *   + gops_bus.isr
 *   + gops_bus.bar2_bind
 *   + gops_bus.bar1_bind
 */
struct gk20a;

/**
 *
 * This structure stores common.bus unit hal pointers.
 *
 * @see gpu_ops
 */
struct gops_bus {
	/**
	 * @brief Initialize h/w access to gpu bus.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * This function enables bus unit stalling interrupt at MC level.
	 * This function enables interrupts for following errors:
	 * NV_PBUS_INTR_EN_0_PRI_SQUASH - Error due to request sent to
	 *				  while h/w block is in reset.
	 * NV_PBUS_INTR_EN_0_PRI_FECSERR - Error due to fecs returning
	 *				   error status from priv ring.
	 * NV_PBUS_INTR_EN_0_PRI_TIMEOUT - Error due to request timing out.
	 * This function also disables access to debug bus on safety build.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*init_hw)(struct gk20a *g);

	/**
	 * @brief ISR handler for gpu bus errors.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * This functions handles interrupts related to generic gpu bus errors.
	 * These interrupts are reported as host errors with following
	 * description:
	 * | Bus Error Type		| Host Error                   |
	 * |:-------------------------: | :------------------------:   |
	 * |SQUASH	 		| GPU_HOST_PBUS_SQUASH_ERROR   |
	 * |FECSERR	 		| GPU_HOST_PBUS_FECS_ERROR     |
	 * |TIMEOUT	 		| GPU_HOST_PBUS_TIMEOUT_ERROR  |
	 * |FB_ACK_TIMEOUT		| GPU_HOST_PBUS_TIMEOUT_ERROR  |
	 * |FB_ACK_EXTRA	 	| GPU_HOST_PBUS_TIMEOUT_ERROR  |
	 * |FB_RDATA_TIMEOUT		| GPU_HOST_PBUS_TIMEOUT_ERROR  |
	 * |FB_RDATA_EXTRA		| GPU_HOST_PBUS_TIMEOUT_ERROR  |
	 * |POSTED_DEADLOCK_TIMEOUT 	| GPU_HOST_PBUS_TIMEOUT_ERROR  |
	 * |ACCESS_TIMEOUT		| GPU_HOST_PBUS_TIMEOUT_ERROR  |
	 */
	void (*isr)(struct gk20a *g);

	/**
	 * @brief Binds instance block address space for BAR1 memory access.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param bar1_inst [in]	Instance memory to be used for
	 *				BAR1 access. Should not be NULL.
	 *
	 * BAR1 is generic memory to support sharing of data between h/w and
	 * s/w. As a part of bar1 binding, bar1 is mapped to the virtual memory
	 * space indicated by the instance block pointer and target memory
	 * space. After programming bar1 block register, s/w will wait until
	 * bar1 binding is done for maximum of 1000 retires with 5 usec delay
	 * before each retry.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 * @retval -EINVAL if h/w bar1 binding times out.
	 * @retval -EINVAL if s/w init of nvgpu timer fails.
	 */
	int (*bar1_bind)(struct gk20a *g, struct nvgpu_mem *bar1_inst);

	/**
	 * @brief Binds instance block address space for BAR2 memory access.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param bar2_inst [in]	Instance memory to be used for
	 *				BAR2 access. Should not be NULL.
	 *
	 * BAR2 is virtual memory set-up done by s/w using system memory for
	 * sharing of data between h/w and s/w, generally used for fault
	 * buffers. As a part of bar2 binding, bar2 is mapped to the virtual
	 * memory space indicated by the instance block pointer and target
	 * memory space. After programming bar2 block register, s/w will wait
	 * until bar2 binding is done for maximum of 1000 retires with 5 usec
	 * delay before each retry.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 * @retval -EINVAL if h/w bar2 binding times out.
	 * @retval -EINVAL if s/w init of nvgpu timer fails.
	 */
	int (*bar2_bind)(struct gk20a *g, struct nvgpu_mem *bar2_inst);

	/** @cond DOXYGEN_SHOULD_SKIP_THIS */
#ifdef CONFIG_NVGPU_DGPU
	u32 (*set_bar0_window)(struct gk20a *g, struct nvgpu_mem *mem,
			struct nvgpu_sgt *sgt, void *sgl, u32 w);
	u32 (*read_sw_scratch)(struct gk20a *g, u32 index);
	void (*write_sw_scratch)(struct gk20a *g, u32 index, u32 val);
#endif
	void (*configure_debug_bus)(struct gk20a *g);
	/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
};

#endif /* NVGPU_GOPS_BUS_H */

