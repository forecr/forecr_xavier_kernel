/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CYCLESTATS_PRIV_H
#define NVGPU_CYCLESTATS_PRIV_H

#include <nvgpu/types.h>

#define MULTICHAR_TAG(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

enum BAR0_DEBUG_OPERATION {
	BARO_ZERO_NOP = 0,
	OP_END = MULTICHAR_TAG('D', 'O', 'N', 'E'),
	BAR0_READ32 = MULTICHAR_TAG('0', 'R', '3', '2'),
	BAR0_WRITE32 = MULTICHAR_TAG('0', 'W', '3', '2'),
};

struct share_buffer_head {
	enum BAR0_DEBUG_OPERATION operation;
/* size of the operation item */
	u32 size;
	u32 completed;
	u32 failed;
	u64 context;
	u64 completion_callback;
};

struct nvgpu_cyclestate_buffer_elem {
	struct share_buffer_head	head;
/* in */
	u64 p_data;
	u64 p_done;
	u32 offset_bar0;
	u16 first_bit;
	u16 last_bit;
/* out */
/* keep 64 bits to be consistent */
	u64 data;
};

#endif /* NVGPU_CYCLESTATS_PRIV_H */
