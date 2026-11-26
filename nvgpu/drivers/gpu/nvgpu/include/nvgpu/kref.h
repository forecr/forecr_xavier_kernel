/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

/*
 * The following structure is used for reference counting of objects in nvgpu.
 */
#ifndef NVGPU_KREF_H
#define NVGPU_KREF_H

#include <nvgpu/atomic.h>
#include <nvgpu/types.h>
#include <nvgpu/utils.h>

struct nvgpu_ref {
	/**
	 * Atomic reference count.
	 */
	nvgpu_atomic_t refcount;
};

/**
 * @brief Initialize the reference object.
 *
 * Initializes the reference count of the object pointed by \a ref by
 * atomically setting it to 1. Invokes function #nvgpu_atomic_set with variable
 * \a refcount in #nvgpu_ref and 1 as parameters to set the value atomically.
 *
 * @param ref [in] The nvgpu_ref object to initialize. Function does not
 *		   perform any validation of the parameter.
 */
static inline void nvgpu_ref_init(struct nvgpu_ref *ref)
{
	nvgpu_atomic_set(&ref->refcount, 1);
}

/**
 * @brief Atomically increment the reference count.
 *
 * Increment the reference count for the object atomically. Invokes function
 * #nvgpu_atomic_inc with variable \a refcount in #nvgpu_ref as parameter.
 *
 * @param ref [in] The nvgpu_ref object. Function does not perform any
 *		   validation of the parameter.
 */
static inline void nvgpu_ref_get(struct nvgpu_ref *ref)
{
	nvgpu_atomic_inc(&ref->refcount);
}

/**
 * @brief Atomically decrement the reference count.
 *
 * Decrement reference count for the object atomically and call \a release if
 * it becomes zero. Invokes the function #nvgpu_atomic_sub_and_test with
 * variable \a refcount in #nvgpu_ref as parameter to decrement atomically.
 *
 * @param ref[in] The nvgpu_ref object. Function does not perform any
 *		  validation of the parameter.
 * @param release [in] Pointer to the function that would be invoked to clean
 *		       up the object when the reference count becomes zero.
 *		       Function uses the parameter only if it is not NULL.
 */
static inline void nvgpu_ref_put(struct nvgpu_ref *ref,
		void (*release)(struct nvgpu_ref *r))
{
	if (nvgpu_atomic_sub_and_test(1, &ref->refcount)) {
		if (release != NULL) {
			release(ref);
		}
	}
}

static inline int nvgpu_ref_put_int_ptr(struct nvgpu_ref *ref,
		int (*release)(struct nvgpu_ref *r))
{
	int err = 0;

	if (nvgpu_atomic_sub_and_test(1, &ref->refcount)) {
		if (release != NULL) {
			err = release(ref);
		}
	}

	return err;
}

/**
 * @brief Atomically decrement the reference count for the object and invoke
 * the callback function if it becomes zero and return the status of the
 * removal.
 *
 * Atomically decrement the reference count for the object pointed by \a ref
 * using the function #nvgpu_atomic_sub_and_test. \a refcount in #nvgpu_ref is
 * passed as the parameter to decrement atomically. Invokes the callback
 * function \a release if the refcount becomes zero and returns the status of
 * the removal.
 *
 * @param ref [in] The nvgpu_ref object. Function does not perform any
 *		   validation of the parameter.
 * @param release [in] Pointer to the function that would be invoked to
 *		       clean up the object when the reference count becomes
 *		       zero, i.e. the last reference corresponding to this
 *		       object is removed. Function uses the parameter only if
 *		       it is not NULL
 *
 * @return Return 1 if object was removed, otherwise return 0. The user should
 * not make any assumptions about the status of the object in the memory when
 * the function returns 0 and should only use it to know that there are no
 * further references to this object.
 *
 * @retval 1 if reference count becomes zero after decrement.
 * @retval 0 if reference count is non-zero after decrement.
 */
static inline int nvgpu_ref_put_return(struct nvgpu_ref *ref,
		void (*release)(struct nvgpu_ref *r))
{
	if (nvgpu_atomic_sub_and_test(1, &ref->refcount)) {
		if (release != NULL) {
			release(ref);
		}
		return 1;
	}
	return 0;
}

/**
 * @brief Atomically increment reference count of the object unless it is zero.
 *
 * Increment the reference count of the object pointed by \a ref unless it
 * is zero. Invokes the function #nvgpu_atomic_add_unless with \a refcount in
 * #nvgpu_ref as the parameter to set.
 *
 * @param ref [in] The nvgpu_ref object. Function does not perform any
 *		   validation of the parameter.
 *
 * @return Return non-zero if the increment succeeds, Otherwise return 0.
 */
static inline int nvgpu_ref_get_unless_zero(struct nvgpu_ref *ref)
{
	return nvgpu_atomic_add_unless(&ref->refcount, 1, 0);
}

#endif /* NVGPU_KREF_H */
