/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_UTILS_H
#define NVGPU_POSIX_UTILS_H

#include <stdlib.h>

#include <nvgpu/static_analysis.h>

/**
 *
 * @page posix posix
 *
 * Overview
 * ========
 *
 * The |nvgpu-rm| posix unit provides a set of core functionalities in an OS
 * agnostic way through a set of defined APIs. The APIs internally utilize the
 * standard library calls to provide the core functionalities. Most of the
 * features provided are implemented with the POSIX standard and rest of them are
 * internal implementation without any dependency on standard libraries.
 *
 * Following is the broad classification of the functionalities provided by the
 * posix unit,
 *
 * - Atomic: Provides APIs for fine-grained atomic operations allowing lock less
 *   concurrent programming. GCC built-ins and standard atomic headers are used for
 *   providing the atomic operations API.
 * - Bitops: Provides APIs for bitmap operations. GCC built-ins functions and
 *   standard headers are used for providing most of the bitmap operations, there
 *   are additional functionalities which are provided as internal implementation.
 *   Bug: Provides handler APIs for bug and warnings.
 * - Circ_Buf: Provides circular buffer handler APIs.
 * - Cond: Provides a mechanism for threads to synchronize. Cond implementation
 *    provides the APIs for using condition variables to synchronize between
 *    threads. Pthread condition variables are used internally to provide the
 *    functionality.
 * - File_Ops: Provides the APIs for file operations.
 * - Kmem: Provides the implementation of dynamic memory allocation and free. Uses
 *   the standard library dynamic allocation APIs internally to provide the
 *   functionality.
 * - Lock: Provides the APIs for various synchronization mechanisms like mutex,
 *   spinlock and raw spinlock. Although the APIs are provided for different
 *   synchronization mechanisms, the internal implementation of all of them are
 *   based on mutex. Pthread mutex is used to provide the functionality.
 * - Log2: Provides APIs for integer log to the base 2 operations.
 * - OS_Sched: Provides APIs to fetch the current process/thread details.
 * - Queue: Provides APIs to create and use message queues.
 * - RWSem: Read-write locks provide concurrent reads and exclusive writes to a
 *   shared resource. RWSem implementation provides APIs for using the read-write
 *   lock mechanism. Pthread read-write lock implementation is used internally to
 *   provide the desired functionality.
 *
 * Data Structures
 * ===============
 *
 * Atomic
 * ------
 *   + struct  nvgpu_posix_atomic
 *   + struct  nvgpu_posix_atomic64
 *
 * Bitops
 * ------
 * N/A
 * This component does not expose any data structures.
 *
 * Bug
 * ---
 *   + struct  nvgpu_bug_cb
 *
 * Circ_Buf
 * --------
 * N/A
 * This component does not expose any data structures.
 *
 * Cond
 * ----
 *   + struct  nvgpu_cond
 *
 * File_Ops
 * --------
 * N/A
 * This component does not expose any data structures.
 *
 * Kmem
 * ----
 *   + struct  nvgpu_kmem_cache
 *
 * Lock
 * ----
 *   + struct  nvgpu_mutex
 *   + struct  nvgpu_spinlock
 *   + struct  nvgpu_raw_spinlock
 *
 * Log2
 * ----
 *
 * N/A
 * This component does not expose any data structures.
 *
 * OS_Sched
 * --------
 *
 * N/A
 * This component does not expose any data structures.
 *
 * Queue
 * -----
 *
 *   + struct  nvgpu_queue
 *
 * RWSem
 * -----
 *
 *   + struct  nvgpu_rwsem
 *
 * Size
 * ----
 * N/A
 * This component does not expose any data structures.
 *
 * Threads
 * -------
 *
 *   + struct  nvgpu_thread
 *   + struct  nvgpu_posix_thread_data
 *
 * Timers
 * ------
 *
 *   + struct  nvgpu_timeout
 *
 * Types
 * -----
 *
 * N/A
 * This component does not expose any data structures.
 *
 * Utils
 * -----
 *
 * N/A
 * This component does not expose any data structures.
 *
 * External APIs
 * -------------
 * posix::Atomic
 * -------------
 *   + nvgpu_atomic_set()
 *   + nvgpu_atomic_read()
 *   + nvgpu_atomic_inc()
 *   + nvgpu_atomic_inc_return()
 *   + nvgpu_atomic_dec()
 *   + nvgpu_atomic_dec_return()
 *   + nvgpu_atomic_cmpxchg()
 *   + nvgpu_atomic_xchg()
 *   + nvgpu_atomic_inc_and_test()
 *   + nvgpu_atomic_dec_and_test()
 *   + nvgpu_atomic_sub_and_test()
 *   + nvgpu_atomic_add()
 *   + nvgpu_atomic_sub()
 *   + nvgpu_atomic_sub_return()
 *   + nvgpu_atomic_add_return()
 *   + nvgpu_atomic_add_unless()
 *   + nvgpu_atomic64_set()
 *   + nvgpu_atomic64_read()
 *   + nvgpu_atomic64_inc()
 *   + nvgpu_atomic64_inc_return()
 *   + nvgpu_atomic64_dec()
 *   + nvgpu_atomic64_dec_return()
 *   + nvgpu_atomic64_cmpxchg()
 *   + nvgpu_atomic64_xchg()
 *   + nvgpu_atomic64_inc_and_test()
 *   + nvgpu_atomic64_dec_and_test()
 *   + nvgpu_atomic64_sub_and_test()
 *   + nvgpu_atomic64_add()
 *   + nvgpu_atomic64_sub()
 *   + nvgpu_atomic64_sub_return()
 *   + nvgpu_atomic64_add_return()
 *   + nvgpu_atomic64_add_unless()
 *   +  #nvgpu_atomic_init_impl
 *   +  #nvgpu_atomic64_init_impl
 *
 * posix::Bitops
 * -------------
 *   + nvgpu_posix_ffs()
 *   + nvgpu_posix_fls()
 *   + find_first_bit()
 *   + find_next_bit()
 *   + find_first_zero_bit()
 *   + nvgpu_test_bit()
 *   + nvgpu_test_and_set_bit()
 *   + nvgpu_test_and_clear_bit()
 *   + nvgpu_set_bit()
 *   + nvgpu_clear_bit()
 *   + nvgpu_bitmap_set()
 *   + nvgpu_bitmap_clear()
 *   + bitmap_find_next_zero_area()
 *   +  #BITS_PER_BYTE
 *   +  #BITS_PER_LONG
 *   +  #BITS_TO_LONGS
 *   +  #BIT
 *   +  #GENMASK
 *   +  #DECLARE_BITMAP
 *   +  #for_each_set_bit
 *   +  #nvgpu_ffs
 *   +  #nvgpu_fls
 *   +  #ffz
 *
 * posix::Bug
 * ----------
 *   +  #BUG
 *   +  #BUG_ON
 *   +  #WARN
 *   +  #WARN_ON
 *   +  #WARN_ONCE
 *   + dump_stack()
 *   + nvgpu_posix_bug()
 *   + nvgpu_posix_warn()
 *   + nvgpu_bug_register_cb()
 *   + nvgpu_bug_unregister_cb()
 *
 * posix::Circ_Buf
 * ---------------
 *   +  #CIRC_CNT
 *   +  #CIRC_SPACE
 *
 * posix::Cond
 * -----------
 *   + nvgpu_cond_init()
 *   + nvgpu_cond_signal()
 *   + nvgpu_cond_signal_interruptible()
 *   + nvgpu_cond_broadcast()
 *   + nvgpu_cond_broadcast_interruptible()
 *   + nvgpu_cond_destroy()
 *   + nvgpu_cond_timedwait()
 *   + nvgpu_cond_signal_locked()
 *   + nvgpu_cond_broadcast_locked()
 *   + nvgpu_cond_lock()
 *   + nvgpu_cond_unlock()
 *   +  #NVGPU_COND_WAIT_LOCKED
 *   +  #NVGPU_COND_WAIT
 *   +  #NVGPU_COND_WAIT_INTERRUPTIBLE
 *   +  #NVGPU_COND_WAIT_TIMEOUT_LOCKED
 *
 * posix::File_Ops
 * ---------------
 *   + nvgpu_fstat()
 *   + nvgpu_fread()
 *   + nvgpu_close()
 *
 * posix::Kmem
 * -----------
 *   + nvgpu_kmem_cache_create()
 *   + nvgpu_kmem_cache_destroy()
 *   + nvgpu_kmem_cache_alloc()
 *   + nvgpu_kmem_cache_free()
 *   + nvgpu_big_alloc_impl()
 *   + nvgpu_big_free()
 *   + nvgpu_kmalloc_impl()
 *   + nvgpu_kzalloc_impl()
 *   + nvgpu_kcalloc_impl()
 *   + nvgpu_vmalloc_impl()
 *   + nvgpu_vzalloc_impl()
 *   + nvgpu_kfree_impl()
 *   + nvgpu_vfree_impl()
 *
 * posix::Lock
 * -----------
 *   + nvgpu_mutex_init()
 *   + nvgpu_mutex_acquire()
 *   + nvgpu_mutex_release()
 *   + nvgpu_mutex_tryacquire()
 *   + nvgpu_mutex_destroy()
 *   + nvgpu_spinlock_init()
 *   + nvgpu_spinlock_acquire()
 *   + nvgpu_spinlock_release()
 *   + nvgpu_raw_spinlock_init()
 *   + nvgpu_raw_spinlock_acquire()
 *   + nvgpu_raw_spinlock_release()
 *   + nvgpu_posix_lock_acquire()
 *   + nvgpu_posix_lock_try_acquire()
 *   + nvgpu_posix_lock_release()
 *
 * posix::Log2
 * -----------
 *   +  #ilog2
 *   +  #roundup_pow_of_two
 *   +  #rounddown_pow_of_two
 *   +  #is_power_of_2
 *
 * posix::OS_Sched
 * ---------------
 *   + nvgpu_current_pid()
 *   + nvgpu_current_tid()
 *   + nvgpu_print_current_impl()
 *
 * posix::Queue
 * ------------
 *   + nvgpu_queue_unused()
 *   + nvgpu_queue_available()
 *   + nvgpu_queue_alloc()
 *   + nvgpu_queue_free()
 *   + nvgpu_queue_in_locked()
 *   + nvgpu_queue_out_locked()
 *
 * posix::RWSem
 * ------------
 *   + nvgpu_rwsem_init()
 *   + nvgpu_rwsem_up_read()
 *   + nvgpu_rwsem_down_read()
 *   + nvgpu_rwsem_up_write()
 *   + nvgpu_rwsem_down_write()
 *
 * posix::Size
 * -----------
 * N/A
 * This component does not expose any public APIs. Size component consists only
 * of macros defined for various sizes.
 *
 * posix::Threads
 * --------------
 *   + nvgpu_thread_create()
 *   + nvgpu_thread_stop()
 *   + nvgpu_thread_stop_graceful()
 *   + nvgpu_thread_should_stop()
 *   + nvgpu_thread_is_running()
 *   + nvgpu_thread_join()
 *   + nvgpu_thread_create_priority()
 *   +  #nvgpu_thread_cleanup_push
 *   +  #nvgpu_thread_cleanup_pop
 *
 * posix::Timers
 * -------------
 *   + nvgpu_timeout_init()
 *   + nvgpu_timeout_peek_expired()
 *   + nvgpu_msleep()
 *   + nvgpu_usleep_range()
 *   + nvgpu_udelay()
 *   + nvgpu_current_time_ms()
 *   + nvgpu_current_time_ns()
 *   + nvgpu_current_time_us()
 *   + nvgpu_timeout_expired_msg_impl
 *
 * posix::Types
 * ------------
 * N/A
 * This component does not expose any public APIs. Types component consists only
 * of aliases for standard data types.
 *
 * posix::Utils
 * ------------
 *   +  #min_t
 *   +  #min
 *   +  #max
 *   +  #min3
 *   +  #ARRAY_SIZE
 *   +  #DIV_ROUND_UP_U64
 *   +  #DIV_ROUND_UP
 *   +  #DIV_ROUND_UP_ULL
 *   +  #do_div
 *   +  #div64_u64
 *   +  #round_mask
 *   +  #round_up
 *   +  #round_down
 *   +  #IS_UNSIGNED_TYPE
 *   +  #IS_UNSIGNED_LONG_TYPE
 *   +  #IS_SIGNED_LONG_TYPE
 *   +  #ALIGN_MASK
 *   +  #ALIGN
 *   +  #PAGE_ALIGN
 *   +  #HZ_TO_KHZ
 *   +  #HZ_TO_MHZ
 *   +  #HZ_TO_MHZ_ULL
 *   +  #KHZ_TO_HZ
 *   +  #MHZ_TO_KHZ
 *   +  #KHZ_TO_MHZ
 *   +  #MHZ_TO_HZ_ULL
 *   +  #hweight32
 *   +  #hweight_long
 *   +  #WRITE_ONCE
 *   +  #container_of
 *   + be32_to_cpu()
 *   + nvgpu_posix_hweight8()
 *   + nvgpu_posix_hweight16()
 *   + nvgpu_posix_hweight32()
 *   + nvgpu_posix_hweight64()
 */

