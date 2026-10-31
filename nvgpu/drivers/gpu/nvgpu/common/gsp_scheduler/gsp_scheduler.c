// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/kmem.h>
#include <nvgpu/log.h>
#include <nvgpu/errno.h>
#include <nvgpu/gsp.h>
#include <nvgpu/gsp_sched.h>
#include <nvgpu/mm.h>
#include <nvgpu/power_features/pg.h>
#include "gsp_scheduler.h"
#include "gsp_runlist.h"
#include "common/gsp/ipc/gsp_cmd.h"

/**
 * @brief Determines and assigns the appropriate firmware file names to the GSP firmware
 *        structure based on the current debug mode setting.
 *
 * The function checks the current debug mode setting for the GSP and then assigns the
 * corresponding firmware file names for either the production or debug environment to the
 * GSP firmware structure. This is essential for ensuring that the correct firmware is loaded
 * depending on the operational mode of the GSP.
 *
 * The steps performed by the function are as follows:
 * -# Check if the debug mode is enabled by invoking is_debug_mode_enabled().
 * -# If the debug mode is not enabled, assign the production firmware file names to the
 *    firmware structure's code_name, data_name, and manifest_name members.
 * -# If the debug mode is enabled, assign the debug firmware file names to the firmware
 *    structure's code_name, data_name, and manifest_name members.
 *
 * @Param [in]  g         Pointer to the GPU driver structure, which contains the operations
 *                        structure including the is_debug_mode_enabled() function.
 * @Param [out] gsp_ucode Pointer to the GSP firmware structure where the firmware file names
 *                        will be stored.
 */
static void gsp_sched_get_file_names(struct gk20a *g, struct gsp_fw *gsp_ucode)
{
	if (!g->ops.gsp.is_debug_mode_enabled(g)) {
		gsp_ucode->code_name = SAFETY_SCHED_RISCV_FW_CODE_PROD;
		gsp_ucode->data_name = SAFETY_SCHED_RISCV_FW_DATA_PROD;
		gsp_ucode->manifest_name = SAFETY_SCHED_RISCV_FW_MANIFEST_PROD;
	} else {
		gsp_ucode->code_name = SAFETY_SCHED_RISCV_FW_CODE;
		gsp_ucode->data_name = SAFETY_SCHED_RISCV_FW_DATA;
		gsp_ucode->manifest_name = SAFETY_SCHED_RISCV_FW_MANIFEST;
	}
}

/* TODO: Redundant, if no gsp_sched specific suspend functionality */
void nvgpu_gsp_sched_suspend(struct gk20a *g, struct nvgpu_gsp_sched *gsp_sched)
{
	struct nvgpu_gsp *gsp = gsp_sched->gsp;

	nvgpu_gsp_dbg(g, " ");

	if (gsp == NULL) {
		nvgpu_err(g, "GSP not initialized");
		return;
	}
}

/**
 * @brief Deinitializes the GSP scheduler.
 *
 * This function is responsible for deinitializing the GSP scheduler by
 * freeing the memory allocated for the GSP scheduler structure.
 *
 * The steps performed by the function are as follows:
 * -# Call gsp_release_firmware() to release the firmware.
 * -# Call nvgpu_kfree() to free the memory allocated for the GSP scheduler
 *    structure.
 * -# Set the pointer to the GSP scheduler structure to NULL to prevent any
 *    further access to the freed memory.
 *
 * @param [in] g          Pointer to the GPU driver structure.
 */
void nvgpu_gsp_sched_sw_deinit(struct gk20a *g)
{
	struct nvgpu_gsp_sched *gsp_sched = g->gsp_sched;

	nvgpu_gsp_dbg(g, " ");

	if (gsp_sched != NULL) {
		gsp_release_firmware(g, gsp_sched->gsp);
		nvgpu_kfree(g, gsp_sched);
		g->gsp_sched = NULL;
	}
}

/**
 * @brief Waits for the GSP scheduler to become ready or until a timeout occurs.
 *
 * This function checks if the GSP scheduler is ready in a loop with a delay
 * between each check. It exits the loop if the GSP scheduler becomes ready or
 * if the specified timeout duration has elapsed.
 *
 * The steps performed by the function are as follows:
 * -# Enter a loop that continues until the GSP scheduler is ready or the timeout
 *    is reached.
 * -# Inside the loop, check if the 'gsp_ready' flag of the GSP scheduler structure
 *    is true. If it is, exit the loop as the GSP scheduler is ready.
 * -# If the timeout value is less than or equal to zero, log an error message
 *    indicating that the wait for the GSP scheduler to initialize has timed out,
 *    and return -1 to indicate a timeout error.
 * -# Call nvgpu_msleep() to sleep for a short duration to prevent busy-waiting.
 * -# Decrement the timeout value by the sleep duration.
 * -# If the loop exits because the GSP scheduler is ready, return 0 to indicate
 *    success.
 *
 * @param [in] g          Pointer to the GPU driver structure.
 * @param [in] timeoutms  The maximum time to wait for the GSP scheduler to become
 *                        ready, in milliseconds.
 *
 * @return 0 if the GSP scheduler becomes ready before the timeout.
 * @return -1 if a timeout occurs before the GSP scheduler becomes ready.
 */
