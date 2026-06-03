/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_FALCON_H
#define NVGPU_GOPS_FALCON_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * Falcon HAL interface.
 */
struct gk20a;

struct gops_falcon {
	/** @cond DOXYGEN_SHOULD_SKIP_THIS */

	int (*falcon_sw_init)(struct gk20a *g, u32 flcn_id);
	void (*falcon_sw_free)(struct gk20a *g, u32 flcn_id);
	void (*reset)(struct nvgpu_falcon *flcn);
	bool (*is_falcon_cpu_halted)(struct nvgpu_falcon *flcn);
	bool (*is_falcon_idle)(struct nvgpu_falcon *flcn);
	bool (*is_falcon_scrubbing_done)(struct nvgpu_falcon *flcn);
	u32 (*get_mem_size)(struct nvgpu_falcon *flcn,
		enum falcon_mem_type mem_type);
	u8 (*get_ports_count)(struct nvgpu_falcon *flcn,
		enum falcon_mem_type mem_type);

	int (*copy_to_dmem)(struct nvgpu_falcon *flcn,
			    u32 dst, u8 *src, u32 size, u8 port);
	int (*copy_to_imem)(struct nvgpu_falcon *flcn,
			    u32 dst, u8 *src, u32 size, u8 port,
			    bool sec, u32 tag);
	void (*set_bcr)(struct nvgpu_falcon *flcn);
	void (*dump_brom_stats)(struct nvgpu_falcon *flcn);
	u32  (*get_brom_retcode)(struct nvgpu_falcon *flcn);
	bool (*is_priv_lockdown)(struct nvgpu_falcon *flcn);
	u32 (*dmemc_blk_mask)(void);
	bool (*check_brom_passed)(u32 retcode);
	bool (*check_brom_failed)(u32 retcode);
	bool (*check_brom_init_done)(u32 retcode);
	void (*brom_config)(struct nvgpu_falcon *flcn, u64 fmc_code_addr,
			u64 fmc_data_addr, u64 manifest_addr);
	u32 (*imemc_blk_field)(u32 blk);
	void (*bootstrap)(struct nvgpu_falcon *flcn,
			 u64 boot_vector);
	u32 (*mailbox_read)(struct nvgpu_falcon *flcn,
			    u32 mailbox_index);
	void (*mailbox_write)(struct nvgpu_falcon *flcn,
			      u32 mailbox_index, u32 data);
	void (*set_irq)(struct nvgpu_falcon *flcn, bool enable,
			u32 intr_mask, u32 intr_dest);
#ifdef CONFIG_NVGPU_FALCON_DEBUG
	void (*dump_falcon_stats)(struct nvgpu_falcon *flcn);
	void (*dump_falcon_info)(struct nvgpu_falcon *flcn);
#endif
	void (*dump_debug_regs)(struct nvgpu_falcon *flcn,
			struct nvgpu_debug_context *o);
#if defined(CONFIG_NVGPU_FALCON_DEBUG) || defined(CONFIG_NVGPU_FALCON_NON_FUSA)
	int (*copy_from_dmem)(struct nvgpu_falcon *flcn,
			      u32 src, u8 *dst, u32 size, u8 port);
#endif
#ifdef CONFIG_NVGPU_FALCON_NON_FUSA
	bool (*clear_halt_interrupt_status)(struct nvgpu_falcon *flcn);
	int (*copy_from_imem)(struct nvgpu_falcon *flcn,
			      u32 src, u8 *dst, u32 size, u8 port);
	void (*get_falcon_ctls)(struct nvgpu_falcon *flcn,
				u32 *sctl, u32 *cpuctl);
#endif

	int (*load_ucode)(struct nvgpu_falcon *flcn,
			struct nvgpu_mem *mem_desc, u32 *ucode_header);
	u32 (*debuginfo_offset)(void);
	void (*write_dmatrfbase)(struct nvgpu_falcon *flcn, u64 dma_base);
	u32 (*video_boot_done_value)(void);
	void (*get_bootplugin_fw_name)(struct gk20a *g, const char **name, u32 falcon_id);

	/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

};

#endif