/**
 * @brief Minimum of two values using the specified type.
 *
 * Macro does not perform any validation of the input parameters.
 *
 * @param type	Type of the input values.
 * @param a	First value.
 * @param b	Second value.
 *
 * @return Returns the minimum value from \a a and \a b.
 */
#define min_t(type, a, b)			\
	({					\
		type t_a = (a);			\
		type t_b = (b);			\
		(t_a < t_b) ? t_a : t_b;	\
	})

#ifndef _QNX_SOURCE
/**
 * @brief Minimum of two values.
 *
 * @param a	First value.
 * @param b	Second value.
 *
 * @return Returns the minimum value from \a a and \a b.
 */
#ifndef min
#define min(a, b)				\
	({					\
		((a) < (b)) ? (a) : (b);	\
	})
#endif
/**
 * @brief Maximum of two values.
 *
 * @param a	First value.
 * @param b	Second value.
 *
 * @return Returns the maximum value from \a a and \a b.
 */
#ifndef max
#define max(a, b)				\
	({					\
		((a) > (b)) ? (a) : (b);	\
	})
#endif
#endif
/**
 * @brief Minimum of three values.
 *
 * Uses the macro #min internally to identify the minimal value from the
 * given three values. Macro does not perform any validation of the parameters.
 *
 * @param a	First value.
 * @param b	Second value.
 * @param c	Third value.
 *
 * @return Returns the minimum value from \a a, \a b and \a c.
 */
