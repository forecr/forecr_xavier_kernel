// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/log2.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/fifo.h>
#include <nvgpu/engines.h>
#include <nvgpu/runlist.h>
#include <nvgpu/types.h>
#include <nvgpu/device.h>
#include <nvgpu/gin.h>

#include "hal/fifo/ramfc_gb10b.h"
#include "hal/fifo/pbdma_gb10b.h"

#include <nvgpu/hw/gb10b/hw_ram_gb10b.h>
#include <nvgpu/hw/gb10b/hw_pbdma_gb10b.h>

static u32 gb10b_ramfc_get_engine_vector(struct gk20a *g, u32 type,
		u32 inst_id)
{
	switch (type) {
	case NVGPU_DEVTYPE_GRAPHICS:
		return nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_GR, inst_id);
	case NVGPU_DEVTYPE_LCE:
		return nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_CE, inst_id);
	case NVGPU_DEVTYPE_SEC:
		return nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_SEC, inst_id);
	case NVGPU_DEVTYPE_NVENC:
		return nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_NVENC, inst_id);
	case NVGPU_DEVTYPE_NVDEC:
		return nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_NVDEC, inst_id);
	case NVGPU_DEVTYPE_NVJPG:
		return nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_NVJPG, inst_id);
	case NVGPU_DEVTYPE_OFA:
		return nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_OFA, inst_id);
	default:
		nvgpu_err(g, "unhandled device type: %u", type);
	}

	return 0;
}

void gb10b_set_channel_info(struct nvgpu_channel *ch)
{
	struct gk20a *g = ch->g;
	struct nvgpu_mem *mem = &ch->inst_block;
	u32 data;

	data = nvgpu_mem_rd32(g, mem, ram_fc_set_channel_info_w());

	data = data | (g->ops.pbdma.set_channel_info_veid(ch->subctx_id) |
		   g->ops.pbdma.set_channel_info_chid(ch->chid));
	nvgpu_mem_wr32(g, mem, ram_fc_set_channel_info_w(), data);
}

