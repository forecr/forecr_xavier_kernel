// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "debug_c2c.h"
#include "os_linux.h"
#include "platform_gk20a.h"

#include <nvgpu/io.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>

static u32 nvgpu_read_c2c_soc(struct gk20a *g, u32 r)
{
	u32 v;

	v = nvgpu_os_readl(nvgpu_safe_add_u64(g->c2c_soc_regs, r));
	nvgpu_log(g, gpu_dbg_reg, "r=0x%x v=0x%x", r, v);

	return v;
}

static u32 nvgpu_read_c2c_gpu(struct gk20a *g, u32 r)
{
	u32 v;

	v = nvgpu_os_readl(nvgpu_safe_add_u64(g->c2c_gpu_regs, r));
	nvgpu_log(g, gpu_dbg_reg, "r=0x%x v=0x%x", r, v);

	return v;
}

static int get_and_dump_c2c_info(struct seq_file *s, void *data)
{
	struct device *dev = s->private;
	struct gk20a *g = gk20a_get_platform(dev)->g;
	u32 reg_val, gpu_reg_val, link_num;
	bool link_in_hs = false;
	u32 replay_b2b_errs = 0;
	u32 replay_errs = 0;
	u32 crc_errs = 0;
	u32 enabled_links = 0;

	// Check if C2C is in HS
	reg_val = nvgpu_read_c2c_soc(g, NV_PCTCLPI_HS_SWITCH_STATUS_REG);
	if (reg_val == NV_PCTCLPI_HS_SWITCH_STATUS_REG_SUCCESS)
		link_in_hs = true;

	// Remove disabled links from link_mask
	reg_val = ~nvgpu_read_c2c_soc(g, NV_PCTCLPI_CTCLPIS_MISC_LINK_DISABLE_STATUS_0);
	enabled_links = reg_val & NV_PCTCLPI_NUM_LINKS_MASK;

	for_each_set_bit(link_num, (unsigned long *)&enabled_links, 32) {
		reg_val = nvgpu_read_c2c_soc(g, NV_C2C_CRC_COUNT_REG(link_num));
		gpu_reg_val = nvgpu_read_c2c_gpu(g, NV_C2C_CRC_COUNT_REG(link_num));

		crc_errs += (reg_val &
			NV_PCTCLPI_CTCLPI0_DL_RX_DBG2_CRC_COUNT_STATUS_0_COUNT_MASK);
		crc_errs += (gpu_reg_val &
			NV_PCTCLPI_CTCLPI0_DL_RX_DBG2_CRC_COUNT_STATUS_0_COUNT_MASK);

		reg_val = nvgpu_read_c2c_soc(g, NV_C2C_REPLAY_COUNT_REG(link_num));
		gpu_reg_val = nvgpu_read_c2c_gpu(g, NV_C2C_REPLAY_COUNT_REG(link_num));

		replay_errs += (reg_val &
			NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0_COUNT_MASK);
		replay_errs += (gpu_reg_val &
			NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0_COUNT_MASK);

		replay_b2b_errs += ((reg_val >>
			NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0_B2B_FID_COUNT_SHIFT) &
			NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0_B2B_FID_COUNT_MASK);
		replay_b2b_errs += ((gpu_reg_val >>
			NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0_B2B_FID_COUNT_SHIFT) &
			NV_PCTCLPI_CTCLPI0_DL_TX_DBG3_REPLAY_COUNT_STATUS_0_B2B_FID_COUNT_MASK);
	}

	seq_printf(s, "Link_in_HS: %u\n", link_in_hs);
	seq_printf(s, "Total_num_CRC_errors: %u\n", crc_errs);
	seq_printf(s, "Total_num_replay_errors: %u\n", replay_errs);
	seq_printf(s, "Total_num_replay_b2b_errors: %u\n", replay_b2b_errs);

	return 0;
}

static int dump_c2c_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, get_and_dump_c2c_info, inode->i_private);
}

static const struct file_operations dump_c2c_info_fops = {
	.open		= dump_c2c_info_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int nvgpu_init_c2c_regions(struct gk20a *g, struct device *dev)
{
	void __iomem *addr;

	addr = devm_ioremap(dev, NV_PCTCLPI_SOC_BASE_ADDRESS, NV_PCTCLPI_SIZE);
	if (IS_ERR(addr)) {
		nvgpu_err(g, "failed to ioremap c2c soc register space");
		return PTR_ERR(addr);
	}

	g->c2c_soc_regs = (uintptr_t)addr;

	addr = devm_ioremap(dev, NV_PCTCLPI_GPU_BASE_ADDRESS, NV_PCTCLPI_SIZE);
	if (IS_ERR(addr)) {
		nvgpu_err(g, "failed to ioremap c2c gpu register space");
		return PTR_ERR(addr);
	}

	g->c2c_gpu_regs = (uintptr_t)addr;

	return 0;
}

void nvgpu_c2c_debugfs_init(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct device *dev = dev_from_gk20a(g);

	// Dumping C2C regs is supported only for native
	if (g->is_virtual)
		return;

	if (nvgpu_init_c2c_regions(g, dev))
		return;

	debugfs_create_file("c2c_info", S_IRUGO, l->debugfs,
		dev, &dump_c2c_info_fops);
}
