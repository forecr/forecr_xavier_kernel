/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OS_SCHED_H
#define NVGPU_OS_SCHED_H

#include <nvgpu/log.h>

struct gk20a;

/**
 * @brief Query the id of current thread.
 *
 * Shall return the thread id of the calling thread. Invokes the library
 * function \a pthread_self to get the thread ID of the calling thread.
 * Function does not perform any validation of the parameter.
 *
 * @param g [in]	GPU driver struct.
 *
 * @return Thread ID of the calling thread.
 */
int nvgpu_current_tid(struct gk20a *g);

/**
 * @brief Query the id of current process.
 *
 * Shall return the process id of the calling process. Invokes the library
 * function \a getpid to get the process ID of the calling process. Function
 * does not perform any validation of the parameter.
 *
 * @param g [in]	GPU driver struct.
 *
 * @return Process ID of the calling process.
 */
int nvgpu_current_pid(struct gk20a *g);

/**
 * @brief  API to get the name of current thread.
 *
 * @param dest [in/out]		Pointer to the string buffer.
 */
void nvgpu_get_thread_name(char *dest);

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
/**
 * @brief Print the name of current thread.
 *
 * Implements the printing of the current thread name along with the function
 * and line details. Implementation of this function is OS specific. For QNX,
 * pthread name is printed along with other provided inputs. Invokes the
 * library function \a pthread_getname_np with parameters 0, a local buffer to
 * hold the name and length of the local buffer as parameters. For POSIX build,
 * pthread name is printed only if the build has support for GNU extensions
 * which provides the thread name. Based on the log level indicated by the
 * parameter \a type one of the following functions is invoked with \a g, the
 * format specifier to print and the thread name to print as parameters,
 * - #nvgpu_err()
 * - #nvgpu_warn()
 * - #nvgpu_log(), a value of 0 is used as parameter to indicate the log mask.
 * - #nvgpu_info()
 * Function does not perform any validation of the parameters.
 *
 * @param g [in]		GPU driver struct.
 * @param func_name [in]	Calling function name.
 * @param line [in]		Calling line number.
 * @param ctx [in]		Context pointer.
 * @param type [in]		Log level.
 */
void nvgpu_print_current_impl(struct gk20a *g, const char *func_name, int line,
		void *ctx, enum nvgpu_log_type type);
/**
 * Print the name of calling thread.
 */
#define nvgpu_print_current(g, ctx, type) \
	nvgpu_print_current_impl(g, __func__, __LINE__, ctx, type)
#endif
#endif /* NVGPU_OS_SCHED_H */
