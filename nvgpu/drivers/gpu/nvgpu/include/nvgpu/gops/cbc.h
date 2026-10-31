/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_CBC_H
#define NVGPU_GOPS_CBC_H

#ifdef CONFIG_NVGPU_COMPRESSION
struct gops_cbc {
	int (*cbc_init_support)(struct gk20a *g);
	void (*cbc_remove_support)(struct gk20a *g);
	void (*init)(struct gk20a *g, struct nvgpu_cbc *cbc, bool is_resume);
	int (*alloc_comptags)(struct gk20a *g,
				struct nvgpu_cbc *cbc);
	int (*ctrl)(struct gk20a *g, enum nvgpu_cbc_op op,
			u32 min, u32 max);
	u32 (*fix_config)(struct gk20a *g, u32 base);
	bool (*use_contig_pool)(struct gk20a *g);
};
#endif

#endif /* NVGPU_GOPS_CBC_H */
