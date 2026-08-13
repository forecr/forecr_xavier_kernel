/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMUIF_BIOS_H
#define NVGPU_PMUIF_BIOS_H

struct nv_pmu_bios_vfield_register_segment_super {
	u8 type;
	u8 low_bit;
	u8 high_bit;
};

struct nv_pmu_bios_vfield_register_segment_reg {
	struct nv_pmu_bios_vfield_register_segment_super super;
	u32 addr;
};

struct nv_pmu_bios_vfield_register_segment_index_reg {
	struct nv_pmu_bios_vfield_register_segment_super super;
	u32 addr;
	u32 reg_index;
	u32 index;
};

union nv_pmu_bios_vfield_register_segment {
	struct nv_pmu_bios_vfield_register_segment_super super;
	struct nv_pmu_bios_vfield_register_segment_reg reg;
	struct nv_pmu_bios_vfield_register_segment_index_reg index_reg;
};


#endif /* NVGPU_PMUIF_BIOS_H */
