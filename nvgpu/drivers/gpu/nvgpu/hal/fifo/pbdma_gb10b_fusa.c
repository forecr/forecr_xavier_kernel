// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/device.h>
#include <nvgpu/nvgpu_err.h>

#include "pbdma_gb10b.h"

#include <nvgpu/hw/gb10b/hw_pbdma_gb10b.h>
#include <nvgpu/hw/gb10b/hw_gin_gb10b.h>

u32 gb10b_pbdma_get_fc_pb_header(void)
{
	return (pbdma_pb_header_method_zero_f() |
		pbdma_pb_header_subchannel_zero_f() |
		pbdma_pb_header_level_main_f() |
		pbdma_pb_header_first_true_f());
}

u32 gb10b_pbdma_intr_0_en_set_tree_mask(void)
{
	u32 mask = pbdma_intr_0_en_set_tree_gpfifo_enabled_f()    |
			pbdma_intr_0_en_set_tree_gpptr_enabled_f()     |
			pbdma_intr_0_en_set_tree_gpentry_enabled_f()   |
			pbdma_intr_0_en_set_tree_pbptr_enabled_f()     |
			pbdma_intr_0_en_set_tree_pbentry_enabled_f()   |
			pbdma_intr_0_en_set_tree_method_enabled_f()    |
			pbdma_intr_0_en_set_tree_device_enabled_f()    |
			pbdma_intr_0_en_set_tree_semaphore_enabled_f() |
			pbdma_intr_0_en_set_tree_pri_enabled_f()       |
			pbdma_intr_0_en_set_tree_pbseg_enabled_f()     |
			pbdma_intr_0_en_set_tree_signature_enabled_f();

	return mask;
}

u32 gb10b_pbdma_intr_0_en_clear_tree_mask(void)
{
	u32 mask = pbdma_intr_0_en_clear_tree_gpfifo_enabled_f()    |
			pbdma_intr_0_en_clear_tree_gpptr_enabled_f()     |
			pbdma_intr_0_en_clear_tree_gpentry_enabled_f()   |
			pbdma_intr_0_en_clear_tree_pbptr_enabled_f()     |
			pbdma_intr_0_en_clear_tree_pbentry_enabled_f()   |
			pbdma_intr_0_en_clear_tree_method_enabled_f()    |
			pbdma_intr_0_en_clear_tree_device_enabled_f()    |
			pbdma_intr_0_en_clear_tree_semaphore_enabled_f() |
			pbdma_intr_0_en_clear_tree_pri_enabled_f()       |
			pbdma_intr_0_en_clear_tree_pbseg_enabled_f()     |
			pbdma_intr_0_en_clear_tree_signature_enabled_f();

	return mask;
}

u32 gb10b_pbdma_get_gp_base_hi(u64 gpfifo_base, u32 gpfifo_entry)
{
	(void)gpfifo_entry;
	return pbdma_gp_base_hi_offset_f(u64_hi32(gpfifo_base));
}

void gb10b_pbdma_report_error(struct gk20a *g, u32 pbdma_id,
		u32 pbdma_intr_0)
{
	u32 err_type = GPU_HOST_INVALID_ERROR;

	/*
	 * Multiple errors have been grouped as part of a single
	 * top-level error.
	 */
	if ((pbdma_intr_0 & (
		pbdma_intr_0_gpfifo_pending_f() |
		pbdma_intr_0_gpptr_pending_f() |
		pbdma_intr_0_pbptr_pending_f() |
		pbdma_intr_0_pbentry_pending_f())) != 0U) {
			err_type = GPU_HOST_PBDMA_GPFIFO_PB_ERROR;
	}
	if ((pbdma_intr_0 & (
		pbdma_intr_0_method_pending_f() |
		pbdma_intr_0_device_pending_f() |
		pbdma_intr_0_semaphore_pending_f() |
		pbdma_intr_0_pri_pending_f() |
		pbdma_intr_0_pbseg_pending_f())) != 0U) {
			err_type = GPU_HOST_PBDMA_METHOD_ERROR;
	}
	if ((pbdma_intr_0 &
		pbdma_intr_0_signature_pending_f()) != 0U) {
			err_type = GPU_HOST_PBDMA_SIGNATURE_ERROR;
	}
	if (err_type != GPU_HOST_INVALID_ERROR) {
		nvgpu_err(g, "pbdma_intr_0(%d)= 0x%08x ",
				pbdma_id, pbdma_intr_0);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_HOST, err_type);
	}
	return;
}

u32 gb10b_pbdma_get_num_of_pbdmas(void)
{
	return pbdma_cfg0__size_1_v();
}

u32 gb10b_pbdma_read_data(struct gk20a *g, u32 pbdma_id)
{
	return nvgpu_readl(g, pbdma_hdr_shadow_r(pbdma_id));
}

u32 gb10b_pbdma_set_clear_intr_offsets(struct gk20a *g,
				       u32 set_clear_size)
{
	u32 ret = 0U;
	switch(set_clear_size) {
		case INTR_SIZE:
			ret = pbdma_intr_0__size_1_v();
			break;
		case INTR_SET_SIZE:
			ret = pbdma_intr_0_en_set_tree__size_1_v();
			break;
		case INTR_CLEAR_SIZE:
			ret = pbdma_intr_0_en_clear_tree__size_1_v();
			break;
		default:
			nvgpu_err(g, "Invalid input for set_clear_intr_offset");
			break;
	}

	return ret;
}

