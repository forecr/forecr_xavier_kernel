// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/channel.h>
#include <nvgpu/runlist.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include "runlist_ram_gb10b.h"

#include <nvgpu/hw/gb10b/hw_ram_gb10b.h>

u32 gb10b_chan_userd_ptr_hi(u32 addr_hi)
{
	return ram_rl_entry_chan_userd_ptr_hi_f(addr_hi);
}

void gb10b_runlist_get_ch_entry(struct nvgpu_channel *ch, u32 *runlist)
{
	struct gk20a *g = ch->g;
	u32 addr_lo, addr_hi, veid = 0U;
	u32 runlist_entry;

	/* Time being use 0 pbdma sequencer */
	runlist_entry = ram_rl_entry_type_channel_v() |
		ram_rl_entry_chan_runqueue_selector_f(ch->runqueue_sel) |
		ram_rl_entry_chan_userd_target_f(
			nvgpu_aperture_mask(g, ch->userd_mem,
				ram_rl_entry_chan_userd_target_sys_mem_ncoh_v(),
				ram_rl_entry_chan_userd_target_sys_mem_coh_v(),
				ram_rl_entry_chan_userd_target_vid_mem_v())) |
		ram_rl_entry_chan_inst_target_f(
			nvgpu_aperture_mask(g, &ch->inst_block,
				ram_rl_entry_chan_inst_target_sys_mem_ncoh_v(),
				ram_rl_entry_chan_inst_target_sys_mem_coh_v(),
				ram_rl_entry_chan_inst_target_vid_mem_v()));

	addr_lo = u64_lo32(ch->userd_iova) >>
			ram_rl_entry_chan_userd_ptr_align_shift_v();
	addr_hi = u64_hi32(ch->userd_iova);
	runlist[0] = runlist_entry | ram_rl_entry_chan_userd_ptr_lo_f(addr_lo);

	if (g->ops.runlist.get_ch_entry_veid != NULL) {
		veid = g->ops.runlist.get_ch_entry_veid(ch->subctx_id);
	}

	runlist[1] = g->ops.runlist.get_chan_userd_ptr_hi(addr_hi) | veid;

	addr_lo = u64_lo32(nvgpu_inst_block_addr(g, &ch->inst_block)) >>
			ram_rl_entry_chan_inst_ptr_align_shift_v();
	addr_hi = u64_hi32(nvgpu_inst_block_addr(g, &ch->inst_block));

	runlist[2] = ram_rl_entry_chan_inst_ptr_lo_f(addr_lo) |
				ram_rl_entry_chid_f(ch->chid);
	runlist[3] = ram_rl_entry_chan_inst_ptr_hi_f(addr_hi) |
				ram_rl_entry_chan_casid_f(ch->subctx_id);

	nvgpu_log_info(g, "gv11b channel runlist [0] %x [1]  %x [2] %x [3] %x",
			runlist[0], runlist[1], runlist[2], runlist[3]);
}
