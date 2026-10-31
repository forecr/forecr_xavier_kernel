// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/posix/log.h>
#include <nvgpu/types.h>

#include <nvgpu/gk20a.h>

/*
 * Define a length for log buffers. This is the buffer that the 'fmt, ...' part
 * of __nvgpu_do_log_print() prints into.
 */
#define LOG_BUFFER_LENGTH	160

/*
 * Keep this roughly the same as the kernel log format.
 */
#define LOG_FMT			"nvgpu: %s %33s:%-4d [%-4s]  %s\n"

static const char *log_types[] = {
	"ERR",
	"WRN",
	"DBG",
	"INFO",
};

static inline const char *nvgpu_log_name(struct gk20a *g)
{
	(void)g;
	return "gpu.USS";
}

static void __nvgpu_really_print_log(const char *gpu_name,
				     const char *func_name, int line,
				     enum nvgpu_log_type type, const char *log)
{
	const char *name = gpu_name ? gpu_name : "";
	const char *log_type;

	if (type >= NVGPU_ERROR) {
		log_type = log_types[type];
		printf(LOG_FMT, name, func_name, line, log_type, log);
	}
}

__attribute__((format (printf, 5, 6)))
void nvgpu_log_msg_impl(struct gk20a *g, const char *func_name, int line,
			enum nvgpu_log_type type, const char *fmt, ...)
{
	char log[LOG_BUFFER_LENGTH];
	va_list args;

	va_start(args, fmt);
	(void) vsnprintf(log, LOG_BUFFER_LENGTH, fmt, args);
	va_end(args);

	__nvgpu_really_print_log(nvgpu_log_name(g),
				 func_name, line, type, log);
}

__attribute__((format (printf, 5, 6)))
void nvgpu_log_dbg_impl(struct gk20a *g, u64 log_mask,
			const char *func_name, int line,
			const char *fmt, ...)
{
	char log[LOG_BUFFER_LENGTH];
	va_list args;

	if ((log_mask & g->log_mask) == 0) {
		return;
	}

	va_start(args, fmt);
	(void) vsnprintf(log, LOG_BUFFER_LENGTH, fmt, args);
	va_end(args);

	__nvgpu_really_print_log(nvgpu_log_name(g),
				 func_name, line, NVGPU_DEBUG, log);
}
