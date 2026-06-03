/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */


#ifndef NVGPU_USER_SYNCPT_PRIV_H
#define NVGPU_USER_SYNCPT_PRIV_H

#include <nvgpu/types.h>
#include <nvgpu/nvgpu_mem.h>

struct nvgpu_channel;
struct nvgpu_nvhost_dev;

struct nvgpu_channel_user_syncpt {
	struct nvgpu_channel *ch;
	struct nvgpu_nvhost_dev *nvhost;
	uint32_t syncpt_id;
	struct nvgpu_mem syncpt_buf;
};

#define SYNCPT_NAME_SZ 32UL

#endif /* NVGPU_USER_SYNC_PRIV_H */