#define min3(a, b, c)			min(min(a, b), c)

/** Size of a Page. */
#define PAGE_SIZE	4096U

/**
 * @brief Number of elements in an array.
 *
 * @param array	Input array.
 *
 * @return Returns the number of elements in \a array.
 */
#define ARRAY_SIZE(array)			\
	(sizeof(array) / sizeof((array)[0]))

/** Maximum schedule timeout. */
#define MAX_SCHEDULE_TIMEOUT	LONG_MAX

/**
 * @brief Round up division for unsigned long.
 *
 * Rounds up the value of the dividend \a n according to the value of the
 * divisor \a d and does the division. The value of \a d should not be passed
 * as 0. Safe addition and subtraction APIs, #nvgpu_safe_add_u64() and
 * #nvgpu_safe_sub_u64() are used to round up the value of the dividend.
 * Macro does not perform any validation of the parameters.
 *
 * @param n	Numerator.
 * @param d	Denominator.
 *
 * @return Returns the rounded up 64 bit division result.
 */
#define DIV_ROUND_UP_U64(n, d)					\
({								\
	u64 divr = (u64)(d);					\
	u64 divm = nvgpu_safe_sub_u64((divr), U64(1));		\
	u64 rup = nvgpu_safe_add_u64((u64)(n), (divm));		\
	u64 ret_val = ((rup) / (divr));				\
	ret_val ;						\
})

