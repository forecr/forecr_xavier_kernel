/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_FLOORSWEEP_H
#define NVGPU_GOPS_FLOORSWEEP_H

#ifdef CONFIG_NVGPU_STATIC_POWERGATE
struct gops_tpc_pg {
	int (*init_tpc_pg)(struct gk20a *g, bool *can_tpc_pg);
	void (*tpc_pg)(struct gk20a *g);
};

struct gops_fbp_pg {
	int (*init_fbp_pg)(struct gk20a *g, bool *can_fbp_pg);
	void (*fbp_pg)(struct gk20a *g);
};

struct gops_gpc_pg {
	int (*init_gpc_pg)(struct gk20a *g, bool *can_gpc_pg);
	void (*gpc_pg)(struct gk20a *g);
};

#endif

#endif /* NVGPU_GOPS_FLOORSWEEP_H */
