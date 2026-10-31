/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_DEBUG_H
#define NVGPU_DEBUG_H

#include <nvgpu/types.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/seq_file.h>
#include <nvgpu/log.h>
#endif

struct gk20a;
struct gpu_ops;
struct nvgpu_device;

/** @cond DOXYGEN_SHOULD_SKIP_THIS */

struct nvgpu_debug_context {
	void (*fn)(void *ctx, const char *str);
	void *ctx;
	char buf[256];
};

#ifdef CONFIG_DEBUG_FS
extern unsigned int gk20a_debug_trace_cmdbuf;

__attribute__((format (printf, 2, 3)))
void gk20a_debug_output(struct nvgpu_debug_context *o,
					const char *fmt, ...);

void gk20a_debug_dump(struct gk20a *g);
void gk20a_debug_show_dump(struct gk20a *g, struct nvgpu_debug_context *o);
void gk20a_fecs_gr_debug_dump(struct gk20a *g);
void gk20a_gr_debug_dump(struct gk20a *g);
void nvgpu_multimedia_debug_dump(struct gk20a *g, const struct nvgpu_device *dev);
void gk20a_init_debug_ops(struct gpu_ops *gops);

void gk20a_debug_init(struct gk20a *g, const char *debugfs_symlink);
void gk20a_debug_deinit(struct gk20a *g);

static inline void gk20a_debug_write_printk(void *ctx, const char *str)
{
	struct gk20a *g = ctx;

	nvgpu_dbg_dump_impl(g, str);
}

static inline void gk20a_debug_write_to_seqfile(void *ctx, const char *str)
{
	seq_printf((struct seq_file *)ctx, "%s\n", str);
}
#else
__attribute__((format (printf, 2, 3)))
static inline void gk20a_debug_output(struct nvgpu_debug_context *o,
					const char *fmt, ...)
{
	(void)o;
	(void)fmt;
}

static inline void gk20a_debug_dump(struct gk20a *g)
{
	(void)g;
}

static inline void gk20a_debug_show_dump(struct gk20a *g,
					 struct nvgpu_debug_context *o)
{
	(void)g;
	(void)o;
}

static inline void gk20a_fecs_gr_debug_dump(struct gk20a *g)
{
	(void)g;
}

static inline void gk20a_gr_debug_dump(struct gk20a *g)
{
	(void)g;
}

static inline void nvgpu_multimedia_debug_dump(struct gk20a *g,
						const struct nvgpu_device *dev)
{
	(void)g;
	(void)dev;
}

static inline void gk20a_debug_init(struct gk20a *g, const char *debugfs_symlink)
{
	(void)g;
	(void)debugfs_symlink;
}

static inline void gk20a_debug_deinit(struct gk20a *g)
{
	(void)g;
}

static inline void gk20a_debug_write_printk(void *ctx, const char *str)
{
	(void)ctx;
	(void)str;
}

static inline void gk20a_debug_write_to_seqfile(void *ctx, const char *str)
{
	(void)ctx;
	(void)str;
}
#endif

/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

#endif /* NVGPU_DEBUG_H */
