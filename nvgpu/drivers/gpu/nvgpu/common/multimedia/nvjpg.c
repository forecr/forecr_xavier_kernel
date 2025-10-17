// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/firmware.h>
#include <nvgpu/falcon.h>
#include <nvgpu/riscv.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/soc.h>
#include <nvgpu/io.h>
#include <nvgpu/dma.h>
#include <nvgpu/string.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/device.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/nvjpg.h>
#include <nvgpu/acr.h>
#include <nvgpu/gsp.h>
#include <nvgpu/fuse.h>
#include "multimedia_priv.h"

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_multimedia.h>
#endif

s32 nvgpu_nvjpg_sw_init(struct gk20a *g)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	s32 err = 0;
	u32 inst_id = 0;
#if defined(CONFIG_NVGPU_NON_FUSA)
	const char *desc_fw_name;
	const char *image_fw_name;
#endif

	nvgpu_log(g, gpu_dbg_mme, " ");

	if (g->nvjpg != NULL) {
		/* Initialized already, so reuse the same to perform boot faster */
		return err;
	}

	g->nvjpg = (struct nvgpu_nvjpg *)nvgpu_kzalloc(g, sizeof(struct nvgpu_nvjpg));
	if (g->nvjpg == NULL) {
		err = -ENOMEM;
		goto done;
	}

	for (inst_id = 0; inst_id < NVJPG_INST_MAX; inst_id++) {
		g->nvjpg->nvjpg_riscv[inst_id] = &g->nvjpg_riscv[inst_id];
	}

	if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		/* Priv sec is enabled, Engine will be secure booted by ACR */
		return err;
	}

#if defined(CONFIG_NVGPU_NON_FUSA)
	if (nvgpu_get_multimedia_fw_names(g, NVGPU_DEVTYPE_NVJPG,
					&desc_fw_name, &image_fw_name)) {
		err = nvgpu_multimedia_copy_nvriscv_fw(g, desc_fw_name, image_fw_name,
					g->nvjpg->nvriscv_firmware_desc,
					&g->nvjpg->nvjpg_mem_desc);

		if (err != 0) {
			nvgpu_kfree(g, g->nvjpg);
			g->nvjpg = NULL;
		}
	} else
#endif
	{
		err = -ENODEV;
		nvgpu_kfree(g, g->nvjpg);
		g->nvjpg = NULL;
		nvgpu_err(g, "no support for GPUID %x", ver);
	}

done:
	nvgpu_log(g, gpu_dbg_mme, "ret %d", err);
	return err;
}

s32 nvgpu_nvjpg_sw_deinit(struct gk20a *g)
{
	struct nvgpu_nvjpg *nvjpg = g->nvjpg;

	nvgpu_log(g, gpu_dbg_mme, " ");
	if (!nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		/* Priv sec is enabled, Engine will be secure booted by ACR */
		nvgpu_dma_free(g, &nvjpg->nvjpg_mem_desc);
	}
	nvgpu_kfree(g, nvjpg);
	g->nvjpg = NULL;
	return 0;
}

