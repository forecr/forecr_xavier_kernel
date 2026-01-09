/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVLINK_MINION_H
#define NVGPU_NVLINK_MINION_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_firmware;

#define MINION_REG_RD32(g, off) nvgpu_readl(g, (g)->nvlink.minion_base + (off))
#define MINION_REG_WR32(g, off, v) nvgpu_writel(g, (g)->nvlink.minion_base + (off), (v))

enum nvgpu_nvlink_minion_dlcmd {
	NVGPU_NVLINK_MINION_DLCMD_INITPHY,
	NVGPU_NVLINK_MINION_DLCMD_INITLANEENABLE,
	NVGPU_NVLINK_MINION_DLCMD_INITDLPL,
	NVGPU_NVLINK_MINION_DLCMD_INITRXTERM,
	NVGPU_NVLINK_MINION_DLCMD_INITTL,
	NVGPU_NVLINK_MINION_DLCMD_LANEDISABLE,
	NVGPU_NVLINK_MINION_DLCMD_SETACMODE,
	NVGPU_NVLINK_MINION_DLCMD_LANESHUTDOWN,
	NVGPU_NVLINK_MINION_DLCMD_TXCLKSWITCH_PLL,
	NVGPU_NVLINK_MINION_DLCMD_INITPLL_1,
	NVGPU_NVLINK_MINION_DLCMD_TURING_INITDLPL_TO_CHIPA,
	NVGPU_NVLINK_MINION_DLCMD_TURING_RXDET,
	NVGPU_NVLINK_MINION_DLCMD__LAST,
};

u32 nvgpu_nvlink_minion_extract_word(struct nvgpu_firmware *fw, u32 idx);
int nvgpu_nvlink_minion_load(struct gk20a *g);
int nvgpu_nvlink_minion_load_ucode(struct gk20a *g,
				struct nvgpu_firmware *nvgpu_minion_fw);
void nvgpu_nvlink_free_minion_used_mem(struct gk20a *g,
				struct nvgpu_firmware *nvgpu_minion_fw);
#endif /* NVGPU_NVLINK_MINION_H */
