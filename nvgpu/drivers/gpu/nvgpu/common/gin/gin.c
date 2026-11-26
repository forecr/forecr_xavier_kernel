// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/device.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/gin.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/gr_intr.h>

int nvgpu_gin_init(struct gk20a *g)
{
	u32 num_vectors = 0U;

	if (g->ops.gin.init == NULL) {
		g->gin.is_present = false;
		return 0;
	}

	if (g->gin.handlers != NULL) {
		return 0;
	}

	g->gin.is_present = true;

	g->ops.gin.init(g);
	nvgpu_assert(g->gin.config != NULL);

	num_vectors = g->ops.gin.get_num_vectors(g);
	g->gin.handlers = nvgpu_kzalloc(g,
			num_vectors * sizeof(struct nvgpu_gin_handler));
	if (g->gin.handlers == NULL) {
		return -ENOMEM;
	}

	nvgpu_mutex_init(&g->gin.intr_thread_mutex);
	nvgpu_spinlock_init(&g->gin.intr_lock);

	return 0;
}

u32 nvgpu_gin_get_unit_stall_vector(struct gk20a *g, u32 unit, u32 subunit)
{
	const struct nvgpu_gin_vector_config *vector_config = NULL;

	nvgpu_assert(unit < NVGPU_GIN_INTR_UNIT_MAX);
	vector_config = &(g->gin.config->vector_configs[unit]);

	nvgpu_assert(subunit < vector_config->num_stall_vectors);
	return vector_config->stall_vectors[subunit];
}

u32 nvgpu_gin_get_unit_nonstall_vector(struct gk20a *g, u32 unit, u32 subunit)
{
	const struct nvgpu_gin_vector_config *vector_config = NULL;

	nvgpu_assert(unit < NVGPU_GIN_INTR_UNIT_MAX);
	vector_config = &(g->gin.config->vector_configs[unit]);

	nvgpu_assert(subunit < vector_config->num_nonstall_vectors);
	return vector_config->nonstall_vectors[subunit];
}

u32 nvgpu_gin_get_intr_ctrl_msg(struct gk20a *g, u32 vector,
		bool cpu_enable, bool gsp_enable)
{
	return g->ops.gin.get_intr_ctrl_msg(g, vector, cpu_enable, gsp_enable);
}

u32 nvgpu_gin_get_vector_from_intr_ctrl_msg(struct gk20a *g, u32 intr_ctrl_msg)
{
	return g->ops.gin.get_vector_from_intr_ctrl_msg(g, intr_ctrl_msg);
}

void nvgpu_gin_set_stall_handler(struct gk20a *g, u32 vector,
		nvgpu_gin_stall_handler handler, u64 cookie)
{
	g->gin.handlers[vector].stall_handler = handler;
	g->gin.handlers[vector].cookie = cookie;
}

void nvgpu_gin_set_nonstall_handler(struct gk20a *g, u32 vector,
		nvgpu_gin_nonstall_handler handler, u64 cookie)
{
	g->gin.handlers[vector].nonstall_handler = handler;
	g->gin.handlers[vector].cookie = cookie;
}

static void nvgpu_gin_vector_config(struct gk20a *g, u32 vector, bool enable)
{
	u32 subtree = NVGPU_GIN_VECTOR_TO_SUBTREE(vector);
	u32 leaf_bit = NVGPU_GIN_VECTOR_TO_LEAF_BIT(vector);
	u64 leaf = BIT(leaf_bit);

	if (enable) {
		g->ops.gin.subtree_leaf_enable(g, subtree, leaf);
	} else {
		g->ops.gin.subtree_leaf_disable(g, subtree, leaf);
	}
}

void nvgpu_gin_stall_unit_config(struct gk20a *g, u32 unit, u32 subunit, bool enable)
{
	u32 vector = nvgpu_gin_get_unit_stall_vector(g, unit, subunit);
	nvgpu_gin_vector_config(g, vector, enable);
}

void nvgpu_gin_nonstall_unit_config(struct gk20a *g, u32 unit, u32 subunit, bool enable)
{
	u32 vector = nvgpu_gin_get_unit_nonstall_vector(g, unit, subunit);
	nvgpu_gin_vector_config(g, vector, enable);
}

bool nvgpu_gin_is_present(struct gk20a *g)
{
	return g->gin.is_present;
}

void nvgpu_gin_intr_mask_all(struct gk20a *g)
{
	unsigned long flags = 0;

	nvgpu_spinlock_irqsave(&g->gin.intr_lock, flags);
	g->ops.gin.write_intr_top_en_clear(g, U64_MAX);
	nvgpu_spinunlock_irqrestore(&g->gin.intr_lock, flags);
}

