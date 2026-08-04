// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/firmware.h>
#include <nvgpu/falcon.h>
#include <nvgpu/enabled.h>
#include <nvgpu/sbr.h>

#include "sbr.h"

static void pub_ucode_patch_sig(struct gk20a *g,
	unsigned int *p_img, unsigned int *p_prod_sig,
	unsigned int *p_dbg_sig, unsigned int *p_patch_loc,
	unsigned int *p_patch_ind, u32 sig_size)
{
	unsigned int i, j, *p_sig;
	nvgpu_info(g, " ");

	if (!g->ops.pmu.is_debug_mode_enabled(g)) {
		p_sig = p_prod_sig;
		nvgpu_info(g, "PRODUCTION MODE\n");
	} else {
		p_sig = p_dbg_sig;
		nvgpu_info(g, "DEBUG MODE\n");
	}

	/* Patching logic:*/
	sig_size = sig_size / 4U;
	for (i = 0U; i < (sizeof(*p_patch_loc)>>2U); i++) {
		for (j = 0U; j < sig_size; j++) {
			p_img[nvgpu_safe_add_u32((p_patch_loc[i]>>2U), j)] =
			p_sig[nvgpu_safe_add_u32((p_patch_ind[i]<<2U), j)];
		}
	}
}

int nvgpu_sbr_pub_load_and_execute(struct gk20a *g)
{
	struct nvgpu_firmware *pub_fw = NULL;
	struct pub_bin_hdr *hs_bin_hdr = NULL;
	struct pub_fw_header *fw_hdr = NULL;
	u32 *ucode_header = NULL;
	u32 *ucode = NULL;
	u32 data = 0;
	int err = 0;

	nvgpu_log_fn(g, " ");

	if (!g->ops.pmu.is_debug_mode_enabled(g)) {
		pub_fw = nvgpu_request_firmware(g, PUB_PROD_BIN,
				NVGPU_REQUEST_FIRMWARE_NO_SOC);
	} else {
		pub_fw = nvgpu_request_firmware(g, PUB_DBG_BIN,
				NVGPU_REQUEST_FIRMWARE_NO_SOC);
	}

	if (pub_fw == NULL) {
		nvgpu_err(g, "pub ucode get fail");
		err = -ENOENT;
		goto exit;
	}

	hs_bin_hdr = (struct pub_bin_hdr *)(void *)pub_fw->data;
	fw_hdr = (struct pub_fw_header *)(void *)(pub_fw->data +
			hs_bin_hdr->header_offset);
	ucode_header = (u32 *)(void *)(pub_fw->data +
					fw_hdr->hdr_offset);
	ucode = (u32 *)(void *)(pub_fw->data + hs_bin_hdr->data_offset);

	/* Patch Ucode signatures */
	pub_ucode_patch_sig(g, ucode,
		(u32 *)(void *)(pub_fw->data + fw_hdr->sig_prod_offset),
		(u32 *)(void *)(pub_fw->data + fw_hdr->sig_dbg_offset),
		(u32 *)(void *)(pub_fw->data + fw_hdr->patch_loc),
		(u32 *)(void *)(pub_fw->data + fw_hdr->patch_sig),
		fw_hdr->sig_dbg_size);

	err = nvgpu_falcon_hs_ucode_load_bootstrap(&g->sec2.flcn, ucode,
			ucode_header);
	if (err != 0) {
		nvgpu_err(g, "pub ucode load & bootstrap failed");
		goto exit;
	}

	if (nvgpu_falcon_wait_for_halt(&g->sec2.flcn, PUB_TIMEOUT) != 0) {
		nvgpu_err(g, "pub ucode boot timed out");
		err = -ETIMEDOUT;
		goto exit;
	}

	data = nvgpu_falcon_mailbox_read(&g->sec2.flcn, FALCON_MAILBOX_0);
	if (data != 0U) {
		nvgpu_err(g, "pub ucode boot failed, err %x", data);
		err = -EAGAIN;
		goto exit;
	}

exit:
#ifdef CONFIG_NVGPU_FALCON_DEBUG
	if (err != 0) {
		nvgpu_falcon_dump_stats(&g->sec2.flcn);
	}
#endif

	if (pub_fw != NULL) {
		nvgpu_release_firmware(g, pub_fw);
	}

	nvgpu_log_fn(g, "pub loaded & executed with status %d", err);
	return err;
}

