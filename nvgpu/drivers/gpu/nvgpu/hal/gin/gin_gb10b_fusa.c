// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/gin.h>
#include <nvgpu/io.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/device.h>

#include "gin_gb10b.h"

#include <nvgpu/hw/gb10b/hw_func_gb10b.h>
#include <nvgpu/hw/gb10b/hw_gin_gb10b.h>

static const struct nvgpu_gin_config gb10b_gin_config = {
	.stall_intr_top_mask = 0xfe,
	.nonstall_intr_top_mask = 0x1,
	.vector_configs = {
		[NVGPU_GIN_INTR_UNIT_LRC] = {
			.num_stall_vectors = 1,
			.stall_vectors = {
				[NVGPU_LRC_UE_VECTOR_OFFSET] = NVGPU_GIN_VECTOR(4, 3),
			}
		},
		[NVGPU_GIN_INTR_UNIT_XAL_EP] = {
			.num_stall_vectors = 2,
			.stall_vectors = {
				[NVGPU_XAL_UE_VECTOR_OFFSET] = NVGPU_GIN_VECTOR(4, 0),
				[NVGPU_XAL_CE_VECTOR_OFFSET] = NVGPU_GIN_VECTOR(5, 0),
			},
		},
		[NVGPU_GIN_INTR_UNIT_XTL_EP] = {
			.num_stall_vectors = 1,
			.stall_vectors = {
				[NVGPU_XTL_UE_VECTOR_OFFSET] = NVGPU_GIN_VECTOR(4, 1),
			},
		},
		[NVGPU_GIN_INTR_UNIT_PRIV_RING] = {
			.num_stall_vectors = 1,
			.stall_vectors = {
				[0] = NVGPU_GIN_VECTOR(2, 11),
			}
		},
		[NVGPU_GIN_INTR_UNIT_SYSCTRL] = {
			.num_stall_vectors = 1,
			.stall_vectors = {
				[NVGPU_SYSCTRL_UE_VECTOR_OFFSET] = NVGPU_GIN_VECTOR(4, 4),
			},
		},
		[NVGPU_GIN_INTR_UNIT_MMU] = {
			.num_stall_vectors = 6,
			.stall_vectors = {
				[NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_ECC_ERROR]                  = NVGPU_GIN_VECTOR(2, 3),
				[NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_INFO_FAULT]                 = NVGPU_GIN_VECTOR(2, 4),
				[NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT]           = NVGPU_GIN_VECTOR(2, 5),
				[NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT]       = NVGPU_GIN_VECTOR(2, 6),
				[NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT_ERROR]     = NVGPU_GIN_VECTOR(2, 7),
				[NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT_ERROR] = NVGPU_GIN_VECTOR(2, 8),
			},
		},
		[NVGPU_GIN_INTR_UNIT_FUSE] = {
			.num_stall_vectors = 1,
			.stall_vectors = {
				[NVGPU_FUSE_UE_VECTOR_OFFSET]  = NVGPU_GIN_VECTOR(4, 17),
			},
		},
		[NVGPU_GIN_INTR_UNIT_GR] = {
			.num_stall_vectors = 2,
			.stall_vectors = {
				[0] = NVGPU_GIN_VECTOR(3, 0),
				[1] = NVGPU_GIN_VECTOR(3, 1),
			},
			.num_nonstall_vectors = 2,
			.nonstall_vectors = {
				[0] = NVGPU_GIN_VECTOR(0, 0),
				[1] = NVGPU_GIN_VECTOR(0, 1),
			},
		},
		[NVGPU_GIN_INTR_UNIT_CE] = {
			.num_stall_vectors = 4,
			.stall_vectors = {
				[0] = NVGPU_GIN_VECTOR(3, 2),
				[1] = NVGPU_GIN_VECTOR(3, 3),
				[2] = NVGPU_GIN_VECTOR(3, 4),
				[3] = NVGPU_GIN_VECTOR(3, 5),
			},
			.num_nonstall_vectors = 4,
			.nonstall_vectors = {
				[0] = NVGPU_GIN_VECTOR(0, 0),
				[1] = NVGPU_GIN_VECTOR(0, 0),
				[2] = NVGPU_GIN_VECTOR(0, 4),
				[3] = NVGPU_GIN_VECTOR(0, 5),
			},
		},
		[NVGPU_GIN_INTR_UNIT_SEC] = {
			.num_stall_vectors = 1,
			.stall_vectors = {
				[0] = NVGPU_GIN_VECTOR(3, 6),
			},
			.num_nonstall_vectors = 1,
			.nonstall_vectors = {
				[0] = NVGPU_GIN_VECTOR(0, 6),
			}
		},
		[NVGPU_GIN_INTR_UNIT_NVENC] = {
			.num_stall_vectors = 6,
			.stall_vectors = {
				[NVGPU_NVENC_LEGACY_VECTOR_OFFSET_0] = NVGPU_GIN_VECTOR(3, 7),
				[NVGPU_NVENC_LEGACY_VECTOR_OFFSET_1] = NVGPU_GIN_VECTOR(3, 8),
				[NVGPU_NVENC_UE_VECTOR_OFFSET_0] = NVGPU_GIN_VECTOR(4, 14),
				[NVGPU_NVENC_UE_VECTOR_OFFSET_1] = NVGPU_GIN_VECTOR(4, 15),
				[NVGPU_NVENC_CE_VECTOR_OFFSET_0] = NVGPU_GIN_VECTOR(5, 14),
				[NVGPU_NVENC_CE_VECTOR_OFFSET_1] = NVGPU_GIN_VECTOR(5, 15),
			},
			.num_nonstall_vectors = 2,
			.nonstall_vectors = {
				[0] = NVGPU_GIN_VECTOR(0, 7),
				[1] = NVGPU_GIN_VECTOR(0, 8),
			},
		},
		[NVGPU_GIN_INTR_UNIT_NVDEC] = {
			.num_stall_vectors = 2,
			.stall_vectors = {
				[0] = NVGPU_GIN_VECTOR(3, 9),
				[1] = NVGPU_GIN_VECTOR(3, 10),
			},
			.num_nonstall_vectors = 2,
			.nonstall_vectors = {
				[0] = NVGPU_GIN_VECTOR(0, 9),
				[1] = NVGPU_GIN_VECTOR(0, 10),
			},
		},
		[NVGPU_GIN_INTR_UNIT_NVJPG] = {
			.num_stall_vectors = 2,
			.stall_vectors = {
				[0] = NVGPU_GIN_VECTOR(3, 11),
				[1] = NVGPU_GIN_VECTOR(3, 12),
			},
			.num_nonstall_vectors = 2,
			.nonstall_vectors = {
				[0] = NVGPU_GIN_VECTOR(0, 11),
				[1] = NVGPU_GIN_VECTOR(0, 12),
			},
		},
		[NVGPU_GIN_INTR_UNIT_OFA] = {
			.num_stall_vectors = 3,
			.stall_vectors = {
				[0] = NVGPU_GIN_VECTOR(3, 13),
				[1] = NVGPU_GIN_VECTOR(4, 13),
				[2] = NVGPU_GIN_VECTOR(5, 13),
			},
			.num_nonstall_vectors = 1,
			.nonstall_vectors = {
				[0] = NVGPU_GIN_VECTOR(0, 13),
			},
		},
		[NVGPU_GIN_INTR_UNIT_RUNLIST_TREE_0] = {
			.num_stall_vectors = 12,
			.stall_vectors = {
				[0]  = NVGPU_GIN_VECTOR(2, 32),
				[1]  = NVGPU_GIN_VECTOR(2, 33),
				[2]  = NVGPU_GIN_VECTOR(2, 34),
				[3]  = NVGPU_GIN_VECTOR(2, 35),
				[4]  = NVGPU_GIN_VECTOR(2, 36),
				[5]  = NVGPU_GIN_VECTOR(2, 37),
				[6]  = NVGPU_GIN_VECTOR(2, 38),
				[7]  = NVGPU_GIN_VECTOR(2, 39),
				[8]  = NVGPU_GIN_VECTOR(2, 40),
				[9]  = NVGPU_GIN_VECTOR(2, 41),
				[10] = NVGPU_GIN_VECTOR(2, 42),
				[11] = NVGPU_GIN_VECTOR(2, 43),
			},
		},
		[NVGPU_GIN_INTR_UNIT_RUNLIST_TREE_1] = {
			.num_stall_vectors = 12,
			.stall_vectors = {
				[0]  = NVGPU_GIN_VECTOR(3, 32),
				[1]  = NVGPU_GIN_VECTOR(3, 33),
				[2]  = NVGPU_GIN_VECTOR(3, 34),
				[3]  = NVGPU_GIN_VECTOR(3, 35),
				[4]  = NVGPU_GIN_VECTOR(3, 36),
				[5]  = NVGPU_GIN_VECTOR(3, 37),
				[6]  = NVGPU_GIN_VECTOR(3, 38),
				[7]  = NVGPU_GIN_VECTOR(3, 39),
				[8]  = NVGPU_GIN_VECTOR(3, 40),
				[9]  = NVGPU_GIN_VECTOR(3, 41),
				[10] = NVGPU_GIN_VECTOR(3, 42),
				[11] = NVGPU_GIN_VECTOR(3, 43),
			},
		},
		[NVGPU_GIN_INTR_UNIT_CTC_CHI] = {
			.num_stall_vectors = 1,
			.stall_vectors = {
				[NVGPU_CTC_CHI_UE_VECTOR_OFFSET] = NVGPU_GIN_VECTOR(4, 2),
			},
		},
		[NVGPU_GIN_INTR_UNIT_LTC] = {
			.num_stall_vectors = 3,
			.stall_vectors = {
				[NVGPU_LTC_LEGACY_VECTOR_OFFSET]  = NVGPU_GIN_VECTOR(2, 9),
				[NVGPU_LTC_UE_VECTOR_OFFSET]  = NVGPU_GIN_VECTOR(4, 9),
				[NVGPU_LTC_CE_VECTOR_OFFSET]  = NVGPU_GIN_VECTOR(5, 9),
			},
		},
		[NVGPU_GIN_INTR_UNIT_PMU] = {
			.num_stall_vectors = 1,
			.stall_vectors = {
				[0]  = NVGPU_GIN_VECTOR(2, 10),
			},
		},
		[NVGPU_GIN_INTR_UNIT_GSP] = {
			.num_stall_vectors = 3,
			.stall_vectors = {
				[NVGPU_GSP_LEGACY_VECTOR_OFFSET]  = NVGPU_GIN_VECTOR(2, 16),
				[NVGPU_GSP_UE_VECTOR_OFFSET]  = NVGPU_GIN_VECTOR(4, 16),
				[NVGPU_GSP_CE_VECTOR_OFFSET]  = NVGPU_GIN_VECTOR(5, 16),
			},
		},

	},
};

