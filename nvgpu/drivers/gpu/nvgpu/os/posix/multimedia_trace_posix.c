// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/multimedia_trace.h>

void nvgpu_multimedia_trace_add_tsg_reset(struct gk20a *g, struct nvgpu_tsg *tsg,
					u32 multimedia_id);
{
	(void)g;
	(void)tsg;
	(void)multimedia_id;
}

int nvgpu_multimedia_trace_write_entry(struct gk20a *g, u32 multimedia_id,
			struct nvgpu_multimedia_trace_entry_internal *entry);
{
	(void)g;
	(void)multimedia_id;
	(void)entry;
	return -EINVAL;
}

u8 nvgpu_multimedia_event_id_to_tag(u8 event_id);
{
	(void)event_id;
	return 0;
}

void nvgpu_multimedia_trace_wake_up(struct gk20a *g, u32 multimedia_id);
{
	(void)g;
	(void)multimedia_id;
}
