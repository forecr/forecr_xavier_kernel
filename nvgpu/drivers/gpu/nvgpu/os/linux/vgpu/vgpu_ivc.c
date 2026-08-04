// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/nvgpu_ivm.h>
#include <nvgpu/vgpu/vgpu.h>

#include <linux/interrupt.h>
#include <linux/kthread.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
#include <linux/tegra-ivc.h>
#else
#include <soc/tegra/virt/hv-ivc.h>
#endif

#include "os/linux/os_linux.h"
#include "os/linux/module.h"

int vgpu_ivc_send(struct vgpu_ivc *ivc, void *data, u32 size)
{
	int len;
	long time_left;

	/*
	 * The ivc should always be can_write on runtime.
	 * During ivc handshake, it waits for the server initializes ivc.
	 */
	time_left = wait_event_timeout(ivc->wq,
			tegra_hv_ivc_can_write(ivc->ivck),
			msecs_to_jiffies(VGPU_IVC_TIMEOUT_MS));
	if (time_left == 0) {
		return -ETIMEDOUT;
	}
	len = tegra_hv_ivc_write(ivc->ivck, data, size);
	return len < 0 ? len : (len == size ? 0 : -ENOMEM);
}

int vgpu_ivc_recv(struct vgpu_ivc *ivc, void *data, u32 size)
{
	int len;
	long time_left;

	/**
	 * Waits for data on ivc, abort when kthread should stop.
	 * VGPU interrupt kthread relies on the kthread_should_stop below to
	 * quit the thread.
	 */
	time_left = wait_event_timeout(ivc->wq, tegra_hv_ivc_can_read(ivc->ivck) ||
		((current->flags & PF_KTHREAD) && kthread_should_stop()),
		msecs_to_jiffies(VGPU_IVC_TIMEOUT_MS));
	if (time_left == 0) {
		return -ETIMEDOUT;
	}
	if ((current->flags & PF_KTHREAD) && kthread_should_stop()) {
		return -ECANCELED;
	}
	len = tegra_hv_ivc_read(ivc->ivck, data, size);
	return len < 0 ? len : (len == size ? 0 : -ENOMEM);
}

static irqreturn_t vgpu_ivc_isr(int irq, void *dev_id)
{
	struct vgpu_ivc *ivc = dev_id;

	if (tegra_hv_ivc_channel_notified(ivc->ivck)) {
		return IRQ_HANDLED;
	}

	wake_up_all(&ivc->wq);

	return IRQ_HANDLED;
}

static int vgpu_ivc_init_one(struct gk20a *g, struct vgpu_ivc *ivc,
	struct device_node *np, int ivc_nr)
{
	int err;

	init_waitqueue_head(&ivc->wq);

	ivc->ivck = tegra_hv_ivc_reserve(np, ivc_nr, NULL);
	if (IS_ERR_OR_NULL(ivc->ivck)) {
		nvgpu_err(g, "reserve ivc %d failed, err=%ld",
			ivc_nr, PTR_ERR(ivc->ivck));
		return IS_ERR(ivc->ivck) ? PTR_ERR(ivc->ivck) : -EINVAL;
	}
	tegra_hv_ivc_channel_reset(ivc->ivck);
	err = request_threaded_irq(ivc->ivck->irq, vgpu_ivc_isr,
		NULL, 0, "vgpu_ivc", ivc);
	if (err != 0) {
		nvgpu_err(g, "request irq failed for ivc %d, err=%d", ivc_nr, err);
		goto irq_fail;
	}

	return 0;

irq_fail:
	tegra_hv_ivc_unreserve(ivc->ivck);
	return err;
}

static void vgpu_ivc_remove_one(struct gk20a *g, struct vgpu_ivc *ivc)
{
	free_irq(ivc->ivck->irq, ivc);
	tegra_hv_ivc_unreserve(ivc->ivck);
}

