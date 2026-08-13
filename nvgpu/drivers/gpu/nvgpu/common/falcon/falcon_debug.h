/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FALCON_DEBUG_H
#define NVGPU_FALCON_DEBUG_H

struct nvgpu_falcon;

struct nvgpu_falcon_dbg_buf_metadata {
	/* Read offset updated by NVGPU */
	u32 read_offset;

	/* Write offset updated by firmware */
	u32 write_offset;

	/* Buffer size configured by NVGPU */
	u32 buffer_size;

	/* Magic number for header validation */
	u32 magic;
};

#endif /* NVGPU_FALCON_DEBUG_H */