/**
 * @brief Round up division.
 *
 * Invokes the macro #DIV_ROUND_UP_U64 internally to perform the round up
 * operation. Refer to #DIV_ROUND_UP_U64 for further details. Macro does not
 * perform any validation of the parameters.
 *
 * @param n	Numerator.
 * @param d	Denominator.
 *
 * @return Returns the rounded up division result.
 */
#define DIV_ROUND_UP(n, d)						\
({									\
	typeof(n) val =  ((sizeof(typeof(n))) == (sizeof(u64))) ?	\
		(typeof(n))(DIV_ROUND_UP_U64(n, d)) :			\
		nvgpu_safe_cast_u64_to_u32(DIV_ROUND_UP_U64(n, d));	\
	val;								\
})

/**
 * @brief Round up division for unsigned long long.
 *
 * Wrapper macro over #DIV_ROUND_UP.
 */
#define DIV_ROUND_UP_ULL	DIV_ROUND_UP

#ifdef CONFIG_NVGPU_NON_FUSA
/**
 * Divide positive or negative dividend by positive or negative divisor
 * and round to closest integer.
 */
#define DIV_ROUND_CLOSEST(a, divisor)(                  \
{                                                       \
	typeof(a) val = (a);                            \
	typeof(divisor) div = (divisor);                \
	(((typeof(a))-1) > 0 ||                         \
	((typeof(divisor))-1) > 0 ||                    \
	(((val) > 0) == ((div) > 0))) ?                 \
		(((val) + ((div) / 2)) / (div)) :       \
		(((val) - ((div) / 2)) / (div));        \
}                                                       \
)
#endif

