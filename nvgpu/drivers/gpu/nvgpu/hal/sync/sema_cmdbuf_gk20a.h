/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_SYNC_SEMA_CMDBUF_GK20A_H
#define NVGPU_SYNC_SEMA_CMDBUF_GK20A_H

#if defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT) && \
	defined(CONFIG_NVGPU_SW_SEMAPHORE)

#include <nvgpu/types.h>

struct gk20a;
struct priv_cmd_entry;
struct nvgpu_semaphore;

u32 gk20a_sema_get_wait_cmd_size(void);
u32 gk20a_sema_get_incr_cmd_size(void);
void gk20a_sema_add_wait_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		struct nvgpu_semaphore *s, u64 sema_va);
void gk20a_sema_add_incr_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		struct nvgpu_semaphore *s, u64 sema_va,
		bool wfi);

#endif /* CONFIG_NVGPU_KERNEL_MODE_SUBMIT && CONFIG_NVGPU_SW_SEMAPHORE */

#endif /* NVGPU_SYNC_SEMA_CMDBUF_GK20A_H */
