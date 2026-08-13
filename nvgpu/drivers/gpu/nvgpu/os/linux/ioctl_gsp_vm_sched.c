// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.


#include <uapi/linux/nvgpu.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/enabled.h>
#include <nvgpu/nvgpu_init.h>

#include "platform_gk20a.h"
#include "os_linux.h"
#include "ioctl.h"

#include <nvgpu/vgpu/tegra_vgpu.h>
#include "common/vgpu/gsp_scheduler/gsp_vm_scheduler_vf.h"
#include "ioctl_gsp_vm_sched.h"

static int nvgpu_gsp_vm_sched_ioctl_get_config(struct gk20a *g,
			struct nvgpu_vm_sched_config_args *args)
{
	struct tegra_vgpu_vf_gsp_vm_sched_config *vm_sched_config;
	struct nvgpu_vm_sched_engine_timeslice_args *temp_buf;
	int err = 0;
	u32 entry, engine_id;
	size_t write_size;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gsp, "get_config");

	/* Allocate memory for vm shched config */
	vm_sched_config = nvgpu_kzalloc(g, sizeof(*vm_sched_config));
	if (!vm_sched_config) {
		return -ENOMEM;
	}

	err = vf_gsp_vm_sched_get_config(g, vm_sched_config);
	if (err) {
		nvgpu_err(g, "vf_gsp_vm_sched_get_config failed: %d", err);
		goto free_vm_sched;
	}

	args->min_timeslice = vm_sched_config->min_timeslice;
	args->max_timeslice = vm_sched_config->max_timeslice;
	args->num_timeslice_entries = vm_sched_config->count;

	if ((args->buffer == 0) || (args->size == 0)) {
		goto free_vm_sched;
	}

	write_size = vm_sched_config->count * sizeof(struct nvgpu_vm_sched_engine_timeslice_args);
	if (args->size < write_size) {
		nvgpu_err(g, "Size mismatch, expected %lu got %u", write_size, args->size);
		err = -EINVAL;
		goto free_vm_sched;
	}

	temp_buf = nvgpu_kzalloc(g, write_size);
	if (temp_buf == NULL) {
		err = -ENOMEM;
		goto free_vm_sched;
	}

	for (entry = 0; entry < vm_sched_config->count; entry++) {
		engine_id = nvgpu_fifo_engine_to_upi_engine_id(
					vm_sched_config->eng_params[entry].engine_type);
		if (engine_id == NVGPU_GPU_ENGINE_ID_MAX) {
			err = -ENODEV;
			nvgpu_err(g, "Got invalid engine type");
			goto free_temp_buf;
		}
		temp_buf[entry].vmid = vm_sched_config->eng_params[entry].vmid;
		temp_buf[entry].engine_id = engine_id;
		temp_buf[entry].instance = vm_sched_config->eng_params[entry].instance;
		temp_buf[entry].timeslice = vm_sched_config->eng_params[entry].timeslice;
	}

	err = copy_to_user((void __user *)(uintptr_t)args->buffer,
			   temp_buf, write_size);

free_temp_buf:
	nvgpu_kfree(g, temp_buf);
free_vm_sched:
	nvgpu_kfree(g, vm_sched_config);

	return err;
}

static int nvgpu_gsp_vm_sched_ioctl_get_eng_timeslice(struct gk20a *g,
				struct nvgpu_vm_sched_engine_timeslice_args *args)
{
	struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params params;
	int err = 0;
	u32 fifo_engine_type;

	fifo_engine_type = nvgpu_uapi_engine_id_to_fifo_engine(args->engine_id);
	if (fifo_engine_type == NVGPU_ENGINE_INVAL) {
		nvgpu_err(g, "Got invalid engine type");
		return -EINVAL;
	}

	params.vmid = args->vmid;
	params.engine_type = fifo_engine_type;
	params.instance = args->instance;

	err = vf_gsp_vm_sched_get_eng_timeslice(g, &params);
	if (err) {
		nvgpu_err(g, "vf_gsp_vm_sched_get_eng_timeslice failed: %d", err);
		return err;
	}
	args->timeslice = params.timeslice;
	return 0;
}

