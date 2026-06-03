/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_RWSEM_H
#define NVGPU_POSIX_RWSEM_H

#include <pthread.h>

struct nvgpu_rwsem {
	/**
	 * pthread_rwlock_t used internally to implement nvgpu rwsem.
	 */
	pthread_rwlock_t rw_sem;
};

#endif /* NVGPU_POSIX_RWSEM_H */
