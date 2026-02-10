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
int gsp_send_cmd_and_wait_for_ack(struct gk20a *g,
		struct nv_flcn_cmd_gsp *cmd, u32 unit_id, u32 size);
#endif // NVGPU_GSP_RUNLIST
