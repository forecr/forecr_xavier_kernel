// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/dma-fence.h>
#include <linux/file.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/sync_file.h>
#include <linux/version.h>

#include <nvgpu/os_fence.h>
#include <nvgpu/os_fence_syncpts.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/gk20a.h>

#include "os_linux.h"

#include "nvhost_priv.h"

#ifdef CONFIG_TEGRA_HOST1X_EMU_DBG_SYMBL
#include <linux/host1x-dispatch.h>
#else
#include <linux/host1x-next.h>
#endif

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_nvhost.h>
#endif

#define TEGRA194_SYNCPT_PAGE_SIZE 0x1000
#define TEGRA194_SYNCPT_SHIM_BASE 0x60000000
#define TEGRA194_SYNCPT_SHIM_SIZE 0x00400000
#define TEGRA234_SYNCPT_PAGE_SIZE 0x10000
#define TEGRA234_SYNCPT_SHIM_BASE 0x60000000
#define TEGRA234_SYNCPT_SHIM_SIZE 0x04000000
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
#define TEGRA264_SYNCPT_PAGE_SIZE (0x10000)
#define TEGRA264_SYNCPT_SHIM_BASE (0x81C0000000)
#define TEGRA264_SYNCPT_SHIM_BASE_DIE2 (0x181C0000000)
#define TEGRA264_SYNCPT_SHIM_SIZE (0x04000000)
#endif

static const struct of_device_id host1x_match[] = {
	{ .compatible = "nvidia,tegra186-host1x", },
	{ .compatible = "nvidia,tegra194-host1x", },
	{ .compatible = "nvidia,tegra234-host1x", },
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	{ .compatible = "nvidia,tegra264-host1x", },
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	{ .compatible = "nvidia,tegraEmu-host1x", },
#endif
	{},
};

int nvgpu_get_nvhost_dev(struct gk20a *g)
{
	u32 i;
	struct platform_device *host1x_pdev = NULL;
	struct device_node *np = NULL;
	const struct of_device_id *host1x_match_rules[2] = {
		host1x_match,
#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		nvgpu_next_of_find_matching_node()
#else
		NULL
#endif
    };

	/*
	 * The code tries to find the host1x with the publicly visible
	 * host1x match pattern. If it fails, it will try to use the nvgpu-next
	 * backend to get the host1x rules and try to find the host1x device
	 * one final time.
	 *
	 * In both searches, the code will check that the GPU and the Host1x
	 * device use the same NUMA node id so that allocated syncpoints are
	 * local to the GPU.
	 *
	 * If host1x is not found, syncpoints are marked as unsupported.
	 */
	for (i = 0; i < ARRAY_SIZE(host1x_match_rules); ++i) {
		if (!host1x_match_rules[i]) {
			continue;
		}
		for_each_matching_node(np, host1x_match_rules[i]) {
			host1x_pdev = of_find_device_by_node(np);
			if (host1x_pdev) {
				if ((strcmp(of_get_property(host1x_pdev->dev.of_node, "compatible", NULL),
					"nvidia,tegraEmu-host1x")) == 0) {
					g->is_host1x_emu = true;
				}
				if (!nvgpu_safe_add_u32_return(g->num_nvhost, 1U, &g->num_nvhost)) {
					nvgpu_err(g, "num_nvhost overflow");
					return -EOVERFLOW;
				}

				break;
			}
		}
		if (host1x_pdev) {
			break;
		}
	}

	if (!host1x_pdev) {
		nvgpu_warn(g, "host1x device not available");
		nvgpu_set_enabled(g, NVGPU_HAS_SYNCPOINTS, false);
		return -ENOSYS;
	}

	nvgpu_info(g, "host1x device found at iteration %u\n", i);

	g->nvhost = nvgpu_kzalloc(g, sizeof(struct nvgpu_nvhost_dev));
	if (!g->nvhost)
		return -ENOMEM;

	g->nvhost->g = g;
	g->nvhost->host1x_pdev = host1x_pdev;

	return 0;
}

int nvgpu_nvhost_module_busy_ext(struct nvgpu_nvhost_dev *nvhost_dev)
{
	return 0;
}

void nvgpu_nvhost_module_idle_ext(struct nvgpu_nvhost_dev *nvhost_dev) { }

