// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/runlist.h>
#include <nvgpu/gin.h>
#include <nvgpu/cic_mon.h>

#include "hal/fifo/fifo_utils_ga10b.h"
#include "fifo_intr_gb10b.h"

#include <nvgpu/hw/gb10b/hw_runlist_gb10b.h>

/*
 *      [runlist's tree 0 bit] <---------. .---------> [runlist's tree 1 bit]
 *                                        Y
 *                                        |
 *                                        |
 *      [runlist intr tree 0]             ^             [runlist intr tree 1]
 *                        ______________/   \______________
 *                       /                                  |
 *          NV_RUNLIST_INTR_CTRL(0) msg       NV_RUNLIST_INTR_CTRL(1) msg
 *                      |                                   |
 *                ______^______                       ______^______
 *               /             \                     /             \
 *              |      OR       |                   |      OR       |
 *              '_______________'                   '_______________'
 *               |||||||       |                     |       |||||||
 *             other tree0     |                     |     other tree1
 *           ANDed intr bits   ^                     ^   ANDed intr bits
 *                            AND                   AND
 *                            | |                   | |
 *                     _______. .______      _______. .________
 *                    /                 \   /                  \
 * RUNLIST_INTR_0_EN_SET_TREE(0)_intr_bit Y RUNLIST_INTR_0_EN_SET_TREE(1)_intr_bit
 *                                        |
 *                            NV_RUNLIST_INTR_0_intr_bit
 */

u32 gb10b_runlist_intr_ctrl(u32 intr_tree)
{
	return runlist_intr_ctrl_r(intr_tree);
}

void gb10b_fifo_intr_top_enable(struct gk20a *g, bool enable)
{
	u32 i = 0U;
	struct nvgpu_runlist *runlist = NULL;

	for (i = 0U; i < g->fifo.num_runlists; i++) {
		runlist = &g->fifo.active_runlists[i];

		if (enable) {
			nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_RUNLIST_TREE_0,
					runlist->id, NVGPU_GIN_INTR_ENABLE);

			/**
			 * RUNLIST_TREE_1 interrupts are not enabled as all runlist
			 * interrupts are routed to runlist_tree_0
			 */
			nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_RUNLIST_TREE_1,
					runlist->id, NVGPU_GIN_INTR_DISABLE);
		} else {
			nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_RUNLIST_TREE_0,
					runlist->id, NVGPU_GIN_INTR_DISABLE);
		}
	}
}

static u32 gb10b_fifo_runlist_get_vectorid(struct gk20a *g,
		struct nvgpu_runlist *runlist, u32 intr_tree)
{
	const u32 gin_units[runlist_intr_ctrl__size_1_v()] = {
		NVGPU_GIN_INTR_UNIT_RUNLIST_TREE_0,
		NVGPU_GIN_INTR_UNIT_RUNLIST_TREE_1,
	};

	nvgpu_assert(intr_tree < runlist_intr_ctrl__size_1_v());

	return nvgpu_gin_get_unit_stall_vector(g, gin_units[intr_tree],
			runlist->id);
}

static void gb10b_fifo_runlist_tree_0_stall_handler(struct gk20a *g, u64 cookie)
{
	(void)cookie;
	g->ops.fifo.intr_0_isr(g);
	g->ops.fifo.runlist_intr_retrigger(g, RUNLIST_INTR_TREE_0);
}

void gb10b_fifo_runlist_intr_vectorid_init(struct gk20a *g)
{
	u32 i = 0U;
	u32 intr_tree = 0U;
	u32 vector = 0U;
	struct nvgpu_runlist *runlist = NULL;

	for (intr_tree = 0U; intr_tree < runlist_intr_ctrl__size_1_v();
			intr_tree++) {
		for (i = 0U; i < g->fifo.num_runlists; i++) {
			runlist = &g->fifo.active_runlists[i];
			vector = gb10b_fifo_runlist_get_vectorid(g, runlist, intr_tree);
			nvgpu_gin_set_stall_handler(g, vector,
					&gb10b_fifo_runlist_tree_0_stall_handler, 0);
		}
	}

}

