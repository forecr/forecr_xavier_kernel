/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_COMMON_FIFO_CHANNEL_WDT_H
#define NVGPU_COMMON_FIFO_CHANNEL_WDT_H

#include <nvgpu/types.h>

struct nvgpu_channel;

#ifdef CONFIG_NVGPU_CHANNEL_WDT
struct nvgpu_worker;

void nvgpu_channel_launch_wdt(struct nvgpu_channel *ch);
void nvgpu_channel_worker_poll_init(struct nvgpu_worker *worker);
void nvgpu_channel_worker_poll_wakeup_post_process_item(
		struct nvgpu_worker *worker);
u32 nvgpu_channel_worker_poll_wakeup_condition_get_timeout(
		struct nvgpu_worker *worker);
#else
/**
 * @brief This function does nothing on safety build.
 */
static inline void nvgpu_channel_launch_wdt(struct nvgpu_channel *ch)
{
	(void)ch;
}
#endif /* CONFIG_NVGPU_CHANNEL_WDT */

#endif /* NVGPU_COMMON_FIFO_CHANNEL_WDT_H */
