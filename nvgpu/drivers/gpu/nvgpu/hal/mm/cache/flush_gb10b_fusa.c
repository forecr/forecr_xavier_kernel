// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/trace.h>
#include <nvgpu/mm.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/timers.h>
#include <nvgpu/utils.h>

#include <nvgpu/hw/gb10b/hw_xal_ep_gb10b.h>

#include "flush_gb10b.h"

void gb10b_mm_fb_flush_internal(struct gk20a *g, struct nvgpu_timeout *timeout)
{
	u32 trigger, poll, token_mask, start_token, complete_token;

	token_mask = nvgpu_safe_sub_u32(BIT32(xal_ep_uflush_fb_flush_token_s()), 1U);
	trigger = nvgpu_readl(g, xal_ep_uflush_fb_flush_r());
	start_token = xal_ep_uflush_fb_flush_token_v(trigger);

	do {
		poll = nvgpu_readl(g, xal_ep_fb_flush_completed_r());

		if (xal_ep_fb_flush_completed_status_v(poll) ==
				xal_ep_fb_flush_completed_status_idle_v()) {
			nvgpu_log(g, gpu_dbg_mm, "fb_flush "
					"trigger(0x%x) poll(0x%x)", trigger, poll);
			break;
		}

		complete_token = xal_ep_fb_flush_completed_token_v(poll);
		/* The loop will wait only when completedToken in the range of
		 * [startToken-NV_XAL_EP_MEMOP_MAX_OUTSTANDING, startToken].
		 */
		if (((start_token - complete_token) & token_mask) > NV_XAL_EP_MEMOP_TOKEN_MAX_ISSUED) {
			break;
		}

		nvgpu_udelay(5);
	} while (nvgpu_timeout_expired(timeout) == 0);

	if (nvgpu_timeout_peek_expired(timeout)) {
		nvgpu_warn(g, "fb_flush too many retries");
	}
}

int gb10b_mm_l2_flush_internal(struct gk20a *g, struct nvgpu_timeout *timeout)
{
	u32 trigger, poll, token_mask, start_token, complete_token, issued_token;
	int err = -ETIMEDOUT;

	if (nvgpu_safe_sub_u32_return(BIT32(xal_ep_uflush_l2_flush_dirty_token_s()), 1U, &token_mask) == false) {
		nvgpu_err(g, "buffer underflow");
		return -EOVERFLOW;
	}

	trigger = nvgpu_readl(g, xal_ep_uflush_l2_flush_dirty_r());
	start_token = xal_ep_uflush_l2_flush_dirty_token_v(trigger);

	do {
		poll = nvgpu_readl(g, xal_ep_l2_flush_dirty_completed_r());

		if (xal_ep_l2_flush_dirty_completed_status_v(poll) ==
				xal_ep_l2_flush_dirty_completed_status_idle_v()) {
			nvgpu_log(g, gpu_dbg_mm, "l2 "
					"trigger(0x%x) poll(0x%x)", trigger, poll);
			err = 0;
			break;
		}

		complete_token = xal_ep_l2_flush_dirty_completed_token_v(poll);
		if (nvgpu_safe_sub_u32_return(start_token, complete_token, &issued_token) == false) {
			nvgpu_err(g, "buffer underflow");
			return -EOVERFLOW;
		}
		if ((issued_token & token_mask) > NV_XAL_EP_MEMOP_TOKEN_MAX_ISSUED) {
			err = 0;
			break;
		}

		nvgpu_udelay(5);
	} while (nvgpu_timeout_expired(timeout) == 0);

	return err;
}

#ifdef CONFIG_NVGPU_COMPRESSION
void gb10b_mm_l2_clean_comptags(struct gk20a *g, struct nvgpu_timeout *timeout)
{
	u32 trigger, poll, token_mask, start_token, complete_token;

	token_mask = nvgpu_safe_sub_u32(BIT32(xal_ep_uflush_l2_clean_comptags_token_s()), 1U);
	trigger = nvgpu_readl(g, xal_ep_uflush_l2_clean_comptags_r());
	start_token = xal_ep_uflush_l2_clean_comptags_token_v(trigger);

	do {
		poll = nvgpu_readl(g, xal_ep_l2_clean_comptags_completed_r());

		if (xal_ep_l2_clean_comptags_completed_status_v(poll) ==
				xal_ep_l2_clean_comptags_completed_status_idle_v()) {
			nvgpu_log(g, gpu_dbg_mm, "l2 "
					"trigger(0x%x) poll(0x%x)", trigger, poll);
			break;
		}

		complete_token = xal_ep_l2_clean_comptags_completed_token_v(poll);
		/* The loop will wait only when completedToken in the range of
		 * [startToken-NV_XAL_EP_MEMOP_MAX_OUTSTANDING, startToken].
		 */
		if (((start_token - complete_token) & token_mask) > NV_XAL_EP_MEMOP_TOKEN_MAX_ISSUED) {
			break;
		}

		nvgpu_udelay(5);
	} while (nvgpu_timeout_expired(timeout) == 0);

}
#endif
