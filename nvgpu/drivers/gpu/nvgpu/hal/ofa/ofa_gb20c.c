// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/firmware.h>
#include <nvgpu/acr.h>

#include "ofa_gb20c.h"
#include "hal/riscv/riscv_gb10b.h"

#include <nvgpu/hw/gb10b/hw_pofa_gb10b.h>

void nvgpu_gb20c_ofa_fw(struct gk20a *g,
			struct nvgpu_firmware **desc_fw,
			struct nvgpu_firmware **image_fw,
			struct nvgpu_firmware **pkc_sig)
{
	*desc_fw = nvgpu_request_firmware(g, GB20C_OFA_RISCV_EB_UCODE_DESC,
					NVGPU_REQUEST_FIRMWARE_NO_WARN);

	if (nvgpu_acr_is_lsb_v3_supported(g)) {
		*pkc_sig = nvgpu_request_firmware(g,
				GB20C_OFA_RISCV_EB_UCODE_LSB_PKC_SIG,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	} else {
		*pkc_sig = nvgpu_request_firmware(g,
				GB20C_OFA_RISCV_EB_UCODE_PKC_SIG,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	}

	if (g->ops.pmu.is_debug_mode_enabled(g)) {
		*image_fw = nvgpu_request_firmware(g,
				GB20C_OFA_RISCV_EB_UCODE_IMAGE,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	} else {
		*image_fw = nvgpu_request_firmware(g,
				GB20C_OFA_RISCV_EB_UCODE_PROD_IMAGE,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	}
}
