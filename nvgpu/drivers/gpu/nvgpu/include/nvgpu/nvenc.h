/*
 * Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_NVENC_H
#define NVGPU_NVENC_H

#include <nvgpu/multimedia.h>

/* NVENC core descriptor */
struct nvgpu_nvenc {

	/* NVENC ucode */
	const char *fw_name;

	/* NVENC ucode header info */
	u32 ucode_header[MULTIMEDIA_UCODE_HEADER_SIZE];

	/* Falcon used to execute NVENC ucode */
	struct nvgpu_falcon *nvenc_flcn;

	/** Memory to store ucode contents locally. */
	struct nvgpu_mem nvenc_mem_desc;

};

struct gk20a;

int nvgpu_nvenc_falcon_boot(struct gk20a *g);
int nvgpu_nvenc_sw_init(struct gk20a *g);
int nvgpu_nvenc_sw_deinit(struct gk20a *g);
int nvgpu_nvenc_reset(struct gk20a *g);

#endif
