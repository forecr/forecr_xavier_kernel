// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

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
#include <nvgpu/ofa.h>
#include <nvgpu/acr.h>
#include <nvgpu/gsp.h>
#include <nvgpu/fuse.h>
#include "multimedia_priv.h"

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_multimedia.h>
#endif

s32 nvgpu_ofa_sw_init(struct gk20a *g)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	s32 err = 0;
	u32 inst_id = 0;
#if defined(CONFIG_NVGPU_NON_FUSA)
	const char *desc_fw_name;
	const char *image_fw_name;
#endif

	nvgpu_log(g, gpu_dbg_mme, " ");

	if (g->ofa != NULL) {
		/* Initialized already, so reuse the same to perform boot faster */
		return err;
	}

	g->ofa = (struct nvgpu_ofa *)nvgpu_kzalloc(g, sizeof(struct nvgpu_ofa));
	if (g->ofa == NULL) {
		err = -ENOMEM;
		goto done;
	}

	for (inst_id = 0; inst_id < OFA_INST_MAX; inst_id++) {
		g->ofa->ofa_riscv[inst_id] = &g->ofa_riscv[inst_id];
	}

	if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		/* Priv sec is enabled, Engine will be secure booted by ACR */
		return err;
	}

#if defined(CONFIG_NVGPU_NON_FUSA)
	if (nvgpu_get_multimedia_fw_names(g, NVGPU_DEVTYPE_OFA,
					&desc_fw_name, &image_fw_name)) {
		err = nvgpu_multimedia_copy_nvriscv_fw(g, desc_fw_name, image_fw_name,
					g->ofa->nvriscv_firmware_desc,
					&g->ofa->ofa_mem_desc);

		if (err != 0) {
			nvgpu_kfree(g, g->ofa);
			g->ofa = NULL;
		}
	} else
#endif
	{
		err = -ENODEV;
		nvgpu_kfree(g, g->ofa);
		g->ofa = NULL;
		nvgpu_err(g, "no support for GPUID %x", ver);
	}

done:
	nvgpu_log(g, gpu_dbg_mme, "ret %d", err);
	return err;
}

s32 nvgpu_ofa_sw_deinit(struct gk20a *g)
{
	struct nvgpu_ofa *ofa = g->ofa;

	nvgpu_log(g, gpu_dbg_mme, " ");
	if (!nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		/* Priv sec is enabled, Engine will be secure booted by ACR */
		nvgpu_dma_free(g, &ofa->ofa_mem_desc);
	}
	nvgpu_kfree(g, ofa);
	g->ofa = NULL;
	return 0;
}

