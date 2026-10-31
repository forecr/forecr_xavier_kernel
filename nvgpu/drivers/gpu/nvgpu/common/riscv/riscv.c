// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/falcon.h>
#include <nvgpu/riscv.h>
#include <nvgpu/timers.h>
#include <nvgpu/firmware.h>
#include <nvgpu/dma.h>
#include <nvgpu/string.h>

#define NVRISCV_AMAP_IMEM_START  0x00100000ULL
#define RIGHT_SHIFT_8_BITS(v)    ((v) >> 8U)
#define DEBUGINFO_DUMMY          (0xDEADBEAFU)

static bool riscv_is_falcon_valid(struct nvgpu_falcon *flcn)
{
	if (flcn == NULL) {
		return false;
	}

	if (!flcn->is_falcon_supported) {
		nvgpu_err(flcn->g, "Core-id %u instance %u not supported", flcn->flcn_id, flcn->inst_id);
		return false;
	}

	return true;
}

u32 nvgpu_riscv_readl(struct nvgpu_falcon *flcn, u32 offset)
{
	return nvgpu_readl(flcn->g,
			   nvgpu_safe_add_u32(flcn->flcn2_base, offset));
}

void nvgpu_riscv_writel(struct nvgpu_falcon *flcn,
				       u32 offset, u32 val)
{
	nvgpu_writel(flcn->g, nvgpu_safe_add_u32(flcn->flcn2_base, offset), val);
}

int nvgpu_riscv_hs_ucode_load_bootstrap(struct nvgpu_falcon *flcn,
						struct nvgpu_firmware *manifest_fw,
						struct nvgpu_firmware *code_fw,
						struct nvgpu_firmware *data_fw,
						u64 ucode_sysmem_desc_addr)
{
	struct gk20a *g;
	u32 dmem_size = 0U;
	int err = 0;

	if (!riscv_is_falcon_valid(flcn)) {
		return -EINVAL;
	}

	g = flcn->g;

	/* core reset */
	err = nvgpu_falcon_reset(flcn);
	if (err != 0) {
		nvgpu_err(g, "core reset failed err=%d", err);
		return err;
	}

	/* Copy dmem desc address to mailbox */
	nvgpu_falcon_mailbox_write(flcn, FALCON_MAILBOX_0,
					u64_lo32(ucode_sysmem_desc_addr));
	nvgpu_falcon_mailbox_write(flcn, FALCON_MAILBOX_1,
					u64_hi32(ucode_sysmem_desc_addr));

	g->ops.falcon.set_bcr(flcn);
	err = nvgpu_falcon_get_mem_size(flcn, MEM_DMEM, &dmem_size);
	err = nvgpu_falcon_copy_to_imem(flcn, 0x0, code_fw->data,
					nvgpu_safe_cast_u64_to_u32(code_fw->size),
					0, true, 0x0);

	if (err != 0) {
		nvgpu_err(g, "RISCV code copy to IMEM failed");
		goto exit;
	}

	err = nvgpu_falcon_copy_to_dmem(flcn, 0x0, data_fw->data,
					nvgpu_safe_cast_u64_to_u32(data_fw->size),
					0x0);
	if (err != 0) {
		nvgpu_err(g, "RISCV data copy to DMEM failed");
		goto exit;
	}

	err = nvgpu_falcon_copy_to_dmem(flcn, nvgpu_safe_sub_u32(dmem_size, nvgpu_safe_cast_u64_to_u32(manifest_fw->size)),
				manifest_fw->data,
				nvgpu_safe_cast_u64_to_u32(manifest_fw->size),
				0x0);
	if (err != 0) {
		nvgpu_err(g, "RISCV manifest copy to DMEM failed");
		goto exit;
	}

	g->ops.falcon.bootstrap(flcn, 0x0);
exit:
	return err;
}

void nvgpu_riscv_dump_brom_stats(struct nvgpu_falcon *flcn)
{
	if (!riscv_is_falcon_valid(flcn)) {
		return;
	}

	flcn->g->ops.falcon.dump_brom_stats(flcn);
}

static void nvgpu_nvriscv_program_bcr(struct gk20a *g, struct nvgpu_falcon *flcn,
				struct nvgpu_mem *mem_desc, u32 *fw_desc)
{
	u64 base_addr = 0;
	u64 fmc_code_addr = 0;
	u64 fmc_data_addr = 0;
	u64 manifest_addr = 0;

	base_addr = nvgpu_mem_get_addr(g, mem_desc);

	fmc_code_addr = nvgpu_safe_add_u64(base_addr, fw_desc[NVGPU_NVRISCV_CODE_OFFSET]);
	fmc_code_addr = RIGHT_SHIFT_8_BITS(fmc_code_addr);

