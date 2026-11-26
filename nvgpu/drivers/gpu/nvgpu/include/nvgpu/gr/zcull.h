/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZCULL_H
#define NVGPU_GR_ZCULL_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;
struct nvgpu_gr_ctx;
struct nvgpu_tsg_subctx;
#ifdef CONFIG_NVGPU_ZCULL
struct nvgpu_gr_zcull;

struct nvgpu_gr_zcull_info {
	u32 width_align_pixels;
	u32 height_align_pixels;
	u32 pixel_squares_by_aliquots;
	u32 aliquot_total;
	u32 region_byte_multiplier;
	u32 region_header_size;
	u32 subregion_header_size;
	u32 subregion_width_align_pixels;
	u32 subregion_height_align_pixels;
	u32 subregion_count;
};

int nvgpu_gr_zcull_init(struct gk20a *g, struct nvgpu_gr_zcull **gr_zcull,
		u32 size, struct nvgpu_gr_config *gr_config);
void nvgpu_gr_zcull_deinit(struct gk20a *g, struct nvgpu_gr_zcull *gr_zcull);

u32 nvgpu_gr_get_ctxsw_zcull_size(struct gk20a *g,
				struct nvgpu_gr_zcull *gr_zcull);
int nvgpu_gr_zcull_init_hw(struct gk20a *g,
			struct nvgpu_gr_zcull *gr_zcull,
			struct nvgpu_gr_config *gr_config);

int nvgpu_gr_zcull_ctx_setup(struct gk20a *g, struct nvgpu_tsg_subctx *subctx,
		struct nvgpu_gr_ctx *gr_ctx);
#endif

#endif /* NVGPU_GR_ZCULL_H */
