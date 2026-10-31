// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <pthread.h>
#include <nvgpu/rwsem.h>
#include <nvgpu/log.h>
#include <nvgpu/bug.h>

void nvgpu_rwsem_init(struct nvgpu_rwsem *rwsem)
{
	int err = pthread_rwlock_init(&rwsem->rw_sem, NULL);
	nvgpu_assert(err == 0);
}

/*
 * Acquire.
 */
void nvgpu_rwsem_down_read(struct nvgpu_rwsem *rwsem)
{
	int err = pthread_rwlock_rdlock(&rwsem->rw_sem);
	nvgpu_assert(err == 0);
}

/*
 * Release.
 */
void nvgpu_rwsem_up_read(struct nvgpu_rwsem *rwsem)
{
	int err = pthread_rwlock_unlock(&rwsem->rw_sem);
	nvgpu_assert(err == 0);
}

void nvgpu_rwsem_down_write(struct nvgpu_rwsem *rwsem)
{
	int err = pthread_rwlock_wrlock(&rwsem->rw_sem);
	nvgpu_assert(err == 0);
}

void nvgpu_rwsem_up_write(struct nvgpu_rwsem *rwsem)
{
	int err = pthread_rwlock_unlock(&rwsem->rw_sem);
	nvgpu_assert(err == 0);
}
