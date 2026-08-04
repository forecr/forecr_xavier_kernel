// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

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
#include <nvgpu/runlist.h>
#include "multimedia_priv.h"

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_multimedia.h>
#endif

/* Engine ctx buffer size. 256-byte aligned */
#define MULTIMEDIA_CTX_BUF_SIZE		(4096U)
#define MULTIMEDIA_ENG_NAME_MAX		(64U)

struct nvgpu_multimedia_eng_info {
	char eng_name[MULTIMEDIA_ENG_NAME_MAX];
	u32 dev_type;
	u32 instance;
};

static const struct nvgpu_multimedia_eng_info multimedia_engine_info[NVGPU_MULTIMEDIA_ENGINE_MAX] = {
	{ "nvenc0", NVGPU_DEVTYPE_NVENC, 0 },
	{ "nvenc1", NVGPU_DEVTYPE_NVENC, 1 },
	{ "ofa0",   NVGPU_DEVTYPE_OFA,   0 },
	{ "nvdec0", NVGPU_DEVTYPE_NVDEC, 0 },
	{ "nvdec1", NVGPU_DEVTYPE_NVDEC, 1 },
	{ "nvjpg0", NVGPU_DEVTYPE_NVJPG, 0 },
	{ "nvjpg1", NVGPU_DEVTYPE_NVJPG, 1 },
};

int nvgpu_multimedia_setup_ctx(struct nvgpu_channel *ch, u32 class_num, u32 flags)
{
	struct gk20a *g = ch->g;
	struct nvgpu_tsg *tsg = NULL;
	struct nvgpu_mem *ctx_mem;
	u32 multimedia_id;
	int err = 0;
	(void) flags;

	nvgpu_log(g, gpu_dbg_mme, " ");
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
	nvgpu_mutex_acquire(&tsg->ctx_init_lock);

	nvgpu_tsg_disable(tsg);

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

	multimedia_id = nvgpu_engine_multimedia_id_from_runlist(g, ch->runlist->id);
	if (multimedia_id >= NVGPU_MULTIMEDIA_ENGINE_MAX) {
		nvgpu_err(g, "Invalid multimedia id %d", multimedia_id);
		err = -EINVAL;
		goto enable_tsg;
	}

	if (tsg->eng_ctx[multimedia_id] != NULL) {
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

	nvgpu_log(g, gpu_dbg_mme, "context created for multimedia id %d class 0x%x",
				multimedia_id, class_num);

	/* Engine ctx setup is fine. Return 0 */
	goto enable_tsg;

free_ctx_mem:
	nvgpu_dma_free(g, ctx_mem);

free_eng_ctx:
	nvgpu_kfree(g, tsg->eng_ctx[multimedia_id]);
	tsg->eng_ctx[multimedia_id] = NULL;

enable_tsg:
	nvgpu_tsg_enable(tsg);
	nvgpu_mutex_release(&tsg->ctx_init_lock);
	nvgpu_mutex_release(&tsg->eng_ctx_lock);

	return err;
}

void nvgpu_multimedia_free_ctx(struct gk20a *g, struct nvgpu_multimedia_ctx *eng_ctx)
{
	struct nvgpu_tsg *tsg;
	struct nvgpu_mem *ctx_mem;

	nvgpu_log(g, gpu_dbg_mme, " ");
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
			nvgpu_log(g, gpu_dbg_mme, "context freed for multimedia id %d", eng);
		}
	}

	nvgpu_mutex_release(&tsg->eng_ctx_lock);
}

static s32 nvgpu_multimedia_alloc_fw_mem(struct gk20a *g, size_t size, struct nvgpu_mem *fw_mem)
{
	s32 err = 0;

	if (!nvgpu_mem_is_valid(fw_mem)) {
		err = nvgpu_dma_alloc_flags_sys(g, NVGPU_DMA_PHYSICALLY_ADDRESSED,
						size, fw_mem);
		if (err != 0) {
			nvgpu_err(g, "sys mem alloc for firmware failed");
		}
	}

	return err;
}

