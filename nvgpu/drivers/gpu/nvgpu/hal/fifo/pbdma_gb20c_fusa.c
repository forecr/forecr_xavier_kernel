// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/io.h>

#include "pbdma_gb20c.h"

#include <nvgpu/hw/gb20c/hw_pbdma_gb20c.h>
#include <nvgpu/hw/gb20c/hw_proj_gb20c.h>

static u32 gb20c_pbdma_method_reg(struct gk20a *g, u32 pbdma_id,
			u32 pbdma_method_index)
{
	u32 pbdma_method_stride;
	u32 pbdma_method_reg;

	u32 pbdma_base = g->pbdma_id_to_base[pbdma_id];
	pbdma_method_stride = nvgpu_safe_sub_u32(pbdma_method_r(1),
				pbdma_method_r(0));

	pbdma_method_reg = nvgpu_safe_add_u32(nvgpu_safe_add_u32(pbdma_base,
		pbdma_method_r(0)), nvgpu_safe_mult_u32(pbdma_method_index,
		pbdma_method_stride));

	return pbdma_method_reg;
}

u32 gb20c_pbdma_read_data(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base;

	pbdma_base = g->pbdma_id_to_base[pbdma_id];
	return nvgpu_readl(g, nvgpu_safe_add_u32(pbdma_base, pbdma_hdr_shadow_r()));
}

u32 gb20c_pbdma_get_mmu_fault_id(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base, pbdma_cfg0;

	pbdma_base = g->pbdma_id_to_base[pbdma_id];
	pbdma_cfg0 = nvgpu_readl(g, nvgpu_safe_add_u32(pbdma_base, pbdma_cfg0_r()));

	return pbdma_cfg0_pbdma_fault_id_v(pbdma_cfg0);
}

u32 gb20c_pbdma_get_fc_target(const struct nvgpu_device *dev)
{
	return (pbdma_misc_execute_state_target_engine_f(dev->rleng_id) |
			pbdma_misc_execute_state_target_eng_ctx_valid_true_f() |
			pbdma_misc_execute_state_target_ce_ctx_valid_true_f());
}

u32 gb20c_pbdma_get_config_auth_level_privileged(void)
{
	return pbdma_misc_execute_state_config_auth_level_privileged_f();
}

u32 gb20c_pbdma_get_num_of_pbdmas(void)
{
	return proj_host_num_pbdma_v();
}

static void gb20c_pbdma_disable_all_intr(struct gk20a *g)
{
	u32 pbdma_id = 0U;
	u32 tree = 0U;
	u32 pbdma_id_max = proj_host_num_pbdma_v();
	u32 pbdma_base = 0U;

	for (pbdma_id = 0U; pbdma_id < pbdma_id_max; pbdma_id++) {
		pbdma_base = g->pbdma_id_to_base[pbdma_id];
		for (tree = 0U; tree < pbdma_intr_0_en_clear_tree__size_1_v();
			tree++) {
			nvgpu_writel(g, nvgpu_safe_add_u32(pbdma_base,
				pbdma_intr_0_en_clear_tree_r(tree)),
				g->ops.pbdma.intr_0_en_clear_tree_mask());
			nvgpu_writel(g, nvgpu_safe_add_u32(pbdma_base,
				pbdma_intr_1_en_clear_tree_r(tree)),
				g->ops.pbdma.intr_1_en_clear_tree_mask());
		}
	}
}

static void gb20c_pbdma_clear_all_intr(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base = g->pbdma_id_to_base[pbdma_id];
	nvgpu_writel(g, nvgpu_safe_add_u32(pbdma_base, pbdma_intr_0_r()), U32_MAX);
	nvgpu_writel(g, nvgpu_safe_add_u32(pbdma_base, pbdma_intr_1_r()), U32_MAX);
}

static void gb20c_pbdma_disable_and_clear_all_intr(struct gk20a *g)
{
	u32 pbdma_id = 0U;
	u32 pbdma_id_max = proj_host_num_pbdma_v();

	gb20c_pbdma_disable_all_intr(g);

	for (pbdma_id = 0U; pbdma_id < pbdma_id_max; pbdma_id++) {
		gb20c_pbdma_clear_all_intr(g, pbdma_id);
	}
}

