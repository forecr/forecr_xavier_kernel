/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_TRACE_GK20A_H
#define NVGPU_POSIX_TRACE_GK20A_H

#include <nvgpu/types.h>

static inline void trace_gk20a_mm_fb_flush(const char *name)
{
	(void)name;
}

static inline void trace_gk20a_mm_fb_flush_done(const char *name)
{
	(void)name;
}

static inline void trace_gk20a_mm_l2_invalidate(const char *name)
{
	(void)name;
}

static inline void trace_gk20a_mm_l2_invalidate_done(const char *name)
{
	(void)name;
}

static inline void trace_gk20a_mm_l2_flush(const char *name)
{
	(void)name;
}

static inline void trace_gk20a_mm_l2_flush_done(const char *name)
{
	(void)name;
}

static inline void trace_nvgpu_channel_open_new(u32 chid)
{
	(void)chid;
}
static inline void trace_gk20a_release_used_channel(u32 chid)
{
	(void)chid;
}
static inline void trace_nvgpu_channel_get(u32 chid, const char *caller)
{
	(void)chid;
	(void)caller;
}
static inline void trace_nvgpu_channel_put(u32 chid, const char *caller)
{
	(void)chid;
	(void)caller;
}
static inline void trace_gk20a_free_channel(u32 chid)
{
	(void)chid;
}
static inline void trace_nvgpu_channel_update(u32 chid)
{
	(void)chid;
}
static inline void trace_gk20a_mmu_fault(u64 fault_addr,
		     u32 fault_type,
		     u32 access_type,
		     u64 inst_ptr,
		     u32 engine_id,
		     const char *client_type_desc,
		     const char *client_id_desc,
		     const char *fault_type_desc)
{
	(void)fault_addr;
	(void)fault_type;
	(void)access_type;
	(void)inst_ptr;
	(void)engine_id;
	(void)client_type_desc;
	(void)client_id_desc;
	(void)fault_type_desc;
}
#ifdef CONFIG_NVGPU_COMPRESSION
static inline void trace_gk20a_ltc_cbc_ctrl_start(const char *name,
		u32 cbc_ctrl, u32 min_value, u32 max_value)
{
	(void)name;
	(void)cbc_ctrl;
	(void)min_value;
	(void)max_value;
}
static inline void trace_gk20a_ltc_cbc_ctrl_done(const char *name)
{
	(void)name;
}
#endif
static inline void trace_gk20a_mm_tlb_invalidate(const char *name)
{
	(void)name;
}
static inline void trace_gk20a_mm_tlb_invalidate_done(const char *name)
{
	(void)name;
}
static inline void trace_gk20a_channel_reset(u32 chid, u32 tsgid)
{
	(void)chid;
	(void)tsgid;
}

static inline void trace_gk20a_channel_submit_gpfifo(const char *name,
		u32 chid,
		u32 num_entries,
		u32 flags,
		u32 wait_id,
		u32 wait_value)
{
	(void)name;
	(void)chid;
	(void)num_entries;
	(void)flags;
	(void)wait_id;
	(void)wait_value;
}
static inline void trace_gk20a_channel_submitted_gpfifo(const char *name,
		u32 chid,
		u32 num_entries,
		u32 flags,
		u32 incr_id,
		u32 incr_value)
{
	(void)name;
	(void)chid;
	(void)num_entries;
	(void)flags;
	(void)incr_id;
	(void)incr_value;
}
static inline void trace_gk20a_push_cmdbuf(const char *name,
		u32 mem_id,
		u32 words,
		u32 offset,
		void *cmdbuf)
{
	(void)name;
	(void)mem_id;
	(void)words;
	(void)offset;
	(void)cmdbuf;
}
#endif
