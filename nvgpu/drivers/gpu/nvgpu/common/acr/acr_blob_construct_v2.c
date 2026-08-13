/*
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/firmware.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/bug.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/gr/gr_utils.h>

#include "nvgpu_acr_interface_v2.h"
#include "acr_blob_construct_v2.h"
#include "acr_wpr.h"
#include "acr_priv.h"

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_firmware.h>
#endif

static void lsfm_fill_static_lsb_hdr_info_pkc(struct gk20a *g,
	u32 falcon_id, struct lsfm_managed_ucode_img *pnode)
{
	u32 full_app_size = 0;

	if (pnode->ucode_img.lsf_desc_wrapper != NULL) {
		nvgpu_memcpy((u8 *)&pnode->lsb_header_v2.u.lsfLsbHdrV2.signature,
			(u8 *)pnode->ucode_img.lsf_desc_wrapper,
			sizeof(struct lsf_ucode_desc_wrapper));
	}
	pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_size = pnode->ucode_img.data_size;
	pnode->lsb_header_v2.u.lsfLsbHdrV2.data_size = LSB_HDR_DATA_SIZE;

	pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_code_size = NVGPU_ALIGN(
		pnode->ucode_img.desc->bootloader_size,
		LSF_BL_CODE_SIZE_ALIGNMENT);
	full_app_size = nvgpu_safe_add_u32(
			NVGPU_ALIGN(pnode->ucode_img.desc->app_size,
				LSF_BL_CODE_SIZE_ALIGNMENT),
				pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_code_size);

	pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_size = nvgpu_safe_add_u32(NVGPU_ALIGN(
			pnode->ucode_img.desc->app_resident_data_offset,
			LSF_BL_CODE_SIZE_ALIGNMENT),
				pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_code_size);

	pnode->lsb_header_v2.u.lsfLsbHdrV2.data_size = nvgpu_safe_sub_u32(full_app_size,
					pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_size);

	pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_imem_off =
	pnode->ucode_img.desc->bootloader_imem_offset;

	pnode->lsb_header_v2.u.lsfLsbHdrV2.flags = NV_FLCN_ACR_LSF_FLAG_FORCE_PRIV_LOAD_FALSE;

	if (g->acr->lsf[falcon_id].is_priv_load) {
		pnode->lsb_header_v2.u.lsfLsbHdrV2.flags |=
			NV_FLCN_ACR_LSF_FLAG_FORCE_PRIV_LOAD_TRUE;
	}

}

/* Populate static LSB header information using the provided ucode image */
static void lsfm_fill_static_lsb_hdr_info(struct gk20a *g,
	u32 falcon_id, struct lsfm_managed_ucode_img *pnode)
{
    pnode->lsb_header_v2.genericHdr.size = LSF_LSB_HEADER_WRAPPER_V2_SIZE_BYTE;
	pnode->lsb_header_v2.genericHdr.version = LSF_LSB_HEADER_VERSION_2;
	pnode->lsb_header_v2.genericHdr.identifier = WPR_GENERIC_HEADER_ID_LSF_LSB_HEADER;

	if (nvgpu_is_enabled(g, NVGPU_PKC_LS_SIG_ENABLED)
			&& (!pnode->ucode_img.is_next_core_img)) {
		lsfm_fill_static_lsb_hdr_info_pkc(g, falcon_id, pnode);
	}
}

/* Adds a ucode image to the list of managed ucode images managed. */
static int lsfm_add_ucode_img(struct gk20a *g, struct ls_flcn_mgr *plsfm,
	struct flcn_ucode_img *ucode_image, u32 falcon_id)
{
	struct lsfm_managed_ucode_img *pnode;

	pnode = nvgpu_kzalloc(g, sizeof(struct lsfm_managed_ucode_img));
	if (pnode == NULL) {
		return -ENOMEM;
	}

	/* Keep a copy of the ucode image info locally */
	nvgpu_memcpy((u8 *)&pnode->ucode_img, (u8 *)ucode_image,
		sizeof(struct flcn_ucode_img));

