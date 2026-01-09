/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

struct gk20a;
struct nvgpu_channel;
struct nvgpu_runlist;
struct nvgpu_runlist_domain;

int vgpu_runlist_update(struct gk20a *g, struct nvgpu_runlist *rl,
			struct nvgpu_channel *ch,
			bool add, bool wait_for_finish);
int vgpu_runlist_reload(struct gk20a *g, struct nvgpu_runlist *rl,
				struct nvgpu_runlist_domain *domain,
				bool add, bool wait_for_finish);
u32 vgpu_runlist_length_max(struct gk20a *g);
u32 vgpu_runlist_entry_size(struct gk20a *g);
