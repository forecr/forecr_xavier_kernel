/*
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gsp.h>
#include <nvgpu/io.h>
#include <nvgpu/device.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/runlist.h>
#include <nvgpu/string.h>

#include "ipc/gsp_cmd.h"
#include "ipc/gsp_msg.h"
#include "gsp_runlist.h"

static void gsp_handle_cmd_ack(struct gk20a *g, struct nv_flcn_msg_gsp *msg,
	void *param, u32 status)
{
	bool *command_ack = param;

	nvgpu_log_fn(g, " ");

	switch (msg->hdr.unit_id) {
	case NV_GSP_UNIT_NULL:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_NULL");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_DOMAIN_SUBMIT:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DOMAIN_SUBMIT");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_DEVICES_INFO:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DEVICES_INFO");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_DOMAIN_ADD:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DOMAIN_ADD");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_DOMAIN_DELETE:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DOMAIN_DELETE");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_DOMAIN_UPDATE:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DOMAIN_UPDATE");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_RUNLIST_UPDATE:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_RUNLIST_UPDATE");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_START_SCHEDULER:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_START_SCHEDULER");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_STOP_SCHEDULER:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_STOP_SCHEDULER");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_QUERY_NO_OF_DOMAINS:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_QUERY_NO_OF_DOMAINS");
		g->gsp_sched->no_of_domains = msg->msg.no_of_domains.no_of_domains;
		*command_ack = true;
		break;
	case NV_GSP_UNIT_QUERY_ACTIVE_DOMAIN:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_QUERY_ACTIVE_DOMAIN");
		g->gsp_sched->active_domain = msg->msg.active_domain.active_domain;
		*command_ack = true;
		break;
	case NV_GSP_UNIT_CONTROL_INFO_SEND:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_CONTROL_INFO_SEND");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_BIND_CTX_REG:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_BIND_CTX_REG");
		*command_ack = true;
		break;
	case NV_GSP_UNIT_CONTROL_FIFO_ERASE:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_CONTROL_FIFO_ERASE");
		*command_ack = true;
		break;
	default:
		nvgpu_err(g, "Un-handled response from GSP");
		*command_ack = false;
		break;
	}

	(void)status;
}

int gsp_send_cmd_and_wait_for_ack(struct gk20a *g,
		struct nv_flcn_cmd_gsp *cmd, u32 unit_id, u32 size)
{
	bool command_ack = false;
	int err = 0;
	size_t tmp_size;

	nvgpu_gsp_dbg(g, " ");

	tmp_size = GSP_CMD_HDR_SIZE + size;
	nvgpu_assert(tmp_size <= U64(U8_MAX));
	cmd->hdr.size = (u8)tmp_size;
	cmd->hdr.unit_id = (u8)unit_id;

	err = nvgpu_gsp_cmd_post(g, cmd, GSP_NV_CMDQ_LOG_ID,
			gsp_handle_cmd_ack, &command_ack, U32_MAX);
	if (err != 0) {
		nvgpu_err(g, "cmd post failed unit_id:0x%x", unit_id);
		goto exit;
	}

	err = nvgpu_gsp_wait_message_cond(g, nvgpu_get_poll_timeout(g),
			&command_ack, U8(true));
	if (err != 0) {
		nvgpu_err(g, "cmd ack receive failed unit_id:0x%x", unit_id);
	}

exit:
	return err;
}

static void gsp_get_device_info(struct gk20a *g, struct nvgpu_gsp_device_info *dev_info,
		const struct nvgpu_device *device)
{
	/* copy domain info into cmd buffer */
	dev_info->device_id = nvgpu_safe_cast_u32_to_u8(device->engine_id);
	dev_info->is_engine = true;
	dev_info->engine_type = device->type;
	dev_info->engine_id = device->engine_id;
	dev_info->instance_id = device->inst_id;
	dev_info->rl_engine_id = device->rleng_id;
	dev_info->dev_pri_base = device->pri_base;
	dev_info->runlist_pri_base = device->rl_pri_base;

	(void)g;
}

static int gsp_sched_send_devices_info(struct gk20a *g,
		 const struct nvgpu_device *device)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	/* copy domain info into cmd buffer */
	gsp_get_device_info(g, &cmd.cmd.device, device);

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_DEVICES_INFO, sizeof(struct nvgpu_gsp_device_info));

	return err;
}

