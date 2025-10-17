// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/types.h>
#include <nvgpu/dma.h>
#include <nvgpu/soc.h>
#include <nvgpu/firmware.h>
#include <nvgpu/riscv.h>
#include <nvgpu/nvdec.h>
#include <nvgpu/nvjpg.h>
#include <nvgpu/ofa.h>
#include <nvgpu/nvenc.h>
#include <nvgpu/acr.h>
#include <nvgpu/pmu.h>
#include <nvgpu/string.h>
#include <nvgpu/nvgpu_gb10b_firmware.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/netlist.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/gr/gr_utils.h>

#include "common/acr/acr_priv.h"
#include "common/acr/acr_blob_construct_v2.h"
#include "common/acr/nvgpu_acr_interface_v2.h"
#include "common/acr/nvgpu_acr_interface.h"

#define GB10B_FECS_RISCV_BOOTPLUGIN                "g_rv_boot_plugin_fecs.text.bin"
#define GB10B_GPCCS_RISCV_BOOTPLUGIN               "g_rv_boot_plugin_gpccs.text.bin"

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
#define MAX_NETLIST_NAME (sizeof("prod_test_nsr/g_xxxcs_riscv_gbxxx_NET?_pkc_ls_sig.bin"))
#else
#define MAX_NETLIST_NAME (sizeof("safety/g_xxxcs_riscv_gbxxx_NET?_pkc_ls_sig.bin"))
#endif

static s32 nvgpu_gb10b_acr_get_ucode_details
(
	struct gk20a *g,
	struct flcn_ucode_img *p_img,
	u32 falcon_id
)
{
	struct lsf_ucode_desc_wrapper *lsf_desc_wrapper = NULL;
	LSF_LSB_HEADER_WRAPPER *lsb_header_wrapper = NULL;
	s32 err = 0;

	nvgpu_log(g, gpu_dbg_falcon | gpu_dbg_mme,
			"Loading firmware for falcon id %d", falcon_id);

	err = nvgpu_get_acr_lsf_fw(g, falcon_id, p_img);
	if(err != 0){
		nvgpu_err(g, "MM get fw failed:%d for falcon:%d", err, falcon_id);
		goto fw_read_failed;
	}

	if (nvgpu_acr_is_lsb_v3_supported(g)) {
		lsb_header_wrapper = nvgpu_kzalloc(g, sizeof(LSF_LSB_HEADER_WRAPPER));
		if (lsb_header_wrapper == NULL) {
			err = -ENOMEM;
			goto fw_read_failed;
		}
		nvgpu_memcpy((u8 *)lsb_header_wrapper, (u8 *)p_img->pkc_sig->data,
				min_t(size_t, sizeof(*lsb_header_wrapper), p_img->pkc_sig->size));
		lsb_header_wrapper->u.lsfLsbHdrV3.signature.nvlsf_ucode_desc_v2.falcon_id =
				falcon_id;
		p_img->lsb_header_wrapper =
			(LSF_LSB_HEADER_WRAPPER *)lsb_header_wrapper;
		p_img->lsf_desc_wrapper = &lsb_header_wrapper->u.lsfLsbHdrV3.signature;
	} else {
		lsf_desc_wrapper = nvgpu_kzalloc(g, sizeof(struct lsf_ucode_desc_wrapper));
		if (lsf_desc_wrapper == NULL) {
			err = -ENOMEM;
			goto fw_read_failed;
		}
		nvgpu_memcpy((u8 *)lsf_desc_wrapper, (u8 *)p_img->pkc_sig->data,
				min_t(size_t, sizeof(*lsf_desc_wrapper), p_img->pkc_sig->size));

		lsf_desc_wrapper->nvlsf_ucode_desc_v2.falcon_id = falcon_id;
		p_img->lsf_desc_wrapper =
			(struct lsf_ucode_desc_wrapper *)lsf_desc_wrapper;
	}

	p_img->rdesc = (struct nvgpu_nvriscv_fw_desc *)(void *)p_img->desc_fw->data;

	p_img->data = (u32 *)(void *)p_img->image_fw->data;
	p_img->data_size = nvgpu_safe_cast_u64_to_u32(p_img->image_fw->size);

	return err;

fw_read_failed:
	if (p_img->desc_fw != NULL) {
		nvgpu_release_firmware(g, p_img->desc_fw);
	}
	if (p_img->image_fw != NULL) {
		nvgpu_release_firmware(g, p_img->image_fw);
	}
	if (p_img->pkc_sig != NULL) {
		nvgpu_release_firmware(g, p_img->pkc_sig);
	}
	return err;
}

