/*
 * Linux common code for legacy VGPU and VF
 *
 * Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/dma-mapping.h>
#ifdef CONFIG_NVGPU_TEGRA_FUSE
#include <soc/tegra/fuse.h>
#endif

#include <nvgpu/cic_rm.h>
#include <nvgpu/defaults.h>
#include <nvgpu/errata.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/regops.h>
#include <nvgpu/soc.h>

#include "vgpu_linux.h"
#include "common/vgpu/ivc/comm_vgpu.h"
#include "common/vgpu/init/init_vgpu.h"
#include "common/vgpu/intr/intr_vgpu.h"

#include "os/linux/module.h"
#include "os/linux/os_linux.h"
#include "os/linux/ioctl.h"
#include "os/linux/driver_common.h"
#include "os/linux/platform_gk20a.h"

#include "vgpu_common.h"

struct vgpu_priv_data *vgpu_get_priv_data(struct gk20a *g)
{
	struct gk20a_platform *plat = gk20a_get_platform(dev_from_gk20a(g));

	return (struct vgpu_priv_data *)plat->vgpu_priv;
}

static void vgpu_init_vars(struct gk20a *g, struct gk20a_platform *platform)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	nvgpu_spinlock_init(&g->power_spinlock);

	nvgpu_mutex_init(&g->power_lock);
	nvgpu_mutex_init(&g->clk_arb_enable_lock);
	nvgpu_mutex_init(&g->cg_pg_lock);
	nvgpu_rwsem_init(&g->deterministic_busy);
	nvgpu_rwsem_init(&(g->ipa_pa_cache.ipa_pa_rw_lock));

	nvgpu_mutex_init(&priv->vgpu_clk_get_freq_lock);

	nvgpu_mutex_init(&l->ctrl_privs_lock);
	nvgpu_init_list_node(&l->ctrl_privs);

	g->regs_saved = g->regs;

	nvgpu_atomic_set(&g->clk_arb_global_nr, 0);

	g->aggressive_sync_destroy_thresh = platform->aggressive_sync_destroy_thresh;
	nvgpu_set_enabled(g, NVGPU_HAS_SYNCPOINTS, platform->has_syncpoints);
	g->ptimer_src_freq = platform->ptimer_src_freq;
	nvgpu_set_enabled(g, NVGPU_CAN_RAILGATE, platform->can_railgate_init);
	g->railgate_delay = platform->railgate_delay_init;

	g->mm.disable_bigpage = NVGPU_CPU_PAGE_SIZE < SZ_64K;
	nvgpu_set_enabled(g, NVGPU_MM_UNIFIED_MEMORY,
			    platform->unified_memory);
	nvgpu_set_enabled(g, NVGPU_MM_UNIFY_ADDRESS_SPACES,
			    platform->unify_address_spaces);

	if (nvgpu_is_vf(g)) {
		/* only VF needs IPA2PA */
		nvgpu_init_soc_vars(g);
	}
}

static int vgpu_init_support(struct gk20a *g)
{
	int err = 0;

	nvgpu_mutex_init(&g->dbg_sessions_lock);
#if defined(CONFIG_NVGPU_CYCLESTATS)
	nvgpu_mutex_init(&g->cs_lock);
#endif
#ifdef CONFIG_NVGPU_TSG_SHARING
	nvgpu_mutex_init(&g->ctrl_dev_id_lock);
#endif
	nvgpu_init_list_node(&g->profiler_objects);

#ifdef CONFIG_NVGPU_DEBUGGER
	g->dbg_regops_tmp_buf = nvgpu_kzalloc(g, SZ_4K);
	if (!g->dbg_regops_tmp_buf) {
		nvgpu_err(g, "couldn't allocate regops tmp buf");
		err = -ENOMEM;
		goto fail;
	}
	g->dbg_regops_tmp_buf_ops =
		SZ_4K / sizeof(g->dbg_regops_tmp_buf[0]);
#endif

	g->remove_support = vgpu_remove_support_common;
	return 0;

 fail:
	vgpu_remove_support_common(g);
	return err;
}

int vgpu_pm_prepare_poweroff(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	int ret = 0;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->power_lock);

	if (nvgpu_is_powered_off(g))
		goto done;

	if (g->ops.channel.suspend_all_serviceable_ch != NULL) {
		ret = g->ops.channel.suspend_all_serviceable_ch(g);
	}

	if (ret != 0) {
		goto done;
	}

	nvgpu_set_power_state(g, NVGPU_STATE_POWERED_OFF);

 done:
	nvgpu_mutex_release(&g->power_lock);

	return ret;
}

