// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.


#include <nvgpu/nvgpu_init.h>

#include "debug_gsp_vm_sched.h"
#include "os_linux.h"

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include <nvgpu/vgpu/tegra_vgpu.h>
#include "common/vgpu/gsp_scheduler/gsp_vm_scheduler_vf.h"

#define NODE_NAME_MAX         (64U)

struct gsp_vm_sched_debugfs_ctx {
	struct gk20a *g;
	u32 vmid;
	enum nvgpu_fifo_engine engine_type;
	u32 instance;
	char node_name[NODE_NAME_MAX];
};

struct gsp_vm_sched_debugfs_engine_info {
	bool is_supported;
	char name[NODE_NAME_MAX];
};

struct gsp_vm_sched_debugfs_engine_info gsp_vm_sched_debugfs_engines[NVGPU_ENGINE_INVAL] = {
	[NVGPU_ENGINE_GR]    = { .is_supported = true, .name = "gr" },
	[NVGPU_ENGINE_NVENC] = { .is_supported = true, .name = "nvenc" },
	[NVGPU_ENGINE_OFA]   = { .is_supported = true, .name = "ofa" },
	[NVGPU_ENGINE_NVDEC] = { .is_supported = true, .name = "nvdec" },
	[NVGPU_ENGINE_NVJPG] = { .is_supported = true, .name = "nvjpg" },
};

static bool gsp_vm_sched_debugfs_engine_supported(enum nvgpu_fifo_engine engine_type)
{
	if ((engine_type < NVGPU_ENGINE_INVAL) &&
		gsp_vm_sched_debugfs_engines[engine_type].is_supported) {
		return true;
	} else {
		return false;
	}
}

static int gsp_vm_sched_debugfs_config_show(struct seq_file *s, void *unused)
{
	struct gk20a *g = s->private;
	struct tegra_vgpu_vf_gsp_vm_sched_config *vm_sched_config;
	char engine_name[NODE_NAME_MAX];
	u32 entry, engine_type;
	int err;

	/* Allocate memory for vm shched config to avoid stack overflow */
	vm_sched_config = nvgpu_kzalloc(g, sizeof(*vm_sched_config));
	if (!vm_sched_config) {
		return -ENOMEM;
	}

	err = vf_gsp_vm_sched_get_config(g, vm_sched_config);
	if (err) {
		nvgpu_err(g, "vf_gsp_vm_sched_set_eng_timeslice failed: %d", err);
		return err;
	}

	seq_printf(s, "-------------------------------------------------\n");
	seq_printf(s, "               GSP VM SCHED CONFIG\n");
	seq_printf(s, "-------------------------------------------------\n");
	seq_printf(s, "Min timeslice: %u (us)\n", vm_sched_config->min_timeslice);
	seq_printf(s, "Max timeslice: %u (us)\n", vm_sched_config->max_timeslice);
	seq_printf(s, "    Timeslice Table\n");
	seq_printf(s, "Vmid\tEngine\tTimeslice(us)\n");
	seq_printf(s, "---------------------------\n");

	for (entry = 0; entry < vm_sched_config->count; entry++) {
		engine_type = vm_sched_config->eng_params[entry].engine_type;
		if (!gsp_vm_sched_debugfs_engine_supported(engine_type)) {
			continue;
		}
		snprintf(engine_name, NODE_NAME_MAX, "%s%d",
					gsp_vm_sched_debugfs_engines[engine_type].name,
					vm_sched_config->eng_params[entry].instance);
		seq_printf(s, "%u\t%s\t%u\n", vm_sched_config->eng_params[entry].vmid,
					engine_name, vm_sched_config->eng_params[entry].timeslice);
	}

	nvgpu_kfree(g, vm_sched_config);
	return 0;
}

static int gsp_vm_sched_debugfs_config_open(struct inode *inode, struct file *file)
{
	return single_open(file, gsp_vm_sched_debugfs_config_show, inode->i_private);
}

