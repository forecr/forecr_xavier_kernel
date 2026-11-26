/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef CLK_MON_TU104_H
#define CLK_MON_TU104_H

/**
 * FMON register types
 */
#define FMON_THRESHOLD_HIGH				0x0U
#define FMON_THRESHOLD_LOW				0x1U
#define FMON_FAULT_STATUS				0x2U
#define FMON_FAULT_STATUS_PRIV_MASK			0x3U
#define CLK_CLOCK_MON_REG_TYPE_COUNT			0x4U
#define CLK_MON_BITS_PER_BYTE				0x8U
/*
 * The Maximum count of clock domains supported
 */
#define CLK_CLOCK_MON_DOMAIN_COUNT			0x40U


struct clk_domain_mon_status {
	u32 clk_api_domain;
	u32 low_threshold;
	u32 high_threshold;
	u32 clk_domain_fault_status;
};

struct clk_domains_mon_status_params {
	u32 clk_mon_domain_mask;
	struct clk_domain_mon_status
		clk_mon_list[CLK_CLOCK_MON_DOMAIN_COUNT];
};

bool tu104_clk_mon_check_master_fault_status(struct gk20a *g);
int tu104_clk_mon_check_status(struct gk20a *g, unsigned long domain_mask);
bool tu104_clk_mon_check_clk_good(struct gk20a *g);
bool tu104_clk_mon_check_pll_lock(struct gk20a *g);

#endif /* CLK_MON_TU104_H */
