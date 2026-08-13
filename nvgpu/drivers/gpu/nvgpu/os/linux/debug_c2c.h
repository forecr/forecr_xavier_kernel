/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_DEBUG_C2C_H__
#define __NVGPU_DEBUG_C2C_H__

#define NV_PCTCLPI_CTCLPIS_MISC_LINK_DISABLE_STATUS_0           0xf3cU
#define NV_PCTCLPI_CTCLPIRISCV_FW_MAILBOX_11                    0xb03cU
#define NV_PCTCLPI_CTCLPI0_DL_RX_DBG2_CRC_COUNT_STATUS_0        0x1434U
#define NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0     0x12b8U

#define NV_PCTCLPI_SOC_BASE_ADDRESS                             0x2C000000
#define NV_PCTCLPI_GPU_BASE_ADDRESS                             0x20000000
#define NV_PCTCLPI_SIZE                                         0x10000
#define NV_PCTCLPI_LINK_STRIDE                                  0x1000
#define NV_PCTCLPI_NUM_LINKS                                    8
#define NV_PCTCLPI_NUM_LINKS_MASK                               0xff
#define NV_PCTCLPI_HS_SWITCH_STATUS_REG                         NV_PCTCLPI_CTCLPIRISCV_FW_MAILBOX_11
#define NV_PCTCLPI_HS_SWITCH_STATUS_REG_SUCCESS                 0x13

#define NV_PCTCLPI_CTCLPI0_DL_RX_DBG2_CRC_COUNT_STATUS_0_COUNT_MASK                     0x0000ffff
#define NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0_COUNT_MASK                  0x0000ffff
#define NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0_B2B_FID_COUNT_SHIFT         16
#define NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0_B2B_FID_COUNT_MASK          0xff

#define NV_C2C_CRC_COUNT_REG(link_num)        (NV_PCTCLPI_CTCLPI0_DL_RX_DBG2_CRC_COUNT_STATUS_0 + (link_num * NV_PCTCLPI_LINK_STRIDE))
#define NV_C2C_REPLAY_COUNT_REG(link_num)     (NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0 + (link_num * NV_PCTCLPI_LINK_STRIDE))

struct gk20a;

void nvgpu_c2c_debugfs_init(struct gk20a *g);

#endif /* __NVGPU_DEBUG_C2C_H__ */
