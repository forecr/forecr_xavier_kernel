/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_KMEM_H
#define NVGPU_POSIX_KMEM_H

#include <nvgpu/types.h>

/**
 * @brief Allocate requested memory.
 *
 * Allocates the requested memory and returns a pointer to the allocated
 * memory.  Returns %NULL if allocation fails. Uses the library function
 * \a malloc internally to allocate memory. Function does not perform any
 * validation of the parameters.
 *
 * @param g [in]	GPU driver struct.
 * @param size [in]	Size to allocate.
 * @param ip [in]	Instruction pointer requesting the allocation.
 *
 * @return Pointer to memory in case of successful allocation, %NULL in case
 * of failure.
 *
 * @retval NULL in case of failure.
 */
void *nvgpu_kmalloc_impl(struct gk20a *g, size_t size, void *ip);

/**
 * @brief Allocate zero initialized memory.
 *
 * Allocates the requested memory and initializes the allocated space with
 * zeros.  On successful allocation a pointer to the allocated memory is
 * returned; otherwise returns %NULL. Uses the library function \a calloc
 * internally to allocate memory. Function does not perform any validation
 * of the parameters.
 *
 * @param g [in]	GPU driver struct.
 * @param size [in]	Size to allocate.
 * @param ip [in]	Instruction pointer requesting the allocation.
 *
 * @return Pointer to zero initialized memory in case of successful allocation,
 * %NULL in case of failure.
 *
 * @retval NULL in case of failure.
 */
void *nvgpu_kzalloc_impl(struct gk20a *g, size_t size, void *ip);

/**
 * @brief Allocate requested memory.
 *
 * Allocates the requested memory of size (\a n * \a size) and returns a pointer
 * to the allocated memory.  The allocated space is initialized with zeros.
 * Returns %NULL if allocation fails. Uses the library function \a calloc
 * internally to allocate memory. Function does not perform any validation
 * of the parameters.
 *
 * @param g [in]	GPU driver struct.
 * @param n [in]	Size of each element.
 * @param size [in]	Number of elements to allocate.
 * @param ip [in]	Instruction pointer requesting the allocation.
 *
 * @return Pointer to zero initialized memory in case of successful allocation,
 * %NULL in case of failure.
 *
 * @retval NULL in case of failure.
 */
void *nvgpu_kcalloc_impl(struct gk20a *g, size_t n, size_t size, void *ip);

/**
 * @brief Allocate requested memory.
 *
 * Allocates the requested memory and returns a pointer to the allocated
 * memory.  Functionality wise this API is same as #nvgpu_kmalloc_impl()
 * in Posix environment.  Returns %NULL if allocation fails. Refer to the
 * function #nvgpu_kmalloc_impl() for further details. Function does not
 * perform any validation of the parameters.
 *
 * @param g [in]	GPU driver struct.
 * @param size [in]	Size to allocate.
 * @param ip [in]	Instruction pointer requesting the allocation.
 *
 * @return Pointer to memory in case of successful allocation, %NULL in case
 * of failure.
 *
 * @retval NULL in case of failure.
 */
void *nvgpu_vmalloc_impl(struct gk20a *g, unsigned long size, void *ip);

/**
 * @brief Allocate and initialize the requested size.
 *
 * Allocates the requested memory and initializes the allocated space with
 * zeros.  On successful allocation a pointer to the allocated memory is
 * returned; otherwise %NULL is returned.  Functionality wise this API is the
 * same as #nvgpu_kzalloc_impl() in Posix environment. Refer to function
 * #nvgpu_kzalloc_impl() for further details. Function does not perform any
 * validation of the parameters.
 *
 * @param g [in]	GPU driver struct.
 * @param size [in]	Size to allocate.
 * @param ip [in]	Instruction pointer requesting the allocation.
 *
 * @return Pointer to memory in case of successful allocation, %NULL in case
 * of failure.
 *
 * @retval NULL in case of failure.
 */
void *nvgpu_vzalloc_impl(struct gk20a *g, unsigned long size, void *ip);

/**
 * @brief Free memory.
 *
 * Uses the library function \a free internally to free the allocated memory
 * pointed by \a addr. Function does not perform any validation of the
 * parameters.
 *
 * @param g [in]	GPU driver struct.
 * @param addr [in]	Address to free.
 */
void nvgpu_kfree_impl(struct gk20a *g, void *addr);

/**
 * @brief Free memory.
 *
 * Free the memory pointed to by \a addr. In Posix environment this API
 * is the same as #nvgpu_kfree_impl() in terms of functionality. Function
 * does not perform any validation of the parameters.
 *
 * @param g [in]	GPU driver struct.
 * @param addr [in]	Address to free.
 */
void nvgpu_vfree_impl(struct gk20a *g, void *addr);

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
/**
 * @brief Get fault injection structure.
 *
 * @param void.
 *
 * Returns the reference to kmem fault injection structure.
 *
 * @return Pointer to kmem fault injection structure.
 */
struct nvgpu_posix_fault_inj *nvgpu_kmem_get_fault_injection(void);
#endif

#endif /* NVGPU_POSIX_KMEM_H */
