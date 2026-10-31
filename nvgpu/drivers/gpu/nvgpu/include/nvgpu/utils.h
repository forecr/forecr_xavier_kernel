/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_UTILS_H
#define NVGPU_UTILS_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#ifdef __KERNEL__
#include <nvgpu/linux/utils.h>
#else
#include <nvgpu/posix/utils.h>
#endif

/*
 * PAGE_SIZE is OS specific and can vary across OSes. Depending on the OS it maybe
 * defined to 4K or 64K.
 */
#define NVGPU_CPU_PAGE_SIZE		PAGE_SIZE

#define NVGPU_CPU_SMALL_PAGE_SIZE	4096U

/**
 * @file
 *
 * SW utilities
 *
 * @page utils utils
 *
 * Overview
 * ========
 *
 * The |nvgpu-rm| utils unit provides a number of general software utility
 * functions.  These fall into these different components:
 *
 * - bit-utils: These are general bit manipulation functions.
 * - enabled: This is a general framework used by the driver to store flags for
 * enabling and disabling features and functions in the driver. These functions
 * may be software or hardware features. These flags are intended to be set
 * during driver initialization, but can be queried during initialization or
 * execution.
 * - rbtree: This is a generic implementation of a red-black tree data structure
 * for maintaining a balanced binary search tree. This component defines a
 * struct nvgpu_rbtree_node that can be included within another struct
 * definition so the parent struct can be manipulated as a red-black tree.
 * - worker:  This is a fifo based producer-consumer worker for the |nvgpu-rm|
 * driver. It provides a generic implementation with hooks to allow each
 * application to implement specific use cases for producing and consuming
 * the work. The user of this API enqueues new work and awakens the worker
 * thread. On being awakened, the worker thread checks for pending work or a
 * user provided terminating condition. The generic poll implementation also
 * provides for early terminating conditions as well as pre and post processing
 * hooks.
 * - string: This includes support functions for managing strings and memory.
 * - kref: This includes support functions for managing the reference count
 * of objects.
 * - list: This includes support functions for creating and managing a list of
 * - messages.
 *
 * Data Structures
 * ===============
 *
 * bit-utils
 * ---------
 * N/A.
 * This component does not expose any data structures.
 *
 * enabled
 * -------
 * N/A.
 * This component does not expose any data structures.
 *
 * rbtree
 * ------
 *  + struct nvgpu_rbtree_node
 *
 * string
 * ------
 * N/A.
 * This component does not expose any data structures.
 *
 * worker
 * ------
 *  + struct nvgpu_worker
 *  + struct nvgpu_worker_ops
 *
 * kref
 * ----
 *  + struct nvgpu_ref
 *
 * list
 * ----
 *  + struct nvgpu_list_node
 *
 * Static Design
 * =============
 *
 * External APIs
 * -------------
 *
 * Bit-utils exposes following APIs
 *   + u64_hi32()
 *   + u64_lo32()
 *   + hi32_lo32_to_u64()
 *   + set_field()
 *   + get_field()
 *
 * Enabled exposes following APIs
 *   + nvgpu_init_enabled_flags()
 *   + nvgpu_set_enabled()
 *   + nvgpu_is_enabled()
 *   + nvgpu_free_enabled_flags()
 *
 * Rbtree exposes following APIs
 *   + nvgpu_rbtree_insert()
 *   + nvgpu_rbtree_unlink()
 *   + nvgpu_rbtree_search()
 *   + nvgpu_rbtree_range_search()
 *   + nvgpu_rbtree_less_than_search()
 *   + nvgpu_rbtree_enum_start()
 *   + nvgpu_rbtree_enum_next()
 *
 * String exposes following APIs
 *   + nvgpu_memcpy()
 *   + nvgpu_memcmp()
 *   + nvgpu_strnadd_u32()
 *   + nvgpu_mem_is_word_aligned()
 *
 * Worker exposes following APIs
 *   + nvgpu_worker_init()
 *   + nvgpu_worker_init_name()
 *   + nvgpu_worker_should_stop()
 *   + nvgpu_worker_enqueue()
 *   + nvgpu_worker_deinit()
 *
 * Kref exposes following APIs
 *   + nvgpu_ref_init()
 *   + nvgpu_ref_get()
 *   + nvgpu_ref_put()
 *   + nvgpu_ref_put_return()
 *   + nvgpu_ref_get_unless_zero()
 *
 * List exposes following APIs
 *   + nvgpu_init_list_node()
 *   + nvgpu_list_add()
 *   + nvgpu_list_add_tail()
 *   + nvgpu_list_del()
 *   + nvgpu_list_empty()
 *   + nvgpu_list_move()
 *   + nvgpu_list_replace_init()
 *
 * @defgroup unit-common-utils
 */

