/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PERFBUF
#define NVGPU_PERFBUF

#ifdef CONFIG_NVGPU_DEBUGGER

/*
 * Size of the GPU VA window within which the PMA unit is allowed to
 * access.
 *
 * From PERFBUF_PMA_MEM_WINDOW_SIZE, PERFBUF_PMA_BUF_MAX_SIZE is reserved for
 * the pma buffers of all channels of the cblock, which is split equally across
 * each channel of the cblock.
 *
 * Remaining of PERFBUF_PMA_MEM_WINDOW_SIZE, i.e PERFBUF_PMA_AVAILABLE_BYTES_BUF_MAX_SIZE
 * is reserved for the pma_available_bytes_buffer of all channels of the cblock,
 * which split equally for each channel of the cblock.
 */
#define PERFBUF_PMA_MEM_WINDOW_SIZE			SZ_4G
#define PERFBUF_PMA_BUF_MAX_SIZE			0xFFE00000ULL
#define PERFBUF_PMA_AVAILABLE_BYTES_BUF_MAX_SIZE	(PERFBUF_PMA_MEM_WINDOW_SIZE - \
							PERFBUF_PMA_BUF_MAX_SIZE)
#define PMA_BYTES_AVAILABLE_BUFFER_SIZE			SZ_4K

#include <nvgpu/types.h>

struct gk20a;
struct perfbuf;

int nvgpu_perfbuf_enable_locked(struct gk20a *g, u32 pma_channel_id, u64 offset,
				u32 size);
int nvgpu_perfbuf_disable_locked(struct gk20a *g, u32 pma_channel_id);

int nvgpu_perfbuf_init_vm(struct gk20a *g, u32 pma_channel_id);
void nvgpu_perfbuf_deinit_vm(struct gk20a *g, u32 pma_channel_id);

int nvgpu_perfbuf_init_inst_block(struct gk20a *g, struct perfbuf *perfbuf,
				u32 pma_channel_id);
void nvgpu_perfbuf_deinit_inst_block(struct gk20a *g,
		struct nvgpu_mem *inst_block, u32 pma_channel_id);

int nvgpu_perfbuf_update_get_put(struct gk20a *g, u32 pma_channel_id,
				 u64 bytes_consumed, u64 *bytes_available,
				 void *cpuva, bool wait, u64 *put_ptr,
				 bool *overflowed);
struct perfbuf *nvgpu_perfbuf_get_perfbuf(struct gk20a *g, u32 pma_channel_id,
						u32 *perfbuf_channel_idx);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