static void gb10b_fifo_runlist_intr_disable(struct gk20a *g)
{
	u32 i, intr_tree, reg_val;
	struct nvgpu_runlist *runlist;

	/* Disable raising interrupt for both runlist trees to CPU and GSP */
	for (i = 0U; i < g->fifo.num_runlists; i++) {
		runlist = &g->fifo.active_runlists[i];
		for (intr_tree = 0U; intr_tree < runlist_intr_ctrl__size_1_v();
				intr_tree++) {
			reg_val = nvgpu_gin_get_intr_ctrl_msg(g,
					gb10b_fifo_runlist_get_vectorid(g, runlist, intr_tree),
					NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
			nvgpu_runlist_writel(g, runlist,
					g->ops.fifo.runlist_intr_ctrl(intr_tree), reg_val);
		}
		/* Clear interrupts */
		reg_val = nvgpu_runlist_readl(g, runlist, g->ops.fifo.runlist_intr_0());
		nvgpu_runlist_writel(g, runlist, g->ops.fifo.runlist_intr_0(), reg_val);
	}
}

static void gb10b_fifo_runlist_intr_enable(struct gk20a *g)
{
	u32 i, intr_tree_0, intr_tree_1, reg_val;
	u32 intr0_en_mask;
	struct nvgpu_runlist *runlist;

	intr_tree_0 = 0U;
	intr_tree_1 = 1U;
	intr0_en_mask = g->ops.fifo.get_runlist_intr_0_mask();

	for (i = 0U; i < g->fifo.num_runlists; i++) {
		runlist = &g->fifo.active_runlists[i];
		/*
		 * runlist_intr_0 interrupts can be routed to either
		 * tree0 or tree1 vector using runlist_intr_0_en_set_tree(0) or
		 * runlist_intr_0_en_set_tree(1) register. For now route all
		 * interrupts to tree0.
		 */

		/* Clear interrupts */
		reg_val = nvgpu_runlist_readl(g, runlist, g->ops.fifo.runlist_intr_0());
		nvgpu_runlist_writel(g, runlist, g->ops.fifo.runlist_intr_0(), reg_val);

		/* Enable interrupts in tree(0) */
		nvgpu_runlist_writel(g, runlist,
			g->ops.fifo.runlist_intr_0_en_set_tree(intr_tree_0),
			intr0_en_mask);

		/* Disable all interrupts in tree(1) */
		nvgpu_runlist_writel(g, runlist,
			g->ops.fifo.runlist_intr_0_en_clear_tree(intr_tree_1),
			U32_MAX);

		/* Enable raising interrupt to cpu, disable raising interrupt to gsp */
		reg_val = nvgpu_gin_get_intr_ctrl_msg(g,
				gb10b_fifo_runlist_get_vectorid(g, runlist, intr_tree_0),
				NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);

		/* Enable runlist tree 0 interrupts at runlist level */
		nvgpu_runlist_writel(g, runlist,
				g->ops.fifo.runlist_intr_ctrl(intr_tree_0), reg_val);

		/* Disable raising interrupt to cpu, disable raising interrupt to gsp */
		reg_val = nvgpu_gin_get_intr_ctrl_msg(g,
				gb10b_fifo_runlist_get_vectorid(g, runlist, intr_tree_1),
				NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);

		/* Disable runlist tree 1 interrupts at runlist level */
		nvgpu_runlist_writel(g, runlist,
				g->ops.fifo.runlist_intr_ctrl(intr_tree_1), reg_val);
	}
}

void gb10b_fifo_intr_0_enable(struct gk20a *g, bool enable)
{
	gb10b_fifo_runlist_intr_disable(g);

	if (!enable) {
		g->ops.fifo.ctxsw_timeout_enable(g, false);
		g->ops.pbdma.intr_enable(g, false);
		return;
	}

	/* Enable interrupts */
	g->ops.fifo.ctxsw_timeout_enable(g, true);
	g->ops.pbdma.intr_enable(g, true);

	gb10b_fifo_runlist_intr_enable(g);
}
