/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OS_LINUX_H
#define NVGPU_OS_LINUX_H

#include <linux/iommu.h>
#include <linux/notifier.h>
#include <linux/version.h>

#include <nvgpu/gk20a.h>
#include <nvgpu/interrupts.h>

#include "cde.h"
#include "sched.h"

struct nvgpu_os_linux_ops {
	struct {
		void (*get_program_numbers)(struct gk20a *g,
					    u32 block_height_log2,
					    u32 shader_parameter,
					    int *hprog, int *vprog);
		bool (*need_scatter_buffer)(struct gk20a *g);
		int (*populate_scatter_buffer)(struct gk20a *g,
					       struct sg_table *sgt,
					       size_t surface_size,
					       void *scatter_buffer_ptr,
					       size_t scatter_buffer_size);
	} cde;

	struct {
		int (*init_debugfs)(struct gk20a *g);
	} clk;

	struct {
		int (*init_debugfs)(struct gk20a *g);
	} therm;

	struct {
		int (*init_debugfs)(struct gk20a *g);
	} fecs_trace;

	struct {
		int (*init_debugfs)(struct gk20a *g);
	} volt;

	struct {
		int (*init_debugfs)(struct gk20a *g);
	} s_param;
};

struct dgpu_thermal_alert {
        struct workqueue_struct *workqueue;
        struct work_struct work;
        u32 therm_alert_irq;
        u32 event_delay;
};

struct nvgpu_os_linux {
	struct gk20a g;
	struct device *dev;
	struct dgpu_thermal_alert thermal_alert;
	struct nvgpu_interrupts interrupts;
#ifdef CONFIG_DEVFREQ_THERMAL
	struct thermal_cooling_device *cooling;
#endif

	struct nvgpu_list_node class_list_head;
	struct nvgpu_list_node cdev_list_head;
	u32 power_cdevs;
	u32 num_cdevs;
	bool dev_nodes_created;
	bool cdev_list_init_done;

	dev_t power_cdev_region;
	dev_t cdev_region;
	atomic_t next_cdev_minor;

	/* see gk20a_ctrl_priv */
	struct nvgpu_list_node ctrl_privs;
	/* guards modifications to the list and its contents */
	struct nvgpu_mutex ctrl_privs_lock;

	struct devfreq *devfreq;

	atomic_t nonstall_ops;

	struct work_struct nonstall_fn_work;
	struct workqueue_struct *nonstall_work_queue;

	struct resource *reg_mem;
	struct resource *bar1_mem;

	struct nvgpu_os_linux_ops ops;

	struct notifier_block nvgpu_reboot_nb;

#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs;
	struct dentry *debugfs_alias;

	struct dentry *debugfs_timeouts_enabled;
	struct dentry *debugfs_disable_bigpage;
	struct dentry *debugfs_dbg_tsg_timeslice_max_us;
	struct dentry *debugfs_disable_syncpts;

	struct dentry *debugfs_allocators;
	struct dentry *debugfs_xve;
	struct dentry *debugfs_kmem;
	struct dentry *debugfs_hal;
	struct dentry *debugfs_ltc;

	struct dentry *debugfs_dump_ctxsw_stats;
	struct dentry *debugfs_gsp;
	struct dentry *debugfs_multimedia;
#endif
	struct dev_ext_attribute *ecc_attrs;

	struct gk20a_cde_app cde_app;

	struct rw_semaphore busy_lock;

	struct nvgpu_mutex dmabuf_priv_list_lock;
	struct nvgpu_list_node dmabuf_priv_list;

	bool init_done;

	bool enable_platform_dbg;

	/*
	 * Profile boot-time latency.
	 *
	 * With NO GPU virtualization, boot-time latency for
	 * T264 is execution time for gk20a_init()+gk20a_pm_finalize_poweron().
	 *     nvgpu_pci_probe() is invoked as nested call within gk20a_init().
	 * T234 is execution time for gk20a_init()+gk20a_probe()+gk20a_pm_finalize_poweron().
	 *
	 * For gk20_init(), struct nvgpu_os_linux is yet to be allocated during gk20a_init()
	 * execution, therefore nvgpu_init_time_ns shall be allocated as global.
	 */
	u64 probe_time_ns;
	u64 poweron_time_ns;
};

static inline struct nvgpu_os_linux *nvgpu_os_linux_from_gk20a(struct gk20a *g)
{
	return container_of(g, struct nvgpu_os_linux, g);
}

static inline struct device *dev_from_gk20a(struct gk20a *g)
{
	return nvgpu_os_linux_from_gk20a(g)->dev;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#define totalram_size_in_mb (totalram_pages() >> (10 - (PAGE_SHIFT - 10)))
#else
#define totalram_size_in_mb (totalram_pages >> (10 - (PAGE_SHIFT - 10)))
#endif

#endif
