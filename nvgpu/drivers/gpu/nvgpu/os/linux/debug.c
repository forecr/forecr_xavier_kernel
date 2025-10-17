// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "debug_cde.h"
#include "debug_fifo.h"
#include "debug_gr.h"
#include "debug_allocator.h"
#include "debug_kmem.h"
#include "debug_pmu.h"
#include "debug_sched.h"
#include "debug_hal.h"
#include "debug_xve.h"
#include "debug_ltc.h"
#include "debug_bios.h"
#ifdef CONFIG_NVGPU_GSP_STRESS_TEST
#include "debug_gsp.h"
#endif
#include "debug_multimedia.h"
#include "debug_boot_time.h"
#include "debug_c2c.h"
#include "os_linux.h"
#include "platform_gk20a.h"

#include <nvgpu/gk20a.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/power_features/pg.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/tsg.h>

#include <linux/debugfs.h>
#include <linux/uaccess.h>

#include <nvgpu/debug.h>

unsigned int gk20a_debug_trace_cmdbuf;

void gk20a_debug_output(struct nvgpu_debug_context *o, const char *fmt, ...)
{
	va_list args;
	int err;

	va_start(args, fmt);
	err = vsnprintf(o->buf, sizeof(o->buf), fmt, args);
	nvgpu_assert(err > 0);

	va_end(args);
	o->fn(o->ctx, o->buf);
}

void gk20a_debug_show_dump(struct gk20a *g, struct nvgpu_debug_context *o)
{
	nvgpu_channel_debug_dump_all(g, o);
	g->ops.pbdma.dump_status(g, o);
	g->ops.engine_status.dump_engine_status(g, o);
}

static int gk20a_gr_dump_regs(struct gk20a *g,
		struct nvgpu_debug_context *o)
{
	if (g->ops.gr.dump_gr_regs)
		nvgpu_pg_elpg_protected_call(g, g->ops.gr.dump_gr_regs(g, o));

	return 0;
}

void gk20a_gr_debug_dump(struct gk20a *g)
{
	struct nvgpu_debug_context o = {
		.fn = gk20a_debug_write_printk,
		.ctx = g,
	};

	gk20a_gr_dump_regs(g, &o);
}

static int gk20a_fecs_gr_dump_regs(struct gk20a *g,
		struct nvgpu_debug_context *o)
{
	if (g->ops.gr.dump_fecs_gr_regs)
		nvgpu_pg_elpg_protected_call(g,
			g->ops.gr.dump_fecs_gr_regs(g, o));

	return 0;
}

void gk20a_fecs_gr_debug_dump(struct gk20a *g)
{
	struct nvgpu_debug_context o = {
		.fn = gk20a_debug_write_printk,
		.ctx = g,
	};

	gk20a_fecs_gr_dump_regs(g, &o);
}

static int gk20a_gr_debug_show(struct seq_file *s, void *unused)
{
	struct device *dev = s->private;
	struct gk20a *g = gk20a_get_platform(dev)->g;
	struct nvgpu_debug_context o = {
		.fn = gk20a_debug_write_to_seqfile,
		.ctx = s,
	};
	int err;

	err = gk20a_busy(g);
	if (err) {
		nvgpu_err(g, "failed to power on gpu: %d", err);
		return -EINVAL;
	}

	gk20a_gr_dump_regs(g, &o);

	gk20a_idle(g);

	return 0;
}

void nvgpu_multimedia_debug_dump(struct gk20a *g, const struct nvgpu_device *dev)
{
	struct nvgpu_debug_context o = {
		.fn = gk20a_debug_write_printk,
		.ctx = g,
	};

	(void) nvgpu_multimedia_debug_common(g, dev, &o);
}

void gk20a_debug_dump(struct gk20a *g)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev_from_gk20a(g));
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct nvgpu_debug_context o = {
		.fn = gk20a_debug_write_printk,
		.ctx = g,
	};

	/* HAL only initialized after 1st power-on */
	if (g->ops.debug.show_dump)
		g->ops.debug.show_dump(g, &o);

	if (platform->dump_platform_dependencies && l->enable_platform_dbg)
		platform->dump_platform_dependencies(dev_from_gk20a(g));
}

