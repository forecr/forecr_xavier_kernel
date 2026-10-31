// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gsp.h>
#include <nvgpu/pmu.h>
#include <nvgpu/acr.h>
#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/device.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/string.h>

#include "acr_priv.h"
#include "acr_bootstrap.h"
#include "common/gsp_scheduler/gsp_scheduler.h"
#include "common/gsp/ipc/gsp_msg.h"

static int nvgpu_acr_wait_message_cond(struct gk20a *g, u32 timeout_ms,
		void *var, u8 val)
{
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;
	bool queue_empty_checked = false;
	u32 intr_stat = 0;

	/*
	 * There is time gap between gsp switching to ACR partition and
	 * triggering privlockdown, this gap can cause PRI errors for
	 * gsp IRQSTAT and IRQMASK reads while polling for it.
	 */

	nvgpu_timeout_init_cpu_timer(g, &timeout, timeout_ms);

	do {
		nvgpu_usleep_range(delay, delay * 2U);
		delay = min_t(u32, delay << 1U, POLL_DELAY_MAX_US);

		if (!queue_empty_checked) {
			if (nvgpu_readl(g, g->ops.gsp.gsp_get_queue_head(0U)) !=
				nvgpu_readl(g, g->ops.gsp.gsp_get_queue_tail(0U))) {
				/* Continue with sleep loop until queue is empty */
				continue;
			} else {
				/*
				 * Extra sleep cycles needed due to other checks
				 * performed in the ucode, after queue has been
				 * emptied, prior to the partition switch.
				 */
				nvgpu_usleep_range(50U, 100U);
				queue_empty_checked = true;
			}
		}

		if (*(u8 *)var == val) {
			return 0;
		}

		if (!g->ops.falcon.is_priv_lockdown(&g->gsp_flcn)) {
			if (g->ops.gsp.gsp_is_interrupted(g, &intr_stat)) {
				g->ops.gsp.gsp_isr(g);
			}
		}
	} while (nvgpu_timeout_expired(&timeout) == 0U);

	/*
	 * Check again after timeout just to be sure incase
	 * the condition was met but timer was bad.
	 */
	if (*(u8 *)var == val) {
		return 0;
	}

	return -ETIMEDOUT;
}

static void acr_handle_cmd_ack(struct gk20a *g, struct nv_flcn_msg_gsp *msg,
			void *param, u32 status)
{
	bool *command_ack = param;
	nvgpu_acr_dbg(g, " ");

	g->acr->msg_status.error_code = msg->msg.acr_msg.acr_status.error_code;
	g->acr->msg_status.error_info = msg->msg.acr_msg.acr_status.error_info;
	g->acr->msg_status.lib_nvriscv_error = msg->msg.acr_msg.acr_status.lib_nvriscv_error;

	nvgpu_acr_dbg(g, "ACR msg type:0x%x", msg->msg.acr_msg.msg_type);
	nvgpu_acr_dbg(g, "ACR error code:0x%x", msg->msg.acr_msg.acr_status.error_code);
	nvgpu_acr_dbg(g, "ACR error info:0x%x", msg->msg.acr_msg.acr_status.error_info);
	nvgpu_acr_dbg(g, "ACR lib nvrisv error:0x%x", msg->msg.acr_msg.acr_status.lib_nvriscv_error);

	*command_ack = true;
	(void)status;
}

int nvgpu_acr_send_cmd_and_wait_for_ack(struct gk20a *g,
		struct nv_flcn_cmd_gsp *cmd, u32 unit_id, u32 size)
{
	bool command_ack = false;
	int err = 0;
	u32 timeout = 0;
	size_t tmp_size;

	nvgpu_acr_dbg(g, " ");

	if (g->acr != NULL) {
		if (!nvgpu_gsp_is_ready(g)) {
			nvgpu_err(g, "acr cmd/msg init not received");
			err = -EPERM;
			goto exit;
		}
	}

	tmp_size = GSP_CMD_HDR_SIZE + size;
	nvgpu_assert(tmp_size <= U64(U8_MAX));

	if (cmd == NULL) {
		nvgpu_err(g, "gsp cmd buffer is empty");
		err = -EINVAL;
		goto exit;
	}

	cmd->hdr.size = (u8)tmp_size;
	cmd->hdr.unit_id = (u8)unit_id;

	if (nvgpu_platform_is_silicon(g)) {
		timeout = ACR_COMPLETION_TIMEOUT_SILICON_MS;
	} else {
		timeout = ACR_COMPLETION_TIMEOUT_NON_SILICON_MS;
	}

	nvgpu_mutex_acquire(&g->gsp->gsp_ipc->cmd_lock);

	err = nvgpu_gsp_cmd_post(g, cmd, GSP_NV_CMDQ_LOG_ID,
			acr_handle_cmd_ack, &command_ack, timeout);
	if (err != 0) {
		nvgpu_err(g, "cmd post failed unit_id:0x%x", unit_id);
		goto exit;
	}

