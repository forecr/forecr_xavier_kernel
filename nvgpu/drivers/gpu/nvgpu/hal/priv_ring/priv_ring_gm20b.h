/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PRIV_RING_GM20B_H
#define NVGPU_PRIV_RING_GM20B_H

struct gk20a;

/**
 * @addtogroup unit-common-priv-ring
 * @{
 */

/**
 * @defgroup PRIV_RING_HAL_DEFINES
 *
 * Priv Ring Hal defines
 */

/**
 * @ingroup PRIV_RING_HAL_DEFINES
 * @{
 */

#define COMMAND_CMD_ENUMERATE_AND_START_RING 0x4
#define CONFIG_RING_WAIT_FOR_RING_START_COMPLETE 0x2

#define GM20B_PRIV_RING_POLL_CLEAR_INTR_RETRIES	100
#define GM20B_PRIV_RING_POLL_CLEAR_INTR_UDELAY	20

/**
 * @}
 */

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gm20b_priv_ring_isr(struct gk20a *g);
#endif
int gm20b_priv_ring_enable(struct gk20a *g);
u32 gm20b_priv_ring_enum_ltc(struct gk20a *g);

u32 gm20b_priv_ring_get_gpc_count(struct gk20a *g);
u32 gm20b_priv_ring_get_fbp_count(struct gk20a *g);

#endif /* NVGPU_PRIV_RING_GM20B_H */
