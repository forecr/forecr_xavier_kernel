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

#include <nvgpu/types.h>
#include <nvgpu/dma.h>
#include <nvgpu/timers.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/firmware.h>
#include <nvgpu/falcon.h>
#include <nvgpu/nvenc.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/soc.h>
#include <nvgpu/io.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/string.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/tsg.h>
#include <nvgpu/channel.h>
#include <nvgpu/device.h>
#include "multimedia_priv.h"

/* Engine ctx buffer size. 256-byte aligned */
#define MULTIMEDIA_CTX_BUF_SIZE		(4096U)

static u32 nvgpu_multimedia_get_id(struct gk20a *g, u32 class)
{
	u32 multimedia_id = NVGPU_MULTIMEDIA_ENGINE_MAX;

	if (g->ops.gpu_class.is_valid_nvenc != NULL) {
		if (g->ops.gpu_class.is_valid_nvenc(class)) {
			multimedia_id = NVGPU_MULTIMEDIA_ENGINE_NVENC;
		}
	}

	return multimedia_id;
}

int nvgpu_multimedia_setup_ctx(struct nvgpu_channel *ch, u32 class_num, u32 flags)
{
	struct gk20a *g = ch->g;
	struct nvgpu_tsg *tsg = NULL;
	struct nvgpu_mem *ctx_mem;
	u32 multimedia_id;
	int err = 0;
	(void) flags;

	/* An address space has already been bound by this point.*/
	if (!nvgpu_channel_as_bound(ch)) {
		nvgpu_err(g, "not bound to an address space by the time of"
				" eng ctx allocation");
		return -EINVAL;
	}

	if ((g->ops.gpu_class.is_valid_multimedia == NULL) ||
		!g->ops.gpu_class.is_valid_multimedia(class_num)) {
		return -EINVAL;
	}

	ch->obj_class = class_num;
	tsg = nvgpu_tsg_from_ch(ch);
	if (tsg == NULL) {
		return -EINVAL;
	}

	nvgpu_mutex_acquire(&tsg->eng_ctx_lock);

	g->ops.tsg.disable(tsg);

	err = nvgpu_tsg_preempt(g, tsg);
	if (err != 0) {
		nvgpu_err(g, "tsg preempt failed %d", err);
		goto enable_tsg;
	}

	err = nvgpu_tsg_validate_class_veid_pbdma(ch);
	if (err != 0) {
		nvgpu_err(g, "Invalid class/veid/pbdma config");
		goto enable_tsg;
	}

	multimedia_id = nvgpu_multimedia_get_id(g, ch->obj_class);
	if (multimedia_id >= NVGPU_MULTIMEDIA_ENGINE_MAX) {
		nvgpu_err(g, "Invalid multimedia id %d", multimedia_id);
		err = -EINVAL;
		goto enable_tsg;
	}

	if (tsg->eng_ctx[multimedia_id] == NULL) {
		/*
		 * Engine ctx buffer has already been allocated for this engine
		 * in this TSG.
		 */
		goto enable_tsg;
	}

	tsg->eng_ctx[multimedia_id] = nvgpu_kzalloc(g, sizeof(struct nvgpu_multimedia_ctx));

	tsg->eng_ctx[multimedia_id]->tsgid = tsg->tsgid;

	ctx_mem = &tsg->eng_ctx[multimedia_id]->ctx_mem;
	err = nvgpu_dma_alloc_sys(g, MULTIMEDIA_CTX_BUF_SIZE, ctx_mem);
	if (err != 0) {
		nvgpu_err(g, "failed to allocate a ctx buffer for engine %d", multimedia_id);
		goto free_eng_ctx;
	}

	ctx_mem->gpu_va = nvgpu_gmmu_map(ch->vm, ctx_mem,
					0, /* not GPU-cacheable */
					gk20a_mem_flag_none, false,
					ctx_mem->aperture);
	if (ctx_mem->gpu_va == 0ULL) {
		nvgpu_err(g, "failed to map ctx buffer");
		err = -ENOMEM;
		goto free_ctx_mem;
	}

	/* Clear the context memory */
	nvgpu_memset(g, ctx_mem, 0U, 0U, MULTIMEDIA_CTX_BUF_SIZE);

	/* Set the context pointer in the inst block */
	g->ops.ramin.set_gr_ptr(g, &ch->inst_block, ctx_mem->gpu_va);

	/* Engine ctx setup is fine. Return 0 */
	goto enable_tsg;

free_ctx_mem:
	nvgpu_dma_free(g, ctx_mem);

free_eng_ctx:
	nvgpu_kfree(g, tsg->eng_ctx[multimedia_id]);
	tsg->eng_ctx[multimedia_id] = NULL;

enable_tsg:
	g->ops.tsg.enable(tsg);
	nvgpu_mutex_release(&tsg->eng_ctx_lock);

	return err;
}