int gb10b_ramfc_setup(struct nvgpu_channel *ch, u64 gpfifo_base,
		u32 gpfifo_entries, u64 pbdma_acquire_timeout, u32 flags)
{
	struct gk20a *g = ch->g;
	struct nvgpu_mem *mem = &ch->inst_block;
	const struct nvgpu_device *dev = NULL;
	u32 data;
	u32 engine_id = 0U;
	u32 eng_intr_vector = 0U;
	u32 eng_bitmask = 0U;

	(void)flags;
	(void)pbdma_acquire_timeout;

	nvgpu_log_fn(g, " ");

	eng_bitmask = ch->runlist->eng_bitmask;
	engine_id = nvgpu_safe_sub_u32(
		nvgpu_safe_cast_u64_to_u32(nvgpu_ffs(eng_bitmask)), 1U);

	nvgpu_memset(g, mem, 0U, 0U, ram_fc_size_val_v());

	nvgpu_mem_wr32(g, mem, ram_fc_gp_base_w(),
		g->ops.pbdma.get_gp_base(gpfifo_base));

	nvgpu_mem_wr32(g, mem, ram_fc_gp_base_hi_w(),
		g->ops.pbdma.get_gp_base_hi(gpfifo_base, gpfifo_entries));

	/**
	 * Checking LIMIT2 width is sufficient to store
	 * nvgpu_ilog2(gpfifo_entries).
	 */
	nvgpu_assert((nvgpu_safe_cast_u64_to_u32(nvgpu_ilog2(gpfifo_entries)) &
			(~(U32_MAX << pbdma_misc_fetch_state_gp_info_limit2_s()))) ==
			nvgpu_safe_cast_u64_to_u32(nvgpu_ilog2(gpfifo_entries)));

	data = nvgpu_mem_rd32(g, mem, ram_fc_misc_fetch_state_w());

	data = data | pbdma_misc_fetch_state_gp_info_limit2_f(
			nvgpu_safe_cast_u64_to_u32(nvgpu_ilog2(gpfifo_entries)));

	nvgpu_mem_wr32(g, mem, ram_fc_misc_fetch_state_w(), data);

	nvgpu_mem_wr32(g, mem, ram_fc_pb_segment_extended_base_w(),
			pbdma_pb_segment_extended_base_value_f(0));

	nvgpu_mem_wr32(g, mem, ram_fc_signature_w(),
		ch->g->ops.pbdma.get_signature(ch->g));

	nvgpu_mem_wr32(g, mem, ram_fc_pb_header_w(),
		g->ops.pbdma.get_fc_pb_header());

	data = nvgpu_mem_rd32(g, mem, ram_fc_misc_fetch_state_w());

	data = data | pbdma_misc_fetch_state_pb_header_type_inc_f();

	nvgpu_mem_wr32(g, mem, ram_fc_misc_fetch_state_w(), data);

	nvgpu_mem_wr32(g, mem, ram_fc_subdevice_w(),
		g->ops.pbdma.get_fc_subdevice());

	data = nvgpu_mem_rd32(g, mem, ram_fc_misc_fetch_state_w());

	data = data | pbdma_misc_fetch_state_subdevice_status_active_f() |
		pbdma_misc_fetch_state_subdevice_channel_dma_enable_f();

	nvgpu_mem_wr32(g, mem, ram_fc_misc_fetch_state_w(), data);

	nvgpu_mem_wr32(g, mem, ram_fc_misc_execute_state_w(),
		g->ops.pbdma.get_fc_target(
			nvgpu_engine_get_active_eng_info(g, engine_id)));

	if (g->ops.ramfc.set_channel_info) {
		g->ops.ramfc.set_channel_info(ch);
	}

	nvgpu_mem_wr32(g, mem, ram_in_engine_wfi_veid_w(),
		ram_in_engine_wfi_veid_f(ch->subctx_id));

	/* get engine interrupt vector */
	dev = nvgpu_engine_get_active_eng_info(g, engine_id);
	nvgpu_assert(dev != NULL);
	eng_intr_vector = gb10b_ramfc_get_engine_vector(g, dev->type,
			dev->inst_id);

	/*
	 * engine_intr_vector can be value between 0 and 255.
	 * For example, engine_intr_vector x translates to subtree x/64,
	 * leaf (x % 64)/32 and leaf entry interrupt bit(x % 64)%32.
	 * ga10b engine_intr_vectors are 0,1,2,3,4,5. They map to
	 * subtree_0 and leaf_0(Engine non-stall interrupts) interrupt
	 * bits.
	 */
	data = g->ops.pbdma.set_intr_notify(eng_intr_vector);
	nvgpu_mem_wr32(g, mem, ram_fc_intr_notify_ctrl_w(), data);

	if (ch->is_privileged_channel) {
		/* Set privilege level for channel */
		data = nvgpu_mem_rd32(g, mem, ram_fc_misc_execute_state_w());
		nvgpu_mem_wr32(g, mem, ram_fc_misc_execute_state_w(),
			data | g->ops.pbdma.get_config_auth_level_privileged());

		/* Enable HCE priv mode for phys mode transfer */
		nvgpu_mem_wr32(g, mem, ram_fc_hce_ctrl_w(),
			g->ops.pbdma.get_ctrl_hce_priv_mode_yes());
	}

	return 0;
}

void gb10b_ramfc_capture_ram_dump_2(struct gk20a *g,
		struct nvgpu_channel *ch, struct nvgpu_channel_dump_info *info)
{
	struct nvgpu_mem *mem = &ch->inst_block;

	info->inst.pb_put = nvgpu_mem_rd32_pair(g, mem,
			ram_fc_pb_put_w(),
			ram_fc_pb_put_hi_w());
	info->inst.pb_get = nvgpu_mem_rd32_pair(g, mem,
			ram_fc_pb_get_w(),
			ram_fc_pb_get_hi_w());
	info->inst.pb_header = nvgpu_mem_rd32(g, mem,
			ram_fc_pb_header_w());
	info->inst.pb_count = nvgpu_mem_rd32(g, mem,
			ram_fc_pb_count_w());
	info->inst.sem_addr = nvgpu_mem_rd32_pair(g, mem,
			ram_fc_sem_addr_lo_w(),
			ram_fc_sem_addr_hi_w());
	info->inst.sem_payload = nvgpu_mem_rd32_pair(g, mem,
			ram_fc_sem_payload_lo_w(),
			ram_fc_sem_payload_hi_w());
	info->inst.sem_execute = nvgpu_mem_rd32(g, mem,
			ram_fc_sem_execute_w());
}