static s32 nvgpu_gb10b_acr_get_ctxsw_riscv_eb_ucode_details
(
	struct gk20a *g,
	struct flcn_ucode_img *p_img,
	u32 falcon_id
)
{
	struct nvgpu_firmware *desc_fw, *pkc_sig;
	struct lsf_ucode_desc_wrapper *lsf_desc_wrapper = NULL;
	LSF_LSB_HEADER_WRAPPER *lsb_header_wrapper = NULL;
	struct nvgpu_mem *ucode_image = NULL;
	const char *desc_fw_name;
	char name[MAX_NETLIST_NAME];
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch,
				g->params.gpu_impl);
	s32 err = 0;
	u8 *code = NULL;
	u8 *data = NULL;
	u32 code_off = 0;
	u32 data_off = 0;
	u32 code_size = 0;
	u32 data_size = 0;
	u32 code_size_unaligned = 0;
	u32 data_size_unaligned = 0;
	u32 total_size = 0;

	// For simulation we want to fetch the ucode data from chiplib
#ifdef CONFIG_NVGPU_SIM
	u8 *manifest = NULL;
	u32 manifest_size = 0;
	u32 manifest_off = 0;
	u32 bp_code_size = 0;
	u32 bp_code_off = 0;
	u32 bp_data_size = 0;
	u32 bp_data_off = 0;

	nvgpu_log_fn(g, " ");

	if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)) {
		if (falcon_id == FALCON_ID_FECS_RISCV_EB) {
			// get code section
			code = (u8 *)(void *)nvgpu_netlist_get_fecs_inst_list(g);
			code_size_unaligned = nvgpu_safe_mult_u32(
					nvgpu_netlist_get_fecs_inst_count(g), (u32)sizeof(u32));
			// get data section
			data = (u8 *)(void *)nvgpu_netlist_get_fecs_data_list(g);
			data_size_unaligned = nvgpu_safe_mult_u32(
				nvgpu_netlist_get_fecs_data_count(g), (u32)sizeof(u32));
			// manifest section
			manifest = (u8 *)(void *)nvgpu_netlist_get_fecs_manifest_list(g);
			manifest_size = nvgpu_netlist_get_fecs_manifest_count(g);
		} else if (falcon_id == FALCON_ID_GPCCS_RISCV_EB){
			// get code section
			code = (u8 *)(void *)nvgpu_netlist_get_gpccs_inst_list(g);
			code_size_unaligned = nvgpu_safe_mult_u32(
									nvgpu_netlist_get_gpccs_inst_count(g),
									(u32)sizeof(u32));
			if ((UINT_MAX - 255) < code_size_unaligned) {
				err = -EOVERFLOW;
				return err;
			}
			code_size = ALIGN_UP(code_size_unaligned, 256);
			// get data section
			data = (u8 *)(void *)nvgpu_netlist_get_gpccs_data_list(g);
			data_size_unaligned = nvgpu_safe_mult_u32(
									nvgpu_netlist_get_gpccs_data_count(g),
									(u32)sizeof(u32));
			if ((UINT_MAX - 255) < data_size_unaligned) {
				err = -EOVERFLOW;
				return err;
			}
			data_size = ALIGN_UP(data_size_unaligned, 256);
			// manifest section
			manifest = (u8 *)(void *)nvgpu_netlist_get_gpccs_manifest_list(g);
			manifest_size = (nvgpu_netlist_get_gpccs_manifest_count(g));
		} else {
			nvgpu_err(g, "Invalid Falcon ID: %d", falcon_id);
			return -EINVAL;
		}

		// get bootplugin from file
		if (g->ops.falcon.get_bootplugin_fw_name != NULL) {
			g->ops.falcon.get_bootplugin_fw_name(g, &desc_fw_name, falcon_id);
		} else {
			nvgpu_err(g, "Failed to fetch CTXSW EB Bootplugin");
			return -ENOENT;
		}
		desc_fw = nvgpu_request_firmware(g, desc_fw_name,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
		if (desc_fw == NULL) {
			nvgpu_err(g, "Failed to read CTXSW EB BP: %s", desc_fw_name);
			return -ENOENT;
		}

		code_size = ALIGN_UP(code_size_unaligned, 256);
		data_size = ALIGN_UP(data_size_unaligned, 256);
		bp_code_off = 0;
		if (desc_fw->size > UINT_MAX) {
			nvgpu_err(g, "bootplugin code size %lu exceeds UINT_MAX", desc_fw->size);
			err = -EOVERFLOW;
			goto free_desc_fw;
		}
		bp_code_size = (u32)desc_fw->size;
		total_size = ALIGN_UP(bp_code_size, 0X1000) + manifest_size;

		bp_data_off = bp_code_off + ALIGN_UP(bp_code_size, 0X1000);
		manifest_off = bp_data_off;
		data_off = nvgpu_safe_add_u32(manifest_off, manifest_size);

		// ACR requires RISCV ucode data before code
		code_off = data_off + data_size;
		total_size += code_size + data_size;
		if ((UINT_MAX - 0XFFF) < total_size) {
			err = -EOVERFLOW;
			goto free_desc_fw;
		}
		total_size = ALIGN_UP(total_size, 0X1000);

		// Allocate memory for ucode_image structure
		ucode_image = nvgpu_kzalloc(g, sizeof(struct nvgpu_mem));
		if (ucode_image == NULL) {
			err = -ENOMEM;
			goto free_desc_fw;
		}
		if (!nvgpu_mem_is_valid(ucode_image)) {
			err = nvgpu_dma_alloc_flags_sys(g,
				NVGPU_DMA_PHYSICALLY_ADDRESSED,
				total_size,
				ucode_image);
		}
		nvgpu_memset(g, ucode_image, 0U, 0U, total_size);
		nvgpu_mem_wr_n(g, ucode_image,
					bp_code_off,
					desc_fw->data,
					bp_code_size);
		nvgpu_mem_wr_n(g, ucode_image,
					manifest_off,
					manifest,
					manifest_size);
		nvgpu_mem_wr_n(g, ucode_image,
					code_off,
					code,
					code_size_unaligned);
		nvgpu_mem_wr_n(g, ucode_image,
					data_off,
					data,
					data_size_unaligned);
		p_img->ucode_image = ucode_image;
		p_img->rdesc = nvgpu_kzalloc(g, sizeof(struct nvgpu_nvriscv_fw_desc));
		if (p_img->rdesc == NULL) {
			err = -ENOMEM;
			goto free_desc_fw;
		}
		p_img->rdesc->version = 5;
		p_img->rdesc->monitorCodeOffset = code_off;
		p_img->rdesc->monitorCodeSize = code_size;
		p_img->rdesc->monitorDataOffset = data_off;
		p_img->rdesc->monitorDataSize = data_size;
		p_img->rdesc->swbromCodeOffset = bp_code_off;
		p_img->rdesc->swbromCodeSize = bp_code_size;
		p_img->rdesc->swbromDataOffset = bp_data_off;
		p_img->rdesc->swbromDataSize = bp_data_size;
		p_img->rdesc->manifestOffset = manifest_off;
		p_img->rdesc->manifestSize = manifest_size;
	} else
#endif //ifdef CONFIG_NVGPU_SIM
	{
		nvgpu_log_fn(g, " ");

		// Load The RISCV descriptor
		if (falcon_id == FALCON_ID_FECS_RISCV_EB) {
			if(g->ops.netlist.get_fecs_desc_name == NULL){
				nvgpu_err(g,"failed to fetch get_fecs_desc_name hal");
				return -EINVAL;
			}
			g->ops.netlist.get_fecs_desc_name(g, name);
			desc_fw_name = name;
		} else if (falcon_id == FALCON_ID_GPCCS_RISCV_EB){
			if(g->ops.netlist.get_gpccs_desc_name == NULL){
				nvgpu_err(g,"failed to fetch get_gpccs_desc_name hal");
				return -EINVAL;
			}
			g->ops.netlist.get_gpccs_desc_name(g, name);
			desc_fw_name = name;
		} else {
			nvgpu_err(g, "Invalid Falcon ID: %d", falcon_id);
			return -EINVAL;
		}

		desc_fw = nvgpu_request_firmware(g, desc_fw_name,
						NVGPU_REQUEST_FIRMWARE_NO_WARN);
		if (desc_fw == NULL) {
			nvgpu_err(g, "Failed to read firmware: %s", desc_fw_name);
			err = -ENOENT;
		}
		p_img->desc_fw = desc_fw;

		if (falcon_id == FALCON_ID_FECS_RISCV_EB) {
			// get code section
			code = (u8 *)(void *)nvgpu_netlist_get_fecs_inst_list(g);
			code_size_unaligned = nvgpu_safe_mult_u32(
					nvgpu_netlist_get_fecs_inst_count(g), (u32)sizeof(u32));
			// get data section
			data = (u8 *)(void *)nvgpu_netlist_get_fecs_data_list(g);
			data_size_unaligned = nvgpu_safe_mult_u32(
					nvgpu_netlist_get_fecs_data_count(g), (u32)sizeof(u32));
		} else if (falcon_id == FALCON_ID_GPCCS_RISCV_EB){
			// get code section
			code = (u8 *)(void *)nvgpu_netlist_get_gpccs_inst_list(g);
			code_size_unaligned = nvgpu_safe_mult_u32(
					nvgpu_netlist_get_gpccs_inst_count(g), (u32)sizeof(u32));
			// get data section
			data = (u8 *)(void *)nvgpu_netlist_get_gpccs_data_list(g);
			data_size_unaligned = nvgpu_safe_mult_u32(
					nvgpu_netlist_get_gpccs_data_count(g), (u32)sizeof(u32));
		} else {
			nvgpu_err(g, "Invalid Falcon ID: %d", falcon_id);
			return -EINVAL;
		}

		if ((UINT_MAX - 3) < code_size_unaligned) {
			err = -EOVERFLOW;
			goto free_desc_fw;
		}
		code_size = ALIGN_UP(code_size_unaligned, 4);
		if ((UINT_MAX - 255) < code_size) {
			err = -EOVERFLOW;
			goto free_desc_fw;
		}
		code_size = ALIGN_UP(code_size, 256);
		if ((UINT_MAX - 3) < data_size_unaligned) {
			err = -EOVERFLOW;
			goto free_desc_fw;
		}
		data_size = ALIGN_UP(data_size_unaligned, 4);
		if ((UINT_MAX - 255) < data_size) {
			err = -EOVERFLOW;
			goto free_desc_fw;
		}
		data_size = ALIGN_UP(data_size, 256);

		code_off = data_off + data_size;
		total_size += (code_size + data_size);
		if ((UINT_MAX - 0XFFF) < total_size) {
			err = -EOVERFLOW;
			goto free_desc_fw;
		}
		total_size = ALIGN_UP(total_size, 0x1000);

		// Allocate memory for ucode_image structure
		ucode_image = nvgpu_kzalloc(g, sizeof(struct nvgpu_mem));
		if (ucode_image == NULL) {
			err = -ENOMEM;
			goto free_desc_fw;
		}
		if (!nvgpu_mem_is_valid(ucode_image)) {
			err = nvgpu_dma_alloc_flags_sys(g,
				NVGPU_DMA_PHYSICALLY_ADDRESSED,
				total_size,
				ucode_image);
		}
		nvgpu_memset(g, ucode_image, 0U, 0U, total_size);
		nvgpu_mem_wr_n(g, ucode_image,
					code_off,
					code,
					code_size_unaligned);
		nvgpu_mem_wr_n(g, ucode_image,
					data_off,
					data,
					data_size_unaligned);
		p_img->ucode_image = ucode_image;
		if (desc_fw == NULL) {
			nvgpu_err(g, "desc_fw is NULL");
			err = -EINVAL;
			goto free_desc_fw;
		}
		if (desc_fw->data == NULL) {
			nvgpu_err(g, "desc_fw->data is NULL");
			err = -EINVAL;
			goto free_desc_fw;
		}
		p_img->rdesc = (struct nvgpu_nvriscv_fw_desc *)(void *)desc_fw->data;
	}

	if (falcon_id == FALCON_ID_FECS_RISCV_EB) {
		pkc_sig = nvgpu_acr_lsf_fecs_get_firmware(g, ver);
	} else if (falcon_id == FALCON_ID_GPCCS_RISCV_EB){
		pkc_sig = nvgpu_acr_lsf_gpccs_get_firmware(g, ver);
	} else {
		nvgpu_err(g, "Invalid Falcon ID: %d", falcon_id);
		return -EINVAL;
	}

	if (pkc_sig == NULL) {
		nvgpu_err(g, "Failed to read CTXSW PKC sig firmware");
		err = -ENOENT;
		nvgpu_kfree(g, lsf_desc_wrapper);
		goto free_desc_fw;
	}
	p_img->pkc_sig = pkc_sig;

	if (nvgpu_acr_is_lsb_v3_supported(g) && !nvgpu_platform_is_simulation(g)) {
		lsb_header_wrapper = nvgpu_kzalloc(g, sizeof(LSF_LSB_HEADER_WRAPPER));
		if (lsb_header_wrapper == NULL) {
			err = -ENOMEM;
			goto free_desc_fw;
		}
		nvgpu_memcpy((u8 *)lsb_header_wrapper, (u8 *)pkc_sig->data,
				min_t(size_t, sizeof(*lsb_header_wrapper), pkc_sig->size));
		lsb_header_wrapper->u.lsfLsbHdrV3.signature.nvlsf_ucode_desc_v2.falcon_id =
				falcon_id;
		p_img->lsb_header_wrapper =
			(LSF_LSB_HEADER_WRAPPER *)lsb_header_wrapper;
		p_img->lsf_desc_wrapper = &lsb_header_wrapper->u.lsfLsbHdrV3.signature;
	} else {
		lsf_desc_wrapper = nvgpu_kzalloc(g, sizeof(struct lsf_ucode_desc_wrapper));
		if (lsf_desc_wrapper == NULL) {
			err = -ENOMEM;
			goto free_desc_fw;
		}
		nvgpu_memcpy((u8 *)lsf_desc_wrapper, (u8 *)pkc_sig->data,
				min_t(size_t, sizeof(*lsf_desc_wrapper), pkc_sig->size));

		lsf_desc_wrapper->nvlsf_ucode_desc_v2.falcon_id = falcon_id;
		p_img->lsf_desc_wrapper =
			(struct lsf_ucode_desc_wrapper *)lsf_desc_wrapper;
	}

	p_img->data = (void *)ucode_image->cpu_va;
	p_img->data_size = total_size;

	p_img->is_riscv_eb_img = true;
	return err;

free_desc_fw:
	if (ucode_image != NULL) {
		if (nvgpu_mem_is_valid(ucode_image)) {
			nvgpu_dma_free_sys(g, ucode_image);
		}
		nvgpu_kfree(g, ucode_image);
	}
	nvgpu_release_firmware(g, desc_fw);
	return err;
}

