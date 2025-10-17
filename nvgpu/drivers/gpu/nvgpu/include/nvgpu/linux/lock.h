/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LOCK_LINUX_H
#define NVGPU_LOCK_LINUX_H

#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>

struct nvgpu_mutex {
	struct mutex mutex;
};
struct nvgpu_spinlock {
	spinlock_t spinlock;
};
struct nvgpu_raw_spinlock {
	raw_spinlock_t spinlock;
};

struct nvgpu_sem {
	struct semaphore sem;
};

static inline void nvgpu_mutex_init(struct nvgpu_mutex *mutex)
{
	mutex_init(&mutex->mutex);
};
static inline void nvgpu_mutex_acquire(struct nvgpu_mutex *mutex)
{
	mutex_lock(&mutex->mutex);
};
static inline void nvgpu_mutex_release(struct nvgpu_mutex *mutex)
{
	mutex_unlock(&mutex->mutex);
};
static inline int nvgpu_mutex_tryacquire(struct nvgpu_mutex *mutex)
{
	return mutex_trylock(&mutex->mutex);
};
static inline void nvgpu_mutex_destroy(struct nvgpu_mutex *mutex)
{
	mutex_destroy(&mutex->mutex);
};

static inline void nvgpu_spinlock_init(struct nvgpu_spinlock *spinlock)
{
	spin_lock_init(&spinlock->spinlock);
};
static inline void nvgpu_spinlock_acquire(struct nvgpu_spinlock *spinlock)
{
	spin_lock(&spinlock->spinlock);
};
static inline void nvgpu_spinlock_release(struct nvgpu_spinlock *spinlock)
{
	spin_unlock(&spinlock->spinlock);
};

#define nvgpu_spinlock_irqsave(lock, flags) \
		spin_lock_irqsave(lock.spinlock, flags)

#define nvgpu_spinunlock_irqrestore(lock, flags) \
		spin_unlock_irqrestore(lock.spinlock, flags)
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
static inline void nvgpu_raw_spinlock_init(struct nvgpu_raw_spinlock *spinlock)
{
	raw_spin_lock_init(&spinlock->spinlock);
};
static inline void nvgpu_raw_spinlock_acquire(struct nvgpu_raw_spinlock *spinlock)
{
	raw_spin_lock(&spinlock->spinlock);
};
static inline void nvgpu_raw_spinlock_release(struct nvgpu_raw_spinlock *spinlock)
{
	raw_spin_unlock(&spinlock->spinlock);
};
#endif

static inline int nvgpu_sem_init(struct nvgpu_sem *sem, unsigned int count)
{
	sema_init(&sem->sem, count);
	return 0;
}

static inline void nvgpu_sem_destroy(struct nvgpu_sem *sem)
{
}

static inline int nvgpu_sem_wait(struct nvgpu_sem *sem)
{
	int err = down_killable(&sem->sem);

	if (err)
		return -ERESTARTSYS;
	else
		return 0;
};

static inline int nvgpu_sem_post(struct nvgpu_sem *sem)
{
	up(&sem->sem);
	return 0;
}

#endif /* NVGPU_LOCK_LINUX_H */
