// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

/*
 * Virtualized GPU for Linux
 */

#include <linux/pm_runtime.h>
#include <linux/platform_device.h>
#ifdef CONFIG_NVGPU_TEGRA_FUSE
#include <linux/version.h>
#endif

#include <nvgpu/soc.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/vgpu/os_init_hal_vgpu.h>

#include "vgpu_linux.h"
#include "common/vgpu/ivc/comm_vgpu.h"
#include "common/vgpu/intr/intr_vgpu.h"

#include "os/linux/os_linux.h"
#include "os/linux/ioctl.h"
#include "os/linux/scale.h"
#include "os/linux/platform_gk20a.h"
#include "os/linux/dmabuf_priv.h"

#include "vgpu_common.h"

#ifdef CONFIG_GK20A_PM_QOS

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)

static int vgpu_qos_notify(struct notifier_block *nb,
			  unsigned long n, void *data)
{
	struct gk20a_scale_profile *profile =
			container_of(nb, struct gk20a_scale_profile,
			qos_notify_block);
	struct gk20a *g = get_gk20a(profile->dev);
	u64 max_freq;
	int err;

	nvgpu_log_fn(g, " ");

	max_freq = (u64)pm_qos_read_max_bound(PM_QOS_GPU_FREQ_BOUNDS) * 1000UL;
	err = vgpu_plat_clk_cap_rate(profile->dev, max_freq);
	if (err)
		nvgpu_err(g, "%s failed, err=%d", __func__, err);

	return NOTIFY_OK; /* need notify call further */
}

static int vgpu_pm_qos_init(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	struct gk20a_scale_profile *profile = g->scale_profile;

	if (IS_ENABLED(CONFIG_GK20A_DEVFREQ)) {
		if (!profile)
			return -EINVAL;
	} else {
		profile = nvgpu_kzalloc(g, sizeof(*profile));
		if (!profile)
			return -ENOMEM;
		g->scale_profile = profile;
	}

	profile->dev = dev;
	profile->qos_notify_block.notifier_call = vgpu_qos_notify;
	pm_qos_add_max_notifier(PM_QOS_GPU_FREQ_BOUNDS,
				&profile->qos_notify_block);
	return 0;
}

static void vgpu_pm_qos_remove(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);

	pm_qos_remove_max_notifier(PM_QOS_GPU_FREQ_BOUNDS,
				&g->scale_profile->qos_notify_block);
	nvgpu_kfree(g, g->scale_profile);
	g->scale_profile = NULL;
}

#endif
#endif

static int vgpu_pm_init(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	int err = 0;

	nvgpu_log_fn(g, " ");

	if (nvgpu_platform_is_simulation(g))
		return 0;

	__pm_runtime_disable(dev, false);

	if (IS_ENABLED(CONFIG_GK20A_DEVFREQ))
		gk20a_scale_init(dev);

#ifdef CONFIG_GK20A_PM_QOS
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	err = vgpu_pm_qos_init(dev);
	if (err)
		return err;
#endif
#endif

	return err;
}

int vgpu_probe(struct platform_device *pdev)
{
	struct nvgpu_os_linux *l;
	struct gk20a *gk20a;
	int err;
	struct device *dev = &pdev->dev;
	struct gk20a_platform *platform = gk20a_get_platform(dev);

	if (!platform) {
		dev_err(dev, "no platform data\n");
		return -ENODATA;
	}

	l = devm_kzalloc(dev, sizeof(*l), GFP_KERNEL);
	if (!l) {
		dev_err(dev, "couldn't allocate gk20a support");
		return -ENOMEM;
	}
	gk20a = &l->g;
	gk20a->log_mask = NVGPU_DEFAULT_DBG_MASK;

	nvgpu_log_fn(gk20a, " ");

	l->dev = dev;

	/*
	 * A default of 16GB is the largest supported DMA size that is
	 * acceptable to all currently supported Tegra SoCs.
	 */
	if (!platform->dma_mask)
		platform->dma_mask = DMA_BIT_MASK(34);

	err = vgpu_probe_common(l);
	if (err) {
		nvgpu_err(gk20a, "common probe failed, err=%d", err);
		return err;
	}

	err = vgpu_pm_init(dev);
	if (err) {
		nvgpu_err(gk20a, "pm init failed");
		return err;
	}

	vgpu_create_sysfs(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	nvgpu_log_info(gk20a, "total ram pages : %lu", totalram_pages());
#else
	nvgpu_log_info(gk20a, "total ram pages : %lu", totalram_pages);
#endif
#ifdef CONFIG_NVGPU_COMPRESSION
	gk20a->max_comptag_mem = totalram_size_in_mb;
#endif

	return 0;
}

int vgpu_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct gk20a *g = get_gk20a(dev);
#ifdef CONFIG_NVGPU_COMPRESSION
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
#endif

	nvgpu_log_fn(g, " ");

#ifdef CONFIG_NVGPU_COMPRESSION
	gk20a_dma_buf_priv_list_clear(l);
	nvgpu_mutex_destroy(&l->dmabuf_priv_list_lock);
#endif

#ifdef CONFIG_GK20A_PM_QOS
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	vgpu_pm_qos_remove(dev);
#endif
#endif
	if (g->remove_support)
		g->remove_support(g);

	vgpu_comm_deinit();
	gk20a_sched_ctrl_cleanup(g);
	gk20a_user_nodes_deinit(dev);
	vgpu_remove_sysfs(dev);
	gk20a_get_platform(dev)->g = NULL;
	nvgpu_put(g);

	return 0;
}

int vgpu_tegra_suspend(struct device *dev)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct gk20a *g = get_gk20a(dev);
	int err = 0;

	msg.cmd = TEGRA_VGPU_CMD_SUSPEND;
	msg.handle = vgpu_get_handle(g);
	err = vgpu_comm_sendrecv(&msg, sizeof(msg), sizeof(msg));
	err = err ? err : msg.ret;
	if (err)
		nvgpu_err(g, "vGPU suspend failed\n");

	return err;
}

int vgpu_tegra_resume(struct device *dev)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct gk20a *g = get_gk20a(dev);
	int err = 0;

	msg.cmd = TEGRA_VGPU_CMD_RESUME;
	msg.handle = vgpu_get_handle(g);
	err = vgpu_comm_sendrecv(&msg, sizeof(msg), sizeof(msg));
	err = err ? err : msg.ret;
	if (err)
		nvgpu_err(g, "vGPU resume failed\n");

	return err;
}

int vgpu_init_hal_os(struct gk20a *g)
{
	return 0;
}