void gb20c_pbdma_intr_enable(struct gk20a *g, bool enable)
{
	u32 pbdma_id = 0U;
	u32 tree = 0U, pbdma_base;
	u32 pbdma_id_max = proj_host_num_pbdma_v();

	if (!enable) {
		gb20c_pbdma_disable_and_clear_all_intr(g);
		return;
	}

	for (pbdma_id = 0U; pbdma_id < pbdma_id_max; pbdma_id++) {
		gb20c_pbdma_clear_all_intr(g, pbdma_id);

		/* enable pbdma interrupts and route to tree_0 */
		pbdma_base = g->pbdma_id_to_base[pbdma_id];
		nvgpu_writel(g, nvgpu_safe_add_u32(pbdma_base,
			pbdma_intr_0_en_set_tree_r(tree)),
			g->ops.pbdma.intr_0_en_set_tree_mask());
		nvgpu_writel(g, nvgpu_safe_add_u32(pbdma_base,
			pbdma_intr_1_en_set_tree_r(tree)),
			g->ops.pbdma.intr_1_en_set_tree_mask());
	}
}

void gb20c_pbdma_reset_method(struct gk20a *g, u32 pbdma_id,
			u32 pbdma_method_index)
{
	u32 pbdma_method_reg = gb20c_pbdma_method_reg(g, pbdma_id, pbdma_method_index);

	nvgpu_writel(g, pbdma_method_reg,
			pbdma_method_valid_true_f() |
			pbdma_method_first_true_f() |
			pbdma_method_addr_f(
			     U32(pbdma_udma_nop_r()) >> 2U));
}

void gb20c_pbdma_dump_intr_0(struct gk20a *g, u32 pbdma_id,
				u32 pbdma_intr_0)
{
	u32 i, pbdma_base;
	u32 header, data;
	u32 shadow[pbdma_gp_shadow_size_v()];
	u32 method[pbdma_method_size_v()];

	pbdma_base = g->pbdma_id_to_base[pbdma_id];

	header = nvgpu_readl(g, nvgpu_safe_add_u32(pbdma_base, pbdma_pb_header_r()));
	data = g->ops.pbdma.read_data(g, pbdma_id);

	for (i = 0U; i < pbdma_gp_shadow_size_v(); i++) {
		shadow[i] = nvgpu_readl(g, nvgpu_safe_add_u32(pbdma_base,
			pbdma_gp_shadow_r(i)));
	}

	for (i = 0U; i < pbdma_method_size_v(); i++) {
		method[i] = nvgpu_readl(g, nvgpu_safe_add_u32(pbdma_base,
			pbdma_method_r(i)));
	}

	nvgpu_err(g,
		"pbdma_intr_0(%d):0x%08x PBH: %08x "
		"SHADOW: %08x gp shadow0: %08x gp shadow1: %08x"
		"M0: %08x %08x %08x %08x ",
		pbdma_id, pbdma_intr_0, header, data,
		shadow[0], shadow[1], method[0], method[1], method[2], method[3]);
}

void gb20c_pbdma_reset_header(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base, data;

	pbdma_base = g->pbdma_id_to_base[pbdma_id];
	data = nvgpu_readl(g, nvgpu_safe_add_u32(pbdma_base,
		pbdma_misc_fetch_state_r()));

	nvgpu_writel(g, nvgpu_safe_add_u32(pbdma_base, pbdma_pb_header_r()),
			pbdma_pb_header_first_true_f());

	nvgpu_writel(g, nvgpu_safe_add_u32(pbdma_base, pbdma_misc_fetch_state_r()),
			data |
			pbdma_misc_fetch_state_pb_header_type_non_inc_f());
}

bool gb20c_pbdma_is_sw_method_subch(struct gk20a *g, u32 pbdma_id,
					u32 pbdma_method_index)
{
	u32 pbdma_method_subch;
	u32 pbdma_method_reg = gb20c_pbdma_method_reg(g, pbdma_id, pbdma_method_index);

	pbdma_method_subch = pbdma_method_subch_v(
			nvgpu_readl(g, pbdma_method_reg));

	if ((pbdma_method_subch == PBDMA_METHOD_SUBCH_FIVE) ||
		(pbdma_method_subch == PBDMA_METHOD_SUBCH_SIX) ||
		(pbdma_method_subch == PBDMA_METHOD_SUBCH_SEVEN)) {
		return true;
	}

	return false;
}

u32 gb20c_intr_0_reg_addr(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base = g->pbdma_id_to_base[pbdma_id];
	return nvgpu_safe_add_u32(pbdma_base, pbdma_intr_0_r());
}

u32 gb20c_intr_1_reg_addr(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base = g->pbdma_id_to_base[pbdma_id];
	return nvgpu_safe_add_u32(pbdma_base, pbdma_intr_1_r());
}

u32 gb20c_hce_dbg0_reg_addr(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base = g->pbdma_id_to_base[pbdma_id];
	return nvgpu_safe_add_u32(pbdma_base, pbdma_hce_dbg0_r());
}

u32 gb20c_hce_dbg1_reg_addr(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base = g->pbdma_id_to_base[pbdma_id];
	return nvgpu_safe_add_u32(pbdma_base, pbdma_hce_dbg1_r());
}