static int vgpu_ivc_init_cmd(struct gk20a *g)
{
	const struct device *dev = dev_from_gk20a(g);
	const struct device_node *np = dev->of_node;
	struct device_node *hv_np;
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct vgpu_ivc_cmd_chs *ivc_cmd_chs = &priv->ivc_cmd_chs;
	int nr_ivc, i, err;

	nr_ivc = of_property_count_u32_elems(np, "ivc-queue3");
	if (nr_ivc < 2) {
		nvgpu_err(g, "invalid dt property ivc-queue3");
		return -EINVAL;
	}
	if (of_property_count_u32_elems(np, "mempool3") != nr_ivc) {
		nvgpu_err(g, "invalid dt property mempools");
		return -EINVAL;
	}
	nr_ivc -= 1; /* don't count the phandle element */
	hv_np = of_parse_phandle(np, "ivc-queue3", 0);
	if (IS_ERR_OR_NULL(hv_np)) {
		nvgpu_err(g, "invalid phandle in ivc-queue3");
		return -EINVAL;
	}

	/* The max nr_ivc is the nr of cpu cores */
	nr_ivc = nr_ivc > num_online_cpus() ? num_online_cpus() : nr_ivc;

	/* alloc memory and init common fields */
	err = vgpu_comm_init(g, nr_ivc);
	if (err != 0) {
		return err;
	}
	/* init each ivc channel */
	for (i = 0; i < nr_ivc; i++) {
		u32 ivc_nr, mempool_nr;
		struct tegra_hv_ivm_cookie *ivm;

		err = of_property_read_u32_index(np, "ivc-queue3",
					i + 1, &ivc_nr);
		if (err != 0) {
			nvgpu_err(g, "dt property ivc-queue3 read failed err=%d", err);
			return err;
		}
		err = of_property_read_u32_index(np, "mempool3",
					i + 1, &mempool_nr);
		if (err != 0) {
			nvgpu_err(g, "dt property mempool3 read failed err=%d", err);
			return err;
		}
		err = vgpu_ivc_init_one(g, &ivc_cmd_chs->ivc_chs[i].ivc, hv_np, ivc_nr);
		if (err != 0) {
			return err;
		}
		ivm = nvgpu_ivm_mempool_reserve(mempool_nr);
		if (IS_ERR_OR_NULL(ivm)) {
			nvgpu_err(g, "reserve mempool %d failed", mempool_nr);
			return -EINVAL;
		}
		ivc_cmd_chs->ivc_chs[i].ivm = ivm;
		ivc_cmd_chs->ivc_chs[i].ivm_va = nvgpu_ivm_mempool_map(ivm);
		if (IS_ERR_OR_NULL(ivc_cmd_chs->ivc_chs[i].ivm_va)) {
			nvgpu_err(g, "map mempool %d failed", mempool_nr);
			return -EINVAL;
		}
	}

	return 0;
}

static void vgpu_ivc_remove_cmd(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct vgpu_ivc_cmd_chs *ivc_cmd_chs = &priv->ivc_cmd_chs;
	int i;

	for (i = 0; i < ivc_cmd_chs->nr_ivc; i++) {
		nvgpu_ivm_mempool_unmap(ivc_cmd_chs->ivc_chs[i].ivm,
					ivc_cmd_chs->ivc_chs[i].ivm_va);
		nvgpu_ivm_mempool_unreserve(ivc_cmd_chs->ivc_chs[i].ivm);
		vgpu_ivc_remove_one(g, &ivc_cmd_chs->ivc_chs[i].ivc);
	}
	vgpu_comm_deinit(g);
}

static int vgpu_ivc_init_intr(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	const struct device *dev = dev_from_gk20a(g);
	const struct device_node *np = dev->of_node;
	struct of_phandle_args args;
	int err;

	err = of_parse_phandle_with_fixed_args(np, "ivc-queue4", 1, 0, &args);
	if (err != 0 || args.args_count != 1) {
		nvgpu_err(g, "invalid property ivc-queue, err=%d %d", err, args.args_count);
		return err != 0 ? err : -EINVAL;
	}
	err = vgpu_ivc_init_one(g, &priv->ivc_intr, args.np, args.args[0]);
	if (err != 0) {
		return err;
	}

	return err;
}

static void vgpu_ivc_remove_intr(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	vgpu_ivc_remove_one(g, &priv->ivc_intr);
}

int vgpu_ivc_init_support(struct gk20a *g)
{
	int err;

	/**
	 * @brief read ivc numbers from dt
	 *
	 * Device tree bindings:
	 * ivc-queue3 = <&tegra_hv 44 ... >; // cmd ivc numbers
	 * mempool3 = <&tegra_hv 0 000 >; // same nr of mempool as cmd ivc
	 * ivc-queue4 = <&tegra_hv 45>; // intr ivc numbers
	 */

	/* init cmd ivc */
	err = vgpu_ivc_init_cmd(g);
	if (err != 0) {
		return err;
	}

	/* init intr ivc */
	err = vgpu_ivc_init_intr(g);
	if (err != 0) {
		return err;
	}

	return 0;
}

void vgpu_ivc_remove_support(struct gk20a *g)
{
	vgpu_ivc_remove_cmd(g);
	vgpu_ivc_remove_intr(g);
}