	/* Fill in static WPR header info*/
    pnode->wpr_header_wrapper.genericHdr.size = LSF_WPR_HEADER_WRAPPER_SIZE_BYTE;
	pnode->wpr_header_wrapper.genericHdr.version = LSF_WPR_HEADER_VERSION;
	pnode->wpr_header_wrapper.genericHdr.identifier = WPR_GENERIC_HEADER_ID_LSF_WPR_HEADER;
	pnode->wpr_header_wrapper.u.lsfWprHdr.falcon_id = falcon_id;
	pnode->wpr_header_wrapper.u.lsfWprHdr.bootstrap_owner = g->acr->bootstrap_owner;
	pnode->wpr_header_wrapper.u.lsfWprHdr.status = LSF_IMAGE_STATUS_COPY;

	pnode->wpr_header_wrapper.u.lsfWprHdr.lazy_bootstrap =
		nvgpu_safe_cast_bool_to_u32(
				g->acr->lsf[falcon_id].is_lazy_bootstrap);

	/* Fill in static LSB header info elsewhere */
	lsfm_fill_static_lsb_hdr_info(g, falcon_id, pnode);
	if (!nvgpu_is_enabled(g, NVGPU_PKC_LS_SIG_ENABLED)) {
		pnode->wpr_header_wrapper.u.lsfWprHdr.bin_version =
			pnode->lsb_header.signature.version;
	} else {
		pnode->wpr_header_wrapper.u.lsfWprHdr.bin_version =
			pnode->lsb_header_v2.u.lsfLsbHdrV2.signature.lsf_ucode_desc_v2.ls_ucode_version;
	}
	pnode->next = plsfm->ucode_img_list;
	plsfm->ucode_img_list = pnode;

	return 0;
}

static int lsfm_check_and_add_ucode_image(struct gk20a *g,
		struct ls_flcn_mgr *plsfm, u32 lsf_index)
{
	struct flcn_ucode_img ucode_img;
	struct nvgpu_acr *acr = g->acr;
	u32 falcon_id = 0U;
	int err = 0;

	if (!nvgpu_test_bit(lsf_index, (void *)&acr->lsf_enable_mask)) {
		return err;
	}

	if (acr->lsf[lsf_index].get_lsf_ucode_details == NULL) {
		nvgpu_err(g, "LS falcon-%d ucode fetch details not initialized",
				lsf_index);
		return -ENOENT;
	}

	(void) memset(&ucode_img, MEMSET_VALUE, sizeof(ucode_img));

	err = acr->lsf[lsf_index].get_lsf_ucode_details(g,
			(void *)&ucode_img);
	if (err != 0) {
		nvgpu_err(g, "LS falcon-%d ucode get failed", lsf_index);
		return err;
	}

	if (!nvgpu_is_enabled(g, NVGPU_PKC_LS_SIG_ENABLED)) {
		falcon_id = ucode_img.lsf_desc->falcon_id;
	} else {
		falcon_id = ucode_img.lsf_desc_wrapper->lsf_ucode_desc_v2.falcon_id;
	}

	err = lsfm_add_ucode_img(g, plsfm, &ucode_img, falcon_id);
	if (err != 0) {
		nvgpu_err(g, " Failed to add falcon-%d to LSFM ", falcon_id);
		return err;
	}

	plsfm->managed_flcn_cnt++;

	return err;
}

/* Discover all managed falcon ucode images */
static int lsfm_discover_ucode_images(struct gk20a *g,
	struct ls_flcn_mgr *plsfm)
{
	u32 i;
	int err = 0;

	/*
	 * Enumerate all constructed falcon objects, as we need the ucode
	 * image info and total falcon count
	 */
	for (i = 0U; i < FALCON_ID_END; i++) {
		err = lsfm_check_and_add_ucode_image(g, plsfm, i);
		if (err != 0) {
			return err;
		}
	}

	return err;
}

