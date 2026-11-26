/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_VIDMEM_H
#define NVGPU_POSIX_VIDMEM_H

#ifdef CONFIG_NVGPU_DGPU

void nvgpu_vidmem_set_page_alloc(struct nvgpu_mem_sgl *sgl, u64 addr);
struct nvgpu_page_alloc *nvgpu_vidmem_get_page_alloc(struct nvgpu_mem_sgl *sgl);

#endif
#endif /* NVGPU_POSIX_VIDMEM_H */
