/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_SYNC_SYNCPT_CMDBUF_GK20A_H
#define NVGPU_SYNC_SYNCPT_CMDBUF_GK20A_H

#include <nvgpu/types.h>

struct gk20a;
struct priv_cmd_entry;
struct nvgpu_mem;
struct nvgpu_channel;

#ifdef CONFIG_TEGRA_GK20A_NVHOST

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
void gk20a_syncpt_add_wait_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		u32 id, u32 thresh, u64 gpu_va_base);
u32 gk20a_syncpt_get_wait_cmd_size(void);
u32 gk20a_syncpt_get_incr_per_release(void);
void gk20a_syncpt_add_incr_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		u32 id, u64 gpu_va, bool wfi);
u32 gk20a_syncpt_get_incr_cmd_size(bool wfi_cmd);
#endif

void gk20a_syncpt_free_buf(struct nvgpu_channel *c,
		struct nvgpu_mem *syncpt_buf);

int gk20a_syncpt_alloc_buf(struct nvgpu_channel *c,
		u32 syncpt_id, struct nvgpu_mem *syncpt_buf);

#else

static inline void gk20a_syncpt_free_buf(struct nvgpu_channel *c,
		struct nvgpu_mem *syncpt_buf)
{
}

static inline int gk20a_syncpt_alloc_buf(struct nvgpu_channel *c,
		u32 syncpt_id, struct nvgpu_mem *syncpt_buf)
{
	return -ENOSYS;
}

#endif /* CONFIG_TEGRA_GK20A_NVHOST */

#endif /* NVGPU_SYNC_SYNCPT_CMDBUF_GK20A_H */