static int gsp_sched_send_grs_dev_info(struct gk20a *g)
{
	const struct nvgpu_device *gr_dev = NULL;
	u32 num_grs;
	int err = 0;
	u8 engine_instance = 0;

	num_grs = nvgpu_device_count(g, NVGPU_DEVTYPE_GRAPHICS);
	if (num_grs == 0) {
		nvgpu_err(g, "GRs not supported");
		err = -EINVAL;
		goto exit;
	}

	for (engine_instance = 0; engine_instance < num_grs; engine_instance++) {
		gr_dev = nvgpu_device_get(g, NVGPU_DEVTYPE_GRAPHICS, engine_instance);
		if (gr_dev == NULL) {
			err = -ENXIO;
			nvgpu_err(g, " Get GR device info failed ID: %d", engine_instance);
			goto exit;
		}

		err = gsp_sched_send_devices_info(g, gr_dev);
		if (err != 0) {
			nvgpu_err(g, "Sending GR engine info failed ID: %d", engine_instance);
			goto exit;
		}
	}

exit:
	return err;
}

static int gsp_sched_send_ces_dev_info(struct gk20a *g)
{
	struct nvgpu_device ce_dev = { };
	const struct nvgpu_device *lces[NVGPU_MIG_MAX_ENGINES] = { };
	int err = 0;
	u32 num_lce;
	u8 engine_instance = 0;

	num_lce = nvgpu_device_get_async_copies(g, lces, NVGPU_MIG_MAX_ENGINES);
	if (num_lce == 0) {
		nvgpu_err(g, "Async CEs not supported");
		err = -EINVAL;
		goto exit;
	}

	for (engine_instance = 0; engine_instance < num_lce; engine_instance++) {
		ce_dev = *lces[engine_instance];
		err = gsp_sched_send_devices_info(g, &ce_dev);
		if (err != 0) {
			nvgpu_err(g, "Sending Async engin info failed ID: %d", engine_instance);
			goto exit;
		}
	}

exit:
	return err;
}

int nvgpu_gsp_sched_send_devices_info(struct gk20a *g)
{
	int err = 0;

	err = gsp_sched_send_grs_dev_info(g);
	if (err != 0) {
		nvgpu_err(g, "sending grs dev info failed");
		goto exit;
	}

	err = gsp_sched_send_ces_dev_info(g);
	if (err != 0) {
		nvgpu_err(g, "sending ces dev info failed");
	}

exit:
	return err;
}

int nvgpu_gsp_sched_domain_add(struct gk20a *g,
		struct nvgpu_gsp_domain_info *gsp_dom)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	/* copy domain info into cmd buffer */
	cmd.cmd.domain = *gsp_dom;

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_DOMAIN_ADD, sizeof(struct nvgpu_gsp_domain_info));

	return err;
}

int nvgpu_gsp_sched_domain_update(struct gk20a *g,
		struct nvgpu_gsp_domain_info *gsp_dom)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	/* copy domain info into cmd buffer */
	cmd.cmd.domain = *gsp_dom;

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_DOMAIN_UPDATE, sizeof(struct nvgpu_gsp_domain_info));

	return err;
}

int nvgpu_gsp_sched_domain_delete(struct gk20a *g, u32 domain_id)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	/* copy domain_id to cmd buffer */
	cmd.cmd.domain_id.domain_id = domain_id;

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_DOMAIN_DELETE, sizeof(struct nvgpu_gsp_domain_id));

	return err;
}

int nvgpu_gsp_sched_domain_submit(struct gk20a *g, u32 domain_id)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	/* copy domain_id to cmd buffer */
	cmd.cmd.domain_id.domain_id = domain_id;

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_DOMAIN_SUBMIT, sizeof(struct nvgpu_gsp_domain_id));

	return err;
}

int nvgpu_gsp_sched_runlist_update(struct gk20a *g,
		struct nvgpu_gsp_runlist_info *gsp_rl)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	/* copy domain info into cmd buffer */
	cmd.cmd.runlist = *gsp_rl;

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_RUNLIST_UPDATE, sizeof(struct nvgpu_gsp_runlist_info));

	return err;
}

int nvgpu_gsp_sched_query_no_of_domains(struct gk20a *g, u32 *no_of_domains)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_QUERY_NO_OF_DOMAINS, 0);
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
		return err;
	}

	*no_of_domains = g->gsp_sched->no_of_domains;
	return err;
}

int nvgpu_gsp_sched_query_active_domain(struct gk20a *g, u32 *active_domain)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_QUERY_ACTIVE_DOMAIN, 0);
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
		return err;
	}

	*active_domain = g->gsp_sched->active_domain;
	return err;
}

int nvgpu_gsp_sched_start(struct gk20a *g)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_START_SCHEDULER, 0);
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
	}

	return err;
}

int nvgpu_gsp_sched_stop(struct gk20a *g)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_STOP_SCHEDULER, 0);

	return err;
}
