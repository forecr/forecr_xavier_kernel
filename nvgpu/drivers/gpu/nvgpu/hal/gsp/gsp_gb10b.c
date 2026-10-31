// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <hal/gsp/gsp_ga10b.h>
#include <nvgpu/gsp.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/nvgpu_err.h>

#include <nvgpu/hw/gb10b/hw_pgsp_gb10b.h>

#include "gsp_gb10b.h"

void gb10b_gsp_legacy_handler(struct gk20a *g, u64 cookie)
{
	u32 intr_stat = 0U;

	if (cookie != 0) {
		nvgpu_err(g, "GSP interrupt instance is incorrect");
	}

	if (!g->ops.falcon.is_priv_lockdown(&g->gsp_flcn)) {
		if (g->ops.gsp.gsp_is_interrupted(g, &intr_stat)) {
			if (g->ops.gsp.gsp_isr != NULL) {
				g->ops.gsp.gsp_isr(g);
			} else {
				nvgpu_err(g, "GSP isr is not set");
			}
		}
	}
}

static void gb10b_gsp_ue_handler(struct gk20a *g, u64 cookie)
{
	if (cookie != 0) {
		nvgpu_err(g, "GSP interrupt instance is incorrect");
	}

	/* Report the error to safety services */
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GSP_SCHED,
					GPU_GSP_SCHED_UNCORRECTED_ERR);

	nvgpu_err(g, "GSP Uncorrected error");
	nvgpu_err(g, "GSP errslice0_missionerr_status: 0x%x",
			nvgpu_readl(g, pgsp_ec_errslice0_missionerr_status_r()));

	/* Trigger GPU quiesce for fatal errors */
	nvgpu_sw_quiesce(g);

	/* No need to clear the error status registers as GPU quiesce is
	 * triggered and GPU reset is required to resume.
	 */
}

static void gb10b_gsp_ce_handler(struct gk20a *g, u64 cookie)
{
	if (cookie != 0) {
		nvgpu_err(g, "GSP interrupt instance is incorrect");
	}

	/* Report the error to safety services */
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GSP_SCHED,
					GPU_GSP_SCHED_CORRECTED_ERR);

	nvgpu_err(g, "GSP Corrected error");
	nvgpu_err(g, "GSP errslice0_missionerr_status: 0x%x",
			nvgpu_readl(g, pgsp_ec_errslice0_missionerr_status_r()));
	nvgpu_writel(g, pgsp_ec_errslice0_missionerr_status_r(), 0xFFFFFFFF);
}

void gb10b_gsp_enable_irq(struct gk20a *g, bool enable)
{
	u32 legacy_vector = 0U;
	u32 ue_vector = 0U;
	u32 ce_vector = 0U;
	u32 intr_ctrl_msg = 0U;
	u32 intr_ctrl2_msg = 0U;
	u32 intr_ctrl3_msg = 0U;
	u32 old_vector = 0U;
	u32 old_intr_ctrl_msg = 0U;

	nvgpu_log_fn(g, " ");

	if (enable) {
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_GSP,
					NVGPU_GSP_LEGACY_VECTOR_OFFSET,
					NVGPU_GIN_INTR_ENABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_GSP,
					NVGPU_GSP_UE_VECTOR_OFFSET,
					NVGPU_GIN_INTR_ENABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_GSP,
					NVGPU_GSP_CE_VECTOR_OFFSET,
					NVGPU_GIN_INTR_ENABLE);
	} else {
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_GSP,
					NVGPU_GSP_LEGACY_VECTOR_OFFSET,
					NVGPU_GIN_INTR_DISABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_GSP,
					NVGPU_GSP_UE_VECTOR_OFFSET,
					NVGPU_GIN_INTR_DISABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_GSP,
					NVGPU_GSP_CE_VECTOR_OFFSET,
					NVGPU_GIN_INTR_DISABLE);
	}
	legacy_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_GSP, NVGPU_GSP_LEGACY_VECTOR_OFFSET);
	ue_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_GSP, NVGPU_GSP_UE_VECTOR_OFFSET);
	ce_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_GSP, NVGPU_GSP_CE_VECTOR_OFFSET);

	nvgpu_gin_set_stall_handler(g, legacy_vector, &gb10b_gsp_legacy_handler, 0);
	nvgpu_gin_set_stall_handler(g, ue_vector, &gb10b_gsp_ue_handler, 0);
	nvgpu_gin_set_stall_handler(g, ce_vector, &gb10b_gsp_ce_handler, 0);

	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, legacy_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	intr_ctrl2_msg = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	intr_ctrl3_msg = nvgpu_gin_get_intr_ctrl_msg(g, ce_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);

	old_intr_ctrl_msg = nvgpu_readl(g, pgsp_falcon_intr_ctrl_r(0));
	old_vector = nvgpu_gin_get_vector_from_intr_ctrl_msg(g, old_intr_ctrl_msg);
	nvgpu_writel(g, pgsp_falcon_intr_ctrl_r(0), intr_ctrl_msg);
	nvgpu_writel(g, pgsp_falcon_intr_retrigger_r(0),
			pgsp_falcon_intr_retrigger_trigger_true_f());
	if (old_vector != legacy_vector) {
		nvgpu_gin_clear_pending_intr(g, old_vector);
	}

	old_intr_ctrl_msg = nvgpu_readl(g, pgsp_falcon_intr_ctrl2_r());
	old_vector = nvgpu_gin_get_vector_from_intr_ctrl_msg(g, old_intr_ctrl_msg);
	nvgpu_writel(g, pgsp_falcon_intr_ctrl2_r(), intr_ctrl2_msg);
	nvgpu_writel(g, pgsp_falcon_intr_retrigger2_r(),
			pgsp_falcon_intr_retrigger2_trigger_true_f());
	if (old_vector != ue_vector) {
		nvgpu_gin_clear_pending_intr(g, old_vector);
	}

	old_intr_ctrl_msg = nvgpu_readl(g, pgsp_falcon_intr_ctrl3_r());
	old_vector = nvgpu_gin_get_vector_from_intr_ctrl_msg(g, old_intr_ctrl_msg);
	nvgpu_writel(g, pgsp_falcon_intr_ctrl3_r(), intr_ctrl3_msg);
	nvgpu_writel(g, pgsp_falcon_intr_retrigger3_r(),
			pgsp_falcon_intr_retrigger3_trigger_true_f());
	if (old_vector != ce_vector) {
		nvgpu_gin_clear_pending_intr(g, old_vector);
	}
}

