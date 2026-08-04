// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/timers.h>
#include <nvgpu/dma.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/falcon.h>
#include <nvgpu/nvenc.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/soc.h>
#include <nvgpu/io.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/string.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/acr.h>
#include <nvgpu/gsp.h>
#include <nvgpu/fuse.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/device.h>
#include "multimedia_priv.h"
#include <nvgpu/gin.h>

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_multimedia.h>
#endif

/* NVENC init */
s32 nvgpu_nvenc_sw_init(struct gk20a *g)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	s32 err = 0;
#if defined(CONFIG_NVGPU_NON_FUSA)
	u32 inst_id = 0;
	const char *desc_fw_name;
	const char *image_fw_name;
#endif

	nvgpu_log(g, gpu_dbg_mme, " ");

	if (g->nvenc == NULL) {
		g->nvenc = (struct nvgpu_nvenc *)nvgpu_kzalloc(g, sizeof(struct nvgpu_nvenc));
		if (g->nvenc == NULL) {
			err = -ENOMEM;
			goto done;
		}
	}

	switch (ver) {
#ifdef CONFIG_NVGPU_DGPU
	case NVGPU_GPUID_TU104:
		if (g->nvenc->nvenc_flcn != NULL) {
			/* Initialized already, reuse the same to perform boot faster. Return EOK */
			return err;
		}

		g->nvenc->nvenc_flcn = &g->nvenc_flcn;
		err = nvgpu_multimedia_copy_falcon_fw(g, TU104_NVENC_UCODE_FW,
						g->nvenc->ucode_header,
						&g->nvenc->nvenc_mem_desc);

		if (err != 0) {
			nvgpu_kfree(g, g->nvenc);
			g->nvenc = NULL;
		}
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA)
		if (g->nvenc->nvenc_riscv[0] != NULL) {
			/* Initialized already, reuse the same to perform boot faster. Return EOK */
			return err;
		}

		for (inst_id = 0; inst_id < NVENC_INST_MAX; inst_id++) {
			g->nvenc->nvenc_riscv[inst_id] = &g->nvenc_riscv[inst_id];
		}

		if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
			/* Priv sec is enabled, Engine will be secure booted by ACR */
			return err;
		}

		if (nvgpu_get_multimedia_fw_names(g, NVGPU_DEVTYPE_NVENC,
						&desc_fw_name, &image_fw_name)) {
			err = nvgpu_multimedia_copy_nvriscv_fw(g,
						desc_fw_name, image_fw_name,
						g->nvenc->nvriscv_firmware_desc,
						&g->nvenc->nvenc_mem_desc);

			if (err != 0) {
				nvgpu_kfree(g, g->nvenc);
				g->nvenc = NULL;
			}
		} else
#endif
		{
			err = -ENODEV;
			nvgpu_kfree(g, g->nvenc);
			g->nvenc = NULL;
			nvgpu_err(g, "no support for GPUID %x", ver);
		}
		break;
	}

done:
	nvgpu_log(g, gpu_dbg_mme, "ret %d", err);
	return err;
}

/* NVENC deinit */
s32 nvgpu_nvenc_sw_deinit(struct gk20a *g)
{
	struct nvgpu_nvenc *nvenc = g->nvenc;

	nvgpu_log(g, gpu_dbg_mme, " ");
	if (!nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		/* Priv sec is enabled, Engine will be secure booted by ACR */
		nvgpu_dma_free(g, &nvenc->nvenc_mem_desc);
	}
	nvgpu_kfree(g, nvenc);
	g->nvenc = NULL;
	return 0;
}

