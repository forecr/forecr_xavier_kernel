// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pmu.h>
#include <nvgpu/log.h>
#include <nvgpu/sec2/sec2.h>
#include <nvgpu/sec2/queue.h>
#include <nvgpu/sec2/cmd.h>

/* command post operation functions */
static bool sec2_validate_cmd(struct nvgpu_sec2 *sec2,
	struct nv_flcn_cmd_sec2 *cmd, u32 queue_id)
{
	struct gk20a *g = sec2->g;
	u32 queue_size;

	if (queue_id != SEC2_NV_CMDQ_LOG_ID) {
		goto invalid_cmd;
	}

	if (cmd->hdr.size < PMU_CMD_HDR_SIZE) {
		goto invalid_cmd;
	}

	queue_size = nvgpu_sec2_queue_get_size(sec2->queues, queue_id);

	if (cmd->hdr.size > (queue_size >> 1)) {
		goto invalid_cmd;
	}

	if (!NV_SEC2_UNITID_IS_VALID(cmd->hdr.unit_id)) {
		goto invalid_cmd;
	}

	return true;

invalid_cmd:
	nvgpu_err(g, "invalid sec2 cmd :");
	nvgpu_err(g, "queue_id=%d, cmd_size=%d, cmd_unit_id=%d\n",
		queue_id, cmd->hdr.size, cmd->hdr.unit_id);

	return false;
}

static int sec2_write_cmd(struct nvgpu_sec2 *sec2,
	struct nv_flcn_cmd_sec2 *cmd, u32 queue_id,
	u32 timeout_ms)
{
	struct nvgpu_timeout timeout;
	struct gk20a *g = sec2->g;
	int err;

	nvgpu_log_fn(g, " ");

	nvgpu_timeout_init_cpu_timer(g, &timeout, timeout_ms);

	do {
		err = nvgpu_sec2_queue_push(sec2->queues, queue_id, &sec2->flcn,
					    cmd, cmd->hdr.size);
		if ((err == -EAGAIN) &&
		    (nvgpu_timeout_expired(&timeout) == 0)) {
			nvgpu_usleep_range(1000U, 2000U);
		} else {
			break;
		}
	} while (true);

	if (err != 0) {
		nvgpu_err(g, "fail to write cmd to queue %d", queue_id);
	}

	return err;
}

int nvgpu_sec2_cmd_post(struct gk20a *g, struct nv_flcn_cmd_sec2 *cmd,
	u32 queue_id, sec2_callback callback,
	void *cb_param, u32 timeout)
{
	struct nvgpu_sec2 *sec2 = &g->sec2;
	struct sec2_sequence *seq = NULL;
	int err = 0;

	if ((cmd == NULL) || (!sec2->sec2_ready)) {
		if (cmd == NULL) {
			nvgpu_warn(g,
				   "%s(): SEC2 cmd buffer is NULL", __func__);
		} else {
			nvgpu_warn(g, "%s(): SEC2 is not ready", __func__);
		}

		err = -EINVAL;
		goto exit;
	}

	/* Sanity check the command input. */
	if (!sec2_validate_cmd(sec2, cmd, queue_id)) {
		err = -EINVAL;
		goto exit;
	}

	/* Attempt to reserve a sequence for this command. */
	err = nvgpu_sec2_seq_acquire(g, &sec2->sequences, &seq,
				     callback, cb_param);
	if (err != 0) {
		goto exit;
	}

	/* Set the sequence number in the command header. */
	cmd->hdr.seq_id = nvgpu_sec2_seq_get_id(seq);

	cmd->hdr.ctrl_flags = 0U;
	cmd->hdr.ctrl_flags = PMU_CMD_FLAGS_STATUS;

	nvgpu_sec2_seq_set_state(seq, SEC2_SEQ_STATE_USED);

	err = sec2_write_cmd(sec2, cmd, queue_id, timeout);
	if (err != 0) {
		nvgpu_sec2_seq_set_state(seq, SEC2_SEQ_STATE_PENDING);
	}

exit:
	return err;
}
