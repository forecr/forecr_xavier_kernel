/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _TEGRA_AGIC_H_
#define _TEGRA_AGIC_H_

#include <linux/irqchip/tegra-t18x-agic.h>
#include <linux/irqchip/tegra-t210-agic.h>

enum tegra_agic_cpu {
	TEGRA_AGIC_T210_APE_HOST = 0,
	TEGRA_AGIC_T210_ADSP = 1,
	MAX_AGIC_T210_INTERFACES = 2,
	TEGRA_AGIC_T18x_APE_HOST0 = 0,
	TEGRA_AGIC_T18x_APE_HOST1 = 1,
	TEGRA_AGIC_T18x_APE_HOST2 = 2,
	TEGRA_AGIC_T18x_APE_HOST3 = 3,
	TEGRA_AGIC_T18x_ADSP = 4,
	MAX_AGIC_T18x_INTERFACES = 5,
};

extern int tegra_agic_route_interrupt(int irq, enum tegra_agic_cpu cpu);
extern bool tegra_agic_irq_is_active(int irq);
extern bool tegra_agic_irq_is_pending(int irq);
extern void tegra_agic_clear_pending(int irq);
extern void tegra_agic_clear_active(int irq);
#endif /* _TEGRA_AGIC_H_ */
