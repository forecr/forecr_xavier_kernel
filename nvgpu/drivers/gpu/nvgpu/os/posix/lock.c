// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/log.h>
#include <nvgpu/lock.h>

void nvgpu_mutex_init(struct nvgpu_mutex *mutex)
{
	int err = pthread_mutex_init(&mutex->lock.mutex, NULL);
	nvgpu_assert(err == 0);
}

void nvgpu_mutex_acquire(struct nvgpu_mutex *mutex)
{
	nvgpu_posix_lock_acquire(&mutex->lock);
}

void nvgpu_mutex_release(struct nvgpu_mutex *mutex)
{
	nvgpu_posix_lock_release(&mutex->lock);
}

int nvgpu_mutex_tryacquire(struct nvgpu_mutex *mutex)
{
	return ((nvgpu_posix_lock_try_acquire(&mutex->lock) == 0) ? 1 : 0);
}

void nvgpu_mutex_destroy(struct nvgpu_mutex *mutex)
{
	int err = pthread_mutex_destroy(&mutex->lock.mutex);
	nvgpu_assert(err == 0);
}

void nvgpu_spinlock_init(struct nvgpu_spinlock *spinlock)
{
	int err = pthread_mutex_init(&spinlock->lock.mutex, NULL);
	nvgpu_assert(err == 0);
}

void nvgpu_spinlock_acquire(struct nvgpu_spinlock *spinlock)
{
	nvgpu_posix_lock_acquire(&spinlock->lock);
}

void nvgpu_spinlock_release(struct nvgpu_spinlock *spinlock)
{
	nvgpu_posix_lock_release(&spinlock->lock);
}

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void nvgpu_raw_spinlock_init(struct nvgpu_raw_spinlock *spinlock)
{
	int err = pthread_mutex_init(&spinlock->lock.mutex, NULL);
	nvgpu_assert(err == 0);
}

void nvgpu_raw_spinlock_acquire(struct nvgpu_raw_spinlock *spinlock)
{
	nvgpu_posix_lock_acquire(&spinlock->lock);
}

void nvgpu_raw_spinlock_release(struct nvgpu_raw_spinlock *spinlock)
{
	nvgpu_posix_lock_release(&spinlock->lock);
}
#endif
