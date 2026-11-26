/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CHANNEL_SYNC_SYNCPT_H
#define NVGPU_CHANNEL_SYNC_SYNCPT_H

#include <nvgpu/types.h>
#include <nvgpu/utils.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/channel_sync.h>

struct nvgpu_channel;
struct nvgpu_channel_sync_syncpt;
struct priv_cmd_entry;

#ifdef CONFIG_NVGPU_NON_FUSA

#ifdef CONFIG_TEGRA_GK20A_NVHOST

/**
 * @brief Get syncpoint id
 *
 * @param s [in]	Syncpoint pointer.
 *
 * @return Syncpoint id of \a s.
 */
u32 nvgpu_channel_sync_get_syncpt_id(struct nvgpu_channel_sync_syncpt *s);

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
/*
 * Generate a gpu wait cmdbuf from raw fence(can be syncpoints or semaphores).
 * Returns a gpu cmdbuf that performs the wait when executed.
 */
int nvgpu_channel_sync_wait_syncpt(struct nvgpu_channel_sync_syncpt *s,
	u32 id, u32 thresh, struct priv_cmd_entry **entry);
#endif

/**
 * @brief Get syncpoint from sync operations
 *
 * @param sync [in]	Pointer to sync operations.
 *
 * Converts a valid struct nvgpu_channel_sync pointer \a sync to
 * struct nvgpu_channel_sync_syncpt pointer else return NULL
 *
 * @return Pointer to syncpoint, if sync is backed by a syncpoint.
 * @retval NULL if sync is backed by a sempahore.
 */
struct nvgpu_channel_sync_syncpt *
nvgpu_channel_sync_to_syncpt(struct nvgpu_channel_sync *sync);

/**
 * @brief Create syncpoint.
 *
 * @param c [in]		Pointer to channel.
 *
 * Constructs a struct nvgpu_channel_sync_syncpt.
 *
 * @return Pointer to nvgpu_channel_sync associated with created syncpoint.
 */
struct nvgpu_channel_sync *
nvgpu_channel_sync_syncpt_create(struct nvgpu_channel *c);

#else

static inline u32 nvgpu_channel_sync_get_syncpt_id(
	struct nvgpu_channel_sync_syncpt *s)
{
	return NVGPU_INVALID_SYNCPT_ID;
}
static inline u64 nvgpu_channel_sync_get_syncpt_address(
	struct nvgpu_channel_sync_syncpt *s)
{
	return 0ULL;
}

static inline int nvgpu_channel_sync_wait_syncpt(
	struct nvgpu_channel_sync_syncpt *s,
	u32 id, u32 thresh, struct priv_cmd_entry **entry)
{
	return -EINVAL;
}

static inline struct nvgpu_channel_sync_syncpt *
nvgpu_channel_sync_to_syncpt(struct nvgpu_channel_sync *sync)
{
	return NULL;
}

static inline struct nvgpu_channel_sync *
nvgpu_channel_sync_syncpt_create(struct nvgpu_channel *c)
{
	return NULL;
}

#endif

#endif /* CONFIG_NVGPU_NON_FUSA */

#endif /* NVGPU_CHANNEL_SYNC_SYNCPT_H */