static int gsp_sched_wait_for_init(struct gk20a *g, signed int timeoutms)
{
	nvgpu_gsp_dbg(g, " ");

	do {
		if (nvgpu_gsp_is_ready(g)) {
			break;
		}

		if (timeoutms <= 0) {
			nvgpu_err(g, "gsp wait for init timedout");
			return -1;
		}

		nvgpu_msleep(10);
		timeoutms -= 10;
	} while (true);

	return 0;
}

int nvgpu_gsp_sched_sw_init(struct gk20a *g)
{
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	if (g->gsp_sched != NULL) {
		/*
		 * Recovery/unrailgate case, we do not need to do gsp_sched init as
		 * gsp_sched is set during cold boot & doesn't execute gsp_sched clean
		 * up as part of power off sequence, so reuse to perform faster boot.
		 */
		return err;
	}

	/* Init struct holding the gsp sched software state */
	g->gsp_sched = (struct nvgpu_gsp_sched *)
					nvgpu_kzalloc(g, sizeof(struct nvgpu_gsp_sched));
	if (g->gsp_sched == NULL) {
		err = -ENOMEM;
		goto de_init;
	}

	g->gsp_sched->gsp = g->gsp;

	nvgpu_gsp_dbg(g, " Done ");
	return err;

de_init:
	nvgpu_gsp_sched_sw_deinit(g);
	return err;
}

int nvgpu_gsp_sched_bootstrap_hs(struct gk20a *g)
{
	struct nvgpu_gsp_sched *gsp_sched = g->gsp_sched;
	int status = 0;

#ifdef CONFIG_NVGPU_FALCON_DEBUG
	status = nvgpu_gsp_debug_buf_init(g, GSP_SCHED_DEBUG_BUFFER_QUEUE,
			GSP_SCHED_DMESG_BUFFER_SIZE);
	if (status != 0) {
		nvgpu_err(g, "GSP sched debug buf init failed");
		goto de_init;
	}
#endif

	/* Get ucode file names */
	gsp_sched_get_file_names(g, &gsp_sched->gsp->gsp_ucode);

#ifdef CONFIG_NVGPU_POWER_PG
	status = nvgpu_pg_elpg_protected_call(g,
		nvgpu_gsp_bootstrap_ns(g, gsp_sched->gsp));
	if (status != 0) {
		nvgpu_err(g, "GSP sched bootstrap failed in elpg");
		goto de_init;
	}
#else
	status = nvgpu_gsp_bootstrap_ns(g, gsp_sched->gsp);
	if (status != 0) {
		nvgpu_err(g, " GSP sched bootstrap failed ");
		goto de_init;
	}
#endif

	status = nvgpu_gsp_wait_for_priv_lockdown_release(gsp_sched->gsp,
				GSP_WAIT_TIME_MS);
	if (status != 0) {
		nvgpu_err(g, "gsp PRIV lockdown release wait failed ");
		goto de_init;
	}

	/* setup gsp ctx instance */
	if (g->ops.gsp.falcon_setup_boot_config != NULL) {
		g->ops.gsp.falcon_setup_boot_config(g);
	}

	status = gsp_sched_wait_for_init(g, GSP_WAIT_TIME_MS);
	if (status != 0) {
		nvgpu_err(g, "gsp wait for basic init failed ");
		goto de_init;
	}

	status = nvgpu_gsp_sched_bind_ctx_reg(g);
	if (status != 0) {
		nvgpu_err(g, "gsp bind ctx register failed");
		goto de_init;
	}

	status = nvgpu_gsp_sched_send_devices_info(g);
	if (status != 0) {
		nvgpu_err(g, "gsp send device info failed");
		goto de_init;
	}

	nvgpu_gsp_dbg(g, "gsp scheduler bootstrapped.\n");
	return status;
de_init:
	nvgpu_gsp_sched_sw_deinit(g);
	return status;
}

static int nvgpu_gsp_get_inst_bind_info(struct gk20a *g, struct nvgpu_gsp_inst_bind_info *nxtctx)
{
	struct mm_gk20a *mm = &g->mm;

	nvgpu_gsp_dbg(g, " ");

	/*
	 * The instance block address to write is the lower 32-bits of the 4K-
	 * aligned physical instance block address.
	 */
	nxtctx->aperture = (u32)mm->gsp.inst_block.aperture;
	if (nxtctx->aperture == 0x0U) {
		nvgpu_err(g, "invalid gsp inst block mem aperture");
		return -EINVAL;
	}

	nxtctx->inst_ptr = nvgpu_inst_block_ptr(g, &mm->gsp.inst_block);
	if (nxtctx->inst_ptr == 0x0U) {
		nvgpu_err(g, "invalid gsp inst block physical address");
		return -EINVAL;
	}

	return 0;
}

int nvgpu_gsp_sched_bind_ctx_reg(struct gk20a *g)
{
	struct nv_flcn_cmd_gsp cmd = {0};
	struct nvgpu_gsp_inst_bind_info nxtctx_info = {0};
	int err = 0;
	nvgpu_gsp_dbg(g, " ");

	err = nvgpu_gsp_get_inst_bind_info(g, &nxtctx_info);
	if (err != 0) {
		nvgpu_err(g, "get inst bind info failed");
		goto go_return;
	}

	cmd.cmd.nxtctx = nxtctx_info;

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_BIND_CTX_REG, (u32)sizeof(struct nvgpu_gsp_inst_bind_info));
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
	}

go_return:
	return err;
}
