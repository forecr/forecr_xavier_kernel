// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/anon_inodes.h>
#include <linux/fs.h>
#include <uapi/linux/nvgpu.h>

#include <nvgpu/kmem.h>
#include <nvgpu/log.h>
#include <nvgpu/enabled.h>
#include <nvgpu/sizes.h>
#include <nvgpu/list.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_init.h>

#include "ioctl.h"

#include "power_ops.h"

#include "platform_gk20a.h"
#include "os_linux.h"
#include "module.h"

#define NVGPU_DRIVER_POWER_ON_NEEDED	1
#define NVGPU_DRIVER_POWER_OFF_NEEDED	0

int gk20a_power_open(struct inode *inode, struct file *filp)
{
	struct gk20a *g;
	struct nvgpu_cdev *cdev;

	cdev = container_of(inode->i_cdev, struct nvgpu_cdev, cdev);
	g = nvgpu_get_gk20a_from_cdev(cdev);
	filp->private_data = g;

	g = nvgpu_get(g);
	if (!g) {
		return -ENODEV;
	}

	return 0;
}

ssize_t gk20a_power_read(struct file *filp, char __user *buf,
		size_t size, loff_t *off)
{
	struct gk20a *g = filp->private_data;
	char power_out[3];
	int len;

	if (!g) {
		return -ENODEV;
	}

	len = scnprintf(power_out, sizeof(power_out), "%c\n",
			g->power_on_state ? '1' : '0');

	return simple_read_from_buffer(buf, size, off, power_out, len);
}

ssize_t gk20a_power_write(struct file *filp, const char __user *buf,
		size_t size, loff_t *off)
{
	struct gk20a *g = filp->private_data;
	unsigned char userinput[3] = {0};
	u32 power_status = 0U;
	int err = 0;

	if (!g) {
		return -ENODEV;
	}

	/* Valid inputs are "0", "1", "0\n", "1\n". */
	if (size >= sizeof(userinput)) {
		return -EINVAL;
	}

	if (copy_from_user(userinput, buf, size)) {
		return -EFAULT;
	}

	if (kstrtouint(userinput, 10, &power_status)) {
		return -EINVAL;
	}

	if (power_status == NVGPU_DRIVER_POWER_ON_NEEDED) {
		if (nvgpu_is_powered_on(g)) {
			// Early exit if already powered on.
			goto out;
		}

		err = gk20a_busy(g);
		if (err) {
			nvgpu_err(g, "power_node_write failed at busy");
			return err;
		}

		gk20a_idle(g);
	} else if (power_status == NVGPU_DRIVER_POWER_OFF_NEEDED) {
		err = gk20a_driver_force_power_off(g);
		if (err) {
			nvgpu_err(g, "power_node_write failed at busy");
			return err;
		}
	} else {
		nvgpu_err(g, "1/0 are the valid values to power-on the GPU");
		return -EINVAL;
	}

out:
	*off += size;
	return size;
}

int gk20a_power_release(struct inode *inode, struct file *filp)
{
	struct gk20a *g;
	struct nvgpu_cdev *cdev;

	cdev = container_of(inode->i_cdev, struct nvgpu_cdev, cdev);
	g = nvgpu_get_gk20a_from_cdev(cdev);
	nvgpu_put(g);
	return 0;
}
