/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef __NVGPU_COMMON_LINUX_MODULE_H__
#define __NVGPU_COMMON_LINUX_MODULE_H__

struct gk20a;
struct device;
struct platform_device;
struct nvgpu_os_linux;

int gk20a_pm_init(struct device *dev);
int gk20a_pm_deinit(struct device *dev);
int gk20a_pm_late_init(struct device *dev);
int nvgpu_read_fuse_overrides(struct gk20a *g);
int nvgpu_kernel_shutdown_notification(struct notifier_block *nb,
					unsigned long event, void *unused);
int gk20a_pm_railgate(struct device *dev);
int gk20a_pm_runtime_suspend(struct device *dev);
int gk20a_pm_runtime_resume(struct device *dev);
int gk20a_pm_prepare_poweroff(struct device *dev);
int gk20a_pm_finalize_poweron(struct device *dev);
int gk20a_pm_suspend(struct device *dev);
int gk20a_pm_resume(struct device *dev);
int nvgpu_finalize_poweron_linux(struct nvgpu_os_linux *l);
void gk20a_remove_support(struct gk20a *g);
/*
 * This method is currently only supported to allow changing
 * MIG configurations. As such only GR state and device nodes
 * are freed as part of this. Any future functionality update
 * can be made by adding more to this.
 */
int gk20a_driver_force_power_off(struct gk20a *g);
void gk20a_driver_start_unload(struct gk20a *g);
int nvgpu_quiesce(struct gk20a *g);
int nvgpu_remove(struct device *dev);
int nvgpu_wait_for_gpu_idle(struct gk20a *g);
void nvgpu_free_irq(struct gk20a *g);
struct device_node *nvgpu_get_node(struct gk20a *g);
void __iomem *nvgpu_devm_ioremap_resource(struct platform_device *dev, int i,
		struct resource **out);
void __iomem *nvgpu_devm_ioremap(struct device *dev, resource_size_t offset,
		resource_size_t size);
u64 nvgpu_resource_addr(struct platform_device *dev, int i);
extern struct class nvgpu_class;
void gk20a_init_linux_characteristics(struct gk20a *g);
#endif
