/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef MULTIMEDIA_TRACE_PRIV_H
#define MULTIMEDIA_TRACE_PRIV_H

#include <nvgpu/multimedia_trace.h>

#define MULTIMEDIA_ENGINE_TRACE_ADDR_OFFSET_LO		(0xF90UL)
#define MULTIMEDIA_ENGINE_TRACE_ADDR_OFFSET_HI		(0xF94UL)
#define MULTIMEDIA_ENGINE_TRACE_ADDR_MEM_TARGET		(0xF98UL)
#define MULTIMEDIA_ENGINE_TRACE_LOG_INFO_OFFSET		(0xFA0UL)
#define MULTIMEDIA_ENGINE_TRACE_LOG_INFO_SIZE		(0x20UL)
#define MULTIMEDIA_ENGINE_TRACE_MAX_VARIABLE_DATA_SIZE	(0x1000UL)
#define MULTIMEDIA_ENGINE_TRACE_EVENT_MAGIC_HI		(0xdeadbeefUL)
#define MULTIMEDIA_ENGINE_TRACE_EVENT_MAGIC_LO		(0xdead0001UL)
#define MULTIMEDIA_ENGINE_TRACE_FLAG_LOGGING_ENABLED	(0x00000001UL)

struct nvgpu_multimedia_log_info {
	u64 user_info;
	u32 context_id;
	u32 pid;
	u32 gfid;
	u8  engine_id;
	u8  reserved[31];
};

/* Data-structure for trace ring buffer */
struct nvgpu_multimedia_trace_buf {
	u32   buffer_size;
	u32   read_ptr;
	u32   write_ptr;
	u32   flags;
	u8    data[];
};

/* Types of log data for multimedia events */
typedef enum
{
    MULTIMEDIA_EVENT_LOG_DATA_TYPE_GENERIC = 0,  /* Generic opaque data */
    MULTIMEDIA_EVENT_LOG_DATA_TYPE_STR,          /* Log data can be interpret as string */
    MULTIMEDIA_EVENT_LOG_DATA_TYPE_BIN,          /* Log data are binary data */
    MULTIMEDIA_EVENT_LOG_DATA_TYPE_NUM
} MULTIMEDIA_EVENT_LOG_DATA_TYPE;

/* Defines structs for various types of trace events */
typedef struct {
    u8     engine_type;
    u8     engine_id;
    u16    codec_id;
    u32    reserved1;
} MULTIMEDIA_EVENT_SESSION_START;

typedef struct {
    u8     engine_type;
    u8     engine_id;
    u16    codec_id;
    u32    status;
} MULTIMEDIA_EVENT_SESSION_END;

typedef struct {
    u16    from;
    u16    to;
    u32    reserved2;
} MULTIMEDIA_EVENT_POWER_STATE_CHANGE;

typedef struct
{
    u8     engine_type;
    u8     engine_id;
    u16    type;
    u32    size;  /* size of data being logged in u32 */
} MULTIMEDIA_EVENT_LOG_DATA;

/* Defines the struct for a trace record, supporting various event types */
typedef struct
{
    u32                 magic_lo;
    u32                 magic_hi;

    union
    {
        u64             ts;
        struct
        {
            u32         ts_lo;
            u32         ts_hi;
        };
    };

    enum nvgpu_multimedia_event_id           event_id;
    u32                                      seq_no;

    union {
        u64                                  event_data;

        MULTIMEDIA_EVENT_SESSION_START       event_start;
        MULTIMEDIA_EVENT_SESSION_END         event_end;
        MULTIMEDIA_EVENT_POWER_STATE_CHANGE  event_pstate_change;
        MULTIMEDIA_EVENT_LOG_DATA            event_log_data;
    };

    /* Client information from nvgpu */
    u64 user_info;
    u32 context_id;
    u32 pid;
    u64 api_id;
    u32 gfid;
    u32 reserved;

} MULTIMEDIA_EVENT_RECORD;

int nvgpu_multimedia_alloc_trace_buf_mem(struct gk20a *g, struct nvgpu_mem *trace_buf_mem);
void nvgpu_multimedia_free_trace_buf_mem(struct gk20a *g, struct nvgpu_mem *trace_buf_mem);
int nvgpu_multimedia_trace_ctx_init(struct nvgpu_channel *ch, struct nvgpu_mem *ctx_mem,
		const struct nvgpu_device *dev, u64 *gpu_va);
void nvgpu_multimedia_unmap_trace_buf(struct gk20a *g, struct vm_gk20a *vm,
		const struct nvgpu_device *dev, u64 gpu_va);
int nvgpu_multimedia_trace_ctrl_ring_read(struct gk20a *g, const struct nvgpu_device *dev,
		u32 multimedia_id);
int nvgpu_multimedia_trace_enable(struct gk20a *g, const struct nvgpu_device *dev, bool enable);
bool nvgpu_multimedia_is_trace_enabled(struct gk20a *g, const struct nvgpu_device *dev);
#endif /* MULTIMEDIA_TRACE_PRIV_H */
