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

#ifndef NVGPU_MULTIMEDIA_H
#define NVGPU_MULTIMEDIA_H

#include <nvgpu/nvgpu_mem.h>

#define UCODE_DMA_ID			(0x6)
#define MULTIMEDIA_UCODE_HEADER_SIZE		(APP_0_CODE_SIZE + 1)
#define MULTIMEDIA_UCODE_HEADER_SIZE_BYTES	(MULTIMEDIA_UCODE_HEADER_SIZE * 4)

struct nvgpu_tsg;
struct vm_gk20a;
struct nvgpu_channel;

/**
 * Multimedia engine enum types supported from driver.
 */
enum {
	/** NVENC engine enum */
	NVGPU_MULTIMEDIA_ENGINE_NVENC = 0U,
	/** OFA engine enum */
	NVGPU_MULTIMEDIA_ENGINE_OFA = 1U,
	/** NVDEC engine enum */
	NVGPU_MULTIMEDIA_ENGINE_NVDEC = 2U,
	/** NVJPG engine enum */
	NVGPU_MULTIMEDIA_ENGINE_NVJPG = 3U,
	/** Invalid engine enum */
	NVGPU_MULTIMEDIA_ENGINE_MAX = 4U
};

/**
 * Multimedia engine context data structure.
 */
struct nvgpu_multimedia_ctx {

	/** Memory to hold context buffer */
	struct nvgpu_mem ctx_mem;

	/** TSG associated with this context */
	struct nvgpu_tsg *tsg;

	/** TSG identifier corresponding to this context */
	u32 tsgid;

	/** Context buffer initialised? */
	bool ctx_initialized;

};

int nvgpu_multimedia_setup_ctx(struct nvgpu_channel *ch, u32 class_num, u32 flags);
void nvgpu_multimedia_free_ctx(struct gk20a *g, struct nvgpu_multimedia_ctx *eng_ctx);
void nvgpu_multimedia_free_all_ctx(struct nvgpu_tsg *tsg);
bool nvgpu_multimedia_get_devtype(s32 multimedia_id, u32 *dev_type, u32 *instance);
#endif
