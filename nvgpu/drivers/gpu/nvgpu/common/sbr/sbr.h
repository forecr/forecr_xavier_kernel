/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_COMMON_SBR_H_
#define NVGPU_COMMON_SBR_H_

#define PUB_PROD_BIN		"pub.bin"
#define PUB_DBG_BIN		"pub_dbg.bin"
#define PUB_TIMEOUT		100U /* msec */

struct pub_bin_hdr {
	u32 bin_magic;
	u32 bin_ver;
	u32 bin_size;
	u32 header_offset;
	u32 data_offset;
	u32 data_size;
};

struct pub_fw_header {
	u32 sig_dbg_offset;
	u32 sig_dbg_size;
	u32 sig_prod_offset;
	u32 sig_prod_size;
	u32 patch_loc;
	u32 patch_sig;
	u32 hdr_offset;
	u32 hdr_size;
};

#endif /* NVGPU_COMMON_SBR_H_ */
