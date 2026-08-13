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

#ifndef NVGPU_ACR_INTERFACE_V2_H
#define NVGPU_ACR_INTERFACE_V2_H

#include "nvgpu_acr_interface.h"

// ID to distinguish WPR component
#define WPR_GENERIC_HEADER_ID_LSF_UCODE_DESC         (1)
#define WPR_GENERIC_HEADER_ID_LSF_WPR_HEADER         (2)
#define WPR_GENERIC_HEADER_ID_LSF_SHARED_SUB_WPR     (3)
#define WPR_GENERIC_HEADER_ID_LSF_LSB_HEADER         (4)
#define WPR_GENERIC_HEADER_ID_ACR_DESC_HEADER        (5)

#define ACR_DESC_GENERIC_HEADER_VERSION_1            (1)
#define LSF_UCODE_DESC_VERSION_2                     (2)
#define LSF_WPR_HEADER_VERSION                       (2)
#define LSF_LSB_HEADER_VERSION_2                     (2)

// The wrapper for LSF_WPR_HEADER
typedef struct
{
    struct acr_generic_header genericHdr;

    union
    {
        struct lsf_wpr_header lsfWprHdr;
    } u;
} LSF_WPR_HEADER_WRAPPER, *PLSF_WPR_HEADER_WRAPPER;

//
// For RSA3K operation, SE engine needs each input component size to be 512 bytes length.
// So even RSA3K signature is 384 bytes, we still need to pad zeros until size is 512.
// Currently, for version 2, we set MAX LS signature size to be 512 as well.
//
#define RSA3K_SIGNATURE_SIZE_BYTE               (384)
#define RSA3K_SIGNATURE_PADDING_SIZE_BYTE       (128)
#define RSA3K_SIGNATURE_PADDED_SIZE_BYTE        (RSA3K_SIGNATURE_SIZE_BYTE + RSA3K_SIGNATURE_PADDING_SIZE_BYTE)
// Size in bytes for RSA 3K (RSA_3K struct from bootrom_pkc_parameters.h
#define RSA3K_PK_SIZE_BYTE                      (2048)

typedef struct
{
    bool bHsFmc; // Whether HS Bootloader/First Mutable Code is used or not. The remaining fields are valid only when this is set.
    u8 padding[3]; // For 32 bit alignment
    u16 pkcAlgo; // Which PKC Algorithm is used. RSA3K for now
    u16 pkcAlgoVersion; // Which version of the PKC Algorithm is used.
    u32 engIdMask;
    u32 ucodeId;
    u32 fuseVer; // fuse version that corresponding to the signature below
    u8 pkcSignature[RSA3K_SIGNATURE_PADDED_SIZE_BYTE]; // Max size, pkcAlgo determines the size of the signature.
    u8 pkcKey[RSA3K_PK_SIZE_BYTE]; // (optional) Max size, pkcAlgo determines the size used. See: pkc_verification_parameters::pk.
    u8 rsvd[4];
} HS_FMC_PARAMS;

typedef struct
{
    bool bHsOvlSigBlobPresent;       // Indicates if Runtime patching of HS Ovl Sig Blob is supported
    u32  hsOvlSigBlobOffset;         // HS Ovl Sig Blob Offset of single version of signature for All HS overlays
    u32  hsOvlSigBlobSize;           // HS Ovl Sig Blob size
} HS_OVL_SIG_BLOB_PARAMS;

