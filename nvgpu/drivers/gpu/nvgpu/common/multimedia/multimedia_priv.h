/*
 * Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#ifndef MULTIMEDIA_PRIV_H
#define MULTIMEDIA_PRIV_H

#define TU104_NVENC_UCODE_FW "nvhost_nvenc072.fw"

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

int nvgpu_multimedia_copy_fw(struct gk20a *g, const char *fw_name, u32 *ucode_header,
				struct nvgpu_mem *ucode_mem_desc);
#endif /* MULTIMEDIA_PRIV_H */