void nvgpu_gin_intr_stall_pause(struct gk20a *g, u64 intr_mask)
{
	unsigned long flags = 0;
	u64 intr = intr_mask & g->gin.config->stall_intr_top_mask;

	nvgpu_spinlock_irqsave(&g->gin.intr_lock, flags);
	g->ops.gin.write_intr_top_en_clear(g, intr);
	nvgpu_spinunlock_irqrestore(&g->gin.intr_lock, flags);
}

void nvgpu_gin_intr_stall_resume(struct gk20a *g, u64 intr_mask)
{
	unsigned long flags = 0;
	u64 intr = intr_mask & g->gin.config->stall_intr_top_mask;

	nvgpu_spinlock_irqsave(&g->gin.intr_lock, flags);
	g->ops.gin.write_intr_top_en_set(g, intr);
	nvgpu_spinunlock_irqrestore(&g->gin.intr_lock, flags);
}

void nvgpu_gin_intr_nonstall_pause(struct gk20a *g)
{
	unsigned long flags = 0;

	nvgpu_spinlock_irqsave(&g->gin.intr_lock, flags);
	g->ops.gin.write_intr_top_en_clear(g, g->gin.config->nonstall_intr_top_mask);
	nvgpu_spinunlock_irqrestore(&g->gin.intr_lock, flags);
}

void nvgpu_gin_intr_nonstall_resume(struct gk20a *g)
{
	unsigned long flags = 0;

	nvgpu_spinlock_irqsave(&g->gin.intr_lock, flags);
	g->ops.gin.write_intr_top_en_set(g, g->gin.config->nonstall_intr_top_mask);
	nvgpu_spinunlock_irqrestore(&g->gin.intr_lock, flags);
}

u64 nvgpu_gin_stall_isr(struct gk20a *g, u64 intr_top_mask)
{
	return g->ops.gin.read_intr_top(g, intr_top_mask);
}

void nvgpu_gin_stall_isr_handle_subtree(struct gk20a *g, u32 subtree, u64 leaf_mask)
{
	u64 leaf = g->ops.gin.subtree_leaf_status(g, subtree);

	leaf = leaf & leaf_mask;

	while (leaf != 0UL) {
		u64 leaf_bit = nvgpu_safe_sub_u64(nvgpu_ffs(leaf), 1UL);
		u64 vector = NVGPU_GIN_VECTOR(subtree, leaf_bit);
		struct nvgpu_gin_handler *handler =  &(g->gin.handlers[vector]);
		nvgpu_gin_stall_handler stall_handler = handler->stall_handler;
		u64 cookie = handler->cookie;

		g->ops.gin.subtree_leaf_clear(g, subtree, BIT(leaf_bit));

		if (stall_handler != NULL) {
			stall_handler(g, cookie);
		} else {
			nvgpu_err(g, "NULL handler for active vector %llu", vector);
		}

		leaf &= ~BIT(leaf_bit);
	}
}

bool nvgpu_gin_handle_gr_illegal_method(struct gk20a *g, u64 top_pending)
{
	const struct nvgpu_device *dev =
		nvgpu_device_get(g, NVGPU_DEVTYPE_GRAPHICS,
				 nvgpu_gr_get_syspipe_id(g, g->mig.cur_gr_instance));
	u32 stall_vector;
	bool handled = false;
	u32 subtree;
	u32 leaf_bit;
	u64 leaf;

	if (dev == NULL) {
		return false;
	}

	stall_vector = nvgpu_gin_get_unit_stall_vector(g,
				NVGPU_GIN_INTR_UNIT_GR, dev->inst_id);

	subtree = NVGPU_GIN_VECTOR_TO_SUBTREE(stall_vector);
	leaf_bit = NVGPU_GIN_VECTOR_TO_LEAF_BIT(stall_vector);
	leaf = BIT(leaf_bit);

	while (top_pending != 0UL) {
		u32 top_bit = nvgpu_safe_cast_u64_to_u32(
				nvgpu_safe_sub_u64(nvgpu_ffs(top_pending), 1UL));

		if ((top_bit == subtree) && nvgpu_gr_is_illegal_method_intr_set(g)) {
			g->in_isr = true;
			nvgpu_gin_stall_isr_handle_subtree(g, top_bit, leaf);
			g->in_isr = false;
			handled = true;
			break;
		}

		top_pending ^= BIT(top_bit);
	}

	return handled;
}