static int gk20a_debug_show(struct seq_file *s, void *unused)
{
	struct device *dev = s->private;
	struct nvgpu_debug_context o = {
		.fn = gk20a_debug_write_to_seqfile,
		.ctx = s,
	};
	struct gk20a *g;
	int err;

	g = gk20a_get_platform(dev)->g;

	err = gk20a_busy(g);
	if (err) {
		nvgpu_err(g, "failed to power on gpu: %d", err);
		return -EFAULT;
	}

	/* HAL only initialized after 1st power-on */
	if (g->ops.debug.show_dump)
		g->ops.debug.show_dump(g, &o);

	gk20a_idle(g);
	return 0;
}

static int gk20a_gr_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, gk20a_gr_debug_show, inode->i_private);
}

static int gk20a_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, gk20a_debug_show, inode->i_private);
}

static const struct file_operations gk20a_gr_debug_fops = {
	.open		= gk20a_gr_debug_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations gk20a_debug_fops = {
	.open		= gk20a_debug_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static ssize_t disable_bigpage_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[3];
	struct gk20a *g = file->private_data;

	if (g->mm.disable_bigpage)
		buf[0] = 'Y';
	else
		buf[0] = 'N';
	buf[1] = '\n';
	buf[2] = 0x00;
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}

static ssize_t disable_bigpage_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	int buf_size;
	bool bv;
	struct gk20a *g = file->private_data;
	int err;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	if (kstrtobool(buf, &bv) == 0) {
		g->mm.disable_bigpage = bv;
		err = nvgpu_init_gpu_characteristics(g);
		if (err != 0) {
			nvgpu_err(g, "failed to init GPU characteristics");
			return -ENOSYS;
		}
	}

	return count;
}

static struct file_operations disable_bigpage_fops = {
	.open =		simple_open,
	.read =		disable_bigpage_read,
	.write =	disable_bigpage_write,
};

static int railgate_residency_show(struct seq_file *s, void *data)
{
	struct gk20a *g = s->private;
	struct gk20a_platform *platform = dev_get_drvdata(dev_from_gk20a(g));
	unsigned long time_since_last_state_transition_ms;
	unsigned long total_rail_gate_time_ms;
	unsigned long total_rail_ungate_time_ms;

	if (platform->is_railgated(dev_from_gk20a(g))) {
		time_since_last_state_transition_ms =
				jiffies_to_msecs(jiffies -
				g->pstats.last_rail_gate_complete);
		total_rail_ungate_time_ms = g->pstats.total_rail_ungate_time_ms;
		total_rail_gate_time_ms =
					g->pstats.total_rail_gate_time_ms +
					time_since_last_state_transition_ms;
	} else {
		time_since_last_state_transition_ms =
				jiffies_to_msecs(jiffies -
				g->pstats.last_rail_ungate_complete);
		total_rail_gate_time_ms = g->pstats.total_rail_gate_time_ms;
		total_rail_ungate_time_ms =
					g->pstats.total_rail_ungate_time_ms +
					time_since_last_state_transition_ms;
	}

	seq_printf(s, "Time with Rails Gated: %lu ms\n"
			"Time with Rails UnGated: %lu ms\n"
			"Total railgating cycles: %llu\n",
			total_rail_gate_time_ms,
			total_rail_ungate_time_ms,
			g->pstats.railgating_cycle_count - 1ULL);
	return 0;

}

static int railgate_residency_open(struct inode *inode, struct file *file)
{
	return single_open(file, railgate_residency_show, inode->i_private);
}

