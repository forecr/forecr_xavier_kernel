// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/timers.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "common/vbios/bios_sw_tu104.h"
#include "common/vbios/bios_sw_ga100.h"

#define NVGPU_PG209_MIN_VBIOS		0

struct nvgpu_vbios_board {
	u16 board_id;
	u32 vbios_version;
};

#define NVGPU_GA100_NUM_BOARDS		1

static struct nvgpu_vbios_board vbios_boards[NVGPU_GA100_NUM_BOARDS] = {
	/* PG209 SKU 200 */
	[0] = {
		.board_id = 0x0209,
		.vbios_version = 0,	/* any VBIOS for now */
	},
};

static int ga100_bios_verify_version(struct gk20a *g)
{
	struct nvgpu_vbios_board *board = NULL;
	u32 i;

	nvgpu_info(g, "VBIOS board id %04x", g->bios->vbios_board_id);

	nvgpu_info(g, "VBIOS version %08x:%02x\n",
		g->bios->vbios_version,
		g->bios->vbios_oem_version);

#if NVGPU_PG209_MIN_VBIOS
	if (g->bios->vbios_version < NVGPU_PG209_MIN_VBIOS) {
		nvgpu_err(g, "unsupported VBIOS version %08x",
			g->bios->vbios_version);
		return -EINVAL;
	}
#endif

	for (i = 0; i < NVGPU_GA100_NUM_BOARDS; i++) {
		if (g->bios->vbios_board_id == vbios_boards[i].board_id) {
			board = &vbios_boards[i];
		}
	}

	if (board == NULL) {
		nvgpu_warn(g, "unknown board id %04x",
			g->bios->vbios_board_id);
		return 0;
	}

	if ((board->vbios_version != 0U) &&
		(g->bios->vbios_version < board->vbios_version)) {
		nvgpu_warn(g, "VBIOS version should be at least %08x",
			board->vbios_version);
	}

	return 0;
}

void nvgpu_ga100_bios_sw_init(struct gk20a *g, struct nvgpu_bios *bios)
{
	bios->init = tu104_bios_init;
	bios->verify_version = ga100_bios_verify_version;
	bios->preos_wait_for_halt = NULL;
	bios->preos_reload_check = NULL;
	bios->preos_bios = NULL;
	bios->devinit_bios = NULL;
	bios->verify_devinit = tu104_bios_verify_devinit;
}