bool nvgpu_gin_handle_nvdec_swgen0(struct gk20a *g, u64 top_pending)
{
	const struct nvgpu_device *dev = NULL;
	u32 inst_ids[NVDEC_INST_MAX];
	u32 subtree[NVDEC_INST_MAX];
	u64 leaf[NVDEC_INST_MAX];
	u32 num_instances = 0U;
	bool handled = false;
	u32 stall_vector;
	u32 leaf_bit;
	u32 i;

	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_NVDEC) {
		stall_vector = nvgpu_gin_get_unit_stall_vector(g,
					NVGPU_GIN_INTR_UNIT_NVDEC, dev->inst_id);
		inst_ids[num_instances] = dev->inst_id;
		subtree[num_instances] = NVGPU_GIN_VECTOR_TO_SUBTREE(stall_vector);
		leaf_bit = NVGPU_GIN_VECTOR_TO_LEAF_BIT(stall_vector);
		leaf[num_instances] = BIT(leaf_bit);
		num_instances++;
	}

	while (top_pending != 0UL) {
		u32 top_bit = nvgpu_safe_cast_u64_to_u32(
				nvgpu_safe_sub_u64(nvgpu_ffs(top_pending), 1UL));

		for (i = 0; i < num_instances; i++) {
			if ((top_bit == subtree[i]) &&
			    g->ops.nvdec.nvdec_is_swgen0_set(g, inst_ids[i])) {
				g->in_isr = true;
				nvgpu_gin_stall_isr_handle_subtree(g, top_bit, leaf[i]);
				g->in_isr = false;
				handled = true;
			}
		}

		top_pending ^= BIT(top_bit);
	}

	return handled;
}

void nvgpu_gin_stall_isr_thread(struct gk20a *g, u64 intr_top_mask)
{
	u64 top_pending = 0UL;

	top_pending = g->ops.gin.read_intr_top(g, intr_top_mask);
	if (top_pending == 0UL) {
		nvgpu_log(g, gpu_dbg_intr, "stall intr already handled");
		return;
	}

	while (top_pending != 0UL) {
		u32 top_bit = nvgpu_safe_cast_u64_to_u32(
				nvgpu_safe_sub_u64(nvgpu_ffs(top_pending), 1UL));

		nvgpu_gin_stall_isr_handle_subtree(g, top_bit, U64_MAX);

		top_pending ^= BIT(top_bit);
	}
}

u64 nvgpu_gin_nonstall_isr(struct gk20a *g, u64 intr_top_mask)
{
	return g->ops.gin.read_intr_top(g, intr_top_mask);
}

static u32 nvgpu_gin_nonstall_isr_handle_subtree(struct gk20a *g, u32 subtree)
{
	u32 ops = 0U;
	u64 leaf = g->ops.gin.subtree_leaf_status(g, subtree);

	while (leaf != 0UL) {
		u64 leaf_bit = nvgpu_safe_sub_u64(nvgpu_ffs(leaf), 1UL);
		u64 vector = NVGPU_GIN_VECTOR(subtree, leaf_bit);
		struct nvgpu_gin_handler *handler = &(g->gin.handlers[vector]);
		nvgpu_gin_nonstall_handler nonstall_handler = handler->nonstall_handler;
		u64 cookie = handler->cookie;

		g->ops.gin.subtree_leaf_clear(g, subtree, BIT(leaf_bit));

		if (nonstall_handler != NULL) {
			ops |= nonstall_handler(g, cookie);
		} else {
			nvgpu_err(g, "NULL handler for active vector %llu", vector);
		}

		leaf &= ~BIT(leaf_bit);
	}

	return ops;
}

u32 nvgpu_gin_nonstall_isr_thread(struct gk20a *g, u64 intr_top_mask)
{
	u64 top_pending = 0UL;
	u32 ops = 0U;

	top_pending = g->ops.gin.read_intr_top(g, intr_top_mask);
	if (top_pending == 0UL) {
		nvgpu_log(g, gpu_dbg_intr, "nonstall intr already handled");
		return 0U;
	}

	while (top_pending != 0UL) {
		u32 top_bit = nvgpu_safe_cast_u64_to_u32(
				nvgpu_safe_sub_u64(nvgpu_ffs(top_pending), 1UL));

		ops |= nvgpu_gin_nonstall_isr_handle_subtree(g, top_bit);

		top_pending ^= BIT(top_bit);
	}

	return ops;
}

bool nvgpu_gin_msi_is_stall(struct gk20a *g, u32 id)
{
	return ((g->gin.config->stall_intr_top_mask & BIT(id)) != 0UL);
}

bool nvgpu_gin_msi_is_nonstall(struct gk20a *g, u32 id)
{
	return ((g->gin.config->nonstall_intr_top_mask & BIT(id)) != 0UL);
}

void nvgpu_gin_clear_pending_intr(struct gk20a *g, u32 vector)
{
	u32 subtree = NVGPU_GIN_VECTOR_TO_SUBTREE(vector);
	u32 leaf_bit = NVGPU_GIN_VECTOR_TO_LEAF_BIT(vector);
	u64 leaf = BIT(leaf_bit);

	g->ops.gin.subtree_leaf_clear(g, subtree, leaf);
}
