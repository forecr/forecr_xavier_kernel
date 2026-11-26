/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_KMEM_LINUX_H__
#define __NVGPU_KMEM_LINUX_H__

struct gk20a;
struct device;

#ifdef CONFIG_NVGPU_TRACK_MEM_USAGE
void *__nvgpu_track_vmalloc(struct gk20a *g, unsigned long size, void *ip);
void *__nvgpu_track_vzalloc(struct gk20a *g, unsigned long size, void *ip);
void *__nvgpu_track_kmalloc(struct gk20a *g, size_t size, void *ip);
void *__nvgpu_track_kzalloc(struct gk20a *g, size_t size, void *ip);
void *__nvgpu_track_kcalloc(struct gk20a *g, size_t n, size_t size, void *ip);
void  __nvgpu_track_vfree(struct gk20a *g, void *addr);
void  __nvgpu_track_kfree(struct gk20a *g, void *addr);
#endif

/**
 * DOC: Linux pass through kmem implementation.
 *
 * These are the Linux implementations of the various kmem functions defined by
 * nvgpu. This should not be included directly - instead include <nvgpu/kmem.h>.
 */
void *nvgpu_kmalloc_impl(struct gk20a *g, size_t size, void *ip);
void *nvgpu_kzalloc_impl(struct gk20a *g, size_t size, void *ip);
void *nvgpu_kcalloc_impl(struct gk20a *g, size_t n, size_t size, void *ip);
void *nvgpu_vmalloc_impl(struct gk20a *g, unsigned long size, void *ip);
void *nvgpu_vzalloc_impl(struct gk20a *g, unsigned long size, void *ip);
void nvgpu_kfree_impl(struct gk20a *g, void *addr);
void nvgpu_vfree_impl(struct gk20a *g, void *addr);

#endif
