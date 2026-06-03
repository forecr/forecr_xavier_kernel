/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef ACR_BLOB_ALLOC_H
#define ACR_BLOB_ALLOC_H

struct gk20a;
struct nvgpu_mem;

int nvgpu_acr_alloc_blob_space_sys(struct gk20a *g, size_t size,
	struct nvgpu_mem *mem);
#ifdef CONFIG_NVGPU_DGPU
int nvgpu_acr_alloc_blob_space_vid(struct gk20a *g, size_t size,
	struct nvgpu_mem *mem);
#endif

#endif /* ACR_BLOB_ALLOC_H */