s32 nvgpu_multimedia_copy_falcon_fw(struct gk20a *g, const char *fw_name, u32 *ucode_header,
				struct nvgpu_mem *ucode_mem_desc)
{
	struct nvgpu_firmware *multimedia_fw;
	struct multimedia_fw_hdr *fw_hdr = NULL;
	void *ucode = NULL;
	s32 err = 0;

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
	err = nvgpu_multimedia_alloc_fw_mem(g, fw_hdr->data_size, ucode_mem_desc);
	if (err != 0) {
		goto free_ucode;
	}

	/* Clear the memory and then copy the ucode */
	nvgpu_memset(g, ucode_mem_desc, 0U, 0U, fw_hdr->data_size);
	nvgpu_mem_wr_n(g, ucode_mem_desc, 0U, ucode, fw_hdr->data_size);

free_ucode:
	nvgpu_release_firmware(g, multimedia_fw);
	return err;
}

s32 nvgpu_multimedia_copy_nvriscv_fw(struct gk20a *g, const char *desc_fw_name,
				const char *image_fw_name, u32 *fw_desc,
				struct nvgpu_mem *fw_mem)
{
	struct nvgpu_firmware *desc_fw, *image_fw;
	struct nvgpu_nvriscv_fw_desc *nvriscv_fw_desc = NULL;
	s32 err = 0;

	nvgpu_log(g, gpu_dbg_falcon | gpu_dbg_mme, "Loading firmware: desc %s image %s",
			desc_fw_name, image_fw_name);

	desc_fw = nvgpu_request_firmware(g, desc_fw_name,
					NVGPU_REQUEST_FIRMWARE_NO_WARN);
	if (desc_fw == NULL) {
		nvgpu_err(g, "Failed to read firmware: %s", desc_fw_name);
		err = -ENOENT;
		goto done;
	}

	image_fw = nvgpu_request_firmware(g, image_fw_name,
					NVGPU_REQUEST_FIRMWARE_NO_WARN);
	if (image_fw == NULL) {
		nvgpu_err(g, "Failed to read firmware: %s", image_fw_name);
		err = -ENOENT;
		goto free_desc_fw;
	}

	nvriscv_fw_desc = (struct nvgpu_nvriscv_fw_desc *)(void *)desc_fw->data;
	fw_desc[NVGPU_NVRISCV_CODE_OFFSET] = nvriscv_fw_desc->monitorCodeOffset;
	fw_desc[NVGPU_NVRISCV_CODE_SIZE] = nvriscv_fw_desc->monitorCodeSize;
	fw_desc[NVGPU_NVRISCV_DATA_OFFSET] = nvriscv_fw_desc->monitorDataOffset;
	fw_desc[NVGPU_NVRISCV_DATA_SIZE] = nvriscv_fw_desc->monitorDataSize;
	fw_desc[NVGPU_NVRISCV_MANIFEST_OFFSET] = nvriscv_fw_desc->manifestOffset;
	fw_desc[NVGPU_NVRISCV_MANIFEST_SIZE] = nvriscv_fw_desc->manifestSize;

	nvgpu_log(g, gpu_dbg_falcon | gpu_dbg_mme,
			"firmware desc: ver 0x%x, bl offset 0x%x, bl size 0x%x",
			nvriscv_fw_desc->version,
			nvriscv_fw_desc->bootloaderOffset,
			nvriscv_fw_desc->bootloaderSize);
	nvgpu_log(g, gpu_dbg_falcon | gpu_dbg_mme,
			"firmware desc: code offset 0x%x, code size 0x%x",
			fw_desc[NVGPU_NVRISCV_CODE_OFFSET],
			fw_desc[NVGPU_NVRISCV_CODE_SIZE]);
	nvgpu_log(g, gpu_dbg_falcon | gpu_dbg_mme,
			"firmware desc: data offset 0x%x, data size 0x%x",
			fw_desc[NVGPU_NVRISCV_DATA_OFFSET],
			fw_desc[NVGPU_NVRISCV_DATA_SIZE]);
	nvgpu_log(g, gpu_dbg_falcon | gpu_dbg_mme,
			"firmware desc: manifest offset 0x%x, manifest size 0x%x",
			fw_desc[NVGPU_NVRISCV_MANIFEST_OFFSET],
			fw_desc[NVGPU_NVRISCV_MANIFEST_SIZE]);