void nvgpu_nvhost_debug_dump_device(struct nvgpu_nvhost_dev *nvhost_dev) { }

const char *nvgpu_nvhost_syncpt_get_name(struct nvgpu_nvhost_dev *nvhost_dev,
					 int id)
{
	return NULL;
}

bool nvgpu_nvhost_syncpt_is_valid_pt_ext(struct nvgpu_nvhost_dev *nvhost_dev,
					 u32 id)
{
	struct host1x_syncpt *sp;
	struct host1x *host1x;

	host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
	if (WARN_ON(!host1x))
		return false;
	sp = host1x_syncpt_get_by_id_noref(host1x,
				HOST1X_GLOBAL_TO_LOCAL_SYNCPOINT(id));

	if (!sp)
		return false;

	return true;
}

bool nvgpu_nvhost_syncpt_is_expired_ext(struct nvgpu_nvhost_dev *nvhost_dev,
					u32 id, u32 thresh)
{
	struct host1x_syncpt *sp;
	struct host1x *host1x;
	u32 dummy_value;

	host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
	if (WARN_ON(!host1x))
		return true;

	sp = host1x_syncpt_get_by_id_noref(host1x,
				HOST1X_GLOBAL_TO_LOCAL_SYNCPOINT(id));

	if (WARN_ON(!sp))
		return true;

	if (host1x_syncpt_wait(sp, thresh,
				0, &dummy_value))
		return false;

	return true;
}

struct nvgpu_host1x_cb {
	struct dma_fence_cb cb;
	struct work_struct work;
	void (*notifier)(void *, int);
	void *notifier_data;
};

static void nvgpu_host1x_work_func(struct work_struct *work)
{
	struct nvgpu_host1x_cb *host1x_cb = container_of(work, struct nvgpu_host1x_cb, work);

	host1x_cb->notifier(host1x_cb->notifier_data, 0);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0))
	kfree_rcu_mightsleep(host1x_cb);
#else
	kfree_rcu(host1x_cb);
#endif

}

static void nvgpu_host1x_cb_func(struct dma_fence *f, struct dma_fence_cb *cb)
{
	struct nvgpu_host1x_cb *host1x_cb = container_of(cb, struct nvgpu_host1x_cb, cb);

	schedule_work(&host1x_cb->work);
	dma_fence_put(f);
}

int nvgpu_nvhost_intr_register_notifier(struct nvgpu_nvhost_dev *nvhost_dev,
					u32 id, u32 thresh,
					void (*notifier)(void *, int),
					void *notifier_data)
{
	struct dma_fence *fence;
	struct nvgpu_host1x_cb *cb;
	struct host1x_syncpt *sp;
	struct host1x *host1x;
	int err;

	host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
	if (!host1x)
		return -ENODEV;

	sp = host1x_syncpt_get_by_id_noref(host1x,
			HOST1X_GLOBAL_TO_LOCAL_SYNCPOINT(id));
	if (!sp)
		return -EINVAL;

	fence = host1x_fence_create(sp, thresh, true);
	if (IS_ERR(fence)) {
		pr_err("error %d during construction of fence!",
			(int)PTR_ERR(fence));
		return PTR_ERR(fence);
	}

	cb = kzalloc(sizeof(*cb), GFP_KERNEL);
	if (!cb)
		return -ENOMEM;

	cb->notifier = notifier;
	cb->notifier_data = notifier_data;

	INIT_WORK(&cb->work, nvgpu_host1x_work_func);

	err = dma_fence_add_callback(fence, &cb->cb, nvgpu_host1x_cb_func);
	if (err < 0) {
		dma_fence_put(fence);
		kfree(cb);
	}

	return err;
}

void nvgpu_nvhost_syncpt_set_minval(struct nvgpu_nvhost_dev *nvhost_dev,
				    u32 id, u32 val)
{
	struct host1x_syncpt *sp;
	struct host1x *host1x;
	u32 cur;

	host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
	if (WARN_ON(!host1x))
		return;

	sp = host1x_syncpt_get_by_id_noref(host1x,
			HOST1X_GLOBAL_TO_LOCAL_SYNCPOINT(id));
	if (WARN_ON(!sp))
		return;

	cur = host1x_syncpt_read(sp);

	while (cur++ < val) {
		host1x_syncpt_incr(sp);
	}
}

