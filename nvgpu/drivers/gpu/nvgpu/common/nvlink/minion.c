// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/firmware.h>
#include <nvgpu/timers.h>
#include <nvgpu/nvlink_minion.h>

#ifdef CONFIG_NVGPU_NVLINK

/* Extract a WORD from the MINION ucode */
u32 nvgpu_nvlink_minion_extract_word(struct nvgpu_firmware *fw, u32 idx)
{
	u32 out_data = 0U;
	u8 byte = 0U;
	u32 i = 0U;

	for (i = 0U; i < 4U; i++) {
		byte = fw->data[idx + i];
		out_data |= ((u32)byte) << (8U * i);
	}

	return out_data;
}

/*
 * Load minion FW and set up bootstrap
 */
int nvgpu_nvlink_minion_load(struct gk20a *g)
{
	int err = 0;
	struct nvgpu_firmware *nvgpu_minion_fw = NULL;
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;
	bool boot_cmplte;

	nvgpu_log_fn(g, " ");

	if (g->ops.nvlink.minion.is_running(g)) {
		return 0;
	}

	/* Get minion ucode binary */
	if (g->ops.nvlink.minion.is_debug_mode != NULL) {
		if (g->ops.nvlink.minion.is_debug_mode(g)) {
			nvgpu_minion_fw = nvgpu_request_firmware(g,
							"dgpu_minion_debug.bin", 0);
		} else {
			nvgpu_minion_fw = nvgpu_request_firmware(g,
							"dgpu_minion_prod.bin", 0);
		}
	}

	if (nvgpu_minion_fw == NULL) {
		nvgpu_err(g, "minion ucode get fail");
		err = -ENOENT;
		goto exit;
	}

	/* Minion reset */
	err = nvgpu_falcon_reset(&g->minion_flcn);
	if (err != 0) {
		nvgpu_err(g, "Minion reset failed");
		goto exit;
	}

	/* Clear interrupts */
	g->ops.nvlink.minion.clear_intr(g);

	err = nvgpu_nvlink_minion_load_ucode(g, nvgpu_minion_fw);
	if (err != 0) {
		goto exit;
	}

	/* set BOOTVEC to start of non-secure code */
	err = nvgpu_falcon_bootstrap(&g->minion_flcn, 0x0);
	if (err != 0) {
		nvgpu_err(g, "Minion bootstrap failed");
		goto exit;
	}

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	do {
		err = g->ops.nvlink.minion.is_boot_complete(g, &boot_cmplte);
		if (err != 0) {
			goto exit;
		}
		if (boot_cmplte) {
			nvgpu_log(g, gpu_dbg_nvlink,"MINION boot successful");
			break;
		}
		nvgpu_usleep_range(delay, delay * 2U);
		delay = min_t(unsigned int,
				delay << 1, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired_msg(&timeout,
						"minion boot timeout") == 0);

	/* Service interrupts */
	g->ops.nvlink.minion.falcon_isr(g);

	if (nvgpu_timeout_peek_expired(&timeout)) {
		err = -ETIMEDOUT;
		goto exit;
	}

	g->ops.nvlink.minion.init_intr(g);

	nvgpu_release_firmware(g, nvgpu_minion_fw);

	return err;

exit:
	nvgpu_nvlink_free_minion_used_mem(g, nvgpu_minion_fw);
	return err;
}

#endif

