/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_BUG_H
#define NVGPU_BUG_H

#ifdef __KERNEL__
#include <linux/bug.h>
#elif defined(NVGPU_HVRTOS)
#include <nvgpu_hvrtos/bug.h>
#else
#include <nvgpu/posix/bug.h>
#endif
#include <nvgpu/cov_whitelist.h>
#include <nvgpu/list.h>

/**
 * @brief Assert macro based on condition check that code within nvgpu can use.
 *
 * The goal of this macro is to support handling an unexpected state in SW
 * based on the \a cond parameter passed. The implementation is OS specific.
 * In QNX and POSIX implementation, this macro will invoke the #BUG_ON() macro
 * with parameter as #true or #false which is based on the evaluation of
 * \a cond. MAcro does not perform any validation of the parameter.
 *
 * @param cond [in]   The condition to check.
 */
#if defined(__KERNEL__)
#define nvgpu_assert(cond)	((void) WARN_ON(!(cond)))
#else
/*
 * When this assert fails, the function will not return.
 */
#define nvgpu_assert(cond)						\
	({								\
		BUG_ON((cond) == ((bool)(0 != 0)));			\
	})
#endif

/**
 * @brief Macro to force a failed assert.
 *
 * The goal of this macro is to force the consequences of a failed assert.
 * Invokes the macro #nvgpu_assert with parameter as #true.
 */
#define nvgpu_do_assert()						\
	nvgpu_assert((bool)(0 != 0))

/*
 * Define compile-time assert check.
 */
#define ASSERT_CONCAT(a, b) a##b
#define ASSERT_ADD_INFO(a, b) ASSERT_CONCAT(a, b)
#define nvgpu_static_assert(e)						\
	enum {								\
		ASSERT_ADD_INFO(assert_line_, __LINE__) = 1 / (!!(e))	\
	}

struct gk20a;

/**
 * @brief Macro to force a failed assert with error prints.
 *
 * The goal of this macro is to print an error message and force the
 * consequences of a failed assert. Invokes the macro #nvgpu_err with
 * parameters \a g, \a fmt and \a arg to print an error info and then invokes
 * #nvgpu_do_assert to force a failed assert.
 */
#define nvgpu_do_assert_print(g, fmt, arg...)				\
	do {								\
		nvgpu_err(g, fmt, ##arg);				\
		nvgpu_do_assert();					\
		NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 14_4), "SWE-NVGPU-053-SWSADR.docx") \
	} while (false)


struct nvgpu_bug_cb
{
	void (*cb)(void *arg);
	void *arg;
	struct nvgpu_list_node node;
	bool sw_quiesce_data;
};

static inline struct nvgpu_bug_cb *
nvgpu_bug_cb_from_node(struct nvgpu_list_node *node)
{
	return (struct nvgpu_bug_cb *)
		((uintptr_t)node - offsetof(struct nvgpu_bug_cb, node));
};

#ifdef __KERNEL__
static inline void nvgpu_bug_exit(void) { }
static inline void nvgpu_bug_register_cb(struct nvgpu_bug_cb *cb) { }
static inline void nvgpu_bug_unregister_cb(struct nvgpu_bug_cb *cb) { }
#endif

#endif /* NVGPU_BUG_H */