void nvgpu_nvhost_syncpt_put_ref_ext(struct nvgpu_nvhost_dev *nvhost_dev,
				     u32 id)
{
	struct host1x_syncpt *sp;
	struct host1x *host1x;

	host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
	if (WARN_ON(!host1x))
		return;
	sp = host1x_syncpt_get_by_id_noref(host1x,
			HOST1X_GLOBAL_TO_LOCAL_SYNCPOINT(id));
	if (WARN_ON(!sp))
		return;

	host1x_syncpt_put(sp);
}

u32 nvgpu_nvhost_get_syncpt_client_managed(struct nvgpu_nvhost_dev *nvhost_dev,
					   const char *syncpt_name)
{
	struct host1x_syncpt *sp;
	struct host1x *host1x;

	host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
	if (!host1x)
		return 0;

	sp = host1x_syncpt_alloc(host1x,
			HOST1X_SYNCPT_CLIENT_MANAGED | HOST1X_SYNCPT_GPU,
			syncpt_name);

	if (!sp)
		return 0;

	return host1x_syncpt_id(sp);
}

int nvgpu_nvhost_syncpt_wait_timeout_ext(struct nvgpu_nvhost_dev *nvhost_dev,
					 u32 id, u32 thresh, u32 timeout,
					 u32 waiter_index)
{
	struct host1x_syncpt *sp;
	struct host1x *host1x;
	u32 dummy_value = 0U;

	host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
	if (!host1x)
		return -ENODEV;

	sp = host1x_syncpt_get_by_id_noref(host1x,
			HOST1X_GLOBAL_TO_LOCAL_SYNCPOINT(id));
	if (!sp)
		return -EINVAL;

	return host1x_syncpt_wait(sp, thresh,
			timeout, &dummy_value);
}

int nvgpu_nvhost_syncpt_read_ext_check(struct nvgpu_nvhost_dev *nvhost_dev,
				       u32 id, u32 *val)
{
	struct host1x_syncpt *sp;
	struct host1x *host1x;

	host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
	if (!host1x)
		return -ENODEV;

	sp = host1x_syncpt_get_by_id_noref(host1x,
			 HOST1X_GLOBAL_TO_LOCAL_SYNCPOINT(id));
	if (!sp)
		return -EINVAL;
	*val = host1x_syncpt_read(sp);

	return 0;
}

void nvgpu_nvhost_syncpt_set_safe_state(struct nvgpu_nvhost_dev *nvhost_dev,
					u32 id)
{
	struct host1x_syncpt *sp;
	struct host1x *host1x;
	u32 val, cur;

	host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
	if (WARN_ON(!host1x))
		return;

	/*
	 * Add large number of increments to current value
	 * so that all waiters on this syncpoint are released
	 */
	sp = host1x_syncpt_get_by_id_noref(host1x,
			 HOST1X_GLOBAL_TO_LOCAL_SYNCPOINT(id));
	if (WARN_ON(!sp))
		return;

	cur = host1x_syncpt_read(sp);
	val = cur + 1000;

	while (cur++ != val) {
		host1x_syncpt_incr(sp);
	}
}

int nvgpu_nvhost_get_syncpt_aperture(
		struct nvgpu_nvhost_dev *nvhost_dev,
		u64 *base, size_t *size, bool is_second_die)
{
	struct device_node *np = nvhost_dev->host1x_pdev->dev.of_node;
#ifdef CONFIG_TEGRA_HOST1X_EMU_DBG_SYMBL
	struct host1x *host1x;
	u32 stride;
	u32 num_syncpts;
	int err;
#endif

	if (of_device_is_compatible(np, "nvidia,tegra194-host1x")) {
		*base = TEGRA194_SYNCPT_SHIM_BASE;
		*size = TEGRA194_SYNCPT_SHIM_SIZE;
		return 0;
	}

	if (of_device_is_compatible(np, "nvidia,tegra234-host1x")) {
		*base = TEGRA234_SYNCPT_SHIM_BASE;
		*size = TEGRA234_SYNCPT_SHIM_SIZE;
		return 0;
	}

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	if (of_device_is_compatible(np, "nvidia,tegra264-host1x")) {
		if (!is_second_die) {
			*base = TEGRA264_SYNCPT_SHIM_BASE;
		} else {
			*base = TEGRA264_SYNCPT_SHIM_BASE_DIE2;
		}
		*size = TEGRA264_SYNCPT_SHIM_SIZE;
		return 0;
	}
#endif

#ifdef CONFIG_TEGRA_HOST1X_EMU_DBG_SYMBL
	if (of_device_is_compatible(np, "nvidia,tegraEmu-host1x")) {
		host1x = platform_get_drvdata(nvhost_dev->host1x_pdev);
		err = host1x_syncpt_get_shim_info(host1x, (phys_addr_t *)base,
				&stride, &num_syncpts);
		if (err) {
			return -1;
		}
		*size = (stride * num_syncpts);

		return 0;
	}
#endif

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
	return nvgpu_next_get_syncpt_shim_info(nvhost_dev->g, base, size, is_second_die);
#else
	return -ENOTSUPP;
#endif
}

