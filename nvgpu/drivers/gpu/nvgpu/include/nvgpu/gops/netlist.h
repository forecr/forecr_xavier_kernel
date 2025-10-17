/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_NETLIST_H
#define NVGPU_GOPS_NETLIST_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * common.netlist interface.
 */
struct gk20a;

/**
 *
 * This structure stores common.netlist unit hal pointers.
 *
 * @see gpu_ops
 */
struct gops_netlist {

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	int (*get_netlist_name)(struct gk20a *g, int index, char *name);
	bool (*is_fw_defined)(void);
	void (*get_fecs_pkc_name)(struct gk20a *g, char *name);
	void (*get_gpccs_pkc_name)(struct gk20a *g, char *name);
	void (*get_fecs_desc_name)(struct gk20a *g, char *name);
	void (*get_gpccs_desc_name)(struct gk20a *g, char *name);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

};

#endif /* NVGPU_GOPS_NETLIST_H */

