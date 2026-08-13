/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_USER_SYNC_H
#define NVGPU_USER_SYNC_H

#ifdef CONFIG_TEGRA_GK20A_NVHOST

#include <nvgpu/types.h>

struct nvgpu_channel;
struct nvgpu_channel_user_syncpt;

/**
 * @brief Create user syncpoint for a channel.
 *
 * @param c [in]		Pointer to Channel.
 *
 * Construct a #nvgpu_channel_user_syncpt that represents a syncpoint allocation
 * to be managed by userspace in conjunction with usermode submits.
 *
 * @return Pointer to #nvgpu_channel_user_syncpt in case of success, or NULL in
 * case of failure.
 */
struct nvgpu_channel_user_syncpt *
nvgpu_channel_user_syncpt_create(struct nvgpu_channel *ch);

/**
 * @brief Get user syncpoint id
 *
 * @param s [in]	User syncpoint pointer.
 *
 * @return Syncpoint id of \a s.
 */
u32 nvgpu_channel_user_syncpt_get_id(struct nvgpu_channel_user_syncpt *s);

/**
 * @brief Get user syncpoint address
 *
 * @param s [in]	User syncpoint pointer.
 *
 * This function returns syncpoint GPU VA. This address can be used in push
 * buffer entries for acquire/release operations.
 *
 * @return Syncpoint address (GPU VA) of syncpoint or 0 if not supported
 */
u64 nvgpu_channel_user_syncpt_get_address(struct nvgpu_channel_user_syncpt *s);

/**
 * @brief Set the user syncpoint to safe state
 *
 * @param s [in]	User syncpoint pointer.
 *
 * This should be used to reset user managed syncpoint since we don't track
 * threshold values for those syncpoints
 */
void nvgpu_channel_user_syncpt_set_safe_state(struct nvgpu_channel_user_syncpt *s);

/**
 * @brief Free user syncpoint
 *
 * @param s [in]	User syncpoint pointer.
 *
 * Free the resources allocated by #nvgpu_channel_user_syncpt_create.
 */
void nvgpu_channel_user_syncpt_destroy(struct nvgpu_channel_user_syncpt *s);

#endif /* CONFIG_TEGRA_GK20A_NVHOST */

#endif /* NVGPU_USER_SYNC_H */