static int gsp_memcpy_params_check(struct gk20a *g, u32 dmem_addr,
	u32 size_in_bytes, u8 port)
{
	u8 max_emem_ports = (u8)pgsp_ememc__size_1_v();
	u32 start_emem = 0;
	u32 end_emem = 0;
	int status = 0;

	if (size_in_bytes == 0U) {
		nvgpu_err(g, "zero-byte copy requested");
		status = -EINVAL;
		goto exit;
	}

	if (port >= max_emem_ports) {
		nvgpu_err(g, "only %d ports supported. Accessed port=%d",
			max_emem_ports, port);
		status = -EINVAL;
		goto exit;
	}

	if ((dmem_addr & 0x3U) != 0U) {
		nvgpu_err(g, "offset (0x%08x) not 4-byte aligned", dmem_addr);
		status = -EINVAL;
		goto exit;
	}

	status = ga10b_gsp_get_emem_boundaries(g, &start_emem, &end_emem);
	if (status != 0) {
		goto exit;
	}

	if (dmem_addr < start_emem ||
		(dmem_addr + size_in_bytes) > end_emem) {
		nvgpu_err(g, "copy must be in emem aperature [0x%x, 0x%x]",
			start_emem, end_emem);
		status = -EINVAL;
		goto exit;
	}

	return 0;

exit:
	return status;
}

static int gb10b_gsp_emem_transfer(struct gk20a *g, u32 dmem_addr, u8 *buf,
	u32 size_in_bytes, u8 port, bool is_copy_from)
{
	u32 *data = (u32 *)(void *)buf;
	u32 num_words = 0;
	u32 num_bytes = 0;
	u32 start_emem = 0;
	u32 reg = 0;
	u32 i = 0;
	u32 emem_c_offset = 0;
	u32 emem_d_offset = 0;
	int status = 0;

	status = gsp_memcpy_params_check(g, dmem_addr, size_in_bytes, port);
	if (status != 0) {
		goto exit;
	}

	/*
	 * Get the EMEMC/D register addresses
	 * for the specified port
	 */
	emem_c_offset = pgsp_ememc_r(port);
	emem_d_offset = pgsp_ememd_r(port);

	/* Only start address needed */
	status = ga10b_gsp_get_emem_boundaries(g, &start_emem, NULL);
	if (status != 0) {
		goto exit;
	}

	/* Convert to emem offset for use by EMEMC/EMEMD */
	dmem_addr -= start_emem;

	/* Mask off all but the OFFSET and BLOCK in EMEM offset */
	reg = dmem_addr & (pgsp_ememc_offs_m() |
		pgsp_ememc_blk_m());

	if (is_copy_from) {
		/* mark auto-increment on read */
		reg |= pgsp_ememc_aincr_m();
	} else {
		/* mark auto-increment on write */
		reg |= pgsp_ememc_aincw_m();
	}

	nvgpu_writel(g, emem_c_offset, reg);

	/* Calculate the number of words and bytes */
	num_words = size_in_bytes >> 2U;
	num_bytes = size_in_bytes & 0x3U;

	/* Directly copy words to emem*/
	for (i = 0; i < num_words; i++) {
		if (is_copy_from) {
			data[i] = nvgpu_readl(g, emem_d_offset);
		} else {
			nvgpu_writel(g, emem_d_offset, data[i]);
		}
	}

	/* Check if there are leftover bytes to copy */
	if (num_bytes > 0U) {
		u32 bytes_copied = num_words << 2U;

		reg = nvgpu_readl(g, emem_d_offset);
		if (is_copy_from) {
			nvgpu_memcpy((buf + bytes_copied), ((u8 *)&reg),
					num_bytes);
		} else {
			nvgpu_memcpy(((u8 *)&reg), (buf + bytes_copied),
					num_bytes);
			nvgpu_writel(g, emem_d_offset, reg);
		}
	}

exit:
	return status;
}

int gb10b_gsp_flcn_copy_to_emem(struct gk20a *g,
	u32 dst, u8 *src, u32 size, u8 port)
{
	return gb10b_gsp_emem_transfer(g, dst, src, size, port, false);
}

int gb10b_gsp_flcn_copy_from_emem(struct gk20a *g,
	u32 src, u8 *dst, u32 size, u8 port)
{
	return gb10b_gsp_emem_transfer(g, src, dst, size, port, true);
}