	/* Alloc mem space for holding the ucode, that will be DMA'ed to the engine */
	err = nvgpu_multimedia_alloc_fw_mem(g, image_fw->size, fw_mem);
	if (err != 0) {
		goto free_fw;
	}

	/* Clear the memory and then copy the ucode */
	nvgpu_memset(g, fw_mem, 0U, 0U, image_fw->size);
	nvgpu_mem_wr_n(g, fw_mem, 0U, image_fw->data, image_fw->size);

free_fw:
	nvgpu_release_firmware(g, image_fw);
free_desc_fw:
	nvgpu_release_firmware(g, desc_fw);
done:
	return err;
}

const struct nvgpu_device *nvgpu_multimedia_get_dev(struct gk20a *g, u32 multimedia_id)
{
	const struct nvgpu_device *dev = NULL;

	if (multimedia_id < (u32)NVGPU_MULTIMEDIA_ENGINE_MAX) {
		dev = nvgpu_device_get(g,
				multimedia_engine_info[multimedia_id].dev_type,
				multimedia_engine_info[multimedia_id].instance);
	}

	return dev;
}

const char *nvgpu_multimedia_get_eng_name(u32 multimedia_id)
{
	const char *str = "Unknown_device";

	if (multimedia_id < NVGPU_MULTIMEDIA_ENGINE_MAX) {
		str = multimedia_engine_info[multimedia_id].eng_name;
	}

	return str;
}

s32 nvgpu_multimedia_sw_init(struct gk20a *g)
{
	s32 err = 0;

	if (nvgpu_device_count(g, NVGPU_DEVTYPE_NVENC) != 0U) {
		nvgpu_log(g, gpu_dbg_mme, "nvenc sw init");
		err = g->ops.nvenc.init(g);
		if (err != 0) {
			nvgpu_err(g, "nvenc sw init failed");
			return err;
		}
	}

	if (nvgpu_device_count(g, NVGPU_DEVTYPE_OFA) != 0U) {
		nvgpu_log(g, gpu_dbg_mme, "ofa sw init");
		err = g->ops.ofa.init(g);
		if (err != 0) {
			nvgpu_err(g, "ofa sw init failed");
			return err;
		}
	}

	if (nvgpu_device_count(g, NVGPU_DEVTYPE_NVDEC) != 0U) {
		nvgpu_log(g, gpu_dbg_mme, "nvdec sw init");
		err = g->ops.nvdec.init(g);
		if (err != 0) {
			nvgpu_err(g, "nvdec sw init failed");
			return err;
		}
	}

	if (nvgpu_device_count(g, NVGPU_DEVTYPE_NVJPG) != 0U) {
		nvgpu_log(g, gpu_dbg_mme, "nvjpg sw init");
		err = g->ops.nvjpg.init(g);
		if (err != 0) {
			nvgpu_err(g, "nvjpg sw init failed");
			return err;
		}
	}

	return 0;
}