static void lsf_calc_wpr_size_pkc(struct lsfm_managed_ucode_img *pnode,
					u32 *wpr_off)
{
	u32 wpr_offset = *wpr_off;

	/* Align, save off, and include an LSB header size */
	wpr_offset = NVGPU_ALIGN(wpr_offset, LSF_LSB_HEADER_ALIGNMENT);
	pnode->wpr_header_wrapper.u.lsfWprHdr.lsb_offset = wpr_offset;
	wpr_offset = nvgpu_safe_add_u32(wpr_offset,
				(u32)sizeof(LSF_LSB_HEADER_WRAPPER));

	wpr_offset = NVGPU_ALIGN(wpr_offset, LSF_UCODE_DATA_ALIGNMENT);
	pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_off = wpr_offset;
	wpr_offset = nvgpu_safe_add_u32(wpr_offset,
					pnode->ucode_img.data_size);

	pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_data_size = NVGPU_ALIGN(
		nvgpu_safe_cast_u64_to_u32(
				sizeof(pnode->bl_gen_desc)),
		LSF_BL_DATA_SIZE_ALIGNMENT);

	wpr_offset = NVGPU_ALIGN(wpr_offset, LSF_BL_DATA_ALIGNMENT);
	pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_data_off = wpr_offset;
	wpr_offset = nvgpu_safe_add_u32(wpr_offset,
				pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_data_size);

	pnode->full_ucode_size = wpr_offset -
		pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_off;
	if (pnode->wpr_header_wrapper.u.lsfWprHdr.falcon_id != FALCON_ID_PMU &&
		pnode->wpr_header_wrapper.u.lsfWprHdr.falcon_id != FALCON_ID_PMU_NEXT_CORE) {
		pnode->lsb_header_v2.u.lsfLsbHdrV2.app_code_off =
			pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_code_size;
		pnode->lsb_header_v2.u.lsfLsbHdrV2.app_code_size =
			pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_size -
			pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_code_size;
		pnode->lsb_header_v2.u.lsfLsbHdrV2.app_data_off =
			pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_size;
		pnode->lsb_header_v2.u.lsfLsbHdrV2.app_data_size =
			pnode->lsb_header_v2.u.lsfLsbHdrV2.data_size;
	}

	*wpr_off = wpr_offset;
}

/* Generate WPR requirements for ACR allocation request */
static int lsf_gen_wpr_requirements(struct gk20a *g,
		struct ls_flcn_mgr *plsfm)
{
	struct lsfm_managed_ucode_img *pnode = plsfm->ucode_img_list;
	u32 wpr_offset;
	u32 flcn_cnt;

	(void)g;

	/*
	 * Start with an array of WPR headers at the base of the WPR.
	 * The expectation here is that the secure falcon will do a single DMA
	 * read of this array and cache it internally so it's OK to pack these.
	 * Also, we add 1 to the falcon count to indicate the end of the array.
	 */
	flcn_cnt = U32(plsfm->managed_flcn_cnt);
	wpr_offset = nvgpu_safe_mult_u32(U32(sizeof(LSF_WPR_HEADER_WRAPPER)),
		nvgpu_safe_add_u32(flcn_cnt, U32(1)));

	/*
	 * Walk the managed falcons, accounting for the LSB structs
	 * as well as the ucode images.
	 */
	while (pnode != NULL) {
		if (nvgpu_is_enabled(g, NVGPU_PKC_LS_SIG_ENABLED)) {
			lsf_calc_wpr_size_pkc(pnode, &wpr_offset);
		}

		pnode = pnode->next;
	}

	plsfm->wpr_size = wpr_offset;
	return 0;
}