u64 gb10b_gin_read_intr_top(struct gk20a *g, u64 intr_top_mask)
{
	u32 intr_top_status_lo = 0U;
	u32 intr_top_mask_lo = u64_lo32(intr_top_mask);

	intr_top_status_lo = nvgpu_func_readl(g, func_priv_cpu_intr_top_r(0U));
	intr_top_status_lo &= intr_top_mask_lo;

	return hi32_lo32_to_u64(0U, intr_top_status_lo);
}

void gb10b_gin_write_intr_top_en_clear(struct gk20a *g, u64 val)
{
	u32 val_lo = u64_lo32(val);

	nvgpu_func_writel(g, func_priv_cpu_intr_top_en_clear_r(0U), val_lo);
}

void gb10b_gin_write_intr_top_en_set(struct gk20a *g, u64 val)
{
	u32 val_lo = u64_lo32(val);

	nvgpu_func_writel(g, func_priv_cpu_intr_top_en_set_r(0U), val_lo);
}

u64 gb10b_gin_subtree_leaf_status(struct gk20a *g, u32 subtree)
{
	u32 leaf0 = 0U;
	u32 leaf1 = 0U;

	leaf0 = nvgpu_func_readl(g,
			func_priv_cpu_intr_leaf_r(
				NVGPU_GIN_SUBTREE_TO_LEAF0(subtree)));

	leaf1 = nvgpu_func_readl(g,
			func_priv_cpu_intr_leaf_r(
				NVGPU_GIN_SUBTREE_TO_LEAF1(subtree)));

	nvgpu_log(g, gpu_dbg_intr,
		"subtree_%u: intr_leaf0: 0x%08x intr_leaf1: 0x%08x",
			subtree, leaf0, leaf1);

	return hi32_lo32_to_u64(leaf1, leaf0);
}

