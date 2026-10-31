// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/nvgpu_init.h>

#include "debug_bios.h"
#include "os_linux.h"

#include <linux/debugfs.h>
#include <linux/uaccess.h>

static int bios_version_show(struct seq_file *s, void *unused)
{
	struct gk20a *g = s->private;
	int err;

	err = gk20a_busy(g);
	if (err != 0) {
		return err;
	}

	seq_printf(s, "Version %02X.%02X.%02X.%02X.%02X\n",
		(g->bios->vbios_version >> 24) & 0xFF,
		(g->bios->vbios_version >> 16) & 0xFF,
		(g->bios->vbios_version >> 8) & 0xFF,
		(g->bios->vbios_version >> 0) & 0xFF,
		(g->bios->vbios_oem_version) & 0xFF);

	gk20a_idle(g);

	return 0;
}

static int bios_version_open(struct inode *inode, struct file *file)
{
	return single_open(file, bios_version_show, inode->i_private);
}

static const struct file_operations bios_version_fops = {
	.open = bios_version_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


int nvgpu_bios_debugfs_init(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct dentry *gpu_root = l->debugfs;

	debugfs_create_file("bios", S_IRUGO,
			    gpu_root, g,
			    &bios_version_fops);

	return 0;
}
