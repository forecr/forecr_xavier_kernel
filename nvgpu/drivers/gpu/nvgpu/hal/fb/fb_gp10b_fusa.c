// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/sizes.h>
#include <nvgpu/gk20a.h>

#include "fb_gp10b.h"

#ifdef CONFIG_NVGPU_COMPRESSION
u64 gp10b_fb_compression_page_size(struct gk20a *g)
{
	(void)g;
	return SZ_64K;
}

unsigned int gp10b_fb_compressible_page_size(struct gk20a *g)
{
	(void)g;
	return (unsigned int)SZ_4K;
}
#endif