typedef struct
{
	/** Code/data signature details of each LS falcon */
	struct lsf_ucode_desc_wrapper signature;
	/**
	 * Offset from non-WPR base where UCODE is located,
	 * Offset = Non-WPR base + #LSF_LSB_HEADER_ALIGNMENT +
	 *          #LSF_UCODE_DATA_ALIGNMENT + ( #LSF_BL_DATA_ALIGNMENT *
	 *          LS Falcon index)
	 */
	u32 ucode_off;
	/**
	 * Size of LS Falcon ucode, required to perform signature verification
	 * of LS Falcon ucode by ACR HS.
	 */
	u32 ucode_size;
	/**
	 * Size of LS Falcon ucode data, required to perform signature
	 * verification of LS Falcon ucode data by ACR HS.
	 */
	u32 data_size;
	/**
	 * Size of bootloader that needs to be loaded by bootstrap owner.
	 *
	 * On GV11B, respective LS Falcon BL code size should not exceed
	 * below mentioned size.
	 * FALCON_ID_FECS IMEM size  - 32k
	 * FALCON_ID_GPCCS IMEM size - 16k
	 */
	u32 bl_code_size;
	/** BL starting virtual address. Need for tagging */
	u32 bl_imem_off;
	/**
	 * Offset from non-WPR base holding the BL data
	 * Offset = (Non-WPR base + #LSF_LSB_HEADER_ALIGNMENT +
	 *          #LSF_UCODE_DATA_ALIGNMENT + #LSF_BL_DATA_ALIGNMENT) *
	 *          #LS Falcon index
	 */
	u32 bl_data_off;
	/**
	 * Size of BL data, BL data will be copied to LS Falcon DMEM of
	 * bl data size
	 *
	 * On GV11B, respective LS Falcon BL data size should not exceed
	 * below mentioned size.
	 * FALCON_ID_FECS DMEM size  - 8k
	 * FALCON_ID_GPCCS DMEM size - 5k
	 */
	u32 bl_data_size;

	u32 manifest_size;
	/**
	 * Offset from non-WPR base address where UCODE Application code is
	 * located.
	 */
	u32 app_code_off;
	/**
	 * Size of UCODE Application code.
	 *
	 * On GV11B, FECS/GPCCS LS Falcon app code size should not exceed
	 * below mentioned size.
	 * FALCON_ID_FECS IMEM size  - 32k
	 * FALCON_ID_GPCCS IMEM size - 16k
	 */
	u32 app_code_size;
	/**
	 * Offset from non-WPR base address where UCODE Application data
	 * is located
	 */
	u32 app_data_off;
	/**
	 * Size of UCODE Application data.
	 *
	 * On GV11B, respective LS Falcon app data size should not exceed
	 * below mentioned size.
	 * FALCON_ID_FECS DMEM size  - 8k
	 * FALCON_ID_GPCCS DMEM size - 5k
	 */
	u32 app_data_size;

	u32 app_imem_off;

	u32 app_dmem_off;
	/**
	 * NV_FLCN_ACR_LSF_FLAG_LOAD_CODE_AT_0 - Load BL at 0th IMEM offset
	 * NV_FLCN_ACR_LSF_FLAG_DMACTL_REQ_CTX - This falcon requires a ctx
	 * before issuing DMAs.
	 * NV_FLCN_ACR_LSF_FLAG_FORCE_PRIV_LOAD - Use priv loading method
	 * instead of bootloader/DMAs
	 */
	u32 flags;

	u32 monitor_code_off;

	u32 monitor_data_off;

	u32 manifest_off;

	HS_FMC_PARAMS hs_fmc_params;

	HS_OVL_SIG_BLOB_PARAMS hs_ovl_sig_blob_params;

	u8 rsvd[20];
}LSF_LSB_HEADER_V2, *PLSF_LSB_HEADER_V2;

// The wrapper for LSF_LSB_HEADER
typedef struct
{
    struct acr_generic_header genericHdr;
    union
    {
        LSF_LSB_HEADER_V2 lsfLsbHdrV2;
    } u;
} LSF_LSB_HEADER_WRAPPER, *PLSF_LSB_HEADER_WRAPPER;

typedef struct
{
    u32   regionID;
    u32   startAddress;
    u32   endAddress;
    u64   nonWprBlobStart;
    u32   nonWprBlobSize;
    u32   reserved[4];
} RM_RISCV_ACR_REGION_DESC, *PRM_RISCV_ACR_REGION_DESC;

typedef struct
{
    RM_RISCV_ACR_REGION_DESC regionDesc[2];
    u32 mode;
    u32 reserved[16];
} RM_RISCV_ACR_DESC_V1, *PRM_RISCV_ACR_DESC_V1;

typedef struct
{
    struct acr_generic_header genericHdr;;
    union
    {
        RM_RISCV_ACR_DESC_V1 riscvAcrDescV1;
    } acrDesc;
} RM_RISCV_ACR_DESC_WRAPPER, *PRM_RISCV_ACR_DESC_WRAPPER;

/**
 * Maximum WPR Header size
 */
#define LSF_WPR_HEADERS_WRAPPER_TOTAL_SIZE_MAX	\
	(ALIGN_UP(((u32)sizeof(LSF_WPR_HEADER_WRAPPER) * FALCON_ID_END), \
		LSF_WPR_HEADER_ALIGNMENT))

// Wrapper and internal structs size defines
#define ACR_GENERIC_HEADER_SIZE_BYTE                           sizeof(struct acr_generic_header)

#define LSF_LSB_HEADER_V2_SIZE_BYTE                            sizeof(LSF_LSB_HEADER_V2)
#define RM_RISCV_ACR_DESC_V1_SIZE_BYTE                         sizeof(RM_RISCV_ACR_DESC_V1)
#define LSF_WPR_HEADER_SIZE_BYTE                               sizeof(struct lsf_wpr_header)
#define LSF_UCODE_DESC_V2_SIZE_BYTE                            sizeof(struct lsf_ucode_desc_v2)

#define LSF_UCODE_DESC_WRAPPER_V2_SIZE_BYTE                    LSF_UCODE_DESC_V2_SIZE_BYTE + ACR_GENERIC_HEADER_SIZE_BYTE
#define LSF_WPR_HEADER_WRAPPER_SIZE_BYTE                       LSF_WPR_HEADER_SIZE_BYTE + ACR_GENERIC_HEADER_SIZE_BYTE
#define RM_RISCV_ACR_DESC_V1_WRAPPER_SIZE_BYTE                 RM_RISCV_ACR_DESC_V1_SIZE_BYTE + ACR_GENERIC_HEADER_SIZE_BYTE
#define LSF_LSB_HEADER_WRAPPER_V2_SIZE_BYTE                    LSF_LSB_HEADER_V2_SIZE_BYTE + ACR_GENERIC_HEADER_SIZE_BYTE

#endif /* NVGPU_ACR_INTERFACE_V2_H */