u32 gb10b_pbdma_set_intr_notify(u32 eng_intr_vector)
{
	return gin_intr_ctrl_access_defines_vector_f(eng_intr_vector);
}

u32 gb10b_pbdma_get_fc_target(const struct nvgpu_device *dev)
{
	return (pbdma_misc_execute_state_target_engine_f(dev->rleng_id) |
			pbdma_misc_execute_state_target_eng_ctx_valid_true_f() |
			pbdma_misc_execute_state_target_ce_ctx_valid_true_f());
}

u32 gb10b_pbdma_get_config_auth_level_privileged(void)
{
	return pbdma_misc_execute_state_config_auth_level_privileged_f();
}

u32 gb10b_pbdma_get_fc_subdevice(void)
{
	return (pbdma_subdevice_id_f(PBDMA_SUBDEVICE_ID));
}

u32 gb10b_pbdma_channel_fatal_0_intr_descs(void)
{
	/*
	 * These are data parsing, framing errors or others which can be
	 * recovered from with intervention... or just resetting the
	 * channel
	 */
	u32 channel_fatal_0_intr_descs =
		pbdma_intr_0_gpfifo_pending_f() |
		pbdma_intr_0_gpptr_pending_f() |
		pbdma_intr_0_pbptr_pending_f() |
		pbdma_intr_0_pbentry_pending_f() |
		pbdma_intr_0_method_pending_f() |
		pbdma_intr_0_pbseg_pending_f() |
		pbdma_intr_0_semaphore_pending_f() |
		pbdma_intr_0_signature_pending_f();

	return channel_fatal_0_intr_descs;
}

void gb10b_pbdma_reset_method(struct gk20a *g, u32 pbdma_id,
			u32 pbdma_method_index)
{
	u32 pbdma_method_stride;
	u32 pbdma_method_reg;

	pbdma_method_stride = nvgpu_safe_sub_u32(pbdma_method1_r(pbdma_id),
				pbdma_method0_r(pbdma_id));

	pbdma_method_reg = nvgpu_safe_add_u32(pbdma_method0_r(pbdma_id),
				nvgpu_safe_mult_u32(pbdma_method_index,
					pbdma_method_stride));

	nvgpu_writel(g, pbdma_method_reg,
			pbdma_method0_valid_true_f() |
			pbdma_method0_first_true_f() |
			pbdma_method0_addr_f(
			     U32(pbdma_udma_nop_r()) >> 2U));
}

void gb10b_pbdma_dump_intr_0(struct gk20a *g, u32 pbdma_id,
				u32 pbdma_intr_0)
{
	u32 header = nvgpu_readl(g, pbdma_pb_header_r(pbdma_id));
	u32 data = g->ops.pbdma.read_data(g, pbdma_id);
	u32 shadow_0 = nvgpu_readl(g, pbdma_gp_shadow_0_r(pbdma_id));
	u32 shadow_1 = nvgpu_readl(g, pbdma_gp_shadow_1_r(pbdma_id));
	u32 method0 = nvgpu_readl(g, pbdma_method0_r(pbdma_id));
	u32 method1 = nvgpu_readl(g, pbdma_method1_r(pbdma_id));
	u32 method2 = nvgpu_readl(g, pbdma_method2_r(pbdma_id));
	u32 method3 = nvgpu_readl(g, pbdma_method3_r(pbdma_id));

	nvgpu_err(g,
		"pbdma_intr_0(%d):0x%08x PBH: %08x "
		"SHADOW: %08x gp shadow0: %08x gp shadow1: %08x"
		"M0: %08x %08x %08x %08x ",
		pbdma_id, pbdma_intr_0, header, data,
		shadow_0, shadow_1, method0, method1, method2, method3);
}

void gb10b_pbdma_reset_header(struct gk20a *g, u32 pbdma_id)
{
	u32 data = nvgpu_readl(g, pbdma_misc_fetch_state_r(pbdma_id));

	nvgpu_writel(g, pbdma_pb_header_r(pbdma_id),
			pbdma_pb_header_first_true_f());

	nvgpu_writel(g, pbdma_misc_fetch_state_r(pbdma_id),
			data |
			pbdma_misc_fetch_state_pb_header_type_non_inc_f());
}

bool gb10b_pbdma_is_sw_method_subch(struct gk20a *g, u32 pbdma_id,
						u32 pbdma_method_index)
{
	u32 pbdma_method_stride;
	u32 pbdma_method_reg, pbdma_method_subch;

	pbdma_method_stride = nvgpu_safe_sub_u32(pbdma_method1_r(pbdma_id),
				pbdma_method0_r(pbdma_id));

	pbdma_method_reg = nvgpu_safe_add_u32(pbdma_method0_r(pbdma_id),
				nvgpu_safe_mult_u32(pbdma_method_index,
					pbdma_method_stride));

	pbdma_method_subch = pbdma_method0_subch_v(
			nvgpu_readl(g, pbdma_method_reg));

	if ((pbdma_method_subch == PBDMA_METHOD_SUBCH_FIVE) ||
		(pbdma_method_subch == PBDMA_METHOD_SUBCH_SIX) ||
		(pbdma_method_subch == PBDMA_METHOD_SUBCH_SEVEN)) {
		return true;
	}

	return false;
}