	fmc_data_addr = nvgpu_safe_add_u64(base_addr, fw_desc[NVGPU_NVRISCV_DATA_OFFSET]);
	fmc_data_addr = RIGHT_SHIFT_8_BITS(fmc_data_addr);

	manifest_addr = nvgpu_safe_add_u64(base_addr, fw_desc[NVGPU_NVRISCV_MANIFEST_OFFSET]);
	manifest_addr = RIGHT_SHIFT_8_BITS(manifest_addr);

	nvgpu_log(g, gpu_dbg_falcon | gpu_dbg_mme,
		"program bcr: base 0x%llx, fmc_code 0x%llx, fmc_data 0x%llx, manifest 0x%llx",
		base_addr, fmc_code_addr, fmc_data_addr, manifest_addr);

	g->ops.falcon.brom_config(flcn, fmc_code_addr, fmc_data_addr, manifest_addr);
}

/* NVRISC-V NS boot */
s32 nvgpu_nvriscv_ns_boot(struct nvgpu_falcon *flcn, struct nvgpu_mem *fw_mem,
			u32 *fw_desc)
{
	struct gk20a *g;
	s32 err = 0;
	u64 boot_vector;
	/* header info needed for dma'ing the firmware */
	u32 header_info[OS_DATA_SIZE + 1];

	if (!riscv_is_falcon_valid(flcn)) {
		return -EINVAL;
	}

	g = flcn->g;

	/* Core reset */
	err = nvgpu_falcon_reset(flcn);
	if (err != 0) {
		nvgpu_err(g, "engine/nvriscv %u instance %u reset failed, err=%d",
			flcn->flcn_id, flcn->inst_id, err);
		goto done;
	}

	/* Enable CG settings */
	if ((g->slcg_enabled) && (flcn->flcn_engine_dep_ops.slcg_load_enable != NULL)) {
		flcn->flcn_engine_dep_ops.slcg_load_enable(flcn->g, flcn->inst_id, true);
	}

	if ((g->blcg_enabled) && (flcn->flcn_engine_dep_ops.blcg_load_enable != NULL)) {
		flcn->flcn_engine_dep_ops.blcg_load_enable(flcn->g, flcn->inst_id, true);
	}

	/* Setup boot-config */
	if (flcn->flcn_engine_dep_ops.riscv_bootstrap_config != NULL) {
		flcn->flcn_engine_dep_ops.riscv_bootstrap_config(flcn->g, flcn->inst_id);
	}

	/* Write debuginfo register with dummy value */
	err = nvgpu_falcon_debuginfo_write(flcn, DEBUGINFO_DUMMY);
	if (err != 0) {
		goto done;
	}

	/* Copy header info (offset & size of code & data segments) needed for dma'ing */
	nvgpu_memcpy((u8 *)header_info, (u8 *)fw_desc, sizeof(header_info));

	if (!flcn->is_brom_boot) {
		/* Load engine ucode */
		err = nvgpu_falcon_load_ucode(flcn, fw_mem, header_info);
		if (err != 0) {
			nvgpu_err(g, "nvriscv %u instance %u ucode loading failed, err=%d",
				flcn->flcn_id, flcn->inst_id, err);
			goto done;
		}
	}

	/* Enable interrupts */
	if (flcn->flcn_engine_dep_ops.enable_irq != NULL) {
		flcn->flcn_engine_dep_ops.enable_irq(g, flcn->inst_id, true);
	}

	/* Program bcr registers */
	nvgpu_nvriscv_program_bcr(g, flcn, fw_mem, fw_desc);

	/* Kick-start the core */
	boot_vector = flcn->is_brom_boot ? 0U : NVRISCV_AMAP_IMEM_START;
	g->ops.falcon.bootstrap(flcn, boot_vector);

	if (!flcn->is_brom_boot) {
		/* Wait for engine idle */
		err = nvgpu_falcon_wait_idle(flcn);
	} else {
		/* Wait for BROM completion */
		err = nvgpu_falcon_wait_for_nvriscv_brom_completion(flcn);
	}

	if (err != 0) {
		goto done;
	}

	/*
	 * Check if it has reached a proper initialized state,
	 * indicated by clearing the debuginfo register.
	 */
	err = nvgpu_falcon_wait_init_completion(flcn);
	if (err != 0) {
		nvgpu_err(g, "nvriscv %u instance %u ucode has not reached proper init state, err=%d",
				flcn->flcn_id, flcn->inst_id, err);
	}

done:
	return err;
}
