// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gsp.h>
#include <nvgpu/io.h>
#include <nvgpu/device.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/runlist.h>
#include <nvgpu/string.h>

#include "common/gsp/ipc/gsp_msg.h"
#include "gsp_runlist.h"

/**
 * @brief Handles the acknowledgment of commands sent to the GSP.
 *
 * This function processes the acknowledgment message received from the GSP
 * after a command has been executed. It sets a flag to indicate whether the
 * command was acknowledged successfully or not, and logs the response. It also
 * updates the scheduler's domain information based on the response.
 *
 * The steps performed by the function are as follows:
 * -# Cast the third parameter to a pointer to a boolean to represent the
 *    command acknowledgment flag.
 * -# Switch on the unit ID from the message header to determine the type of
 *    acknowledgment received.
 * -# For each case of the switch statement, log the corresponding acknowledgment
 *    message using nvgpu_gsp_dbg() and set the command acknowledgment flag to
 *    true.
 * -# For specific cases like #NV_GSP_UNIT_QUERY_NO_OF_DOMAINS and
 *    #NV_GSP_UNIT_QUERY_ACTIVE_DOMAIN, update the scheduler's domain information
 *    from the message data.
 * -# If the unit ID does not match any case, log an error message using
 *    nvgpu_err() and set the command acknowledgment flag to false.
 * -# The status parameter is unused.
 *
 * @Param [in]  g         Pointer to the GPU structure.
 * @Param [in]  msg       Pointer to the GSP message structure.
 * @Param [out] param     Pointer to the command acknowledgment flag.
 * @Param [in]  status    Status of the command execution (unused).
 *
 * @return None.
 */
static void gsp_handle_cmd_ack(struct gk20a *g, struct nv_flcn_msg_gsp *msg,
	void *param, u32 status)
{
	struct nvgpu_cmd_ack_status *ack = (struct nvgpu_cmd_ack_status *)param;

	nvgpu_log_fn(g, " ");

	ack->ack_status = msg->hdr.ctrl_flags;
	switch (msg->hdr.unit_id) {
	case NV_GSP_UNIT_NULL:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_NULL");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_DOMAIN_SUBMIT:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DOMAIN_SUBMIT");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_DEVICES_INFO:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DEVICES_INFO");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_DOMAIN_ADD:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DOMAIN_ADD");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_DOMAIN_DELETE:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DOMAIN_DELETE");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_DOMAIN_UPDATE:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_DOMAIN_UPDATE");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_RUNLIST_UPDATE:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_RUNLIST_UPDATE");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_START_SCHEDULER:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_START_SCHEDULER");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_STOP_SCHEDULER:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_STOP_SCHEDULER");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_QUERY_NO_OF_DOMAINS:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_QUERY_NO_OF_DOMAINS");
		g->gsp_sched->no_of_domains = msg->msg.no_of_domains.no_of_domains;
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_QUERY_ACTIVE_DOMAIN:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_QUERY_ACTIVE_DOMAIN");
		g->gsp_sched->active_domain = msg->msg.active_domain.active_domain;
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_CONTROL_INFO_SEND:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_CONTROL_INFO_SEND");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_BIND_CTX_REG:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_BIND_CTX_REG");
		ack->command_ack = true;
		break;
	case NV_GSP_UNIT_CONTROL_FIFO_ERASE:
		nvgpu_gsp_dbg(g, "Reply to NV_GSP_UNIT_CONTROL_FIFO_ERASE");
		ack->command_ack = true;
		break;
	default:
		nvgpu_err(g, "Un-handled response from GSP");
		ack->command_ack = false;
		break;
	}

	(void)status;
}

int gsp_send_cmd_and_wait_for_ack(struct gk20a *g,
		struct nv_flcn_cmd_gsp *cmd, u32 unit_id, u32 size)
{
	int err = 0;
	u32 tmp_size;
	struct nvgpu_cmd_ack_status ack = {0};

	nvgpu_gsp_dbg(g, " ");

	tmp_size = GSP_CMD_HDR_SIZE + size;
	nvgpu_assert(tmp_size <= U32(U8_MAX));
	cmd->hdr.size = (u8)tmp_size;
	cmd->hdr.unit_id = nvgpu_safe_cast_u32_to_u8(unit_id);

	err = nvgpu_gsp_cmd_post(g, cmd, GSP_NV_CMDQ_LOG_ID,
			gsp_handle_cmd_ack, &ack, U32_MAX);
	if (err != 0) {
		nvgpu_err(g, "cmd post failed unit_id:0x%x", unit_id);
		goto go_return;
	}