/*
 * Joys of userspace: usually division just works since the compiler can link
 * against external division functions implicitly.
 */

/**
 * @brief Division of two values.
 *
 * Divides the input paramater \a a by \a b and updates the quotient value in
 * \a a. Macro does not perform any validation of the input parameters.
 *
 * @param a	Dividend, should be an lvalue.
 * @param b	Divisor.
 *
 * @return Param \a a is updated with the quotient value of the division.
 */
#define do_div(a, b)		((a) /= (b))

/**
 * @brief Division of two 64 bit values.
 *
 * Divides the input paramater \a a by \a b. Macro does not perform any
 * validation of the input parameters.
 *
 * @param a	Dividend.
 * @param b	Divisor.
 *
 * @return Quotient is returned.
 */
#define div64_u64(a, b)		((a) / (b))

/**
 * @brief Generate mask value for round operations.
 *
 * Macro does not perform any validation of the parameters.
 *
 * @param x	Data type of this param is used to type cast.
 * @param y	Value for which the mask is generated.
 *
 * @return Mask value based on \a y is returned.
 */
#define round_mask(x, y)	((__typeof__(x))((y) - 1U))

/**
 * @brief Round up the value of its argument \a x.
 *
 * Uses the macro #round_mask to generate the mask value based on input
 * parameter \a y and uses that value to round up the value of the input
 * parameter \a x. Macro does not perform any validation of the parameters.
 *
 * @param x	Value to be rounded.
 * @param y	Value to be used to round up x. Must be power-of-two.
 *
 * @return Rounded up value of \a x.
 */
#define round_up(x, y)		((((x) - 1U) | round_mask(x, y)) + 1U)

/**
 * @brief Round down the value of its argument \a x.
 *
 * Uses the macro #round_mask to generate the mask value based on input
 * parameter \a y and uses that value to round down the value of the input
 * parameter \a x. Macro does not perform any validation of the parameters.
 *
 * @param x	Value to be rounded.
 * @param y	Value to be used to round down x.
 *
 * @return Rounded down value of \a x.
 */
