/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_RUNLIST
#define NVGPU_GSP_RUNLIST

struct nv_flcn_cmd_gsp;
struct gk20a;
struct nvgpu_gsp_device_info {
	/*
	 * Device index
	 */
	u8 device_id;
	/*
	 * TRUE when the device is a Host-driven method engine. FALSE otherwise.
	 */
	bool is_engine;
	/*
	 * The device's DEV_RUNLIST_PRI_BASE is the offset into BAR0 for the device's
	 * NV_RUNLIST PRI space.
	 */
	u32 runlist_pri_base;
	/*
	 * Engine description, like graphics, or copy engine.
	 */
	u32 engine_type;
	/*
	 * The unique per-device ID that host uses to identify any given engine.
	 */
	u32 engine_id;
	/*
	 * Specifies instance of a device, allowing SW to distinguish between
	 * multiple copies of a device present on the chip.
	 */
	u32 instance_id;
	/*
	 * Device's runlist-based engine ID.
	 */
	u32 rl_engine_id;
	/*
	 * The device's DEV_PRI_BASE is the offset into BAR0 for accessing the
	 * register space for the target device.
	 */
	u32 dev_pri_base;
};

struct nvgpu_gsp_domain_id {
	/*
	 * Domain id
	 */
	u32 domain_id;
};

/**
 * @brief Sends a command to the GSP and waits for an acknowledgment.
 *
 * The function posts a command to the GSP command queue and waits for the GSP to
 * acknowledge that it has received and processed the command. It ensures that the
 * command size does not exceed the maximum allowed size and sets the command header
 * size and unit ID before posting. If the command post is successful, it waits for
 * a condition that signifies the command acknowledgment. If any step fails, it
 * reports the error and exits.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the total size of the command by adding the header size to the
 *    provided size.
 * -# Assert that the total size does not exceed the maximum value that can be
 *    represented by an 8-bit unsigned integer.
 * -# Set the size and unit ID in the command header.
 * -# Post the command to the GSP command queue using nvgpu_gsp_cmd_post() and
 *    pass a callback function to handle the command acknowledgment.
 * -# If the command post is successful, wait for the acknowledgment using
 *    nvgpu_gsp_wait_message_cond().
 * -# If the acknowledgment is not received within the timeout, report an error.
 * -# Return the error status of the command post and acknowledgment wait operations.
 *
 * @Param [in] g         Pointer to the GPU driver struct.
 * @Param [in] cmd       Pointer to the command structure to be sent to the GSP.
 * @Param [in] unit_id   Unit ID of the command, which identifies the target unit
 *                       within the GSP. Range: [0, NV_GSP_UNIT_END)
 * @Param [in] size      Size of the command data, excluding the header.
 *                       Range: [0, U8_MAX - GSP_CMD_HDR_SIZE]
 *
 * @return 0 if the command was posted and acknowledged successfully.
 * @return Non-zero error code if the command post failed or if the acknowledgment
 *         was not received.
 */
int gsp_send_cmd_and_wait_for_ack(struct gk20a *g,
		struct nv_flcn_cmd_gsp *cmd, u32 unit_id, u32 size);
#endif // NVGPU_GSP_RUNLIST