/* NVJPG NVRISC-V EB boot */
s32 nvgpu_nvjpg_nvriscv_eb_boot(struct gk20a *g, u32 inst_id)
{
	s32 err = 0;
	u32 intr_ctrl = 0U;
	u32 intr_notify_ctrl = 0U;
	struct nvgpu_nvjpg *nvjpg = g->nvjpg;

	nvgpu_log(g, gpu_dbg_mme, " ");

	nvgpu_assert(inst_id < NVJPG_INST_MAX);

	/* Enable GIN interrupts for nvjpg */
	if (nvgpu_gin_is_present(g)) {
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_NVJPG, inst_id,
					NVGPU_GIN_INTR_ENABLE);
#ifdef CONFIG_NVGPU_NONSTALL_INTR
		nvgpu_gin_nonstall_unit_config(g, NVGPU_GIN_INTR_UNIT_NVJPG, inst_id,
					NVGPU_GIN_INTR_ENABLE);
#endif
	} else {
		nvgpu_err(g, "MC level interrupts are not supported");
	}

	if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		nvgpu_log(g, gpu_dbg_mme, "Falcon-%u instance %u Secureboot",
			FALCON_ID_NVJPG_RISCV_EB, inst_id);

		if (g->ops.nvjpg.get_intr_ctrl_msg != NULL) {
			g->ops.nvjpg.get_intr_ctrl_msg(g, inst_id, true,
			&intr_ctrl, &intr_notify_ctrl);
		} else {
			nvgpu_err(g, "Un-supported NVJPG interrupt msg ctrl");
			err = -EINVAL;
			goto done;
		}

		err = nvgpu_acr_bootstrap_engine(g, FALCON_ID_NVJPG_RISCV_EB, inst_id, 0,
					intr_ctrl, intr_notify_ctrl);
		if (err != 0) {
			nvgpu_err(g, "flcn-%u instance %u: EB ucode boot failed: %d",
				FALCON_ID_NVJPG_RISCV_EB, inst_id, err);
			err = ACR_BOOT_TIMEDOUT;
			goto done;
		}
		/* Enable CG settings */
		nvgpu_cg_load_enable(g, nvjpg->nvjpg_riscv[inst_id]);

		/* Wait for falcon idle */
		err = nvgpu_falcon_wait_idle(nvjpg->nvjpg_riscv[inst_id]);
		if (err != 0) {
			err = -ETIMEDOUT;
			nvgpu_err(g, "nvriscv %u instance %u ucode wait for idle failed, err=%d",
				FALCON_ID_NVJPG_RISCV_EB, inst_id, err);
			goto done;
		}
		/*
		* Check if it has reached a proper initialized state,
		* indicated by clearing the debuginfo register.
		*/
		err = nvgpu_falcon_wait_init_completion(nvjpg->nvjpg_riscv[inst_id]);
		if (err != 0) {
			nvgpu_err(g, "nvriscv %u instance %u ucode has not reached proper init state, err=%d",
				FALCON_ID_NVJPG_RISCV_EB, inst_id, err);
			goto done;
		}

	} else {
		err = nvgpu_nvriscv_ns_boot(nvjpg->nvjpg_riscv[inst_id], &nvjpg->nvjpg_mem_desc,
					nvjpg->nvriscv_firmware_desc);
	}
	if (err != 0) {
		nvgpu_err(g, "Failed to boot nvjpg%u nvriscv, err=%d", inst_id, err);
		nvgpu_multimedia_debug_dump(g,
				nvgpu_device_get(g, NVGPU_DEVTYPE_NVJPG, inst_id));
	}

done:
	nvgpu_log(g, gpu_dbg_mme, "NVJPG%u NS boot %s!",
			inst_id, (err == 0) ? "SUCCESS" : "FAILED");
	return err;
}

static void nvgpu_nvjpg_halt(struct gk20a *g, const struct nvgpu_device *dev)
{
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_mme, "Halting nvjpg%u", dev->inst_id);

	if (g->ops.nvjpg.halt_engine != NULL) {
		g->ops.nvjpg.halt_engine(g, dev->inst_id);
	}

	/* Halt sequence requires device runlist id to wait for preemption */
	if (g->ops.fifo.wait_for_preempt_before_reset != NULL) {
		g->ops.fifo.wait_for_preempt_before_reset(g, dev->runlist_id);
	}
}

s32 nvgpu_nvjpg_reset_and_boot(struct gk20a *g, const struct nvgpu_device *dev)
{
	s32 err = 0;

	/* Halt the engine first, before the reset */
	nvgpu_nvjpg_halt(g, dev);

	/* Reset nvjpg HW unit and load ucode */
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_mme, "Resetting nvjpg%u", dev->inst_id);
	err = g->ops.nvjpg.bootstrap(g, dev->inst_id);

	return err;
}

void nvgpu_nvjpg_dump_status(struct gk20a *g, u32 inst_id,
			struct nvgpu_debug_context *dbg_ctx)
{
	struct nvgpu_falcon *flcn = g->nvjpg->nvjpg_riscv[inst_id];

	gk20a_debug_output(dbg_ctx, "--------------------------------");
	gk20a_debug_output(dbg_ctx, "          NVJPG%u Status", inst_id);
	gk20a_debug_output(dbg_ctx, "--------------------------------");

	if (g->ops.falcon.dump_debug_regs != NULL) {
		g->ops.falcon.dump_debug_regs(flcn, dbg_ctx);
	}
}
