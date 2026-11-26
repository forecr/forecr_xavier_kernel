/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SEMAPHORE_H
#define NVGPU_SEMAPHORE_H

#include <nvgpu/log.h>
#include <nvgpu/atomic.h>
#include <nvgpu/kref.h>
#include <nvgpu/list.h>
#include <nvgpu/nvgpu_mem.h>

struct gk20a;
struct nvgpu_semaphore_pool;
struct nvgpu_hw_semaphore;
struct nvgpu_semaphore;
struct vm_gk20a;
struct nvgpu_allocator;
struct nvgpu_channel;

#define gpu_sema_dbg(g, fmt, args...)		\
	nvgpu_log(g, gpu_dbg_sema, fmt, ##args)
#define gpu_sema_verbose_dbg(g, fmt, args...)	\
	nvgpu_log(g, gpu_dbg_sema_v, fmt, ##args)

/*
 * Semaphore sea functions.
 */
struct nvgpu_semaphore_sea *nvgpu_semaphore_sea_create(struct gk20a *g);
void nvgpu_semaphore_sea_lock(struct nvgpu_semaphore_sea *s);
void nvgpu_semaphore_sea_unlock(struct nvgpu_semaphore_sea *s);
struct nvgpu_semaphore_sea *nvgpu_semaphore_get_sea(struct gk20a *g);
void nvgpu_semaphore_sea_destroy(struct gk20a *g);
void nvgpu_semaphore_sea_allocate_gpu_va(struct nvgpu_semaphore_sea *s,
	struct nvgpu_allocator *a, u64 base, u64 len, u32 page_size);
u64 nvgpu_semaphore_sea_get_gpu_va(struct nvgpu_semaphore_sea *s);

/*
 * Semaphore pool functions.
 */
int nvgpu_semaphore_pool_alloc(struct nvgpu_semaphore_sea *sea,
			       struct nvgpu_semaphore_pool **pool);
int nvgpu_semaphore_pool_map(struct nvgpu_semaphore_pool *p,
			     struct vm_gk20a *vm);
void nvgpu_semaphore_pool_unmap(struct nvgpu_semaphore_pool *p,
				struct vm_gk20a *vm);
u64 nvgpu_semaphore_pool_gpu_va(struct nvgpu_semaphore_pool *p, bool global);
void nvgpu_semaphore_pool_get(struct nvgpu_semaphore_pool *p);
void nvgpu_semaphore_pool_put(struct nvgpu_semaphore_pool *p);
u64 nvgpu_semaphore_pool_get_page_idx(struct nvgpu_semaphore_pool *p);

/*
 * Hw semaphore functions
 */
int nvgpu_hw_semaphore_init(struct vm_gk20a *vm, u32 chid,
		struct nvgpu_hw_semaphore **new_sema);
void nvgpu_hw_semaphore_free(struct nvgpu_hw_semaphore *hw_sema);
u64 nvgpu_hw_semaphore_addr(struct nvgpu_hw_semaphore *hw_sema);
u32 nvgpu_hw_semaphore_read(struct nvgpu_hw_semaphore *hw_sema);
bool nvgpu_hw_semaphore_reset(struct nvgpu_hw_semaphore *hw_sema);
void nvgpu_hw_semaphore_set(struct nvgpu_hw_semaphore *hw_sema, u32 val);
void nvgpu_hw_semaphore_init_next(struct nvgpu_hw_semaphore *hw_sema);
int nvgpu_hw_semaphore_read_next(struct nvgpu_hw_semaphore *hw_sema);
int nvgpu_hw_semaphore_update_next(struct nvgpu_hw_semaphore *hw_sema);

/*
 * Semaphore functions.
 */
struct nvgpu_semaphore *nvgpu_semaphore_alloc(
		struct nvgpu_hw_semaphore *hw_sema);
void nvgpu_semaphore_put(struct nvgpu_semaphore *s);
void nvgpu_semaphore_get(struct nvgpu_semaphore *s);

u64 nvgpu_semaphore_gpu_rw_va(struct nvgpu_semaphore *s);
u64 nvgpu_semaphore_gpu_ro_va(struct nvgpu_semaphore *s);

u32 nvgpu_semaphore_read(struct nvgpu_semaphore *s);
u32 nvgpu_semaphore_get_value(struct nvgpu_semaphore *s);
bool nvgpu_semaphore_is_released(struct nvgpu_semaphore *s);
bool nvgpu_semaphore_is_acquired(struct nvgpu_semaphore *s);
bool nvgpu_semaphore_can_wait(struct nvgpu_semaphore *s);

void nvgpu_semaphore_prepare(struct nvgpu_semaphore *s,
		struct nvgpu_hw_semaphore *hw_sema);
void nvgpu_semaphore_prepare_for_gpfifo_get(struct nvgpu_channel *c,
		struct nvgpu_semaphore *s,
		struct nvgpu_hw_semaphore *hw_sema, u32 new_entries);
u64 nvgpu_semaphore_get_hw_pool_page_idx(struct nvgpu_semaphore *s);

#endif /* NVGPU_SEMAPHORE_H */