static const struct file_operations railgate_residency_fops = {
	.open		= railgate_residency_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int railgate_transitions_show(struct seq_file *s, void *data)
{
	struct gk20a *g = s->private;

	seq_printf(s, "%llu\n",
		nvgpu_safe_sub_u64(g->pstats.railgating_cycle_count, 1ULL));

	return 0;
}

static int railgate_transitions_open(struct inode *inode, struct file *file)
{
	return single_open(file, railgate_transitions_show, inode->i_private);
}

static const struct file_operations railgate_transitions_fops = {
	.open		= railgate_transitions_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int gk20a_railgating_debugfs_init(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct dentry *d;

	if (!g->is_virtual) {
		d = debugfs_create_file(
			"railgate_residency", S_IRUGO|S_IWUSR, l->debugfs, g,
						&railgate_residency_fops);
		if (!d)
			return -ENOMEM;

		d = debugfs_create_file(
			"railgate_transitions", S_IRUGO|S_IWUSR, l->debugfs, g,
						&railgate_transitions_fops);
		if (!d)
			return -ENOMEM;
	}
	return 0;
}

static int elcg_residency_show(struct seq_file *s, void *data)
{
	struct device *dev = s->private;
	struct gk20a *g = gk20a_get_platform(dev)->g;
	u32 elcg_residency;

	elcg_residency = nvgpu_cg_get_elcg_residency(g);

	seq_printf(s, "Sub Feature ID : %#X\nResidency : %u\n",
			g->elcg_ctrl->sub_feature_id,
			elcg_residency);

	return 0;
}

static int elcg_residency_open(struct inode *inode, struct file *file)
{
	return single_open(file, elcg_residency_show, inode->i_private);
}

static const struct file_operations elcg_residency_fops = {
	.open		= elcg_residency_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int elcg_entry_count_show(struct seq_file *s, void *data)
{
	struct device *dev = s->private;
	struct gk20a *g = gk20a_get_platform(dev)->g;
	u32 elcg_entry_count;

	elcg_entry_count = nvgpu_cg_get_elcg_entry_count(g);

	seq_printf(s, "Sub Feature ID : %#X\nEntry Count : %u\n",
			g->elcg_ctrl->sub_feature_id,
			elcg_entry_count);

	return 0;
}

static int elcg_entry_count_open(struct inode *inode, struct file *file)
{
	return single_open(file, elcg_entry_count_show, inode->i_private);
}

static const struct file_operations elcg_entry_count_fops = {
	.open		= elcg_entry_count_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int lpwr_residency_show(struct seq_file *s, void *data)
{
	struct device *dev = s->private;
	struct gk20a *g = gk20a_get_platform(dev)->g;
	struct lpwr_mon_params *lpwr_mon = &g->pmu->lpwr_mon;
	u32 lpwr_residency;

	lpwr_residency = nvgpu_pg_get_lpwr_lp_residency(g);

	seq_printf(s, "LPWR_MON Index : %d\nResidency : %u\n",
			lpwr_mon->lpwr_mon_index,
			lpwr_residency);

	return 0;
}

static int lpwr_residency_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpwr_residency_show, inode->i_private);
}

static const struct file_operations lpwr_residency_fops = {
	.open		= lpwr_residency_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int sysclk_slowdown_entry_count_show(struct seq_file *s, void *data)
{
	struct device *dev = s->private;
	struct gk20a *g = gk20a_get_platform(dev)->g;
	u32 sysclk_slowdown_entry_count;
	int err;

	err = gk20a_busy(g);
	if (err)
		return err;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "ELPG not booted yet, can't process RPCs");
		return 0;
	}

	sysclk_slowdown_entry_count = nvgpu_cg_get_sysclk_slowdown_entry_count(g);

	seq_printf(s,
			"SYSCLK Slowdown Entry Count : %u\n", sysclk_slowdown_entry_count);

	gk20a_idle(g);

	return 0;
}

static int sysclk_slowdown_entry_count_open(struct inode *inode,
							struct file *file)
{
	return single_open(file, sysclk_slowdown_entry_count_show,
							inode->i_private);
}

static const struct file_operations sysclk_slowdown_entry_count_fops = {
	.open		= sysclk_slowdown_entry_count_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static ssize_t timeouts_enabled_read(struct file *file,
			char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[3];
	struct gk20a *g = file->private_data;

	if (nvgpu_is_timeouts_enabled(g))
		buf[0] = 'Y';
	else
		buf[0] = 'N';
	buf[1] = '\n';
	buf[2] = 0x00;
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}

static ssize_t timeouts_enabled_write(struct file *file,
			const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[3];
	int buf_size;
	bool timeouts_enabled;
	struct gk20a *g = file->private_data;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	if (kstrtobool(buf, &timeouts_enabled) == 0) {
		nvgpu_mutex_acquire(&g->dbg_sessions_lock);
		if (timeouts_enabled == false) {
			/* requesting to disable timeouts */
			if (g->timeouts_disabled_by_user == false) {
				nvgpu_atomic_inc(&g->timeouts_disabled_refcount);
				g->timeouts_disabled_by_user = true;
			}
		} else {
			/* requesting to enable timeouts */
			if (g->timeouts_disabled_by_user == true) {
				nvgpu_atomic_dec(&g->timeouts_disabled_refcount);
				g->timeouts_disabled_by_user = false;
			}
		}
		nvgpu_mutex_release(&g->dbg_sessions_lock);
	}

	return count;
}

static const struct file_operations timeouts_enabled_fops = {
	.open =		simple_open,
	.read =		timeouts_enabled_read,
	.write =	timeouts_enabled_write,
};

static ssize_t dbg_tsg_timeslice_max_read(struct file *file,
					char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[10];
	struct gk20a *g = file->private_data;
	unsigned int val = g->tsg_dbg_timeslice_max_us;

	memcpy(buf, (char*)&val, sizeof(unsigned int));
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}

static ssize_t dbg_tsg_timeslice_max_write(struct file *file,
					const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[10];
	int buf_size;
	unsigned int val = 0;
	struct gk20a *g = file->private_data;
	unsigned int max_hw_timeslice_us = g->ops.runlist.get_tsg_max_timeslice();

	(void) memset(buf, 0, sizeof(buf));
	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	if (kstrtouint(buf, 10, &val) < 0)
		return -EINVAL;

	if (val < NVGPU_TSG_TIMESLICE_MIN_US ||
			val > max_hw_timeslice_us)
		return -EINVAL;

	g->tsg_dbg_timeslice_max_us = val;

	return count;
}

static const struct file_operations dbg_tsg_timeslice_max_fops = {
		.open =         simple_open,
		.read =      	dbg_tsg_timeslice_max_read,
		.write =        dbg_tsg_timeslice_max_write,
};

static ssize_t disable_syncpts_read(struct file *file,
			char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[3];
	struct gk20a *g = file->private_data;

	if (!nvgpu_is_enabled(g, NVGPU_HAS_SYNCPOINTS))
		buf[0] = 'Y';
	else
		buf[0] = 'N';
	buf[1] = '\n';
	buf[2] = 0x00;
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}

static ssize_t disable_syncpts_write(struct file *file,
			const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[3];
	int buf_size;
	bool disable_syncpts;
	struct gk20a *g = file->private_data;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	if (!g->nvhost)
		return -ENOSYS;

	if (kstrtobool(buf, &disable_syncpts) == 0)
		nvgpu_set_enabled(g, NVGPU_HAS_SYNCPOINTS, !disable_syncpts);

	return count;
}

static const struct file_operations disable_syncpts_fops = {
	.open =		simple_open,
	.read =		disable_syncpts_read,
	.write =	disable_syncpts_write,
};

void gk20a_debug_init(struct gk20a *g, const char *debugfs_symlink)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct device *dev = dev_from_gk20a(g);

	l->debugfs = debugfs_create_dir(dev_name(dev), NULL);
	if (!l->debugfs)
		return;

	if (debugfs_symlink)
		l->debugfs_alias =
			debugfs_create_symlink(debugfs_symlink,
					NULL, dev_name(dev));

	debugfs_create_file("status", S_IRUGO, l->debugfs,
		dev, &gk20a_debug_fops);
	debugfs_create_file("gr_status", S_IRUGO, l->debugfs,
		dev, &gk20a_gr_debug_fops);
	debugfs_create_u32("trace_cmdbuf", S_IRUGO|S_IWUSR,
		l->debugfs, &gk20a_debug_trace_cmdbuf);

	debugfs_create_u32("ch_wdt_init_limit_ms", S_IRUGO|S_IWUSR,
		l->debugfs, &g->ch_wdt_init_limit_ms);

	l->debugfs_disable_syncpts =
			debugfs_create_file("disable_syncpoints",
					S_IRUGO|S_IWUSR,
					l->debugfs,
					g,
					&disable_syncpts_fops);

	debugfs_create_bool("enable_platform_dbg", S_IRUGO|S_IWUSR,
		l->debugfs, &l->enable_platform_dbg);

	/* New debug logging API. */
	debugfs_create_u64("log_mask", S_IRUGO|S_IWUSR,
		l->debugfs, &g->log_mask);
	debugfs_create_u32("log_trace", S_IRUGO|S_IWUSR,
		l->debugfs, &g->log_trace);

	debugfs_create_bool("ltc_enabled", S_IRUGO|S_IWUSR, l->debugfs,
			    &g->mm.ltc_enabled_target);

	debugfs_create_u32("poll_timeout_default_ms", S_IRUGO|S_IWUSR,
				l->debugfs, &g->poll_timeout_default);

	l->debugfs_timeouts_enabled =
			debugfs_create_file("timeouts_enabled",
					S_IRUGO|S_IWUSR,
					l->debugfs,
					g,
					&timeouts_enabled_fops);

	l->debugfs_disable_bigpage =
			debugfs_create_file("disable_bigpage",
					S_IRUGO|S_IWUSR,
					l->debugfs,
					g,
					&disable_bigpage_fops);

	debugfs_create_u32("tsg_timeslice_low_priority_us", S_IRUGO|S_IWUSR,
				l->debugfs, &g->tsg_timeslice_low_priority_us);

	debugfs_create_u32("tsg_timeslice_medium_priority_us", S_IRUGO|S_IWUSR,
				l->debugfs,
				&g->tsg_timeslice_medium_priority_us);

	debugfs_create_u32("tsg_timeslice_high_priority_us", S_IRUGO|S_IWUSR,
				l->debugfs, &g->tsg_timeslice_high_priority_us);

	l->debugfs_dbg_tsg_timeslice_max_us =
                        debugfs_create_file("max_dbg_tsg_timeslice_us",
                                        S_IRUGO|S_IWUSR,
                                        l->debugfs, g,
                                        &dbg_tsg_timeslice_max_fops);

	debugfs_create_bool("runlist_interleave", S_IRUGO|S_IWUSR, l->debugfs,
			    &g->runlist_interleave);

	debugfs_create_file("elcg_residency", S_IRUGO, l->debugfs,
		dev, &elcg_residency_fops);

	debugfs_create_file("elcg_entry_count", S_IRUGO, l->debugfs,
		dev, &elcg_entry_count_fops);

	debugfs_create_file("lpwr_residency", S_IRUGO, l->debugfs,
		dev, &lpwr_residency_fops);

	debugfs_create_file("sysclk_slowdown_entry_count", S_IRUGO, l->debugfs,
		dev, &sysclk_slowdown_entry_count_fops);

	gr_gk20a_debugfs_init(g);
#ifndef CONFIG_NVGPU_EMB_LINUX_PROD_BUILD
	gk20a_pmu_debugfs_init(g);
#endif
	gk20a_railgating_debugfs_init(g);
#ifdef CONFIG_NVGPU_SUPPORT_CDE
	gk20a_cde_debugfs_init(g);
#endif
	nvgpu_alloc_debugfs_init(g);
	nvgpu_hal_debugfs_init(g);
	gk20a_fifo_debugfs_init(g);
	gk20a_sched_debugfs_init(g);
#ifdef CONFIG_NVGPU_TRACK_MEM_USAGE
	nvgpu_kmem_debugfs_init(g);
#endif
	nvgpu_ltc_debugfs_init(g);
#ifdef CONFIG_NVGPU_DGPU
	if (g->pci_vendor_id) {
		nvgpu_xve_debugfs_init(g);
		nvgpu_bios_debugfs_init(g);
	}
#endif
#ifdef CONFIG_NVGPU_GSP_STRESS_TEST
	nvgpu_gsp_debugfs_init(g);
#endif
	nvgpu_multimedia_debugfs_init(g);

	nvgpu_boot_time_debugfs_init(g);

	nvgpu_c2c_debugfs_init(g);
}

void gk20a_debug_deinit(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);

	if (!l->debugfs)
		return;

	debugfs_remove_recursive(l->debugfs);
	debugfs_remove(l->debugfs_alias);
}