/* OFA RISC-V EB boot */
s32 nvgpu_ofa_nvriscv_eb_boot(struct gk20a *g, u32 inst_id)
{
	s32 err = 0;
	u32 intr_ctrl = 0U;
	u32 intr_notify_ctrl = 0U;
	struct nvgpu_ofa *ofa = g->ofa;

	nvgpu_log(g, gpu_dbg_mme, " ");

	nvgpu_assert(inst_id == 0U);

	/* By default, the opt_global_error_containment_en fuse is tied to 0 in
	 * iGPU. Use SW Override to allow error containment for OFA.
	 */
	if (g->ops.fuse.feature_override_ofa_err_contain_en != NULL) {
		g->ops.fuse.feature_override_ofa_err_contain_en(g);
	}

	/* Enable GIN interrupts for ofa */
	if (nvgpu_gin_is_present(g)) {
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_OFA,
					inst_id + NVGPU_OFA_LEGACY_VECTOR_OFFSET,
					NVGPU_GIN_INTR_ENABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_OFA,
					inst_id + NVGPU_OFA_UE_VECTOR_OFFSET,
					NVGPU_GIN_INTR_ENABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_OFA,
					inst_id + NVGPU_OFA_CE_VECTOR_OFFSET,
					NVGPU_GIN_INTR_ENABLE);
#ifdef CONFIG_NVGPU_NONSTALL_INTR
		nvgpu_gin_nonstall_unit_config(g, NVGPU_GIN_INTR_UNIT_OFA, inst_id,
					NVGPU_GIN_INTR_ENABLE);
#endif
	} else {
		nvgpu_err(g, "MC level interrupts are not supported");
	}

	if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		nvgpu_log(g, gpu_dbg_mme, "Falcon-%u instance %u Secureboot",
			FALCON_ID_OFA_RISCV_EB, inst_id);

		if (g->ops.ofa.get_intr_ctrl_msg != NULL) {
			g->ops.ofa.get_intr_ctrl_msg(g, inst_id, true,
			&intr_ctrl, &intr_notify_ctrl);
		} else {
			nvgpu_err(g, "Un-supported OFA interrupt msg ctrl");
			err = -EINVAL;
			goto done;
		}

		err = nvgpu_acr_bootstrap_engine(g, FALCON_ID_OFA_RISCV_EB, inst_id, 0,
					intr_ctrl, intr_notify_ctrl);
		if (err != 0) {
			nvgpu_err(g, "flcn-%u instance %u: EB ucode boot failed: %d",
				FALCON_ID_OFA_RISCV_EB, inst_id, err);
			err = ACR_BOOT_TIMEDOUT;
			goto done;
		}

		/* Enable error correction interrupts */
		if (g->ops.ofa.set_ecc_intr != NULL) {
			g->ops.ofa.set_ecc_intr(g, inst_id, true);
		} else {
			nvgpu_err(g, "Un-supported OFA ECC interrupt ctrl");
			err = -EINVAL;
			goto done;
		}

		/* Enable CG settings */
		nvgpu_cg_load_enable(g, ofa->ofa_riscv[inst_id]);

		/* Wait for falcon idle */
		err = nvgpu_falcon_wait_idle(ofa->ofa_riscv[inst_id]);
		if (err != 0) {
			err = -ETIMEDOUT;
			nvgpu_err(g, "nvriscv %u instance %u ucode wait for idle failed, err=%d",
				FALCON_ID_OFA_RISCV_EB, inst_id, err);
			goto done;
		}
		/*
		* Check if it has reached a proper initialized state,
		* indicated by clearing the debuginfo register.
		*/
		err = nvgpu_falcon_wait_init_completion(ofa->ofa_riscv[inst_id]);
		if (err != 0) {
			nvgpu_err(g, "nvriscv %u instance %u ucode has not reached proper init state, err=%d",
				FALCON_ID_OFA_RISCV_EB, inst_id, err);
			goto done;
		}
	} else {
		err = nvgpu_nvriscv_ns_boot(ofa->ofa_riscv[inst_id], &ofa->ofa_mem_desc,
					ofa->nvriscv_firmware_desc);
	}
	if (err != 0) {
		nvgpu_err(g, "Failed to boot ofa nvriscv, err=%d", err);
		nvgpu_multimedia_debug_dump(g,
				nvgpu_device_get(g, NVGPU_DEVTYPE_OFA, inst_id));
	}

done:
	nvgpu_log(g, gpu_dbg_mme, "OFA NS boot %s!",
			(err == 0) ? "SUCCESS" : "FAILED");
	return err;
}

static void nvgpu_ofa_halt(struct gk20a *g, const struct nvgpu_device *dev)
{
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_mme, "Halting ofa%u", dev->inst_id);

	if (g->ops.ofa.halt_engine != NULL) {
		g->ops.ofa.halt_engine(g, dev->inst_id);
	}

	/* Halt sequence requires device runlist id to wait for preemption */
	if (g->ops.fifo.wait_for_preempt_before_reset != NULL) {
		g->ops.fifo.wait_for_preempt_before_reset(g, dev->runlist_id);
	}
}

s32 nvgpu_ofa_reset_and_boot(struct gk20a *g, const struct nvgpu_device *dev)
{
	s32 err = 0;

	/* Halt the engine first, before the reset */
	nvgpu_ofa_halt(g, dev);

	/* Reset ofa HW unit and load ucode */
	nvgpu_log(g, gpu_dbg_info | gpu_dbg_mme, "Resetting ofa%u", dev->inst_id);
	err = g->ops.ofa.bootstrap(g, dev->inst_id);

	return err;
}

void nvgpu_ofa_dump_status(struct gk20a *g, u32 inst_id,
			struct nvgpu_debug_context *dbg_ctx)
{
	struct nvgpu_falcon *flcn = g->ofa->ofa_riscv[inst_id];

	gk20a_debug_output(dbg_ctx, "--------------------------------");
	gk20a_debug_output(dbg_ctx, "          OFA%u Status", inst_id);
	gk20a_debug_output(dbg_ctx, "--------------------------------");

	if (g->ops.falcon.dump_debug_regs != NULL) {
		g->ops.falcon.dump_debug_regs(flcn, dbg_ctx);
	}
}
