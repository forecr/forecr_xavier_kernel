/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef RAMIN_GM20B_H
#define RAMIN_GM20B_H

struct gk20a;

void gm20b_ramin_set_big_page_size(struct gk20a *g,
		struct nvgpu_mem *mem, u32 size);

#endif /* RAMIN_GM20B_H */