u32 nvgpu_nvhost_syncpt_unit_interface_get_byte_offset(struct gk20a *g,
						       u32 syncpt_id)
{
	struct platform_device *host1x_pdev = g->nvhost->host1x_pdev;
	struct device_node *np = host1x_pdev->dev.of_node;

	if (of_device_is_compatible(np, "nvidia,tegra194-host1x"))
		return syncpt_id * TEGRA194_SYNCPT_PAGE_SIZE;

	if (of_device_is_compatible(np, "nvidia,tegra234-host1x"))
		return syncpt_id * TEGRA234_SYNCPT_PAGE_SIZE;

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	if (of_device_is_compatible(np, "nvidia,tegra264-host1x"))
		return syncpt_id * TEGRA264_SYNCPT_PAGE_SIZE;
#endif
	if (of_device_is_compatible(np, "nvidia,tegraEmu-host1x"))
		return syncpt_id * PAGE_SIZE;/** TODO Fix required to use the host1x API*/
#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
	return nvgpu_next_get_syncpt_byte_offset(g, syncpt_id);
#else
	return 0;
#endif
}

int nvgpu_nvhost_fence_install(struct dma_fence *f, int fd)
{
	struct sync_file *file = sync_file_create(f);

	if (!file)
		return -ENOMEM;

	fd_install(fd, file->file);

	return 0;
}

void nvgpu_nvhost_fence_put(struct dma_fence *fence)
{
	dma_fence_put(fence);
}

void nvgpu_nvhost_fence_dup(struct dma_fence *fence)
{
	dma_fence_get(fence);
}

struct dma_fence *nvgpu_nvhost_fence_create(struct platform_device *pdev,
					struct nvhost_ctrl_sync_fence_info *pts,
					u32 num_pts, const char *name)
{
	struct host1x_syncpt *sp;
	struct host1x *host1x;

	if (num_pts != 1)
		return ERR_PTR(-EINVAL);

	host1x = platform_get_drvdata(pdev);
	if (!host1x)
		return ERR_PTR(-ENODEV);
	sp = host1x_syncpt_get_by_id_noref(host1x,
			HOST1X_GLOBAL_TO_LOCAL_SYNCPOINT(pts->id));
	if (WARN_ON(!sp))
		return ERR_PTR(-EINVAL);

	return host1x_fence_create(sp,
			pts->thresh, true);
}

u32 nvgpu_nvhost_fence_num_pts(struct dma_fence *fence)
{
	struct dma_fence_array *array;

	array = to_dma_fence_array(fence);
	if (!array)
		return 1;

	return array->num_fences;
}

int nvgpu_nvhost_fence_foreach_pt(struct dma_fence *fence,
	int (*iter)(struct nvhost_ctrl_sync_fence_info, void *),
	void *data)
{
	struct nvhost_ctrl_sync_fence_info info;
	struct dma_fence_array *array;
	int i, err;

	array = to_dma_fence_array(fence);
	if (!array) {
		err = host1x_fence_extract(fence,
				&info.id, &info.thresh);
		if (err)
			return err;

		return iter(info, data);
	}

	for (i = 0; i < array->num_fences; ++i) {
		err = host1x_fence_extract(array->fences[i], &info.id,
			   &info.thresh);

		if (err)
			return err;

		err = iter(info, data);
		if (err)
			return err;
	}

	return 0;
}

bool nvgpu_nvhost_fence_is_signaled(struct dma_fence *fence)
{
	return dma_fence_is_signaled(fence);
}