/* NVENC falcon boot */
s32 nvgpu_nvenc_falcon_boot(struct gk20a *g, u32 inst_id)
{
	s32 err = 0;
	s32 completion = 0;

	struct nvgpu_nvenc *nvenc = g->nvenc;
	struct nvgpu_falcon *flcn;
	struct nvgpu_mem *nvenc_mem_desc = &nvenc->nvenc_mem_desc;
	u32 *ucode_header = nvenc->ucode_header;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, " ");

	nvgpu_assert(inst_id == 0U);
	flcn = nvenc->nvenc_flcn;

	/* Reset nvenc HW unit */
	err = nvgpu_mc_reset_units(g, NVGPU_UNIT_NVENC);
	if (err != 0) {
		nvgpu_err(g, "Failed to reset NVENC unit");
		goto done;
	}

	/* Falcon reset */
	err = nvgpu_falcon_reset(flcn);
	if (err != 0) {
		nvgpu_err(g, "nvenc falcon_reset failed, err=%d", err);
		goto done;
	}

	/* Setup falcon apertures, boot-config */
	if (g->ops.nvenc.setup_boot_config != NULL) {
		g->ops.nvenc.setup_boot_config(g, inst_id);
	}

	/* Load nvenc ucode */
	err = nvgpu_falcon_load_ucode(flcn, nvenc_mem_desc, ucode_header);
	if (err != 0) {
		nvgpu_err(g, "nvenc ucode loading failed, err=%d", err);
		goto done;
	}

	/* Enable falcon interfaces */
	if (g->ops.nvenc.interface_enable != NULL)
		g->ops.nvenc.interface_enable(g, inst_id);

	/* Kick-start falcon cpu */
	g->ops.falcon.bootstrap(flcn, 0U);

	/* Wait for falcon idle */
	completion = nvgpu_falcon_wait_idle(flcn);
	if (completion != 0) {
		nvgpu_err(g, "nvenc flcn: ucode boot timed out");
		err = -ETIMEDOUT;
	}

	nvgpu_log(g, gpu_dbg_info | gpu_dbg_mme,
		"NVENC NS boot %s!", err ? "SUCCESS" : "FAILED");

done:
	return err;
}

static void nvgpu_nvenc_halt(struct gk20a *g, const struct nvgpu_device *dev)
{
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_mme, "Halting nvenc%u", dev->inst_id);

	if (g->ops.nvenc.halt_engine != NULL) {
		g->ops.nvenc.halt_engine(g, dev->inst_id);
	}

	/* Halt sequence requires device runlist id to wait for preemption */
	if (g->ops.fifo.wait_for_preempt_before_reset != NULL) {
		g->ops.fifo.wait_for_preempt_before_reset(g, dev->runlist_id);
	}
}

s32 nvgpu_nvenc_reset_and_boot(struct gk20a *g, const struct nvgpu_device *dev)
{
	s32 err = 0;

	/* Halt the engine first, before the reset */
	nvgpu_nvenc_halt(g, dev);

	/* Reset nvenc HW unit and load ucode */
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_mme, "Resetting nvenc%u", dev->inst_id);
	err = g->ops.nvenc.bootstrap(g, dev->inst_id);

	return err;
}

