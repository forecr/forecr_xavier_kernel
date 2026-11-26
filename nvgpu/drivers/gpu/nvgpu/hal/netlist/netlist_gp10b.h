/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_NETLIST_GP10B_H
#define NVGPU_NETLIST_GP10B_H

#include <nvgpu/types.h>
#include <nvgpu/netlist_defs.h>

struct gk20a;

/* production netlist, one and only one from below */
#define GP10B_NETLIST_IMAGE_FW_NAME NVGPU_NETLIST_IMAGE_A

int gp10b_netlist_get_name(struct gk20a *g, int index, char *name);
bool gp10b_netlist_is_firmware_defined(void);

#endif /* NVGPU_NETLIST_GP10B_H */