void gb10b_gin_subtree_leaf_clear(struct gk20a *g, u32 subtree, u64 leaf)
{
	u32 leaf0 = u64_lo32(leaf);
	u32 leaf1 = u64_hi32(leaf);

	nvgpu_func_writel(g,
			func_priv_cpu_intr_leaf_r(NVGPU_GIN_SUBTREE_TO_LEAF0(subtree)),
			leaf0);

	nvgpu_func_writel(g,
			func_priv_cpu_intr_leaf_r(NVGPU_GIN_SUBTREE_TO_LEAF1(subtree)),
			leaf1);

	nvgpu_log(g, gpu_dbg_intr,
		"clear subtree_%u: intr_leaf0: 0x%08x intr_leaf1: 0x%08x",
			subtree, leaf0, leaf1);
}

void gb10b_gin_subtree_leaf_enable(struct gk20a *g, u32 subtree, u64 leaf)
{
	u32 leaf0 = u64_lo32(leaf);
	u32 leaf1 = u64_hi32(leaf);

	/* TODO top_en manipulation needs to be decoupled from leaf_en enablement */
	nvgpu_func_writel(g,
			func_priv_cpu_intr_top_en_set_r(
				NVGPU_GIN_SUBTREE_TO_TOP_IDX(subtree)),
				BIT32(NVGPU_GIN_SUBTREE_TO_TOP_BIT(subtree)));

	nvgpu_func_writel(g,
			func_priv_cpu_intr_leaf_en_set_r(NVGPU_GIN_SUBTREE_TO_LEAF0(subtree)),
			leaf0);

	nvgpu_func_writel(g,
			func_priv_cpu_intr_leaf_en_set_r(NVGPU_GIN_SUBTREE_TO_LEAF1(subtree)),
			leaf1);

	nvgpu_log(g, gpu_dbg_intr,
		"enable subtree_%u: intr_leaf0: 0x%08x intr_leaf1: 0x%08x",
			subtree, leaf0, leaf1);
}

