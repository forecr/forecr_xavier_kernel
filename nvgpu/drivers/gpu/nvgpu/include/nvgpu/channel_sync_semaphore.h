/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CHANNEL_SYNC_SEMAPHORE_H
#define NVGPU_CHANNEL_SYNC_SEMAPHORE_H

#include <nvgpu/types.h>
#include <nvgpu/channel_sync.h>
#include "../../common/sync/channel_sync_priv.h"

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT

struct nvgpu_channel;
struct nvgpu_channel_sync;

struct nvgpu_channel_sync_semaphore {
	struct nvgpu_channel_sync base;
	struct nvgpu_channel *c;
	struct nvgpu_hw_semaphore *hw_sema;
};

/*
 * Converts a valid struct nvgpu_channel_sync ptr to
 * struct nvgpu_channel_sync_semaphore ptr else return NULL.
 */
struct nvgpu_channel_sync_semaphore *
nvgpu_channel_sync_to_semaphore(struct nvgpu_channel_sync *sync);

/*
 * Returns the underlying hw semaphore.
 */
struct nvgpu_hw_semaphore *
nvgpu_channel_sync_semaphore_hw_sema(
		struct nvgpu_channel_sync_semaphore *sema);

/*
 * Constructs an instance of struct nvgpu_channel_sync_semaphore and
 * returns a pointer to the struct nvgpu_channel_sync associated with it.
 */
struct nvgpu_channel_sync *
nvgpu_channel_sync_semaphore_create(struct nvgpu_channel *c);
void nvgpu_channel_sync_hw_semaphore_init(struct nvgpu_channel_sync *sync);
void nvgpu_channel_update_gpfifo_get(struct nvgpu_channel *c);
s32 nvgpu_submit_create_gpfifo_tracking_semaphore(
		struct nvgpu_channel_sync *s,
		struct nvgpu_semaphore **semaphore,
		struct priv_cmd_entry **incr_cmd,
		u32 gpfifo_entries);


#endif

#endif /* NVGPU_CHANNEL_SYNC_SEMAPHORE_H */