static const struct file_operations nvgpu_gsp_vm_sched_debugfs_config_fops = {
	.open		= gsp_vm_sched_debugfs_config_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static ssize_t gsp_vm_sched_read(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct gsp_vm_sched_debugfs_ctx *debugfs_ctx = file->private_data;
	struct gk20a *g = debugfs_ctx->g;
	char buf[32];
	int len;
	int err;
	struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params params;

	params.vmid = debugfs_ctx->vmid;
	params.engine_type = debugfs_ctx->engine_type;
	params.instance = debugfs_ctx->instance;

	err = vf_gsp_vm_sched_get_eng_timeslice(g, &params);
	if (err) {
		nvgpu_err(g, "vf_gsp_vm_sched_get_eng_timeslice failed: %d", err);
		return err;
	}

	len = snprintf(buf, sizeof(buf), "%d\n", params.timeslice);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gsp_vm_sched_write(struct file *file,
				   const char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct gsp_vm_sched_debugfs_ctx *debugfs_ctx = file->private_data;
	struct gk20a *g = debugfs_ctx->g;
	char buf[32];
	int err;
	struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params params;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	buf[count] = '\0';
	if (kstrtouint(buf, 10, &params.timeslice) != 0)
		return -EINVAL;

	params.vmid = debugfs_ctx->vmid;
	params.engine_type = debugfs_ctx->engine_type;
	params.instance = debugfs_ctx->instance;

	err = vf_gsp_vm_sched_set_eng_timeslice(g, &params);
	if (err) {
		nvgpu_err(g, "vf_gsp_vm_sched_set_eng_timeslice failed: %d", err);
		return err;
	}

	return count;
}

static const struct file_operations nvgpu_gsp_vm_sched_debugfs_fops = {
	.open = simple_open,
	.read = gsp_vm_sched_read,
	.write = gsp_vm_sched_write,
};

int nvgpu_gsp_vm_sched_debugfs_init(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct dentry *gpu_root = l->debugfs;
	struct dentry *gsp_vm[TEGRA_VGPU_NUM_VM_MAX], *d;
	int err = 0;
	struct tegra_vgpu_vf_gsp_vm_sched_config *vm_sched_config;
	u32 vmid, entry, engine_type;
	struct gsp_vm_sched_debugfs_ctx *debugfs_ctx;
	char vm_dir_name[NODE_NAME_MAX];

	if (!nvgpu_is_vf(g) || !vf_gsp_vm_sched_control_allowed(g)) {
		return 0;
	}

	if (!gpu_root) {
		err = -ENODEV;
		goto exit;
	}

	d = debugfs_create_dir("gsp_vm_sched", gpu_root);
	if (IS_ERR(l->debugfs_gsp_vm_sched)) {
		err = PTR_ERR(d);
		goto exit;
	}

	l->debugfs_gsp_vm_sched = d;

	nvgpu_log(g, gpu_dbg_info, "g=%p", g);

	/* Allocate memory for vm shched config to avoid stack overflow */
	vm_sched_config = nvgpu_kzalloc(g, sizeof(*vm_sched_config));
	if (!vm_sched_config) {
		return -ENOMEM;
	}

	err = vf_gsp_vm_sched_get_config(g, vm_sched_config);
	if (err) {
		nvgpu_err(g, "vf_gsp_vm_sched_set_eng_timeslice failed: %d", err);
		return err;
	}

	d = debugfs_create_file("config", 0644, l->debugfs_gsp_vm_sched, g,
					&nvgpu_gsp_vm_sched_debugfs_config_fops);
	if (IS_ERR(d)) {
		nvgpu_err(g, "%s debugfs node creation failed", "config");
	} else {
		nvgpu_log(g, gpu_dbg_gsp, "%s debugfs node created", "config");
	}

	/* Allocate private memory for all nodes */
	debugfs_ctx = nvgpu_kzalloc(g, vm_sched_config->count * sizeof(struct gsp_vm_sched_debugfs_ctx));
	if (!debugfs_ctx) {
		return -ENOMEM;
	}
	l->gsp_vm_sched_debugfs_ptr = debugfs_ctx;

	for (entry = 0; entry < vm_sched_config->count; entry++) {
		vmid = vm_sched_config->eng_params[entry].vmid;
		if (vmid >= TEGRA_VGPU_NUM_VM_MAX) {
			nvgpu_err(g, "Invalid vmid %u in the entry", vmid);
			err = -EINVAL;
		}

		if (gsp_vm[vmid] == NULL) {
			snprintf(vm_dir_name, NODE_NAME_MAX, "vm%d_timeslice", vmid);
			gsp_vm[vmid] = debugfs_create_dir(vm_dir_name, l->debugfs_gsp_vm_sched);

			if (IS_ERR(d)) {
				nvgpu_err(g, "%s debugfs node creation failed", vm_dir_name);
			} else {
				nvgpu_log(g, gpu_dbg_gsp, "%s debugfs node created", vm_dir_name);
			}
		}

		engine_type = vm_sched_config->eng_params[entry].engine_type;
		if (!gsp_vm_sched_debugfs_engine_supported(engine_type)) {
			continue;
		}

		debugfs_ctx[entry].g = g;
		debugfs_ctx[entry].vmid = vm_sched_config->eng_params[entry].vmid;
		debugfs_ctx[entry].engine_type = engine_type;
		debugfs_ctx[entry].instance = vm_sched_config->eng_params[entry].instance;
		snprintf(debugfs_ctx[entry].node_name, NODE_NAME_MAX, "%s%d_us",
					gsp_vm_sched_debugfs_engines[engine_type].name,
					debugfs_ctx[entry].instance);

		d = debugfs_create_file(debugfs_ctx[entry].node_name, 0644, gsp_vm[vmid],
					&debugfs_ctx[entry], &nvgpu_gsp_vm_sched_debugfs_fops);
		if (IS_ERR(d)) {
			nvgpu_err(g, "%s debugfs node creation failed", debugfs_ctx[entry].node_name);
		} else {
			nvgpu_log(g, gpu_dbg_gsp, "%s debugfs node created",
							debugfs_ctx[entry].node_name);
		}
	}

exit:
	nvgpu_kfree(g, vm_sched_config);
	return err;
}

void nvgpu_gsp_vm_sched_debugfs_free(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);

	if (nvgpu_is_vf(g) && vf_gsp_vm_sched_control_allowed(g)) {
		nvgpu_kfree(g, l->gsp_vm_sched_debugfs_ptr);
	}
}
