// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include <nvgpu/hw/ga10b/hw_gmmu_ga10b.h>

#include "mm_ga10b.h"

int ga10b_mm_bar2_vm_size(struct gk20a *g, u32 *buffer_size)
{
	u32 num_pbdma = nvgpu_get_litter_value(g, GPU_LIT_HOST_NUM_PBDMA);
	u32 fb_size;

	/* Calculate engine method buffer size */
	if (nvgpu_safe_mult_u32_return((9U + 1U + 3U), g->ops.ce.get_num_pce(g),
				buffer_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	if (nvgpu_safe_add_u32_return(*buffer_size, 2U, buffer_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	if (nvgpu_safe_mult_u32_return((27U * 5U), *buffer_size, buffer_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	if ((UINT_MAX - *buffer_size) < PAGE_SIZE) {
		nvgpu_err(g, "PAGE_ALIGN would overflow buffer");
		return -EOVERFLOW;
	}
	*buffer_size = PAGE_ALIGN(*buffer_size);

	nvgpu_log_info(g, "method buffer size in bytes %u", *buffer_size);

	if (nvgpu_safe_mult_u32_return(num_pbdma, *buffer_size, buffer_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	if (nvgpu_safe_mult_u32_return(*buffer_size, g->ops.channel.count(g), buffer_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	nvgpu_log_info(g, "method buffer size in bytes for max TSGs %u", *buffer_size);

	/*
	 * Calculate fault buffers size.
	 * Max entries take care of 1 entry used for full detection.
	 */
	if (nvgpu_safe_add_u32_return(g->ops.channel.count(g), 1U, &fb_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}
	if (nvgpu_safe_mult_u32_return(fb_size, gmmu_fault_buf_size_v(), &fb_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	/* Consider replayable and non replayable faults */
	if (nvgpu_safe_mult_u32_return(fb_size, 2U, &fb_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	nvgpu_log_info(g, "fault buffers size in bytes %u", fb_size);

	if (nvgpu_safe_add_u32_return(*buffer_size, fb_size, buffer_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	/* Add PAGE_SIZE for vab buffer size */
	if (nvgpu_safe_add_u32_return(*buffer_size, PAGE_SIZE, buffer_size) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}


	if ((UINT_MAX - *buffer_size) < PAGE_SIZE) {
		nvgpu_err(g, "PAGE_ALIGN would overflow buffer");
		return -EOVERFLOW;
	}
	*buffer_size = PAGE_ALIGN(*buffer_size);

	nvgpu_log_info(g, "bar2 vm size in bytes %u", *buffer_size);
	return 0;
}