/* NVENC NVRISC-V EB boot */
s32 nvgpu_nvenc_nvriscv_eb_boot(struct gk20a *g, u32 inst_id)
{
	s32 err = 0;
	u32 intr_ctrl = 0U;
	u32 intr_notify_ctrl = 0U;
	struct nvgpu_nvenc *nvenc = g->nvenc;

	nvgpu_log(g, gpu_dbg_mme, " ");

	nvgpu_assert(inst_id < NVENC_INST_MAX);

	/* By default, the opt_global_error_containment_en fuse is tied to 0 in
	 * iGPU. Use SW Override to allow error containment for NvENC.
	 */
	if (g->ops.fuse.feature_override_nvenc_err_contain_en != NULL) {
		g->ops.fuse.feature_override_nvenc_err_contain_en(g);
	}

	/* Enable GIN interrupts for nvenc */
	if (nvgpu_gin_is_present(g)) {
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_NVENC, inst_id,
					NVGPU_GIN_INTR_ENABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_NVENC,
					NVGPU_NVENC_UE_VECTOR_OFFSET_0 + inst_id,
					NVGPU_GIN_INTR_ENABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_NVENC,
					NVGPU_NVENC_CE_VECTOR_OFFSET_0 + inst_id,
					NVGPU_GIN_INTR_ENABLE);
#ifdef CONFIG_NVGPU_NONSTALL_INTR
		nvgpu_gin_nonstall_unit_config(g, NVGPU_GIN_INTR_UNIT_NVENC, inst_id,
					NVGPU_GIN_INTR_ENABLE);
#endif
	} else {
		nvgpu_err(g, "MC level interrupts are not supported");
	}

	if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		nvgpu_log(g, gpu_dbg_mme, "Falcon-%u instance %u Secureboot",
			FALCON_ID_NVENC_RISCV_EB, inst_id);

		if (g->ops.nvenc.get_intr_ctrl_msg != NULL) {
			g->ops.nvenc.get_intr_ctrl_msg(g, inst_id, true,
			&intr_ctrl, &intr_notify_ctrl);
		} else {
			nvgpu_err(g, "Un-supported NVENC interrupt msg ctrl");
			err = -EINVAL;
			goto done;
		}

		err = nvgpu_acr_bootstrap_engine(g, FALCON_ID_NVENC_RISCV_EB, inst_id, 0,
					intr_ctrl, intr_notify_ctrl);
		if (err != 0) {
			nvgpu_err(g, "flcn-%u instance %u: EB ucode boot failed: %d",
				FALCON_ID_NVENC_RISCV_EB, inst_id, err);
			err = ACR_BOOT_TIMEDOUT;
			goto done;
		}
		/* Enable CG settings */
		nvgpu_cg_load_enable(g, nvenc->nvenc_riscv[inst_id]);

		/* Enable Safety interrupts */
		if (g->ops.nvenc.set_safety_intr != NULL) {
			g->ops.nvenc.set_safety_intr(g, inst_id, true);
		}

		/* Wait for falcon idle */
		err = nvgpu_falcon_wait_idle(nvenc->nvenc_riscv[inst_id]);
		if (err != 0) {
			err = -ETIMEDOUT;
			nvgpu_err(g, "nvriscv %u instance %u ucode wait for idle failed, err=%d",
				FALCON_ID_NVENC_RISCV_EB, inst_id, err);
			goto done;
		}
		/*
		* Check if it has reached a proper initialized state,
		* indicated by clearing the debuginfo register.
		*/
		err = nvgpu_falcon_wait_init_completion(nvenc->nvenc_riscv[inst_id]);
		if (err != 0) {
			nvgpu_err(g, "nvriscv %d instance %u ucode has not reached proper init state, err=%d",
				FALCON_ID_NVENC_RISCV_EB, inst_id, err);
			goto done;
		}
	} else {
		err = nvgpu_nvriscv_ns_boot(nvenc->nvenc_riscv[inst_id], &nvenc->nvenc_mem_desc,
					nvenc->nvriscv_firmware_desc);
	}
	if (err != 0) {
		nvgpu_err(g, "Failed to boot nvenc%u nvriscv, err=%d", inst_id, err);
		nvgpu_multimedia_debug_dump(g,
				nvgpu_device_get(g, NVGPU_DEVTYPE_NVENC, inst_id));
	}

done:
	nvgpu_log(g, gpu_dbg_mme, "NVENC%u NS boot %s!", inst_id,
			(err == 0) ? "SUCCESS" : "FAILED");
	return err;
}

void nvgpu_nvenc_dump_status(struct gk20a *g, u32 inst_id,
			struct nvgpu_debug_context *dbg_ctx)
{
	struct nvgpu_falcon *flcn = g->nvenc->nvenc_riscv[inst_id];

	gk20a_debug_output(dbg_ctx, "--------------------------------");
	gk20a_debug_output(dbg_ctx, "          NVENC%u Status", inst_id);
	gk20a_debug_output(dbg_ctx, "--------------------------------");

	if (g->ops.falcon.dump_debug_regs != NULL) {
		g->ops.falcon.dump_debug_regs(flcn, dbg_ctx);
	}
}
