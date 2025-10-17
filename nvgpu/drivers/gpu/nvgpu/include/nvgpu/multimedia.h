/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_MULTIMEDIA_H
#define NVGPU_MULTIMEDIA_H

#include <nvgpu/nvgpu_mem.h>

#define NVENC_INST_MAX				(2U)
#define OFA_INST_MAX				(1U)
#define NVDEC_INST_MAX				(2U)
#define NVJPG_INST_MAX				(2U)

#define UCODE_DMA_ID				(0x6)
#define MULTIMEDIA_UCODE_HEADER_SIZE		(APP_0_CODE_SIZE + 1)
#define MULTIMEDIA_UCODE_HEADER_SIZE_BYTES	(MULTIMEDIA_UCODE_HEADER_SIZE * 4)

/* Below MM firmwares are used in ACR boot. */
#define NVDEC_RISCV_UCODE_IMAGE             "g_nvdec_t264_riscv_dbg_image_debug_encrypted.bin"
#define NVDEC_RISCV_UCODE_DESC              "g_nvdec_t264_riscv_dbg_desc.bin"
#define NVDEC_RISCV_UCODE_PKC_SIG           "g_nvdec_t264_riscv_dbg_pkc_ls_sig.bin"
#define NVDEC_RISCV_UCODE_PROD_IMAGE        "g_nvdec_t264_riscv_prod_image_prod_encrypted.bin"
#define NVDEC_RISCV_UCODE_PROD_DESC         "g_nvdec_t264_riscv_prod_desc.bin"
#define NVDEC_RISCV_UCODE_PROD_PKC_SIG      "g_nvdec_t264_riscv_prod_pkc_ls_sig.bin"
#define NVJPG_RISCV_EB_UCODE_IMAGE          "g_nvjpg_t264_riscv_eb_image_debug_encrypted.bin"
#define NVJPG_RISCV_EB_UCODE_PROD_IMAGE     "g_nvjpg_t264_riscv_eb_image_prod_encrypted.bin"
#define NVJPG_RISCV_EB_UCODE_DESC           "g_nvjpg_t264_riscv_eb_desc.bin"
#define NVJPG_RISCV_EB_UCODE_PKC_SIG        "g_nvjpg_t264_riscv_eb_pkc_ls_sig.bin"
#define OFA_RISCV_EB_UCODE_IMAGE            "g_ofa_t264_riscv_eb_image_debug_encrypted.bin"
#define OFA_RISCV_EB_UCODE_PROD_IMAGE       "g_ofa_t264_riscv_eb_image_prod_encrypted.bin"
#define OFA_RISCV_EB_UCODE_DESC             "g_ofa_t264_riscv_eb_desc.bin"
#define OFA_RISCV_EB_UCODE_PKC_SIG          "g_ofa_t264_riscv_eb_pkc_ls_sig.bin"
#define NVENC_RISCV_EB_UCODE_IMAGE          "g_nvenc_t264_riscv_eb_image_debug_encrypted.bin"
#define NVENC_RISCV_EB_UCODE_PROD_IMAGE     "g_nvenc_t264_riscv_eb_image_prod_encrypted.bin"
#define NVENC_RISCV_EB_UCODE_DESC           "g_nvenc_t264_riscv_eb_desc.bin"
#define NVENC_RISCV_EB_UCODE_PKC_SIG        "g_nvenc_t264_riscv_eb_pkc_ls_sig.bin"

/* Below MM firmwares are used in non-ACR boot. */
#define GB10B_NVENC_UCODE_IMAGE  "g_nvenc_t264_riscv_eb_image_sim.bin"
#define GB10B_NVENC_UCODE_DESC   "g_nvenc_t264_riscv_eb_desc_sim.bin"
#define GB10B_OFA_UCODE_IMAGE    "g_ofa_t264_riscv_eb_image_sim.bin"
#define GB10B_OFA_UCODE_DESC     "g_ofa_t264_riscv_eb_desc_sim.bin"
#define GB10B_NVDEC_UCODE_IMAGE  "g_nvdec_t264_riscv_dbg_image_sim.bin"
#define GB10B_NVDEC_UCODE_DESC   "g_nvdec_t264_riscv_dbg_desc_sim.bin"
#define GB10B_NVJPG_UCODE_IMAGE  "g_nvjpg_t264_riscv_eb_image_sim.bin"
#define GB10B_NVJPG_UCODE_DESC   "g_nvjpg_t264_riscv_eb_desc_sim.bin"

#define GB20C_NVENC_UCODE_IMAGE  "g_nvenc_t256_riscv_eb_image_sim.bin"
#define GB20C_NVENC_UCODE_DESC   "g_nvenc_t256_riscv_eb_desc_sim.bin"
#define GB20C_OFA_UCODE_IMAGE    "g_ofa_t256_riscv_image_sim.bin"
#define GB20C_OFA_UCODE_DESC     "g_ofa_t256_riscv_desc_sim.bin"
#define GB20C_NVDEC_UCODE_IMAGE  "g_nvdec_t256_riscv_dbg_image_sim.bin"
#define GB20C_NVDEC_UCODE_DESC   "g_nvdec_t256_riscv_dbg_desc_sim.bin"

struct nvgpu_tsg;
struct vm_gk20a;
struct nvgpu_channel;

/**
 * Multimedia engine enum types supported from driver.
 */
enum {
	/** NVENC engine enum */
	NVGPU_MULTIMEDIA_ENGINE_NVENC = 0U,
	/** NVENC1 engine enum */
	NVGPU_MULTIMEDIA_ENGINE_NVENC1 = 1U,
	/** OFA engine enum */
	NVGPU_MULTIMEDIA_ENGINE_OFA = 2U,
	/** NVDEC engine enum */
	NVGPU_MULTIMEDIA_ENGINE_NVDEC = 3U,
	/** NVDEC1 engine enum */
	NVGPU_MULTIMEDIA_ENGINE_NVDEC1 = 4U,
	/** NVJPG engine enum */
	NVGPU_MULTIMEDIA_ENGINE_NVJPG = 5U,
	/** NVJPG1 engine enum */
	NVGPU_MULTIMEDIA_ENGINE_NVJPG1 = 6U,
	/** Invalid engine enum */
	NVGPU_MULTIMEDIA_ENGINE_MAX = 7U
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
const struct nvgpu_device *nvgpu_multimedia_get_dev(struct gk20a *g, u32 multimedia_id);
const char *nvgpu_multimedia_get_eng_name(u32 multimedia_id);
s32 nvgpu_multimedia_sw_init(struct gk20a *g);
s32 nvgpu_multimedia_bootstrap(struct gk20a *g);
void nvgpu_multimedia_deinit(struct gk20a *g);
bool nvgpu_get_multimedia_fw_names(struct gk20a *g, u32 dev_type,
		const char **desc_fw_name, const char **image_fw_name);

#endif
