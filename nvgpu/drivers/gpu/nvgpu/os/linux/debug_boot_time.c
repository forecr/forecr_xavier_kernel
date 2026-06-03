// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <nvgpu/nvgpu_init.h>

#include "debug_boot_time.h"
#include "os_linux.h"

extern u64 nvgpu_init_time_ns;

static int nvgpu_boot_time_debugfs_show(struct seq_file *s, void *unused)
{
	struct gk20a *g = s->private;
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	u64 init_time_ms = 0ULL, probe_time_ms = 0ULL, poweron_time_ms = 0ULL;

	init_time_ms = (nvgpu_init_time_ns / 1000000ULL);
	poweron_time_ms = (l->poweron_time_ns / 1000000ULL);
	/*
	 * For GPU as PCIe device, probe is called from init, and therefore init
	 * already includes time elapsed for probe.
	 */
	if (l->probe_time_ns)
		probe_time_ms = (l->probe_time_ns / 1000000ULL);

	seq_printf(s, "Total: %llu (ms)\n", (init_time_ms + probe_time_ms + poweron_time_ms));
	seq_printf(s, "\t Init: %llu (ms)\n", init_time_ms);
	if (probe_time_ms)
		seq_printf(s, "\t Probe: %llu (ms)\n", probe_time_ms);
	seq_printf(s, "\t Poweron: %llu (ms)\n", poweron_time_ms);

	return 0;
}

static int nvgpu_boot_time_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvgpu_boot_time_debugfs_show, inode->i_private);
}

static const struct file_operations nvgpu_boot_time_debugfs_fops = {
	.open		= nvgpu_boot_time_debugfs_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

void nvgpu_boot_time_debugfs_init(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);

	debugfs_create_file("boot_time_ms", 0444, l->debugfs,
		g, &nvgpu_boot_time_debugfs_fops);
}
