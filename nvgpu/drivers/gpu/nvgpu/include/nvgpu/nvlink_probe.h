/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVLINK_PROBE_H
#define NVGPU_NVLINK_PROBE_H

struct gk20a;

int nvgpu_nvlink_setup_ndev(struct gk20a *g);
int nvgpu_nvlink_init_ops(struct gk20a *g);
int nvgpu_nvlink_read_dt_props(struct gk20a *g);
int nvgpu_nvlink_probe(struct gk20a *g);
int nvgpu_nvlink_register_device(struct gk20a *g);
int nvgpu_nvlink_unregister_device(struct gk20a *g);
int nvgpu_nvlink_register_link(struct gk20a *g);
int nvgpu_nvlink_unregister_link(struct gk20a *g);

#endif /* NVGPU_NVLINK_PROBE_H */