#define round_down(x, y)	((x) & ~round_mask(x, y))

/**
 * @brief To identify whether the data type of input value is unsigned.
 *
 * Uses the builtin function \a __builtin_types_compatible_p to identify if the
 * input paramater \x is of unsigned type.
 *
 * @param x	Input value.
 *
 * @return Returns TRUE for unsigned data types, FALSE otherwise.
 */
#define IS_UNSIGNED_TYPE(x)						\
	(__builtin_types_compatible_p(typeof(x), unsigned int) ||	\
		__builtin_types_compatible_p(typeof(x), unsigned long) || \
		__builtin_types_compatible_p(typeof(x), unsigned long long))

/**
 * @brief To identify whether the data type of input value is unsigned long.
 *
 * Uses the builtin function \a __builtin_types_compatible_p to identify if the
 * input paramater \x is of unsigned long type.
 *
 * @param x	Input value.
 *
 * @return Returns TRUE for unsigned long data types, FALSE otherwise.
 */
#define IS_UNSIGNED_LONG_TYPE(x)					\
	(__builtin_types_compatible_p(typeof(x), unsigned long) ||	\
		__builtin_types_compatible_p(typeof(x), unsigned long long))

/**
 * @brief To identify whether the data type of input value is signed long.
 *
 * Uses the builtin function \a __builtin_types_compatible_p to identify if the
 * input paramater \x is of signed long type.
 *
 * @param x	Input value.
 *
 * @return Returns TRUE for signed long data types, FALSE otherwise.
 */
#define IS_SIGNED_LONG_TYPE(x)						\
	(__builtin_types_compatible_p(typeof(x), signed long) ||	\
		__builtin_types_compatible_p(typeof(x), signed long long))

/**
 * @brief Align with mask value.
 *
 * Uses the builtin function \a __builtin_choose_expr to evaluate code
 * depending on the input parameters. The types of the input parameters are
 * confirmed to be unsigned using the macro #IS_UNSIGNED_TYPE and uses the
 * macro #IS_UNSIGNED_LONG_TYPE to check if the input parameters are of type
 * unsigned long or not. Respective safe addition API for the data type is
 * invoked to fetch the result. Macro performs the validation of the input
 * parameters to check if they are of unsgined type.
 *
 * @param x	Value to be aligned.
 * @param mask	Mask value to align with.
 *
 * @return Returns \a x aligned with \a mask.
 */
#define ALIGN_MASK(x, mask)						\
	__builtin_choose_expr(						\
		(IS_UNSIGNED_TYPE(x) && IS_UNSIGNED_TYPE(mask)),	\
		(NVGPU_SAFE_ADD_UNSIGNED((x), (mask)) &			\
			~(typeof(x))(mask)),				\
		/* Results in build error. Make x/mask type unsigned */ \
		(void)0)

/**
 * @brief Align the parameter \a x with \a a.
 *
 * Uses the builtin function \a __builtin_choose_expr to evaluate the code.
 * Uses the macros #IS_UNSIGNED_TYPE, #IS_UNSIGNED_LONG_TYPE and #ALIGN_MASK
 * internally to perform the alignment operation. Macro verifies if both the
 * input parameters are of unsigned type.
 *
 * @param x	Value to be aligned.
 * @param mask	Value to align with.
 *
 * @return Returns \a x aligned with the value mentioned in \a a.
 */
#define NVGPU_ALIGN(x, a)						\
	__builtin_choose_expr(						\
		(IS_UNSIGNED_TYPE(x) && IS_UNSIGNED_TYPE(a)),		\
		ALIGN_MASK((x), NVGPU_SAFE_SUB_UNSIGNED(a, 1)),		\
		/* Results in build error. Make x/a type unsigned */	\
		(void)0)

/**
 * @brief Align with #PAGE_SIZE.
 *
 * Wrapper macro over #NVGPU_ALIGN to align the input paramater \a x with
 * \a PAGE_SIZE. Macro does ont perform any validation of the input parameter.
 *
 * @param x	Input value to be aligned.
 *
 * @return Returns \a x aligned with the page size value.
 */