int vgpu_pm_finalize_poweron(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	int err = 0;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->power_lock);

	if (nvgpu_is_powered_on(g))
		goto done;

	nvgpu_set_power_state(g, NVGPU_STATE_POWERING_ON);

	err = vgpu_finalize_poweron_common(g);
	if (err)
		goto done;

	if (!l->dev_nodes_created) {
		err = gk20a_user_nodes_init(dev);
		if (err) {
			goto done;
		}

		l->dev_nodes_created = true;
	}

	/* Initialize linux specific flags */
	gk20a_init_linux_characteristics(g);

	err = nvgpu_finalize_poweron_linux(l);
	if (err)
		goto done;

	gk20a_sched_ctrl_init(g);

	g->sw_ready = true;

	nvgpu_set_power_state(g, NVGPU_STATE_POWERED_ON);

done:
	nvgpu_mutex_release(&g->power_lock);
	return err;
}

int vgpu_probe_common(struct nvgpu_os_linux *l)
{
	struct gk20a *g = &l->g;
	struct device *dev = l->dev;
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	struct vgpu_priv_data *priv;
	int err;

	nvgpu_log_fn(g, " ");

	nvgpu_init_gk20a(g);

	nvgpu_kmem_init(g);

	err = nvgpu_init_errata_flags(g);
	if (err) {
		return err;
	}

	err = nvgpu_init_enabled_flags(g);
	if (err) {
		goto free_errata;
	}

	#ifdef CONFIG_NVGPU_TEGRA_FUSE
	if (tegra_platform_is_vdk())
		nvgpu_set_enabled(g, NVGPU_IS_FMODEL, true);
#endif

	g->is_virtual = true;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		err = -ENOMEM;
		goto free_errata;
	}

	platform->g = g;
	platform->vgpu_priv = priv;

	err = vgpu_init_support(g);
	if (err != 0) {
		goto free_errata;
	}

	err = nvgpu_cic_rm_setup(g);
	if (err != 0) {
		nvgpu_err(g, "CIC-RM setup failed");
		goto free_errata;
	}

	err = nvgpu_cic_rm_init_vars(g);
	if (err != 0) {
		nvgpu_err(g, "CIC-RM init vars failed");
		goto remove_cic;
	}

	nvgpu_read_support_gpu_tools(g);

	vgpu_init_vars(g, platform);

	init_rwsem(&l->busy_lock);

	nvgpu_spinlock_init(&g->mc.enable_lock);

	nvgpu_spinlock_init(&g->mc.intr_lock);

	g->ch_wdt_init_limit_ms = platform->ch_wdt_init_limit_ms;

	/* Initialize the platform interface. */
	err = platform->probe(dev);
	if (err) {
		if (err == -EPROBE_DEFER)
			nvgpu_info(g, "platform probe failed");
		else
			nvgpu_err(g, "platform probe failed");
		goto remove_cic;
	}

	if (platform->late_probe) {
		err = platform->late_probe(dev);
		if (err) {
			nvgpu_err(g, "late probe failed");
			goto remove_cic;
		}
	}

	err = gk20a_power_node_init(dev);
	if (err) {
		nvgpu_err(g, "power_node creation failed");
		goto remove_cic;
	}

	err = vgpu_comm_init(g);
	if (err) {
		nvgpu_err(g, "failed to init comm interface");
		goto remove_cic;
	}

	priv->virt_handle = vgpu_connect();
	if (!priv->virt_handle) {
		nvgpu_err(g, "failed to connect to server node");
		goto comm_deinit;
	}

	err = vgpu_get_constants(g);
	if (err) {
		goto comm_deinit;
	}

	gk20a_debug_init(g, "gpu.0");

	/* Set DMA parameters to allow larger sgt lists */
	dev->dma_parms = &l->dma_parms;
	dma_set_max_seg_size(dev, UINT_MAX);

	dma_set_mask(dev, platform->dma_mask);
	dma_set_coherent_mask(dev, platform->dma_mask);
	dma_set_seg_boundary(dev, platform->dma_mask);

	g->poll_timeout_default = NVGPU_DEFAULT_POLL_TIMEOUT_MS;
	g->timeouts_disabled_by_user = false;
	nvgpu_atomic_set(&g->timeouts_disabled_refcount, 0);
	g->tsg_dbg_timeslice_max_us = NVGPU_TSG_DBG_TIMESLICE_MAX_US_DEFAULT;

	nvgpu_mutex_init(&l->dmabuf_priv_list_lock);
	nvgpu_init_list_node(&l->dmabuf_priv_list);

	nvgpu_ref_init(&g->refcount);

	priv = platform->vgpu_priv;
	err = nvgpu_thread_create(&priv->intr_handler, g,
			vgpu_intr_thread, "gk20a");
	if (err) {
		goto comm_deinit;
	}

	return 0;

comm_deinit:
	vgpu_comm_deinit();
remove_cic:
	(void) nvgpu_cic_rm_remove(g);
free_errata:
	nvgpu_free_errata_flags(g);
	return err;
}