s32 nvgpu_multimedia_bootstrap(struct gk20a *g)
{
	const struct nvgpu_device *dev;
	s32 err = 0;

	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_NVENC) {
		nvgpu_log(g, gpu_dbg_mme, "booting nvenc%d", dev->inst_id);
		err = g->ops.nvenc.bootstrap(g, dev->inst_id);
		if (err != 0) {
			nvgpu_err(g, "failed to boot nvenc%d", dev->inst_id);
			return err;
		}
	}

	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_OFA) {
		nvgpu_log(g, gpu_dbg_mme, "booting ofa%d", dev->inst_id);
		err = g->ops.ofa.bootstrap(g, dev->inst_id);
		if (err != 0) {
			nvgpu_err(g, "failed to boot ofa%d", dev->inst_id);
			return err;
		}
	}

	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_NVDEC) {
		nvgpu_log(g, gpu_dbg_mme, "booting nvdec%d", dev->inst_id);
		err = g->ops.nvdec.bootstrap(g, dev->inst_id);
		if (err != 0) {
			nvgpu_err(g, "failed to boot nvdec%d", dev->inst_id);
			return err;
		}
	}

	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_NVJPG) {
		nvgpu_log(g, gpu_dbg_mme, "booting nvjpg%d", dev->inst_id);
		err = g->ops.nvjpg.bootstrap(g, dev->inst_id);
		if (err != 0) {
			nvgpu_err(g, "failed to boot nvjpg%d", dev->inst_id);
			return err;
		}
	}

	return 0;
}

void nvgpu_multimedia_deinit(struct gk20a *g)
{

	if (g->nvenc != NULL) {
		(void)g->ops.nvenc.deinit(g);
	}

	if (g->ofa != NULL) {
		(void)g->ops.ofa.deinit(g);
	}

	if (g->nvdec != NULL) {
		(void)g->ops.nvdec.deinit(g);
	}

	if (g->nvjpg != NULL) {
		(void)g->ops.nvjpg.deinit(g);
	}
}

bool nvgpu_get_multimedia_fw_names(struct gk20a *g, u32 dev_type,
		const char **desc_fw_name, const char **image_fw_name)
{
	bool is_supported = true;
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);

	(void) dev_type;
	(void) desc_fw_name;
	(void) image_fw_name;

	switch (ver) {
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
		switch (dev_type) {
		case NVGPU_DEVTYPE_NVENC:
			*desc_fw_name = GB10B_NVENC_UCODE_DESC;
			*image_fw_name = GB10B_NVENC_UCODE_IMAGE;
			break;

		case NVGPU_DEVTYPE_OFA:
			*desc_fw_name = GB10B_OFA_UCODE_DESC;
			*image_fw_name = GB10B_OFA_UCODE_IMAGE;
			break;

		case NVGPU_DEVTYPE_NVDEC:
			*desc_fw_name = GB10B_NVDEC_UCODE_DESC;
			*image_fw_name = GB10B_NVDEC_UCODE_IMAGE;
			break;

		case NVGPU_DEVTYPE_NVJPG:
			*desc_fw_name = GB10B_NVJPG_UCODE_DESC;
			*image_fw_name = GB10B_NVJPG_UCODE_IMAGE;
			break;

		default:
			is_supported = false;
			break;
		}
		break;
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	case NVGPU_GPUID_GB20C:
	case NVGPU_GPUID_GB20C_NEXT:
		switch (dev_type) {
		case NVGPU_DEVTYPE_NVENC:
			*desc_fw_name = GB20C_NVENC_UCODE_DESC;
			*image_fw_name = GB20C_NVENC_UCODE_IMAGE;
			break;

		case NVGPU_DEVTYPE_OFA:
			*desc_fw_name = GB20C_OFA_UCODE_DESC;
			*image_fw_name = GB20C_OFA_UCODE_IMAGE;
			break;

		case NVGPU_DEVTYPE_NVDEC:
			*desc_fw_name = GB20C_NVDEC_UCODE_DESC;
			*image_fw_name = GB20C_NVDEC_UCODE_IMAGE;
			break;

		default:
			is_supported = false;
			break;
		}
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		is_supported = nvgpu_next_get_multimedia_fw_names(g, dev_type,
						desc_fw_name, image_fw_name);
#else
		is_supported = false;
#endif
		break;
	}

	return is_supported;
}
