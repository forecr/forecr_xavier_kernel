/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CE_APP_H
#define NVGPU_CE_APP_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_fence_type;

#define NVGPU_CE_INVAL_CTX_ID	~U32(0U)

/* CE command utility macros */
/* CE Supports 57-bit addresses in Blackwell */
#define NVGPU_CE_LOWER_ADDRESS_OFFSET_MASK U32_MAX
#define NVGPU_CE_UPPER_ADDRESS_OFFSET_MASK 0x1ffffffU
#define NVGPU_CE_MAX_ADDRESS \
	((U64(NVGPU_CE_UPPER_ADDRESS_OFFSET_MASK) << 32U) | \
	 U64(NVGPU_CE_LOWER_ADDRESS_OFFSET_MASK))
/*
 * Maximum copy size supported by CE operations
 * Size should be multiple of 4KB, so the the maximum it can support is 4GB-4KB.
 * This limit applies to all chips up to Blackwell.
 */
#define NVGPU_CE_MAX_COPY_SIZE	(0xFFFFF000U)

#define NVGPU_CE_MAX_INFLIGHT_JOBS 32U

/*
 * A copyengine job for any buffer size needs at most:
 *
 * - two u32 words for class header
 * - two operations, both either 16 words (transfer) or 15 words (memset)
 *
 * The size does not need to be exact, so this uses the upper bound:
 * 2 + 2 * 16 = 34 words, or 136 bytes.
 */
#define NVGPU_CE_MAX_COMMAND_BUFF_BYTES_PER_SUBMIT \
	((2U + 2U * 16U) * sizeof(u32))

/* CE method address and values */
#define NVC86F_DMA_SEC_OP_INC_METHOD (0x1 << 29)
#define NVC86F_DMA_METHOD_ADDRESS(x) ((x) >> 2)
#define NVC86F_DMA_METHOD_COUNT(x)	((x) << 16)
#define NV_PUSH_INC_METHOD(method, count)	\
	(NVC86F_DMA_SEC_OP_INC_METHOD |		\
	 NVC86F_DMA_METHOD_ADDRESS(method) |    \
	 NVC86F_DMA_METHOD_COUNT(count))
#define NVC86F_SET_OBJECT	0x00000000

#define NVC8B5_SET_REMAP_CONST_A	0x00000700
#define NVC8B5_SET_REMAP_CONST_B	0x00000704
#define NVC8B5_SET_SRC_PHYS_MODE        0x00000260
#define NVC8B5_SET_DST_PHYS_MODE        0x00000264
#define NVC8B5_SET_DST_PHYS_MODE_TARGET_NONCOHERENT_SYSMEM (0x2U << 0)

#define NVC8B5_OFFSET_OUT_UPPER         0x00000408
#define NVC8B5_OFFSET_OUT_LOWER         0x0000040C
#define NVC8B5_LINE_LENGTH_IN           0x00000418
#define NVC8B5_LAUNCH_DMA               0x00000300
#define NVC8B5_LAUNCH_DMA_DATA_TRANSFER_TYPE_PIPELINED	(0x1U << 0)
#define NVC8B5_LAUNCH_DMA_FLUSH_ENABLE_TRUE		(0x1U << 2)
#define NVC8B5_LAUNCH_DMA_SRC_MEMORY_LAYOUT_PITCH	(0x1U << 7)
#define NVC8B5_LAUNCH_DMA_DST_MEMORY_LAYOUT_PITCH	(0x1U << 8)
#define NVC8B5_LAUNCH_DMA_SRC_TYPE_PHYSICAL		(0x1U << 12)
#define NVC8B5_LAUNCH_DMA_DST_TYPE_PHYSICAL		(0x1U << 13)
#define NVC8B5_LAUNCH_DMA_MEMORY_SCRUB_ENABLE_TRUE	(0x1U << 23)
#define NVC8B5_LAUNCH_DMA_DISABLE_PLC_TRUE		(0x1U << 26)

/* CE app state machine flags */
enum {
	NVGPU_CE_ACTIVE                    = (1 << 0),
	NVGPU_CE_SUSPEND                   = (1 << 1),
};

/* gpu context state machine flags */
enum {
	NVGPU_CE_GPU_CTX_ALLOCATED         = (1 << 0),
	NVGPU_CE_GPU_CTX_DELETED           = (1 << 1),
};

/* global CE app related apis */
int nvgpu_ce_app_init_support(struct gk20a *g);
void nvgpu_ce_app_suspend(struct gk20a *g);
void nvgpu_ce_app_destroy(struct gk20a *g);

/* CE app utility functions */
u32 nvgpu_ce_app_create_context(struct gk20a *g,
		u32 runlist_id,
		int timeslice,
		int runlist_level);
void nvgpu_ce_app_delete_context(struct gk20a *g,
		u32 ce_ctx_id);
int nvgpu_ce_execute_ops(struct gk20a *g,
		u32 ce_ctx_id,
		u64 dst_paddr,
		u64 size,
		u32 payload,
		struct nvgpu_fence_type **fence_out);
#endif /*NVGPU_CE_APP_H*/
