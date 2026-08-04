/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GSP_TEST_H
#define GSP_TEST_H

#define GSP_TEST_DEBUG_BUFFER_QUEUE	3U
#define GSP_TEST_DMESG_BUFFER_SIZE	0xC00U

#define GSPDBG_RISCV_STRESS_TEST_FW_MANIFEST  "gsp-stress.manifest.encrypt.bin.out.bin"
#define GSPDBG_RISCV_STRESS_TEST_FW_CODE      "gsp-stress.text.encrypt.bin"
#define GSPDBG_RISCV_STRESS_TEST_FW_DATA      "gsp-stress.data.encrypt.bin"

#define GSPPROD_RISCV_STRESS_TEST_FW_MANIFEST  "gsp-stress.manifest.encrypt.bin.out.bin.prod"
#define GSPPROD_RISCV_STRESS_TEST_FW_CODE      "gsp-stress.text.encrypt.bin.prod"
#define GSPPROD_RISCV_STRESS_TEST_FW_DATA      "gsp-stress.data.encrypt.bin.prod"

#define GSP_STRESS_TEST_MAILBOX_PASS 0xAAAAAAAA

struct gsp_stress_test {
	bool load_stress_test;
	bool enable_stress_test;
	bool stress_test_fail_status;
	u32 test_iterations;
	u32 test_name;
	struct nvgpu_mem gsp_test_sysmem_block;
};

/* GSP descriptor's */
struct nvgpu_gsp_test {
	struct nvgpu_gsp *gsp;
	struct gsp_stress_test gsp_test;
};

#endif /* GSP_TEST_H */