void gb10b_gin_subtree_leaf_disable(struct gk20a *g, u32 subtree, u64 leaf)
{
	u32 leaf0 = u64_lo32(leaf);
	u32 leaf1 = u64_hi32(leaf);

	nvgpu_func_writel(g,
			func_priv_cpu_intr_leaf_en_clear_r(NVGPU_GIN_SUBTREE_TO_LEAF0(subtree)),
			leaf0);

	nvgpu_func_writel(g,
			func_priv_cpu_intr_leaf_en_clear_r(NVGPU_GIN_SUBTREE_TO_LEAF1(subtree)),
			leaf1);

	nvgpu_log(g, gpu_dbg_intr,
		"disable subtree_%u: intr_leaf0: 0x%08x intr_leaf1: 0x%08x",
			subtree, leaf0, leaf1);
}

u32 gb10b_gin_get_intr_ctrl_msg(struct gk20a *g, u32 vector,
		bool cpu_enable, bool gsp_enable)
{
	(void)g;
	return gin_intr_ctrl_access_defines_vector_f(vector) |
		(cpu_enable ?
		 gin_intr_ctrl_access_defines_cpu_enable_f() :
		 gin_intr_ctrl_access_defines_cpu_disable_f()) |
		(gsp_enable ?
		 gin_intr_ctrl_access_defines_gsp_enable_f() :
		 gin_intr_ctrl_access_defines_gsp_disable_f());
}

