/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef MULTIMEDIA_PRIV_H
#define MULTIMEDIA_PRIV_H

#define TU104_NVENC_UCODE_FW    "nvhost_nvenc072.fw"

struct multimedia_fw_hdr {
	/* 0x10de */
	u32 fw_magic;
	/* Versioning of firmware format */
	u32 fw_ver;
	/* Entire image size including this header */
	u32 fw_size;
	/* Header offset of executable firmware metadata */
	u32 header_offset;
	/* Start of executable firmware data */
	u32 data_offset;
	/* Size of executable firmware */
	u32 data_size;
	/* Reserved */
	u32 reserved[4];
};

s32 nvgpu_multimedia_copy_falcon_fw(struct gk20a *g, const char *fw_name, u32 *ucode_header,
				struct nvgpu_mem *ucode_mem_desc);
s32 nvgpu_multimedia_copy_nvriscv_fw(struct gk20a *g, const char *desc_fw_name,
				const char *image_fw_name, u32 *fw_desc,
				struct nvgpu_mem *fw_mem);
#endif /* MULTIMEDIA_PRIV_H */
