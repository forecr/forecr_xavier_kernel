/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_SEC2_H
#define NVGPU_GOPS_SEC2_H

#include <nvgpu/types.h>

struct gk20a;

struct gops_sec2 {
	int (*init_sec2_setup_sw)(struct gk20a *g);
	int (*init_sec2_support)(struct gk20a *g);
	int (*sec2_destroy)(struct gk20a *g);
	void (*secured_sec2_start)(struct gk20a *g);
	void (*enable_irq)(struct nvgpu_sec2 *sec2, bool enable);
	bool (*is_interrupted)(struct nvgpu_sec2 *sec2);
	u32 (*get_intr)(struct gk20a *g);
	bool (*msg_intr_received)(struct gk20a *g);
	void (*set_msg_intr)(struct gk20a *g);
	void (*clr_intr)(struct gk20a *g, u32 intr);
	void (*process_intr)(struct gk20a *g, struct nvgpu_sec2 *sec2);
	void (*msgq_tail)(struct gk20a *g, struct nvgpu_sec2 *sec2,
		u32 *tail, bool set);
	u32 (*falcon_base_addr)(void);
	int (*sec2_reset)(struct gk20a *g, u32 inst_id);
	int (*sec2_copy_to_emem)(struct gk20a *g, u32 dst,
				 u8 *src, u32 size, u8 port);
	int (*sec2_copy_from_emem)(struct gk20a *g,
				   u32 src, u8 *dst, u32 size, u8 port);
	int (*sec2_queue_head)(struct gk20a *g,
			       u32 queue_id, u32 queue_index,
			       u32 *head, bool set);
	int (*sec2_queue_tail)(struct gk20a *g,
			       u32 queue_id, u32 queue_index,
			       u32 *tail, bool set);
	void (*flcn_setup_boot_config)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_SEC2_H */