	err = nvgpu_acr_wait_message_cond(g, timeout,
			&command_ack, U8(true));
	if (err != 0) {
		nvgpu_err(g, "cmd ack receive failed unit_id:0x%x", unit_id);
	}

	nvgpu_mutex_release(&g->gsp->gsp_ipc->cmd_lock);
exit:
	return err;
}

int nvgpu_acr_lock_wpr(struct gk20a *g, u32 wpr_address_lo, u32 wpr_address_hi)
{
	struct nv_flcn_cmd_gsp cmd = {};
	int err = 0;

	nvgpu_acr_dbg(g, " ");

	cmd.cmd.acr_cmd.lock_wpr.cmd_type = 0x1U;
	cmd.cmd.acr_cmd.lock_wpr.wpr_address_lo = wpr_address_lo;
	cmd.cmd.acr_cmd.lock_wpr.wpr_address_hi = wpr_address_hi;

	err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, 0xFU,
			sizeof(union nvgpu_gsp_acr_cmd));
	if (err == -ETIMEDOUT) {
		nvgpu_err(g, "GSP ACR Lock WPR cmd timeout");
		nvgpu_acr_report_error_to_sdl(g, ACR_ERROR_CMD_TIMEOUT, 0U);
	} else if (err != 0) {
		nvgpu_err(g, "sending control fifo queue to GSP failed");
	}

	if ((err == 0) && (g->acr->msg_status.error_code != 0)) {
		nvgpu_err(g, "ACR LOCK WPR FAILED!");
		nvgpu_err(g, "err_code: %d ",
				g->acr->msg_status.error_code);
		nvgpu_err(g, "err_info: %d ",
				g->acr->msg_status.error_info);
		nvgpu_err(g, "libnvriscv_err: %d ",
				g->acr->msg_status.lib_nvriscv_error);
		nvgpu_acr_report_error_to_sdl(g, g->acr->msg_status.error_code,
				g->acr->msg_status.error_info);
		err = (int)g->acr->msg_status.error_code;
	}
	return err;
}

int nvgpu_acr_bootstrap_engine(struct gk20a *g,
	u32 engine_id, u32 engine_instance, u32 engine_index_mask,
	u32 intr_ctrl_0, u32 intr_ctrl_1)
{
	struct nv_flcn_cmd_gsp cmd = {};
	int err = 0;

	nvgpu_acr_dbg(g, " ");

	cmd.cmd.acr_cmd.bootstrap_engine.cmd_type = 0x2U;
	cmd.cmd.acr_cmd.bootstrap_engine.engine_id = engine_id;
	cmd.cmd.acr_cmd.bootstrap_engine.engine_instance = engine_instance;
	cmd.cmd.acr_cmd.bootstrap_engine.engine_index_mask = engine_index_mask;

	cmd.cmd.acr_cmd.bootstrap_engine.intr_ctrl_0 = intr_ctrl_0;
	cmd.cmd.acr_cmd.bootstrap_engine.intr_ctrl_1 = intr_ctrl_1;

	cmd.cmd.acr_cmd.bootstrap_engine.intr_ctrl_uncorrected_err = SKIP_INTR_CTRL_WRITE;
	cmd.cmd.acr_cmd.bootstrap_engine.intr_ctrl_corrected_err = SKIP_INTR_CTRL_WRITE;

	if (engine_id == FALCON_ID_PMU_RISCV_EB) {
		cmd.cmd.acr_cmd.bootstrap_engine.boot_arg_lo =
					nvgpu_falcon_mailbox_read(g->pmu->flcn, FALCON_MAILBOX_0);
		cmd.cmd.acr_cmd.bootstrap_engine.boot_arg_hi =
					nvgpu_falcon_mailbox_read(g->pmu->flcn, FALCON_MAILBOX_1);
	}

	err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, 0xFU,
	sizeof(union nvgpu_gsp_acr_cmd));
	if (err == -ETIMEDOUT) {
		nvgpu_err(g, "GSP ACR Engine: %d boot cmd timeout", engine_id);
		nvgpu_acr_report_error_to_sdl(g, ACR_ERROR_CMD_TIMEOUT, 0U);
	} else if (err != 0) {
		nvgpu_err(g, "sending control fifo queue to GSP failed");
	}

	/* Received a response but the engine load failed in acr */
	if ((err == 0) && (g->acr->msg_status.error_code != 0)) {
		nvgpu_err(g, "ACR ENGINE BOOT FAILED for %d", engine_id);
		nvgpu_err(g, "err_code: %d ",
				g->acr->msg_status.error_code);
		nvgpu_err(g, "err_info: %d ",
				g->acr->msg_status.error_info);
		nvgpu_err(g, "libnvriscv_err: %d ",
				g->acr->msg_status.lib_nvriscv_error);
		nvgpu_acr_report_error_to_sdl(g, g->acr->msg_status.error_code,
				g->acr->msg_status.error_info);
		err = (int)g->acr->msg_status.error_code;
	}
	return err;
}