#define PAGE_ALIGN(x)		NVGPU_ALIGN(x, PAGE_SIZE)

/**
 * @brief Convert hertz to kilo hertz.
 *
 * Macro does not perform any validation of the parameter.
 *
 * @param x	Value to convert.
 *
 * @return Converts \a x into kilo hertz and returns.  Fractional value is not
 * obtained in the conversion.
 */
#define HZ_TO_KHZ(x) ((x) / KHZ)

/**
 * @brief Convert hertz to mega hertz.
 *
 * Macro does not perform any validation of the parameter.
 *
 * @param x	Value to convert.
 *
 * @return Converts \a x into mega hertz and returns.  Fractional value is not
 * obtained in the conversion.
 */
#define HZ_TO_MHZ(a) (u16)(a/MHZ)

/**
 * @brief Convert hertz value in unsigned long long to mega hertz.
 *
 * Macro does not perform any validation of the parameter.
 *
 * @param x	Value to convert.
 *
 * @return Converts \a a into mega hertz and returns.
 */
#define HZ_TO_MHZ_ULL(a) (((a) > 0xF414F9CD7ULL) ? (u16) 0xffffU :\
		(((a) >> 32) > 0U) ? (u16) (((a) * 0x10C8ULL) >> 32) :\
		(u16) ((u32) (a)/MHZ))

/**
 * @brief Convert kilo hertz to hertz.
 *
 * Macro does not perform any validation of the parameter.
 *
 * @param x	Value to convert.
 *
 * @return Equivalent value of \a x in hertz.
 */
#define KHZ_TO_HZ(x) ((x) * KHZ)

/**
 * @brief Convert mega hertz to kilo hertz.
 *
 * Macro does not perform any validation of the parameter.
 *
 * @param x	Value to convert.
 *
 * @return Equivalent value of \a x in hertz.
 */
#define MHZ_TO_KHZ(x) ((x) * KHZ)

/**
 * @brief Convert kilo hertz to mega hertz.
 *
 * Macro does not perform any validation of the parameter.
 *
 * @param x	Value to convert.
 *
 * @return Equivalent value of \a a in mega hertz.
 */
#define KHZ_TO_MHZ(a) (u16)(a/KHZ)

/**
 * @brief Convert mega hertz to 64 bit hertz value.
 *
 * Macro does not perform any validation of the parameter.
 *
 * @param x	Value to convert.
 *
 * @return Equivalent value of \a a in 64 bit hertz.
 */
#define MHZ_TO_HZ_ULL(a) ((u64)(a) * MHZ)

#ifndef NVGPU_HVRTOS
/**
 * @brief Endian conversion.
 *
 * Converts the input value \a x in big endian to CPU byte order.
 * Uses the library function \a ntohl internally. Function does not perform
 * any validation of the input paramater.
 *
 * @param x [in] Value to be converted.
 *
 * @return Endianness converted value of \a x.
 */
static inline u32 be32_to_cpu(u32 x)
{
	/*
	 * Conveniently big-endian happens to be network byte order as well so
	 * we can use ntohl() for this.
	 */
	return ntohl(x);
}
#endif

/*
 * Hamming weights.
 */

/**
 * @brief Hamming weight of 8 bit input value.
 *
 * Returns the hamming weight(number of non zero bits) of the input param \a x.
 * Function does not perform any validation of the input parameter.
 *
 * @param x [in]	Input to find the hamming weight of.
 *
 * @return Hamming weight of \a x.
 */
static inline unsigned int nvgpu_posix_hweight8(uint8_t x)
{
	unsigned int ret;
	const u8 mask1 = 0x55;
	const u8 mask2 = 0x33;
	const u8 mask3 = 0x0f;
	const u8 shift1 = 1;
	const u8 shift2 = 2;
	const u8 shift4 = 4;

	uint8_t result = ((U8(x) >> shift1) & mask1);

	result = nvgpu_safe_sub_u8(x, result);

	result = (u8)((result & mask2) + ((result >> shift2) & mask2));
	result = (result + (result >> shift4)) & mask3;
	ret = (unsigned int)result;

	return ret;
}