static int nvgpu_gsp_vm_sched_ioctl_set_eng_timeslice(struct gk20a *g,
				struct nvgpu_vm_sched_engine_timeslice_args *args)
{
	struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params params;
	int err = 0;
	u32 fifo_engine_type;

	fifo_engine_type = nvgpu_uapi_engine_id_to_fifo_engine(args->engine_id);
	if (fifo_engine_type == NVGPU_ENGINE_INVAL) {
		nvgpu_err(g, "Got invalid engine type");
		return -EINVAL;
	}

	params.vmid = args->vmid;
	params.engine_type = fifo_engine_type;
	params.instance = args->instance;
	params.timeslice = args->timeslice;

	err = vf_gsp_vm_sched_set_eng_timeslice(g, &params);
	if (err) {
		nvgpu_err(g, "vf_gsp_vm_sched_set_eng_timeslice failed: %d", err);
		return err;
	}

	return 0;
}

int nvgpu_gsp_vm_sched_dev_open(struct inode *inode, struct file *filp)
{
	struct gk20a *g;
	struct nvgpu_cdev *cdev;

	cdev = container_of(inode->i_cdev, struct nvgpu_cdev, cdev);
	g = nvgpu_get_gk20a_from_cdev(cdev);

	if (!nvgpu_is_vf(g) || !vf_gsp_vm_sched_control_allowed(g)) {
		return -EPERM;
	}

	filp->private_data = g;
	nvgpu_log(g, gpu_dbg_gsp, "filp=%p g=%p", filp, g);

	return 0;
}

int nvgpu_gsp_vm_sched_dev_release(struct inode *inode, struct file *filp)
{
	struct gk20a *g = filp->private_data;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gsp, "dev release");

	return 0;
}

long nvgpu_gsp_vm_sched_dev_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	struct gk20a *g = filp->private_data;
	void *buf = NULL;
	int err = 0;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gsp, "nr=%d", _IOC_NR(cmd));

	if ((_IOC_TYPE(cmd) != NVGPU_VM_SCHED_IOCTL_MAGIC) ||
		(_IOC_NR(cmd) == 0) ||
		(_IOC_NR(cmd) > NVGPU_VM_SCHED_IOCTL_LAST) ||
		(_IOC_SIZE(cmd) > NVGPU_VM_SCHED_IOCTL_MAX_ARG_SIZE)) {
		return -EINVAL;
	}

	buf = nvgpu_kzalloc(g, NVGPU_VM_SCHED_IOCTL_MAX_ARG_SIZE);
	if (buf == NULL) {
		return -ENOMEM;
	}

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (copy_from_user(buf, (void __user *) arg, _IOC_SIZE(cmd))) {
			err = -EFAULT;
			goto cleanup;
		}
	}

	switch (cmd) {
	case NVGPU_VM_SCHED_IOCTL_GET_CONFIG:
		err = nvgpu_gsp_vm_sched_ioctl_get_config(g,
			(struct nvgpu_vm_sched_config_args *) buf);
		break;
	case NVGPU_VM_SCHED_IOCTL_GET_ENG_TIMESLICE:
		err = nvgpu_gsp_vm_sched_ioctl_get_eng_timeslice(g,
			(struct nvgpu_vm_sched_engine_timeslice_args *) buf);
		break;
	case NVGPU_VM_SCHED_IOCTL_SET_ENG_TIMESLICE:
		err = nvgpu_gsp_vm_sched_ioctl_set_eng_timeslice(g,
			(struct nvgpu_vm_sched_engine_timeslice_args *) buf);
		break;
	default:
		dev_dbg(dev_from_gk20a(g), "unrecognized gpu ioctl cmd: 0x%x",
			cmd);
		err = -ENOTTY;
	}

	if ((err == 0) && (_IOC_DIR(cmd) & _IOC_READ)) {
		err = copy_to_user((void __user *) arg, buf, _IOC_SIZE(cmd));
	}

cleanup:
	nvgpu_kfree(g, buf);
	return err;
}