s32  nvgpu_gb10b_acr_lsf_fecs_riscv_eb_ucode_details
(
	struct gk20a *g,
	void *lsf_ucode_img
)
{
	int err = 0;
	struct flcn_ucode_img *p_img =
			(struct flcn_ucode_img *)lsf_ucode_img;

	nvgpu_log_fn(g, " ");

	err = nvgpu_gb10b_acr_get_ctxsw_riscv_eb_ucode_details(g,
			p_img, FALCON_ID_FECS_RISCV_EB);
	if(err != 0) {
		nvgpu_err(g, "Get FECS EB lsf deatils failed!");
	}
	return err;
}

s32  nvgpu_gb10b_acr_lsf_gpccs_riscv_eb_ucode_details
(
	struct gk20a *g,
	void *lsf_ucode_img
)
{
	int err = 0;
	struct flcn_ucode_img *p_img =
			(struct flcn_ucode_img *)lsf_ucode_img;

	nvgpu_log_fn(g, " ");

	err = nvgpu_gb10b_acr_get_ctxsw_riscv_eb_ucode_details(g,
			p_img, FALCON_ID_GPCCS_RISCV_EB);
	if(err != 0) {
		nvgpu_err(g, "Get GPCCS EB lsf deatils failed!");
	}
	return err;
}

