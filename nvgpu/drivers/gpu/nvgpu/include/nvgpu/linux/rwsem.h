/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_RWSEM_LINUX_H__
#define __NVGPU_RWSEM_LINUX_H__

#include <linux/rwsem.h>

struct nvgpu_rwsem {
	struct rw_semaphore rwsem;
};

#endif
