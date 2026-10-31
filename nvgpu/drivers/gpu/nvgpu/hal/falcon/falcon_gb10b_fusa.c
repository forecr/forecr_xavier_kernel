// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include <nvgpu/hw/gb10b/hw_falcon_gb10b.h>
#include <nvgpu/hw/gb10b/hw_priscv_gb10b.h>

#include "falcon_gb10b.h"
#include "hal/riscv/riscv_gb10b.h"

#ifdef CONFIG_NVGPU_FALCON_DEBUG
void gb10b_falcon_dump_info(struct nvgpu_falcon *flcn)
{
	struct gk20a *g = NULL;

	g = flcn->g;

	/*
	 * Common Falcon code accesses each engine's falcon registers
	 * using engine's falcon base address + offset.
	 * So generate offset for falcon_falcon_exterrstat_r()
	 * and falcon_falcon_exterraddr_r() registers by applying
	 * the mask 0xFFF
	 */
	nvgpu_err(g, "falcon_falcon_exterrstat_r : 0x%x",
		nvgpu_falcon_readl(flcn,
			(falcon_falcon_exterrstat_r() & 0x0FFF)));
	nvgpu_err(g, "falcon_falcon_exterraddr_r : 0x%x",
		nvgpu_falcon_readl(flcn,
			(falcon_falcon_exterraddr_r() & 0x0FFF)));
}
#endif

void gb10b_falcon_dump_debug_regs(struct nvgpu_falcon *flcn,
				struct nvgpu_debug_context *o)
{
	gb10b_riscv_dump_debug_regs(flcn, o);

	gk20a_debug_output(o, "NV_PFALCON_FALCON_PRGNVER : %d",
		nvgpu_falcon_readl(flcn, falcon_falcon_prgnver_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_OS : %d",
		nvgpu_falcon_readl(flcn, falcon_falcon_os_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_IDLESTATE : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_idlestate_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_MAILBOX0 : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_mailbox0_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_MAILBOX1 : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_mailbox1_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_IRQSTAT : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_irqstat_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_IRQMODE : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_irqmode_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_IRQMASK : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_irqmask_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_IRQDEST : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_irqdest_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_DEBUGINFO : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_debuginfo_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_HWCFG : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_hwcfg_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_HWCFG2 : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_hwcfg2_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_ENGCTL : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_engctl_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_SCTL : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_sctl_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_CURCTX : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_curctx_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_NXTCTX : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_nxtctx_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_CTXACK : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_ctxack_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_CTXSW_ERROR : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_ctxsw_error_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_FHSTATE : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_fhstate_r()));
	gk20a_debug_output(o, "NV_PFALCON_FALCON_DMATRFCMD : 0x%x",
		nvgpu_falcon_readl(flcn, falcon_falcon_dmatrfcmd_r()));

	/* RISCV Trace */
	gb10b_riscv_dump_trace_info(flcn, o);
}

u32 gb10b_falcon_debuginfo_offset(void)
{
	return falcon_falcon_debuginfo_r();
}

void gb10b_falcon_write_dmatrfbase(struct nvgpu_falcon *flcn, u64 dma_base)
{
	struct gk20a *g = flcn->g;

	nvgpu_writel(g, nvgpu_safe_add_u32(flcn->flcn_base, falcon_falcon_dmatrfbase_r()),
			u64_lo32(dma_base >> 8));
	nvgpu_writel(g, nvgpu_safe_add_u32(flcn->flcn_base, falcon_falcon_dmatrfbase1_r()),
			0U);
}

u32 gb10b_falcon_get_mem_size(struct nvgpu_falcon *flcn,
		enum falcon_mem_type mem_type)
{
	u32 mem_size = 0U;
	u32 hwcfg_val = 0U;

	if (!flcn->is_falcon2_enabled) {
		hwcfg_val = nvgpu_falcon_readl(flcn, falcon_falcon_hwcfg_r());

		if (mem_type == MEM_DMEM) {
			mem_size = falcon_falcon_hwcfg_dmem_size_v(hwcfg_val)
				<< FALCON_DMEM_BLKSIZE2;
		} else {
			mem_size = falcon_falcon_hwcfg_imem_size_v(hwcfg_val)
				<< FALCON_DMEM_BLKSIZE2;
		}
	} else {
		hwcfg_val = nvgpu_falcon_readl(flcn, falcon_falcon_hwcfg3_r());

		if (mem_type == MEM_DMEM) {
			mem_size = falcon_falcon_hwcfg3_dmem_size_v(hwcfg_val)
				<< FALCON_DMEM_BLKSIZE2;
		} else {
			mem_size = falcon_falcon_hwcfg3_imem_size_v(hwcfg_val)
				<< FALCON_DMEM_BLKSIZE2;
		}
	}

	return mem_size;
}

bool gb10b_falcon_check_brom_init_done(u32 retcode)
{
	return (priscv_priscv_br_retcode_result_v(retcode) ==
			priscv_priscv_br_retcode_result_init_f());
}

void gb10b_get_bootplugin_fw_name(struct gk20a *g, const char **name, u32 falcon_id)
{
	(void) g;
	if (falcon_id == FALCON_ID_FECS_RISCV_EB || falcon_id == FALCON_ID_FECS)
		*name = GB10B_FECS_RISCV_BOOTPLUGIN;
	else if (falcon_id == FALCON_ID_GPCCS_RISCV_EB || falcon_id == FALCON_ID_GPCCS)
		*name = GB10B_GPCCS_RISCV_BOOTPLUGIN;
}
