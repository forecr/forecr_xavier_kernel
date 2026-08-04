/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_VM_H
#define NVGPU_POSIX_VM_H

#include <nvgpu/types.h>

struct nvgpu_os_buffer {
	/*
	 * We just use malloc() buffers in userspace.
	 */
	void *buf;
	size_t size;
};

struct nvgpu_mapped_buf_priv {
	void *buf;
	size_t size;
};

#endif /* NVGPU_POSIX_VM_H */
