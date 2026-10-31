/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef MINION_GV100_H
#define MINION_GV100_H

#include <nvgpu/types.h>
enum nvgpu_nvlink_minion_dlcmd;
struct gk20a;

u32 gv100_nvlink_minion_base_addr(struct gk20a *g);
bool gv100_nvlink_minion_is_running(struct gk20a *g);
int gv100_nvlink_minion_is_boot_complete(struct gk20a *g, bool *boot_cmplte);
u32 gv100_nvlink_minion_get_dlcmd_ordinal(struct gk20a *g,
			enum nvgpu_nvlink_minion_dlcmd dlcmd);
int gv100_nvlink_minion_send_dlcmd(struct gk20a *g, u32 link_id,
			enum nvgpu_nvlink_minion_dlcmd dlcmd, bool sync);
void gv100_nvlink_minion_clear_intr(struct gk20a *g);
void gv100_nvlink_minion_init_intr(struct gk20a *g);
void gv100_nvlink_minion_enable_link_intr(struct gk20a *g, u32 link_id,
								bool enable);
void gv100_nvlink_minion_falcon_isr(struct gk20a *g);
void gv100_nvlink_minion_isr(struct gk20a *g);

#endif /* MINION_GV100_H */