u32 gb10b_gin_get_vector_from_intr_ctrl_msg(struct gk20a *g, u32 intr_ctrl_msg)
{
	(void)g;
	return gin_intr_ctrl_access_defines_vector_v(intr_ctrl_msg);
}

u32 gb10b_gin_get_num_vectors(struct gk20a *g)
{
	return func_priv_intr_count_size_v(
			nvgpu_func_readl(g, func_priv_intr_count_r()));
}

static bool gb10b_gin_is_vector_pending(struct gk20a *g, u32 vector)
{
	u32 subtree = NVGPU_GIN_VECTOR_TO_SUBTREE(vector);
	u32 leaf_bit = NVGPU_GIN_VECTOR_TO_LEAF_BIT(vector);
	u64 leaf = g->ops.gin.subtree_leaf_status(g, subtree);

	return leaf & BIT(leaf_bit);
}

bool gb10b_gin_is_mmu_fault_pending(struct gk20a *g)
{
	u32 i = 0U;
	const struct nvgpu_gin_vector_config *mmu_vector_config =
		&(g->gin.config->vector_configs[NVGPU_GIN_INTR_UNIT_MMU]);

	for (i = 0U; i < mmu_vector_config->num_stall_vectors; i++) {
		u32 vector = mmu_vector_config->stall_vectors[i];
		if (gb10b_gin_is_vector_pending(g, vector)) {
			return true;
		}
	}

	return false;
}

static bool gb10b_gin_is_eng_stall_pending(struct gk20a *g, u32 engine_id)
{
	const struct nvgpu_device *dev =
		nvgpu_engine_get_active_eng_info(g, engine_id);
	const struct nvgpu_gin_vector_config *eng_vector_config = NULL;
	u32 unit = 0U;
	u32 vector = 0U;

	if (dev == NULL) {
		nvgpu_err(g, "Engine info not found");
		return false;
	}

	switch (dev->type) {
	case NVGPU_DEVTYPE_GRAPHICS:
		unit = NVGPU_GIN_INTR_UNIT_GR;
		break;
	case NVGPU_DEVTYPE_LCE:
		unit = NVGPU_GIN_INTR_UNIT_CE;
		break;
	case NVGPU_DEVTYPE_SEC:
		unit = NVGPU_GIN_INTR_UNIT_SEC;
		break;
	case NVGPU_DEVTYPE_NVENC:
		unit = NVGPU_GIN_INTR_UNIT_NVENC;
		break;
	case NVGPU_DEVTYPE_NVDEC:
		unit = NVGPU_GIN_INTR_UNIT_NVDEC;
		break;
	case NVGPU_DEVTYPE_NVJPG:
		unit = NVGPU_GIN_INTR_UNIT_NVJPG;
		break;
	case NVGPU_DEVTYPE_OFA:
		unit = NVGPU_GIN_INTR_UNIT_OFA;
		break;
	default:
		nvgpu_err(g, "unhandled device type %u", dev->type);
		return false;
	}

	eng_vector_config = &(g->gin.config->vector_configs[unit]);
	nvgpu_assert(dev->inst_id < eng_vector_config->num_stall_vectors);
	vector = eng_vector_config->stall_vectors[dev->inst_id];
	return gb10b_gin_is_vector_pending(g, vector);
}

bool gb10b_gin_is_stall_and_eng_intr_pending(struct gk20a *g, u32 engine_id,
		u32 *eng_intr_pending)
{
	u64 stall_pending =
		g->ops.gin.read_intr_top(g, g->gin.config->stall_intr_top_mask);

	*eng_intr_pending = gb10b_gin_is_eng_stall_pending(g, engine_id);

	return (stall_pending != 0UL);
}

void gb10b_gin_init(struct gk20a *g)
{
	g->gin.config = &gb10b_gin_config;
}