void nvgpu_multimedia_free_ctx(struct gk20a *g, struct nvgpu_multimedia_ctx *eng_ctx)
{
	struct nvgpu_tsg *tsg;
	struct nvgpu_mem *ctx_mem;

	if (eng_ctx != NULL) {
		ctx_mem = &eng_ctx->ctx_mem;
		tsg = nvgpu_tsg_get_from_id(g, eng_ctx->tsgid);

		nvgpu_mutex_acquire(&tsg->eng_ctx_lock);
		if (nvgpu_mem_is_valid(ctx_mem)) {
			nvgpu_dma_free(g, ctx_mem);
		}
		nvgpu_kfree(g, eng_ctx);
		nvgpu_mutex_release(&tsg->eng_ctx_lock);
	}
}

void nvgpu_multimedia_free_all_ctx(struct nvgpu_tsg *tsg)
{
	s32 eng;
	struct nvgpu_multimedia_ctx *eng_ctx;
	struct gk20a *g = tsg->g;

	nvgpu_mutex_acquire(&tsg->eng_ctx_lock);

	for (eng = NVGPU_MULTIMEDIA_ENGINE_NVENC; eng < NVGPU_MULTIMEDIA_ENGINE_MAX; eng++) {
		eng_ctx = tsg->eng_ctx[eng];
		if (eng_ctx != NULL) {
			if (nvgpu_mem_is_valid(&eng_ctx->ctx_mem)) {
				nvgpu_dma_free(g, &eng_ctx->ctx_mem);
			}
			nvgpu_kfree(g, eng_ctx);
			tsg->eng_ctx[eng] = NULL;
		}
	}

	nvgpu_mutex_release(&tsg->eng_ctx_lock);
}

int nvgpu_multimedia_copy_fw(struct gk20a *g, const char *fw_name, u32 *ucode_header,
				struct nvgpu_mem *ucode_mem_desc)
{
	struct nvgpu_firmware *multimedia_fw;
	struct multimedia_fw_hdr *fw_hdr = NULL;
	void *ucode = NULL;
	int err = 0;

	nvgpu_log(g, gpu_dbg_mme, "Loading the firmware: %s", fw_name);

	multimedia_fw = nvgpu_request_firmware(g, fw_name,
						NVGPU_REQUEST_FIRMWARE_NO_WARN);
	if (multimedia_fw == NULL) {
		nvgpu_err(g, "Failed to read firmware: %s", fw_name);
		return -ENOENT;
	}

	fw_hdr = (struct multimedia_fw_hdr *) multimedia_fw->data;
	nvgpu_memcpy((u8 *)ucode_header, multimedia_fw->data + fw_hdr->header_offset,
			MULTIMEDIA_UCODE_HEADER_SIZE_BYTES);
	ucode = multimedia_fw->data + fw_hdr->data_offset;

	nvgpu_log(g, gpu_dbg_mme, "firmware header: magic= 0x%x ver= 0x%x size= 0x%x",
			fw_hdr->fw_magic,
			fw_hdr->fw_ver,
			fw_hdr->fw_size);
	nvgpu_log(g, gpu_dbg_mme, "firmware header: ucode header offset= 0x%x, "
					"data (offset,size): 0x%x 0x%x",
			fw_hdr->header_offset,
			fw_hdr->data_offset,
			fw_hdr->data_size);
	nvgpu_log(g, gpu_dbg_mme, "ucode header: code (offset,size): 0x%x, 0x%x",
			ucode_header[OS_CODE_OFFSET],
			ucode_header[OS_CODE_SIZE]);
	nvgpu_log(g, gpu_dbg_mme, "ucode header: data (offset,size): 0x%x, 0x%x",
			ucode_header[OS_DATA_OFFSET],
			ucode_header[OS_DATA_SIZE]);

	/* Alloc mem space for holding the ucode, that will be DMA'ed to the engine */
	if (!nvgpu_mem_is_valid(ucode_mem_desc)) {
		err = nvgpu_dma_alloc_flags_sys(g,
			NVGPU_DMA_PHYSICALLY_ADDRESSED,
			fw_hdr->data_size,
			ucode_mem_desc);
		if (err != 0) {
			nvgpu_err(g, "sys mem alloc for ucode failed");
			goto free_ucode;
		}
	}

	/* Clear the memory and then copy the ucode */
	nvgpu_memset(g, ucode_mem_desc, 0U, 0U, fw_hdr->data_size);
	nvgpu_mem_wr_n(g, ucode_mem_desc, 0U, ucode, fw_hdr->data_size);

free_ucode:
	nvgpu_release_firmware(g, multimedia_fw);
	return err;
}

bool nvgpu_multimedia_get_devtype(s32 multimedia_id, u32 *dev_type, u32 *instance)
{
	bool isValid = true;

	switch (multimedia_id) {

	case NVGPU_MULTIMEDIA_ENGINE_NVENC:
		*dev_type = NVGPU_DEVTYPE_NVENC;
		*instance = 0;
		break;

	case NVGPU_MULTIMEDIA_ENGINE_OFA:
		*dev_type = NVGPU_DEVTYPE_OFA;
		*instance = 0;
		break;

	case NVGPU_MULTIMEDIA_ENGINE_NVDEC:
		*dev_type = NVGPU_DEVTYPE_NVDEC;
		*instance = 0;
		break;

	case NVGPU_MULTIMEDIA_ENGINE_NVJPG:
		*dev_type = NVGPU_DEVTYPE_NVJPG;
		*instance = 0;
		break;

	default:
		isValid = false;
		break;
	}
	return isValid;
}