/**
 * @defgroup bit-utils
 * @ingroup unit-common-utils
 * @{
 */

/** Stringification macro. */
#define nvgpu_stringify(x)          #x

/**
 * @brief Higher 32 bits from 64 bit.
 *
 * Returns the most significant 32 bits of the 64 bit input value. Invokes the
 * function #nvgpu_safe_cast_u64_to_u32 with \a n right shifted by 32 as
 * parameter.
 *
 * @param n [in] Input value. Function does not perform any validation
 *		 of the parameter.
 *
 * @return Most significant 32 bits of \a n.
 */
static inline u32 u64_hi32(u64 n)
{
	return nvgpu_safe_cast_u64_to_u32(n >> 32);
}

/**
 * @brief Lower 32 bits from 64 bit.
 *
 * Returns the least significant 32 bits of the 64 bit input value. Invokes the
 * function #nvgpu_safe_cast_u64_to_u32 with higher 32 bits masked out \a n as
 * parameter.
 *
 * @param n [in] Input value. Function does not perform any validation of
 *		 the parameter.
 *
 * @return Least significant 32 bits of \a n.
 */
static inline u32 u64_lo32(u64 n)
{
	return nvgpu_safe_cast_u64_to_u32(n & ~(u32)0);
}

/**
 * @brief 64 bit from two 32 bit values.
 *
 * Returns a 64 bit value by combining the two 32 bit input values.
 *
 * @param hi [in] Higher 32 bits. Function does not perform any validation
 *		  of the parameter.
 * @param lo [in] Lower 32 bits. Function does not perform any validation
 *		  of the parameter.
 *
 * @return 64 bit value of which the least significant 32 bits are \a lo and
 * most significant 32 bits are \a hi.
 */
static inline u64 hi32_lo32_to_u64(u32 hi, u32 lo)
{
	return  (((u64)hi) << 32) | (u64)lo;
}

/**
 * @brief Sets a particular field value in input data.
 *
 * Uses the \a mask value to clear those bit positions in \a val and the value
 * of \a field is used to set the bits in the value to be returned.
 *
 * @param val [in] Value to set the field in. Function does not perform any
 *		   validation of the parameter.
 * @param mask [in] Mask for the field. Function does not perform any
 *		    validation of the parameter.
 * @param field [in] Field value. Function does not perform any validation
 *		     of the parameter.
 *
 * @return Returns a value with updated bits.
 */
static inline u32 set_field(u32 val, u32 mask, u32 field)
{
	return ((val & ~mask) | field);
}

/**
 * @brief Gets a particular field value from input data.
 *
 * Returns the field value at \a mask position in \a reg.
 *
 * @param reg [in] Value to get the field from. Function does not perform any
 *		   validation of the parameter.
 * @param mask [in] Mask for the field. Function does not perform any
 *		    validation of the parameter.
 *
 * @return Field value from \a reg according to the \a mask.
 */
static inline u32 get_field(u32 reg, u32 mask)
{
	return (reg & mask);
}

/*
 * MISRA Rule 11.6 compliant IP address generator.
 */
/**
 * Instruction pointer address generator. Used to get the virtual address
 * of the current instruction.
 */
#define NVGPU_GET_IP		\
	({ __label__ label_here; label_here: &&label_here; })

/**
 * @}
 */

#endif /* NVGPU_UTILS_H */
