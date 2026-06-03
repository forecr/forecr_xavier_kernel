// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "platform_gk20a.h"
#include "debug_multimedia.h"
#include "os_linux.h"

#include <nvgpu/nvgpu_init.h>
#include <nvgpu/device.h>
#include <nvgpu/multimedia.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>

#define NODE_NAME_MAX         (64U)

struct nvgpu_multimedia_debugfs_ctx {
	struct gk20a *g;
	u32 mm_engine;
	char node_name[NODE_NAME_MAX];
};

int nvgpu_multimedia_debug_common(struct gk20a *g, const struct nvgpu_device *dev,
				struct nvgpu_debug_context *o)
{
	int err = 0;

	if (dev == NULL) {
		gk20a_debug_output(o, "engine not supported");
		return -EINVAL;
	}

	switch (dev->type) {
	case NVGPU_DEVTYPE_NVENC:
		if (g->ops.nvenc.dump_engine_status) {
			g->ops.nvenc.dump_engine_status(g, dev->inst_id, o);
		}
		break;
	case NVGPU_DEVTYPE_OFA:
		if (g->ops.ofa.dump_engine_status) {
			g->ops.ofa.dump_engine_status(g, dev->inst_id, o);
		}
		break;
	case NVGPU_DEVTYPE_NVDEC:
		if (g->ops.nvdec.dump_engine_status) {
			g->ops.nvdec.dump_engine_status(g, dev->inst_id, o);
		}
		break;
	case NVGPU_DEVTYPE_NVJPG:
		if (g->ops.nvjpg.dump_engine_status) {
			g->ops.nvjpg.dump_engine_status(g, dev->inst_id, o);
		}
		break;
	default:
		gk20a_debug_output(o, "invalid engine type %u", dev->type);
		err = -EINVAL;
		break;
	}

	return err;
}

static int nvgpu_multimedia_debug_show(struct seq_file *s, void *unused)
{
	struct nvgpu_multimedia_debugfs_ctx *ctx = s->private;
	struct gk20a *g = ctx->g;
	const struct nvgpu_device *dev = NULL;
	struct nvgpu_debug_context o = {
		.fn = gk20a_debug_write_to_seqfile,
		.ctx = s,
	};
	int err = 0;

	err = gk20a_busy(g);
	if (err) {
		nvgpu_err(g, "failed to power on gpu: %d", err);
		return err;
	}

	dev = nvgpu_multimedia_get_dev(g, ctx->mm_engine);
	err = nvgpu_multimedia_debug_common(g, dev, &o);

	gk20a_idle(g);

	return err;
}

static int nvgpu_multimedia_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvgpu_multimedia_debug_show, inode->i_private);
}

#define MULTIMEDIA_DEBUGFS_OPS { \
		.open		= nvgpu_multimedia_debugfs_open, \
		.read		= seq_read, \
		.llseek		= seq_lseek, \
		.release	= single_release, \
	}

static const struct file_operations multimedia_debug_fops[NVGPU_MULTIMEDIA_ENGINE_MAX] = {
				[0 ... (NVGPU_MULTIMEDIA_ENGINE_MAX-1)] = MULTIMEDIA_DEBUGFS_OPS };
static struct nvgpu_multimedia_debugfs_ctx multimedia_debugfs_ctxs[NVGPU_MULTIMEDIA_ENGINE_MAX];

int nvgpu_multimedia_debugfs_init(struct gk20a *g)
{
	struct dentry *d;
	u32 mm_engine;
	char *node_name;
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct dentry *gpu_root = l->debugfs;

	l->debugfs_multimedia = debugfs_create_dir("multimedia", gpu_root);
	if (IS_ERR_OR_NULL(l->debugfs_multimedia)) {
		return -ENODEV;
	}

	for (mm_engine = NVGPU_MULTIMEDIA_ENGINE_NVENC; mm_engine < NVGPU_MULTIMEDIA_ENGINE_MAX;
		mm_engine++) {
		node_name = multimedia_debugfs_ctxs[mm_engine].node_name;
		(void) snprintf(node_name, NODE_NAME_MAX,
			"%s_status", nvgpu_multimedia_get_eng_name(mm_engine));
		multimedia_debugfs_ctxs[mm_engine].mm_engine = mm_engine;
		multimedia_debugfs_ctxs[mm_engine].g = g;
		d = debugfs_create_file(node_name, S_IRUGO,
			l->debugfs_multimedia, &multimedia_debugfs_ctxs[mm_engine],
			&multimedia_debug_fops[mm_engine]);
		if (!d) {
			nvgpu_err(g, "%s debugfs node creation failed", node_name);
		} else {
			nvgpu_log(g, gpu_dbg_mme, "%s debugfs node created", node_name);
		}
	}

	return 0;
}
