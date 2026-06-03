/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_NETLIST_GA100_H
#define NVGPU_NETLIST_GA100_H

#include <nvgpu/types.h>
#include <nvgpu/netlist_defs.h>

struct gk20a;

#define GA100_NETLIST_IMAGE_FW_NAME NVGPU_NETLIST_IMAGE_C

int ga100_netlist_get_name(struct gk20a *g, int index, char *name);
bool ga100_netlist_is_firmware_defined(void);

#endif /*NVGPU_NETLIST_GA100_H*/
