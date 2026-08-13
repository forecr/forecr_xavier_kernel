/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_RWSEM_H
#define NVGPU_RWSEM_H

#ifdef __KERNEL__
#include <nvgpu/linux/rwsem.h>
#elif defined(NVGPU_HVRTOS)
#include <nvgpu_hvrtos/rwsem.h>
#else
#include <nvgpu/posix/rwsem.h>
#endif

/*
 * struct nvgpu_rwsem
 *
 * Should be implemented per-OS in a separate library
 * But implementation should adhere to rw_semaphore implementation
 * as specified in Linux Documentation
 */
struct nvgpu_rwsem;

/**
 * @brief Initialize read write lock.
 *
 * Initializes the read,write lock referenced by \a rwsem. Uses the library
 * function #pthread_rwlock_init with \a rw_sem in #nvgpu_rwsem and NULL as
 * parameters to initialize the lock. Assert using function #nvgpu_assert()
 * if the return value from #pthread_rwlock_init is not 0.
 *
 * @param rwsem [in]	Read,write lock to initialize.
 */
void nvgpu_rwsem_init(struct nvgpu_rwsem *rwsem);

/**
 * @brief Release read lock.
 *
 * Releases the lock held on \a rwsem. Uses the library function
 * #pthread_rwlock_unlock with \a rw_sem in #nvgpu_rwsem as parameter to
 * release the read lock. Assert using #nvgpu_assert() if the return value
 * from #pthread_rwlock_unlock is not 0.
 *
 * @param rwsem [in]	Read,write lock to be released.
 */
void nvgpu_rwsem_up_read(struct nvgpu_rwsem *rwsem);

/**
 * @brief Acquire read lock.
 *
 * Acquires a read lock on the read,write lock object referenced by \a rwsem.
 * Uses the library function #pthread_rwlock_rdlock with \a rw_sem in
 * #nvgpu_rwsem as parameter to acquire the read lock. The calling thread
 * acquires a read lock if no writer is holding the lock on \a rwsem. Assert
 * using #nvgpu_assert() if the return value from #pthread_rwlock_rdlock is
 * not 0.
 *
 * @param rwsem [in]	Read,write lock to be locked for read.
 */
void nvgpu_rwsem_down_read(struct nvgpu_rwsem *rwsem);

/**
 * @brief Release the write lock.
 *
 * Releases the write lock held on \a rwsem. Uses the library function
 * #pthread_rwlock_unlock with \a rw_sem in #nvgpu_rwsem as parameter to
 * release the write lock. Assert using #nvgpu_assert() if the return value
 * from #pthread_rwlock_unlock is not 0.
 *
 * @param rwsem [in]	Read,write lock to be released.
 */
void nvgpu_rwsem_up_write(struct nvgpu_rwsem *rwsem);

/**
 * @brief Acquire write lock.
 *
 * Acquires the write lock on \a rwsem. The calling thread acquires write lock
 * if no other thread is holding a lock on rwsem. Uses the library function
 * #pthread_rwlock_wrlock with \a rw_sem in #nvgpu_rwsem as parameter to
 * acquire the write lock. Assert using #nvgpu_assert() if the return value
 * from #pthread_rwlock_wrlock is not 0.
 *
 * @param rwsem [in]	Read,write lock to be locked for write.
 */
void nvgpu_rwsem_down_write(struct nvgpu_rwsem *rwsem);

#endif /* NVGPU_RWSEM_H */