s32 nvgpu_gb10b_acr_lsf_nvdec_riscv_ucode_details
(
	struct gk20a *g,
	void *lsf_ucode_img
)
{
	struct flcn_ucode_img *p_img =
		(struct flcn_ucode_img *)lsf_ucode_img;
	s32 err = 0;

	nvgpu_log_fn(g, " ");

	err = nvgpu_gb10b_acr_get_ucode_details(g, p_img,
				FALCON_ID_NVDEC_RISCV);
	if(err != 0){
		nvgpu_err(g, "failed to fetch nvdec fw details");
		return err;
	}

	p_img->is_riscv_core_img = true;
	return err;
}

s32 nvgpu_gb10b_acr_lsf_nvenc_riscv_eb_ucode_details
(
	struct gk20a *g,
	void *lsf_ucode_img
)
{
	struct flcn_ucode_img *p_img =
		(struct flcn_ucode_img *)lsf_ucode_img;
	s32 err = 0;

	nvgpu_log_fn(g, " ");

	err = nvgpu_gb10b_acr_get_ucode_details(g, p_img,
				FALCON_ID_NVENC_RISCV_EB);
	if(err != 0){
		nvgpu_err(g, "failed to fetch nvenc fw details");
		return err;
	}

	p_img->is_riscv_eb_img = true;
	return err;
}

