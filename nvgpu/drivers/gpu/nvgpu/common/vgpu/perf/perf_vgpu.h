/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PERF_VGPU_H
#define NVGPU_PERF_VGPU_H

struct gk20a;
struct nvgpu_mem;
struct perfbuf;

int vgpu_perfbuffer_enable(struct gk20a *g, u32 pma_channel_id, u64 offset,
			u32 size, struct nvgpu_tsg *tsg, bool is_ctx_resident);
int vgpu_perfbuffer_disable(struct gk20a *g, u32 pma_channel_id,
			struct nvgpu_tsg *tsg, bool is_ctx_resident);

int vgpu_perf_update_get_put(struct gk20a *g, u32 pma_channel_id, u64 bytes_consumed,
			bool update_available_bytes, u64 *put_ptr, bool *overflowed,
			struct nvgpu_tsg *tsg, bool is_ctx_resident,
			u32 *ctx_bytes_available);

#endif