/* Initialize WPR contents */
static int lsfm_populate_flcn_bl_dmem_desc(struct gk20a *g,
	void *lsfm, u32 *p_bl_gen_desc_size, u32 falconid)
{
	struct wpr_carveout_info wpr_inf;
	struct lsfm_managed_ucode_img *p_lsfm =
			(struct lsfm_managed_ucode_img *)lsfm;
	struct flcn_ucode_img *p_img = &(p_lsfm->ucode_img);
	struct flcn_bl_dmem_desc *ldr_cfg =
			&(p_lsfm->bl_gen_desc);
	u64 addr_base = 0;
	struct ls_falcon_ucode_desc *desc;
	u64 addr_code, addr_data;

	if (p_img->desc == NULL) {
		/*
		 * This means its a header based ucode,
		 * and so we do not fill BL gen desc structure
		 */
		return -EINVAL;
	}
	desc = p_img->desc;

	/*
	 * Calculate physical and virtual addresses for various portions of
	 * the PMU ucode image
	 * Calculate the 32-bit addresses for the application code, application
	 * data, and bootloader code. These values are all based on IM_BASE.
	 * The 32-bit addresses will be the upper 32-bits of the virtual or
	 * physical addresses of each respective segment.
	 */
	if (!nvgpu_is_enabled(g, NVGPU_PKC_LS_SIG_ENABLED)) {
		addr_base = p_lsfm->lsb_header.ucode_off;
	} else {
		addr_base = p_lsfm->lsb_header_v2.u.lsfLsbHdrV2.ucode_off;
	}
	g->acr->get_wpr_info(g, &wpr_inf);
	addr_base = nvgpu_safe_add_u64(addr_base, wpr_inf.wpr_base);

	nvgpu_acr_dbg(g, "falcon ID %x", p_lsfm->wpr_header_wrapper.u.lsfWprHdr.falcon_id);
	nvgpu_acr_dbg(g, "gen loader cfg addrbase %llx ", addr_base);
	addr_code = nvgpu_safe_add_u64(addr_base, desc->app_start_offset);
	addr_data = nvgpu_safe_add_u64(addr_code,
					desc->app_resident_data_offset);

	nvgpu_acr_dbg(g, "gen cfg addrcode %llx data %llx load offset %x",
			addr_code, addr_data, desc->bootloader_start_offset);

	/* Populate the LOADER_CONFIG state */
	(void) memset((void *) ldr_cfg, MEMSET_VALUE,
		sizeof(struct flcn_bl_dmem_desc));

	ldr_cfg->ctx_dma = g->acr->lsf[falconid].falcon_dma_idx;
	flcn64_set_dma(&ldr_cfg->code_dma_base, addr_code);
	ldr_cfg->non_sec_code_off = desc->app_resident_code_offset;
	ldr_cfg->non_sec_code_size = desc->app_resident_code_size;
	flcn64_set_dma(&ldr_cfg->data_dma_base, addr_data);
	ldr_cfg->data_size = desc->app_resident_data_size;
	ldr_cfg->code_entry_point = desc->app_imem_entry;

	/* Update the argc/argv members*/
	ldr_cfg->argc = UCODE_PARAMS;

	*p_bl_gen_desc_size = (u32)sizeof(struct flcn_bl_dmem_desc);
	return 0;
}

/* Populate falcon boot loader generic desc.*/
static int lsfm_fill_flcn_bl_gen_desc(struct gk20a *g,
		struct lsfm_managed_ucode_img *pnode)
{
	return lsfm_populate_flcn_bl_dmem_desc(g, pnode,
		&pnode->bl_gen_desc_size,
		pnode->wpr_header_wrapper.u.lsfWprHdr.falcon_id);
}

static int lsfm_init_wpr_contents(struct gk20a *g,
		struct ls_flcn_mgr *plsfm, struct nvgpu_mem *ucode)
{
	struct lsfm_managed_ucode_img *pnode = plsfm->ucode_img_list;
	LSF_WPR_HEADER_WRAPPER last_wpr_hdr;
	u32 i = 0;
	u64 tmp;
	int err = 0;

	/* The WPR array is at the base of the WPR */
	pnode = plsfm->ucode_img_list;
	(void) memset(&last_wpr_hdr, MEMSET_VALUE, sizeof(LSF_WPR_HEADER_WRAPPER));