s32 nvgpu_gb10b_acr_lsf_pmu_riscv_eb_ucode_details
(
	struct gk20a *g,
	void *lsf_ucode_img
)
{
	struct flcn_ucode_img *p_img =
		(struct flcn_ucode_img *)lsf_ucode_img;
	s32 err = 0;

	nvgpu_log_fn(g, " ");

	err = nvgpu_gb10b_acr_get_ucode_details(g, p_img,
				FALCON_ID_PMU_RISCV_EB);
	if(err != 0){
		nvgpu_err(g, "failed to fetch pmu fw details");
		return err;
	}

	p_img->is_riscv_eb_img = true;
	return err;
}

s32 nvgpu_gb10b_acr_lsf_ofa_riscv_eb_ucode_details
(
	struct gk20a *g,
	void *lsf_ucode_img
)
{
	struct flcn_ucode_img *p_img =
		(struct flcn_ucode_img *)lsf_ucode_img;
	s32 err = 0;

	nvgpu_log_fn(g, " ");

	err = nvgpu_gb10b_acr_get_ucode_details(g, p_img,
				FALCON_ID_OFA_RISCV_EB);
	if(err != 0){
		nvgpu_err(g, "failed to fetch ofa fw details");
		return err;
	}

	p_img->is_riscv_eb_img = true;

	return err;
}

s32 nvgpu_gb10b_acr_lsf_nvjpg_riscv_eb_ucode_details
(
	struct gk20a *g,
	void *lsf_ucode_img
)
{
	struct flcn_ucode_img *p_img =
		(struct flcn_ucode_img *)lsf_ucode_img;
	s32 err = 0;

	nvgpu_log_fn(g, " ");

	err = nvgpu_gb10b_acr_get_ucode_details(g, p_img,
				FALCON_ID_NVJPG_RISCV_EB);
	if(err != 0){
		nvgpu_err(g, "failed to fetch nvjpg fw details");
		return err;
	}

	p_img->is_riscv_eb_img = true;
	return err;
}
