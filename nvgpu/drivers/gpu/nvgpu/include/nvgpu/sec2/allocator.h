/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SEC2_ALLOCATOR_H
#define NVGPU_SEC2_ALLOCATOR_H

struct gk20a;
struct nvgpu_allocator;
struct sec2_init_msg_sec2_init;

int nvgpu_sec2_dmem_allocator_init(struct gk20a *g,
				struct nvgpu_allocator *dmem,
				struct sec2_init_msg_sec2_init *sec2_init);
void nvgpu_sec2_dmem_allocator_destroy(struct nvgpu_allocator *dmem);

#endif /* NVGPU_SEC2_ALLOCATOR_H */