/**
 * @brief Hamming weight of 16 bit input value.
 *
 * Returns the hamming weight(number of non zero bits) of the input param \a x.
 * Uses the function #nvgpu_posix_hweight8() internally to compute the result.
 * Function does not perform any validation of the input parameter.
 *
 * @param x [in]	Input to find the hamming weight of.
 *
 * @return Hamming weight of \a x.
 */
static inline unsigned int nvgpu_posix_hweight16(uint16_t x)
{
	unsigned int ret;
	const u8 mask = 0xff;
	const u8 shift8 = 8;

	ret = nvgpu_posix_hweight8((uint8_t)(x & mask));
	ret += nvgpu_posix_hweight8((uint8_t)((x >> shift8) & mask));

	return ret;
}

/**
 * @brief Hamming weight of 32 bit input value.
 *
 * Returns the hamming weight(number of non zero bits) of the input param \a x.
 * Uses the function #nvgpu_posix_hweight16() internally to compute the result.
 * Function does not perform any validation of the input parameter.
 *
 * @param x [in]	Input to find the hamming weight of.
 *
 * @return Hamming weight of \a x.
 */
static inline unsigned int nvgpu_posix_hweight32(uint32_t x)
{
	unsigned int ret;
	const u16 mask = 0xffff;
	const u16 shift16 = 16;

	ret = nvgpu_posix_hweight16((uint16_t)(x & mask));
	ret += nvgpu_posix_hweight16((uint16_t)((x >> shift16) & mask));

	return ret;
}

/**
 * @brief Hamming weight of 64 bit input value.
 *
 * Returns the hamming weight(number of non zero bits) of the input param \a x.
 * Uses the function #nvgpu_posix_hweight32() internally to compute the result.
 * Function does not perform any validation of the input parameter.
 *
 * @param x [in]	Input to find the hamming weight of.
 *
 * @return Hamming weight of \a x.
 */
static inline unsigned int nvgpu_posix_hweight64(uint64_t x)
{
	unsigned int ret;
	u32 lo, hi;
	const u32 tmp0 = 0;
	const u32 shift32 = 32;

	lo = nvgpu_safe_cast_u64_to_u32(x & ~tmp0);
	hi = nvgpu_safe_cast_u64_to_u32(x >> shift32) & ~tmp0;

	ret =  nvgpu_posix_hweight32(lo);
	ret += nvgpu_posix_hweight32(hi);

	return ret;
}

/**
 * Hamming weight of 32 bit input value. Wrapper define for
 * #nvgpu_posix_hweight32().
 */
#define hweight32		nvgpu_posix_hweight32

/**
 * Hamming weight of 64 bit input value. Wrapper define for
 * #nvgpu_posix_hweight64().
 */
#define hweight_long		nvgpu_posix_hweight64

/** Dummy macro to match kernel names. */
#define nvgpu_user

/** Defined to match kernel macro names. */
#define unlikely(x)	(x)
/** Defined to match kernel macro names. */
#define likely(x)	(x)

/**
 * @brief Get the container which holds the member.
 *
 * Macro does not perform any validation of the input parameters.
 *
 * @param ptr		Address of the member.
 * @param type		Type of the container holding member.
 * @param member	The member name in the container.
 *
 * @return Reference to the container holding \a member.
 */
#define container_of(ptr, type, member) ({                    \
	typeof(((type *)0)->member) *__mptr = (ptr);    \
	(type *)((uintptr_t)__mptr - offsetof(type, member)); })

/** Define for maximum error number. */
#define MAX_ERRNO	4095

/** Error define to indicate that a system call should restart. */
#define ERESTARTSYS ERESTART

#endif /* NVGPU_POSIX_UTILS_H */
