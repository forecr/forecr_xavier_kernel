/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_POSIX_CHANNEL_H
#define NVGPU_POSIX_CHANNEL_H

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>

struct nvgpu_posix_channel {
	struct {
		u32 error;
		u32 status;
	} err_notifier;
};

#endif