	/*
	 * Walk the managed falcons, flush WPR and LSB headers to FB.
	 * flush any bl args to the storage area relative to the
	 * ucode image (appended on the end as a DMEM area).
	 */
	while (pnode != NULL) {
		/* Flush WPR header to memory*/
		nvgpu_mem_wr_n(g, ucode,
			nvgpu_safe_mult_u32(i,
				nvgpu_safe_cast_u64_to_u32(sizeof(
			pnode->wpr_header_wrapper))), &pnode->wpr_header_wrapper,
			nvgpu_safe_cast_u64_to_u32(sizeof(pnode->wpr_header_wrapper)));

		nvgpu_acr_dbg(g, "wpr header");
		nvgpu_acr_dbg(g, "falconid :%d",
				pnode->wpr_header_wrapper.u.lsfWprHdr.falcon_id);
		nvgpu_acr_dbg(g, "lsb_offset :%x",
				pnode->wpr_header_wrapper.u.lsfWprHdr.lsb_offset);
		nvgpu_acr_dbg(g, "bootstrap_owner :%d",
			pnode->wpr_header_wrapper.u.lsfWprHdr.bootstrap_owner);
		nvgpu_acr_dbg(g, "lazy_bootstrap :%d",
				pnode->wpr_header_wrapper.u.lsfWprHdr.lazy_bootstrap);
		nvgpu_acr_dbg(g, "status :%d",
				pnode->wpr_header_wrapper.u.lsfWprHdr.status);

		/*Flush LSB header to memory*/
		if (!nvgpu_is_enabled(g, NVGPU_PKC_LS_SIG_ENABLED)) {
			nvgpu_mem_wr_n(g, ucode, pnode->wpr_header_wrapper.u.lsfWprHdr.lsb_offset,
				&pnode->lsb_header,
				nvgpu_safe_cast_u64_to_u32(
					sizeof(pnode->lsb_header)));
		} else {
			nvgpu_mem_wr_n(g, ucode, pnode->wpr_header_wrapper.u.lsfWprHdr.lsb_offset,
				&pnode->lsb_header_v2,
				nvgpu_safe_cast_u64_to_u32(
					sizeof(pnode->lsb_header_v2)));
		}

		nvgpu_acr_dbg(g, "lsb header");
		if (nvgpu_is_enabled(g, NVGPU_PKC_LS_SIG_ENABLED)) {
			nvgpu_acr_dbg(g, "ucode_off :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_off);
			nvgpu_acr_dbg(g, "ucode_size :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_size);
			nvgpu_acr_dbg(g, "data_size :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.data_size);
			nvgpu_acr_dbg(g, "bl_code_size :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_code_size);
			nvgpu_acr_dbg(g, "bl_imem_off :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_imem_off);
			nvgpu_acr_dbg(g, "bl_data_off :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_data_off);
			nvgpu_acr_dbg(g, "bl_data_size :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_data_size);
			nvgpu_acr_dbg(g, "app_code_off :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.app_code_off);
			nvgpu_acr_dbg(g, "app_code_size :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.app_code_size);
			nvgpu_acr_dbg(g, "app_data_off :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.app_data_off);
			nvgpu_acr_dbg(g, "app_data_size :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.app_data_size);
			nvgpu_acr_dbg(g, "flags :%x",
					pnode->lsb_header_v2.u.lsfLsbHdrV2.flags);
		}

		if (!pnode->ucode_img.is_next_core_img) {
			/*
			 * If this falcon has a boot loader and related args,
			 * flush them.
			 */
			/* Populate gen bl and flush to memory */
			err = lsfm_fill_flcn_bl_gen_desc(g, pnode);
			if (err != 0) {
				nvgpu_err(g, "bl_gen_desc failed err=%d", err);
				return err;
			}
			if (!nvgpu_is_enabled(g, NVGPU_PKC_LS_SIG_ENABLED)) {
				nvgpu_mem_wr_n(g, ucode,
					pnode->lsb_header.bl_data_off,
					&pnode->bl_gen_desc,
					pnode->bl_gen_desc_size);
			} else {
				nvgpu_mem_wr_n(g, ucode,
					pnode->lsb_header_v2.u.lsfLsbHdrV2.bl_data_off,
					&pnode->bl_gen_desc,
					pnode->bl_gen_desc_size);
			}
		}

		/* Copying of ucode */
		if (!nvgpu_is_enabled(g, NVGPU_PKC_LS_SIG_ENABLED)) {
			nvgpu_mem_wr_n(g, ucode, pnode->lsb_header.ucode_off,
				pnode->ucode_img.data,
				pnode->ucode_img.data_size);
		} else {
			nvgpu_mem_wr_n(g, ucode, pnode->lsb_header_v2.u.lsfLsbHdrV2.ucode_off,
				pnode->ucode_img.data,
				pnode->ucode_img.data_size);
		}

		pnode = pnode->next;
		i = nvgpu_safe_add_u32(i, 1U);
	}

	/* Tag the terminator WPR header with an invalid falcon ID. */
	last_wpr_hdr.u.lsfWprHdr.falcon_id = FALCON_ID_INVALID;
    /* Fill in static WPR header info*/
    last_wpr_hdr.genericHdr.size = LSF_WPR_HEADER_WRAPPER_SIZE_BYTE;
    last_wpr_hdr.genericHdr.version = LSF_WPR_HEADER_VERSION;
    last_wpr_hdr.genericHdr.identifier = WPR_GENERIC_HEADER_ID_LSF_WPR_HEADER;
	tmp = nvgpu_safe_mult_u32(plsfm->managed_flcn_cnt,
					(u32)sizeof(LSF_WPR_HEADER_WRAPPER));
	nvgpu_assert(tmp <= U32_MAX);
	nvgpu_mem_wr_n(g, ucode, (u32)tmp, &last_wpr_hdr,
		(u32)sizeof(LSF_WPR_HEADER_WRAPPER));

	return err;
}

int nvgpu_acr_prepare_ucode_blob_v2(struct gk20a *g)
{
	int err = 0;
	struct ls_flcn_mgr lsfm_l, *plsfm;

	struct wpr_carveout_info wpr_inf;
	struct nvgpu_gr_falcon *gr_falcon = nvgpu_gr_get_falcon_ptr(g);

	/* Recovery case, we do not need to form non WPR blob of ucodes */
	if (g->acr->ucode_blob.cpu_va != NULL) {
		return err;
	}


	plsfm = &lsfm_l;
	(void) memset((void *)plsfm, MEMSET_VALUE, sizeof(struct ls_flcn_mgr));
	err = nvgpu_gr_falcon_init_ctxsw_ucode(g, gr_falcon);
	if (err != 0) {
		nvgpu_err(g, "gr_falcon_init_ctxsw_ucode failed err=%d", err);
		return err;
	}

	g->acr->get_wpr_info(g, &wpr_inf);
	nvgpu_acr_dbg(g, "wpr carveout base:%llx\n", (wpr_inf.wpr_base));
	nvgpu_acr_dbg(g, "wpr carveout size :%llx\n", wpr_inf.size);

	/* Discover all managed falcons */
	err = lsfm_discover_ucode_images(g, plsfm);
	nvgpu_acr_dbg(g, " Managed Falcon cnt %d\n", plsfm->managed_flcn_cnt);
	if (err != 0) {
		goto cleanup_exit;
	}

	if ((plsfm->managed_flcn_cnt != 0U) &&
		(g->acr->ucode_blob.cpu_va == NULL)) {
		/* Generate WPR requirements */
		err = lsf_gen_wpr_requirements(g, plsfm);
		if (err != 0) {
			goto cleanup_exit;
		}

		/* Alloc memory to hold ucode blob contents */
		err = g->acr->alloc_blob_space(g, plsfm->wpr_size,
							&g->acr->ucode_blob);
		if (err != 0) {
			goto cleanup_exit;
		}

		nvgpu_acr_dbg(g, "managed LS falcon %d, WPR size %d bytes.\n",
			plsfm->managed_flcn_cnt, plsfm->wpr_size);

		err = lsfm_init_wpr_contents(g, plsfm, &g->acr->ucode_blob);
		if (err != 0) {
			nvgpu_kfree(g, &g->acr->ucode_blob);
			goto cleanup_exit;
		}
	} else {
		nvgpu_acr_dbg(g, "LSFM is managing no falcons.\n");
	}
	nvgpu_acr_dbg(g, "prepare ucode blob return 0\n");

cleanup_exit:
	nvgpu_acr_free_resources(g, plsfm);
	return err;
}
