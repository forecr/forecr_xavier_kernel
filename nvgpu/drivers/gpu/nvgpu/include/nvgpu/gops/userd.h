/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_USERD_H
#define NVGPU_GOPS_USERD_H

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;

struct gops_userd {
	int (*setup_sw)(struct gk20a *g);
	void (*cleanup_sw)(struct gk20a *g);
	void (*init_mem)(struct gk20a *g, struct nvgpu_channel *c);
	u32 (*entry_size)(struct gk20a *g);

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	u32 (*gp_get)(struct gk20a *g, struct nvgpu_channel *c);
	void (*gp_put)(struct gk20a *g, struct nvgpu_channel *c);
	u64 (*pb_get)(struct gk20a *g, struct nvgpu_channel *c);
#endif

};
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

#endif
