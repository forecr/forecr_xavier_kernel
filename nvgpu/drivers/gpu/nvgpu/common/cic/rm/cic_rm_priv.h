/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CIC_RM_PRIV_H
#define CIC_RM_PRIV_H

#include <nvgpu/types.h>
#include <nvgpu/cond.h>
#include <nvgpu/atomic.h>

struct nvgpu_cic_rm {
	/**
	 * One of the condition variables needed to keep track of deferred
	 * interrupts.
	 * The condition variable that is signalled upon handling of the
	 * stalling interrupt. Function #nvgpu_cic_wait_for_stall_interrupts
	 * waits on this condition variable.
	 */
	struct nvgpu_cond sw_irq_stall_last_handled_cond;

	/**
	 * One of the counters needed to keep track of deferred interrupts.
	 * Stalling interrupt status counter - Set to 1 on entering stalling
	 * interrupt handler and reset to 0 on exit.
	 */
	nvgpu_atomic_t sw_irq_stall_pending;

#ifdef CONFIG_NVGPU_NONSTALL_INTR
	/**
	 * One of the condition variables needed to keep track of deferred
	 * interrupts.
	 * The condition variable that is signalled upon handling of the
	 * non-stalling interrupt. Function #nvgpu_cic_wait_for_nonstall_interrupts
	 * waits on this condition variable.
	 */
	struct nvgpu_cond sw_irq_nonstall_last_handled_cond;

	/**
	 * One of the counters needed to keep track of deferred interrupts.
	 * Non-stalling interrupt status counter - Set to 1 on entering
	 * non-stalling interrupt handler and reset to 0 on exit.
	 */
	nvgpu_atomic_t sw_irq_nonstall_pending;
#endif
};

#endif /* CIC_RM_PRIV_H */
