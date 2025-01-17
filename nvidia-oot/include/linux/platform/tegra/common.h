/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __MACH_TEGRA_COMMON_H
#define __MACH_TEGRA_COMMON_H

extern struct smp_operations tegra_smp_ops;

extern phys_addr_t tegra_tsec_start;
extern phys_addr_t tegra_tsec_size;

#ifdef CONFIG_CACHE_L2X0
void tegra_init_cache(bool init);
#else
static inline void tegra_init_cache(bool init) {}
#endif

extern void tegra_cpu_die(unsigned int cpu);
extern int tegra_cpu_kill(unsigned int cpu);
extern phys_addr_t tegra_avp_kernel_start;
extern phys_addr_t tegra_avp_kernel_size;
void ahb_gizmo_writel(unsigned long val, void __iomem *reg);

extern struct device tegra_generic_cma_dev;
extern struct device tegra_vpr_cma_dev;
extern int tegra_with_secure_firmware;

extern struct device tegra_generic_dev;
extern struct device tegra_vpr_dev;
#ifdef CONFIG_TEGRA_VPR
extern struct dma_resize_notifier_ops vpr_dev_ops;
#endif

u32 tegra_get_bct_strapping(void);
u32 tegra_get_fuse_opt_subrevision(void);
enum tegra_revision tegra_chip_get_revision(void);
void __init display_tegra_dt_info(void);

bool tegra_is_vpr_resize_enabled(void);
void tegra_register_idle_unidle(int (*do_idle)(void *),
				int (*do_unidle)(void *),
				void *data);
void tegra_unregister_idle_unidle(int (*do_idle)(void *));

static inline int tegra_cpu_is_secure(void)
{
	return tegra_with_secure_firmware;
}

int tegra_state_idx_from_name(char *state_name);
#endif
