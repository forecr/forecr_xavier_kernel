/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_NETLIST_GV11B_H
#define NVGPU_NETLIST_GV11B_H

#include <nvgpu/types.h>
#include <nvgpu/netlist_defs.h>

struct gk20a;

/* Define netlist for silicon only */

#define GV11B_NETLIST_IMAGE_FW_NAME NVGPU_NETLIST_IMAGE_D

#ifdef CONFIG_NVGPU_NON_FUSA
int gv11b_netlist_get_name(struct gk20a *g, int index, char *name);
#endif
bool gv11b_netlist_is_firmware_defined(void);

#endif /* NVGPU_NETLIST_GV11B_H */
