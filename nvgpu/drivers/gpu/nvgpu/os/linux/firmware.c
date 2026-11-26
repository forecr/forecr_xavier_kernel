// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/firmware.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

#include <nvgpu/kmem.h>
#include <nvgpu/bug.h>
#include <nvgpu/firmware.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>

#include "platform_gk20a.h"
#include "os_linux.h"

struct nvgpu_firmware_work {
	struct work_struct work;
	struct device *dev;
	const char *prefix;
	const char *fw_name;
	u32 flags;
	const struct firmware *fw;
};

static const struct firmware *do_request_firmware_sync(struct device *dev,
		const char *prefix, const char *fw_name, u32 flags)
{
	const struct firmware *fw;
	char *fw_path = NULL;
	int path_len, err;

	if (prefix) {
		path_len = strlen(prefix) + strlen(fw_name);
		path_len += 2; /* for the path separator and zero terminator*/

		fw_path = nvgpu_kzalloc(get_gk20a(dev),
					sizeof(*fw_path) * path_len);
		if (!fw_path)
			return NULL;

		(void) sprintf(fw_path, "%s/%s", prefix, fw_name);
		fw_name = fw_path;
	}

	if (flags & NVGPU_REQUEST_FIRMWARE_NO_WARN)
		err = request_firmware_direct(&fw, fw_name, dev);
	else
		err = request_firmware(&fw, fw_name, dev);

	nvgpu_kfree(get_gk20a(dev), fw_path);
	if (err)
		return NULL;
	return fw;
}

static void nvgpu_firmware_worker(struct work_struct *work)
{
	struct nvgpu_firmware_work *fw_work = container_of(work,
			struct nvgpu_firmware_work, work);

	fw_work->fw = do_request_firmware_sync(fw_work->dev,
			fw_work->prefix, fw_work->fw_name, fw_work->flags);
}

static const struct firmware *do_request_firmware(struct device *dev,
		const char *prefix, const char *fw_name, u32 flags)
{
	struct nvgpu_firmware_work fw_work = {
		.dev = dev,
		.prefix = prefix,
		.fw_name = fw_name,
		.flags = flags,
		.fw = NULL,
	};
	const struct firmware *fw = NULL;

	/* Only pathname lookup callers need a worker to isolate VFS state. */
	if (current->nameidata == NULL)
		return do_request_firmware_sync(dev, prefix, fw_name, flags);

	if (get_device(dev) == NULL)
		return NULL;

	/*
	 * Firmware loading can be nested below a userspace file open. Run the
	 * lookup from another task so it does not inherit the outer name lookup
	 * state, including its accumulated symbolic-link count.
	 */
	INIT_WORK_ONSTACK(&fw_work.work, nvgpu_firmware_worker);
	(void) queue_work(system_unbound_wq, &fw_work.work);
	flush_work(&fw_work.work);
	fw = fw_work.fw;
	put_device(dev);
	destroy_work_on_stack(&fw_work.work);

	return fw;
}

/* This is a simple wrapper around request_firmware that takes 'fw_name' and
 * applies an IP specific relative path prefix to it. The caller is
 * responsible for calling nvgpu_release_firmware later. */
struct nvgpu_firmware *nvgpu_request_firmware(struct gk20a *g,
					      const char *fw_name,
					      u32 flags)
{
	struct device *dev = dev_from_gk20a(g);
	struct nvgpu_firmware *fw;
	const struct firmware *linux_fw = NULL;
	char *prefix_path = NULL;
	int prefix_len = 0;

	/* current->fs is NULL when calling from SYS_EXIT.
	   Add a check here to prevent crash in request_firmware */
	if (!current->fs || !fw_name)
		return NULL;

	fw = nvgpu_kzalloc(g, sizeof(*fw));
	if (!fw)
		return NULL;

	/* Search GPU firmware under /lib/firmware/nvidia */
	prefix_len = strlen("nvidia") + strlen(g->name);
	prefix_len += 2; /* for the path separator and zero terminator */
	prefix_path = nvgpu_kzalloc(g, sizeof(*prefix_path) * prefix_len);
	if (prefix_path) {
		(void) snprintf(prefix_path, prefix_len,
						"%s/%s", "nvidia", g->name);
		linux_fw = do_request_firmware(dev,
			prefix_path, fw_name, NVGPU_REQUEST_FIRMWARE_NO_WARN);
		nvgpu_kfree(g, prefix_path);
	}

	if (!linux_fw)
		linux_fw = do_request_firmware(dev, g->name, fw_name, flags);

#ifdef CONFIG_TEGRA_GK20A
	/* TO BE REMOVED - Support loading from legacy SOC specific path. */
	if (!linux_fw && !(flags & NVGPU_REQUEST_FIRMWARE_NO_SOC)) {
		struct gk20a_platform *platform = gk20a_get_platform(dev);
		linux_fw = do_request_firmware(dev,
				platform->soc_name, fw_name, flags);
	}
#endif

	if (!linux_fw)
		goto err;

	fw->data = nvgpu_kmalloc(g, linux_fw->size);
	if (!fw->data)
		goto err_release;

	nvgpu_memcpy((u8 *)fw->data, (u8 *)linux_fw->data, linux_fw->size);
	fw->size = linux_fw->size;

	release_firmware(linux_fw);

	return fw;

err_release:
	release_firmware(linux_fw);
err:
	nvgpu_kfree(g, fw);
	return NULL;
}

void nvgpu_release_firmware(struct gk20a *g, struct nvgpu_firmware *fw)
{
	if(!fw)
		return;

	nvgpu_kfree(g, fw->data);
	nvgpu_kfree(g, fw);
}
