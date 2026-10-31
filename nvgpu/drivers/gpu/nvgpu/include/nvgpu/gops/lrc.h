/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GOPS_LRC_H
#define NVGPU_GOPS_LRC_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * common.lrc interface.
 */
struct gk20a;

/**
 * common.lrc unit hal operations.
 *
 * @see gpu_ops
 */
struct gops_lrc {
#ifdef CONFIG_NVGPU_DEBUGGER
	bool (*pri_is_lrcc_addr)(struct gk20a *g, u32 addr);
	bool (*pri_is_lrcs_addr)(struct gk20a *g, u32 addr);
	bool (*pri_is_lrcs_lrccs_addr)(struct gk20a *g, u32 addr);
	void (*split_lrcc_broadcast_addr)(struct gk20a *g, u32 addr,
						u32 *priv_addr_table,
						u32 *priv_addr_table_index);
	void (*split_lrc_broadcast_addr)(struct gk20a *g, u32 addr,
						u32 *priv_addr_table,
						u32 *priv_addr_table_index);
#endif
	int (*init_hw)(struct gk20a *g);
	bool is_lrc_supported;
};

#endif /* NVGPU_GOPS_LRC_H */