	err = nvgpu_gsp_wait_message_cond(g, nvgpu_get_poll_timeout(g),
			&ack.command_ack, U8(true));
	if (err != 0) {
		nvgpu_err(g, "cmd ack receive failed unit_id:0x%x", unit_id);
		goto go_return;
	}

	//check for cmd response status
	err = (int)ack.ack_status;
	if (err != 0) {
		nvgpu_err(g, "cmd status failed unit_id:0x%x, status:0x%x", unit_id, err);
	} else {
		nvgpu_gsp_dbg(g, "cmd status success unit_id:0x%x, status:0x%x", unit_id, err);
	}

go_return:
	return err;
}

/**
 * @brief Populates the GSP device information structure with data from the given
 *        device structure.
 *
 * The function extracts device-specific information from the provided device
 * structure and copies it into the GSP device information structure. It sets the
 * device as an engine and copies the engine type, engine ID, instance ID, runlist
 * engine ID, device's primary base address, and runlist's primary base address.
 *
 * The steps performed by the function are as follows:
 * -# Cast the engine ID from the device structure to an 8-bit unsigned integer and
 *    store it in the device information structure's device ID field using
 *    nvgpu_safe_cast_u32_to_u8().
 * -# Set the is_engine flag in the device information structure to true, indicating
 *    that the device is an engine.
 * -# Copy the engine type from the device structure to the device information
 *    structure.
 * -# Copy the engine ID from the device structure to the device information
 *    structure.
 * -# Copy the instance ID from the device structure to the device information
 *    structure.
 * -# Copy the runlist engine ID from the device structure to the device information
 *    structure.
 * -# Copy the device's primary base address from the device structure to the device
 *    information structure.
 * -# Copy the runlist's primary base address from the device structure to the device
 *    information structure.
 *
 * @Param [in] g         Pointer to the GPU driver struct. Not used in this function.
 * @Param [out] dev_info Pointer to the GSP device information structure where the
 *                       device data will be copied.
 * @Param [in] device    Pointer to the device structure containing the device data
 *                       to be copied.
 */
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

/**
 * @brief Sends device information to the GSP.
 *
 * This function prepares a command with the device information and sends it to the
 * GSP. It calls another function to populate the command structure with the device
 * information and then sends the command to the GSP, waiting for an acknowledgment.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure to zero.
 * -# Populate the command structure with the device information by calling
 *    gsp_get_device_info().
 * -# Send the command to the GSP and wait for an acknowledgment by calling
 *    gsp_send_cmd_and_wait_for_ack() with the command structure, the unit ID for
 *    device information, and the size of the device information structure.
 * -# If an error occurs while sending the command or waiting for the acknowledgment,
 *    report the error.
 * -# Return the error code from the command send and acknowledgment wait operation.
 *
 * @Param [in] g         Pointer to the GPU driver struct.
 * @Param [in] device    Pointer to the device structure containing the device data
 *                       to be sent to the GSP.
 *
 * @return 0 if the command was sent and acknowledged successfully.
 * @return Non-zero error code if the command send failed or if the acknowledgment
 *         was not received.
 */
static int gsp_sched_send_devices_info(struct gk20a *g,
		 const struct nvgpu_device *device)
{
	struct nv_flcn_cmd_gsp cmd = { };
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	/* copy domain info into cmd buffer */
	gsp_get_device_info(g, &cmd.cmd.device, device);

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd,
		NV_GSP_UNIT_DEVICES_INFO, (u32)sizeof(struct nvgpu_gsp_device_info));
		if (err != 0) {
		nvgpu_err(g, "send cmd failed");
	}

	return err;
}

/**
 * @brief Sends information of all graphics engines to the GSP scheduler.
 *
 * The function iterates over all graphics engine instances and sends their
 * information to the GSP scheduler. If there are no graphics engines or if
 * fetching the device information fails, it reports an error.
 *
 * The steps performed by the function are as follows:
 * -# Count the number of graphics engines present in the system using
 *    nvgpu_device_count().
 * -# If no graphics engines are found, return an error indicating graphics
 *    engines are not supported.
 * -# Iterate over each graphics engine instance:
 *    -## Retrieve the device information for the current graphics engine
 *       instance using nvgpu_device_get().
 *    -## If the device information cannot be retrieved, return an error
 *       indicating the failure to get the device information.
 *    -## Send the retrieved device information to the GSP scheduler using
 *       gsp_sched_send_devices_info().
 *    -## If sending the device information fails, return an error indicating
 *       the failure to send the graphics engine information.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 *
 * @return 0 if all graphics engine information is successfully sent.
 * @return -EINVAL if no graphics engines are supported.
 * @return -ENXIO if fetching the device information for any graphics engine
 *         instance fails.
 * @return Error code from gsp_sched_send_devices_info() if sending the device
 *         information fails for any graphics engine instance.
 */
