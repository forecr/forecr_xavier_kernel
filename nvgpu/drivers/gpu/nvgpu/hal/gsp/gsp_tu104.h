/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GSP_TU104_H
#define GSP_TU104_H

int tu104_gsp_reset(struct gk20a *g, u32 inst_id);
void tu104_gsp_flcn_setup_boot_config(struct gk20a *g);
u32 tu104_gsp_falcon_base_addr(void);

#endif /* GSP_TU104_H */
