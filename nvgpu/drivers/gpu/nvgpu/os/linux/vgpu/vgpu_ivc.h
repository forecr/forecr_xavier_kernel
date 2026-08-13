/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef VGPU_IVC_LINUX_H
#define VGPU_IVC_LINUX_H

#include <linux/wait.h>

struct tegra_hv_ivc_cookie;

struct vgpu_ivc {
	struct tegra_hv_ivc_cookie *ivck;
	struct wait_queue_head wq;
};

#endif