static int gsp_sched_send_grs_dev_info(struct gk20a *g)
{
	const struct nvgpu_device *gr_dev = NULL;
	u32 num_grs;
	int err = 0;
	u8 engine_instance = 0;

	num_grs = nvgpu_device_count(g, NVGPU_DEVTYPE_GRAPHICS);
	if (num_grs == 0U) {
		nvgpu_err(g, "GRs not supported");
		err = -EINVAL;
		goto go_return;
	}

	for (engine_instance = 0; engine_instance < num_grs; engine_instance++) {
		gr_dev = nvgpu_device_get(g, NVGPU_DEVTYPE_GRAPHICS, engine_instance);
		if (gr_dev == NULL) {
			err = -ENXIO;
			nvgpu_err(g, " Get GR device info failed ID: %d", engine_instance);
			goto go_return;
		}

		err = gsp_sched_send_devices_info(g, gr_dev);
		if (err != 0) {
			nvgpu_err(g, "Sending GR engine info failed ID: %d", engine_instance);
			goto go_return;
		}
	}

go_return:
	return err;
}

/**
 * @brief Sends information of all asynchronous copy engines to the GSP scheduler.
 *
 * The function retrieves the list of asynchronous copy engines and sends their
 * information to the GSP scheduler. If there are no asynchronous copy engines
 * or if an error occurs while sending their information, it reports an error.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the list of asynchronous copy engines and their count using
 *    nvgpu_device_get_async_copies().
 * -# If no asynchronous copy engines are found, return an error indicating
 *    they are not supported.
 * -# Iterate over each asynchronous copy engine instance:
 *    -## Assign the device information of the current asynchronous copy engine
 *       instance to a local variable.
 *    -## Send the device information of the current asynchronous copy engine
 *       instance to the GSP scheduler using gsp_sched_send_devices_info().
 *    -## If sending the device information fails, return an error indicating
 *       the failure to send the asynchronous copy engine information.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 *
 * @return 0 if all asynchronous copy engine information is successfully sent.
 * @return -EINVAL if no asynchronous copy engines are supported.
 * @return Error code from gsp_sched_send_devices_info() if sending the device
 *         information fails for any asynchronous copy engine instance.
 */
static int gsp_sched_send_ces_dev_info(struct gk20a *g)
{
	struct nvgpu_device ce_dev = { };
	const struct nvgpu_device *lces[NVGPU_MIG_MAX_ENGINES] = { };
	int err = 0;
	u32 num_lce;
	u8 engine_instance = 0;

	num_lce = nvgpu_device_get_async_copies(g, lces, NVGPU_MIG_MAX_ENGINES);
	if (num_lce == 0U) {
		nvgpu_err(g, "Async CEs not supported");
		err = -EINVAL;
		goto go_return;
	}

	for (engine_instance = 0; engine_instance < num_lce; engine_instance++) {
		ce_dev = *lces[engine_instance];
		err = gsp_sched_send_devices_info(g, &ce_dev);
		if (err != 0) {
			nvgpu_err(g, "Sending Async engin info failed ID: %d", engine_instance);
			goto go_return;
		}
	}

go_return:
	return err;
}

int nvgpu_gsp_sched_send_devices_info(struct gk20a *g)
{
	int err = 0;

	err = gsp_sched_send_grs_dev_info(g);
	if (err != 0) {
		nvgpu_err(g, "sending grs dev info failed");
		goto go_return;
	}

	err = gsp_sched_send_ces_dev_info(g);
	if (err != 0) {
		nvgpu_err(g, "sending ces dev info failed");
	}

go_return:
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
		NV_GSP_UNIT_DOMAIN_ADD, (u32)sizeof(struct nvgpu_gsp_domain_info));
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
	}

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
		NV_GSP_UNIT_DOMAIN_UPDATE, (u32)sizeof(struct nvgpu_gsp_domain_info));
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
	}

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
		NV_GSP_UNIT_DOMAIN_DELETE, (u32)sizeof(struct nvgpu_gsp_domain_id));
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
	}

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
		NV_GSP_UNIT_DOMAIN_SUBMIT, (u32)sizeof(struct nvgpu_gsp_domain_id));
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
	}

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
		NV_GSP_UNIT_RUNLIST_UPDATE, (u32)sizeof(struct nvgpu_gsp_runlist_info));
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
	}

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
	if (err != 0) {
		nvgpu_err(g, "send cmd failed");
	}

	return err;
}
