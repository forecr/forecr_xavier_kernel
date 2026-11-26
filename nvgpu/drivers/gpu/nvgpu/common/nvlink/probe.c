// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/enabled.h>
#include <nvgpu/nvlink_probe.h>

int nvgpu_nvlink_probe(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_NVLINK
	int err;

	err = nvgpu_nvlink_setup_ndev(g);
	if (err != 0) {
		return err;
	}

	err = nvgpu_nvlink_read_dt_props(g);
	if (err != 0) {
		goto free_ndev;
	}

	err = nvgpu_nvlink_init_ops(g);
	if (err != 0) {
		goto free_ndev;
	}

	/* Register device with core driver*/
	err = nvgpu_nvlink_register_device(g);
	if (err != 0) {
		nvgpu_err(g, "failed on nvlink device registration");
		goto free_ndev;
	}

	/* Register link with core driver */
	err = nvgpu_nvlink_register_link(g);
	if (err != 0) {
		nvgpu_err(g, "failed on nvlink link registration");
		goto unregister_ndev;
	}

	/* Enable NVLINK support */
	nvgpu_set_enabled(g, NVGPU_SUPPORT_NVLINK, true);
	return 0;

unregister_ndev:
	err = nvgpu_nvlink_unregister_device(g);

free_ndev:
	nvgpu_kfree(g, g->nvlink.priv);
	g->nvlink.priv = NULL;
	return err;
#else
	return -ENODEV;
#endif
}

