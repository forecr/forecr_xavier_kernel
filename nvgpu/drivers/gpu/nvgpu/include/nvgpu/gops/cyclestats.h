/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_CYCLESTATS_H
#define NVGPU_GOPS_CYCLESTATS_H

#ifdef CONFIG_NVGPU_CYCLESTATS
struct gops_css {
	int (*enable_snapshot)(struct gk20a *g,
			struct gk20a_cs_snapshot_client *client);
	void (*disable_snapshot)(struct gk20a *g);
	int (*check_data_available)(struct nvgpu_channel *ch,
					u32 *pending,
					bool *hw_overflow);
	void (*set_handled_snapshots)(struct gk20a *g, u32 num);
	u32 (*allocate_perfmon_ids)(struct gk20a_cs_snapshot *data,
					u32 count);
	u32 (*release_perfmon_ids)(struct gk20a_cs_snapshot *data,
					u32 start,
					u32 count);
	int (*detach_snapshot)(struct gk20a *g,
			struct gk20a_cs_snapshot_client *client);
	bool (*get_overflow_status)(struct gk20a *g);
	u32 (*get_pending_snapshots)(struct gk20a *g);
	u32 (*get_max_buffer_size)(struct gk20a *g);
};
#endif

#endif /* NVGPU_GOPS_CYCLESTATS_H */
