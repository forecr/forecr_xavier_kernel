/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RAMFC_VGPU_H
#define NVGPU_RAMFC_VGPU_H

#include <nvgpu/types.h>

struct nvgpu_channel;

int vgpu_ramfc_setup(struct nvgpu_channel *ch, u64 gpfifo_base,
		u32 gpfifo_entries, u64 pbdma_acquire_timeout, u32 flags);

#endif /* NVGPU_RAMFC_VGPU_H */
