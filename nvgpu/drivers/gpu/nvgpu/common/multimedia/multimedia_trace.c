// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/dma.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/nvenc.h>
#include <nvgpu/ofa.h>
#include <nvgpu/nvdec.h>
#include <nvgpu/nvjpg.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/string.h>
#include <nvgpu/channel.h>
#include "multimedia_trace_priv.h"

/* Engine trace buffer size. 256-byte aligned */
#define MULTIMEDIA_TRACE_BUF_SIZE	(65536U)

/* Virtual memory of the trace buffer is given to engine */
#define MULTIMEDIA_TRACE_MEM_TYPE_VIRT	(1)

/*
 * Allocate a scratch pad for variable data. Variable data can include log message strings,
 * binary data, or opaque data that firmware may share with application for debug purpose.
 * It is just a placeholder for now, needed functionality can be implemented later
 * based on the requirements.
 */
static u8 trace_variable_data[MULTIMEDIA_ENGINE_TRACE_MAX_VARIABLE_DATA_SIZE];

/* Unique context id generation */
static nvgpu_atomic_t multimedia_ctx_id = NVGPU_ATOMIC_INIT(0);
static u32 generate_multimedia_ctx_id(void)
{
	return (u32)nvgpu_atomic_add_return(1, &multimedia_ctx_id);
}

int nvgpu_multimedia_alloc_trace_buf_mem(struct gk20a *g, struct nvgpu_mem *trace_buf_mem)
{
	int err = 0;
	struct nvgpu_multimedia_trace_buf *trace_buf_ptr;

	if (!nvgpu_mem_is_valid(trace_buf_mem)) {
		err = nvgpu_dma_alloc_sys(g, MULTIMEDIA_TRACE_BUF_SIZE, trace_buf_mem);
		if (err != 0) {
			nvgpu_err(g, "failed to allocate mem for trace buffer");
			return -ENOMEM;
		}

		/* Clear the trace buffer memory */
		nvgpu_memset(g, trace_buf_mem, 0U, 0U, MULTIMEDIA_TRACE_BUF_SIZE);

		trace_buf_ptr = trace_buf_mem->cpu_va;
		if (trace_buf_ptr == NULL) {
			nvgpu_err(g, "no valid trace buffer ptr");
			return -ENOMEM;
		}

		trace_buf_ptr->buffer_size = MULTIMEDIA_TRACE_BUF_SIZE -
						sizeof(struct nvgpu_multimedia_trace_buf);
		trace_buf_ptr->read_ptr = 0;
		trace_buf_ptr->write_ptr = 0;
		trace_buf_ptr->flags = 0;
	}

	return err;
}

void nvgpu_multimedia_free_trace_buf_mem(struct gk20a *g, struct nvgpu_mem *trace_buf_mem)
{
	if (nvgpu_mem_is_valid(trace_buf_mem)) {
		nvgpu_dma_free(g, trace_buf_mem);
	}
}

static struct nvgpu_mem *nvgpu_multimedia_get_trace_mem(struct gk20a *g,
							const struct nvgpu_device *dev)
{
	struct nvgpu_mem *mem = NULL;

	switch (dev->type) {
	case NVGPU_DEVTYPE_NVENC:
		if (g->nvenc) {
			mem = &g->nvenc->trace_buf_mem[dev->inst_id];
		}
		break;

	case NVGPU_DEVTYPE_OFA:
		if (g->ofa) {
			mem = &g->ofa->trace_buf_mem[dev->inst_id];
		}
		break;

	case NVGPU_DEVTYPE_NVDEC:
		if (g->nvdec) {
			mem = &g->nvdec->trace_buf_mem[dev->inst_id];
		}
		break;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	case NVGPU_DEVTYPE_NVJPG:
		if (g->nvjpg) {
			mem = &g->nvjpg->trace_buf_mem[dev->inst_id];
		}
		break;
#endif

	default:
		mem = NULL;
		break;
	}

	return mem;
}

int nvgpu_multimedia_trace_ctx_init(struct nvgpu_channel *ch, struct nvgpu_mem *ctx_mem,
		const struct nvgpu_device *dev, u64 *gpu_va)
{
	struct nvgpu_mem *trace_buf_mem;
	struct nvgpu_multimedia_log_info log_info = {0};
	struct gk20a *g = ch->g;
	int err = 0;

	if (dev == NULL) {
		return  -ENODEV;
	}

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_MULTIMEDIA_TRACE) ||
			!nvgpu_multimedia_is_trace_enabled(g, dev)) {
		/* Skip the mapping and sharing of the trace buffer */
		return 0;
	}

	/* Check if log info data fits in the ctx buffer */
	if (ctx_mem->size < (MULTIMEDIA_ENGINE_TRACE_LOG_INFO_OFFSET +
				MULTIMEDIA_ENGINE_TRACE_LOG_INFO_SIZE)) {
		return -ENOMEM;
	}

	/* Fill channel specific information for tracing */
	log_info.pid = (u32)ch->pid;
	log_info.context_id = generate_multimedia_ctx_id();
	log_info.engine_id = (u8)dev->inst_id;

	/* Initialize client information in the context buffer */
	nvgpu_mem_wr_n(g, ctx_mem, MULTIMEDIA_ENGINE_TRACE_LOG_INFO_OFFSET,
			&log_info, sizeof(struct nvgpu_multimedia_log_info));

	trace_buf_mem = nvgpu_multimedia_get_trace_mem(g, dev);
	if ((trace_buf_mem == NULL) || !nvgpu_mem_is_valid(trace_buf_mem)) {
		nvgpu_err(g, "no valid trace buffer for dev type %u inst %u",
				dev->type, dev->inst_id);
		err = -ENOMEM;
		goto done;
	}

	*gpu_va = nvgpu_gmmu_map(ch->vm, trace_buf_mem,
				0, /* not GPU-cacheable */
				gk20a_mem_flag_none, false,
				trace_buf_mem->aperture);
	if (*gpu_va == 0ULL) {
		nvgpu_err(g, "failed to map multimedia trace buffer");
		err = -ENOMEM;
		goto done;
	}

	nvgpu_mem_wr(g, ctx_mem, MULTIMEDIA_ENGINE_TRACE_ADDR_OFFSET_LO, u64_lo32(*gpu_va));
	nvgpu_mem_wr(g, ctx_mem, MULTIMEDIA_ENGINE_TRACE_ADDR_OFFSET_HI, u64_hi32(*gpu_va));
	nvgpu_mem_wr(g, ctx_mem, MULTIMEDIA_ENGINE_TRACE_ADDR_MEM_TARGET,
						MULTIMEDIA_TRACE_MEM_TYPE_VIRT);
	nvgpu_log(g, gpu_dbg_mme, "trace ctx (dev type %u inst %u): addr 0x%llx context_id %u pid %u",
				dev->type, dev->inst_id, *gpu_va, log_info.context_id, log_info.pid);

done:
	return err;
}

void nvgpu_multimedia_unmap_trace_buf(struct gk20a *g, struct vm_gk20a *vm,
					const struct nvgpu_device *dev, u64 gpu_va)
{
	struct nvgpu_mem *trace_buf_mem;

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_MULTIMEDIA_TRACE)) {
		/* Skip the unmapping of the trace buffer */
		return;
	}

	trace_buf_mem = nvgpu_multimedia_get_trace_mem(g, dev);

	if ((trace_buf_mem != NULL) && nvgpu_mem_is_valid(trace_buf_mem) && gpu_va) {
		nvgpu_gmmu_unmap_addr(vm, trace_buf_mem, gpu_va);
	}
}

int nvgpu_multimedia_trace_enable(struct gk20a *g, const struct nvgpu_device *dev, bool enable)
{
	struct nvgpu_multimedia_trace_buf *trace_buf_ptr;
	struct nvgpu_mem *trace_mem;

	trace_mem = nvgpu_multimedia_get_trace_mem(g, dev);
	if ((trace_mem == NULL) || !nvgpu_mem_is_valid(trace_mem)) {
		nvgpu_err(g, "no valid multimedia (dev type %u inst %u) trace buffer",
				dev->type, dev->inst_id);
		return -ENOMEM;
	}

	trace_buf_ptr = trace_mem->cpu_va;
	if (trace_buf_ptr == NULL) {
		nvgpu_err(g, "no valid multimedia (dev type %u inst %u) trace buffer ptr",
				dev->type, dev->inst_id);
		return -ENOMEM;
	}

	trace_buf_ptr->flags = enable;

	return 0;
}

bool nvgpu_multimedia_is_trace_enabled(struct gk20a *g, const struct nvgpu_device *dev)
{
	struct nvgpu_multimedia_trace_buf *trace_buf_ptr;
	struct nvgpu_mem *trace_mem;

	trace_mem = nvgpu_multimedia_get_trace_mem(g, dev);
	if ((trace_mem == NULL) || !nvgpu_mem_is_valid(trace_mem)) {
		nvgpu_log(g, gpu_dbg_mme, "no valid multimedia (dev type %u inst %u) trace buffer",
				dev->type, dev->inst_id);
		return false;
	}

	trace_buf_ptr = trace_mem->cpu_va;
	if (trace_buf_ptr == NULL) {
		nvgpu_log(g, gpu_dbg_mme, "no valid multimedia (dev type %u inst %u) trace buffer ptr",
				dev->type, dev->inst_id);
		return false;
	}

	return trace_buf_ptr->flags;
}

/* This function is used to get the data size of a ring buffer */
static u32 nvgpu_multimedia_trace_get_data_size(struct nvgpu_multimedia_trace_buf *trace_buf_ptr)
{
	u32 read_ptr;
	u32 write_ptr;
	u32 size = 0;

	if (unlikely(!trace_buf_ptr)) {
		return 0;
	}

	/* Read the read/write ptrs first to avoid the need for re-sync between driver and uCode */
	read_ptr = trace_buf_ptr->read_ptr;
	write_ptr = trace_buf_ptr->write_ptr;


	if (write_ptr < read_ptr) {
		/* Cross over 32-bit boundary */
		size = (U32_MAX - read_ptr) + write_ptr + 1;
	} else {
		size = write_ptr - read_ptr;
	}

	if (size > trace_buf_ptr->buffer_size) {
		size = trace_buf_ptr->buffer_size;
	}

	return size;
}

/*
 * This helper function is responsible for freeing the space from ring buffer, by
 * advancing the read pointer by a given size. This function is called to skip the data
 * between next trace entry, and update the read pointer accordingly.
 */
static void nvgpu_multimedia_trace_make_space(u32 old_read_ptr, u32 size,
			struct nvgpu_multimedia_trace_buf *trace_buf_ptr)
{
	u32 has_size = 0;
	u32 old_write_ptr;
	u64 new_read_ptr, adjusted_read_ptr;

	if (unlikely(!trace_buf_ptr)) {
		return;
	}

	/* Read the write ptr first to avoid the need for re-sync between driver and uCode */
	old_write_ptr = trace_buf_ptr->write_ptr;
	adjusted_read_ptr = (u64)old_read_ptr;

	if (old_write_ptr < old_read_ptr) {
		/* Cross over 32-bit boundary */
		has_size = (U32_MAX - old_read_ptr) + old_write_ptr + 1;
	} else {
		has_size = old_write_ptr - old_read_ptr;
	}

	if (has_size > trace_buf_ptr->buffer_size) {
		has_size = trace_buf_ptr->buffer_size;
	}

	/* Make sure we don't pass over the write pointer */
	if (size > has_size) {
		size = has_size;
	}

	/* Newly adjusted read_ptr in 64-bits */
	adjusted_read_ptr += size;

	if (old_read_ptr != trace_buf_ptr->read_ptr) {
		new_read_ptr = 0;
		if (old_read_ptr > trace_buf_ptr->read_ptr) {
			/* 32-bit turn over */
			new_read_ptr = (u64)(trace_buf_ptr->read_ptr) + 0x100000000ULL;
		}
		/* Only adjust the read ptr if newly freed space by other readers is not enough */
		if (adjusted_read_ptr > new_read_ptr) {
			trace_buf_ptr->read_ptr = old_read_ptr + size;
		}
	} else {
		trace_buf_ptr->read_ptr += size;
	}

	return;
}

/*
 * This function is responsible for reading data from ring buffer.
 * It copies the trace data pointed by the read pointer to the given out pointer.
 * Performs single or two-chunk reads for wrapped data. Returns the size of the copied data.
 */
static u32 nvgpu_multimedia_trace_get_data(u8 *data_out_ptr, u32 size_out,
			struct nvgpu_multimedia_trace_buf *trace_buf_ptr)
{
	u32 old_write_ptr, old_read_ptr, used_read_ptr;
	u32 write_offset = 0;
	u32 read_offset = 0;
	u32 size_to_top = 0;
	u32 has_size = 0;

	if (unlikely(data_out_ptr == NULL || trace_buf_ptr == NULL)) {
		return 0;
	}

	/* Read the write ptr first to avoid the need for re-sync between driver and uCode */
	old_write_ptr = trace_buf_ptr->write_ptr;
	old_read_ptr = trace_buf_ptr->read_ptr;
	used_read_ptr = trace_buf_ptr->read_ptr;

	if (old_write_ptr < old_read_ptr) {
		/* Cross over 32-bit boundary */
		has_size = (U32_MAX - old_read_ptr) + old_write_ptr + 1;
	} else {
		has_size = old_write_ptr - old_read_ptr;
	}

	if (has_size >= trace_buf_ptr->buffer_size) {
		/*
		 * The reader is too far behind, the data is over-written and invalid.
		 * Adjust read pointer used.
		 */
		has_size = trace_buf_ptr->buffer_size;
		if (old_write_ptr >= trace_buf_ptr->buffer_size) {
			used_read_ptr = old_write_ptr - trace_buf_ptr->buffer_size;
		} else {
			used_read_ptr =
				(U32_MAX - (trace_buf_ptr->buffer_size - old_write_ptr)) + 1;
		}
	}

	if ((size_out > has_size) || (old_write_ptr == used_read_ptr)) {
		/* Not enough data */
		return 0;
	}

	write_offset = old_write_ptr % trace_buf_ptr->buffer_size;
	read_offset = used_read_ptr % trace_buf_ptr->buffer_size;
	size_to_top = trace_buf_ptr->buffer_size - read_offset;

	if ((write_offset > read_offset) ||
		((write_offset <= read_offset) && (size_out <= size_to_top))) {
		nvgpu_memcpy(data_out_ptr, &(trace_buf_ptr->data[read_offset]), size_out);

		if (trace_buf_ptr->read_ptr == old_read_ptr) {
			trace_buf_ptr->read_ptr = used_read_ptr + size_out;
		} else {
			/* Output data could be corrupted. Invalidate the output by returning 0 */
			size_out = 0;
		}
	} else if ((write_offset <= read_offset) && (size_out > size_to_top)) {
		/* Has the data accross top of the buffer, do 2 chunk read */
		nvgpu_multimedia_trace_get_data(data_out_ptr, size_to_top, trace_buf_ptr);
		nvgpu_multimedia_trace_get_data(&(data_out_ptr[size_to_top]),
						size_out - size_to_top, trace_buf_ptr);
	}

	return size_out;
}

/*
 * This helper function is looking for the starting point of an event record.
 * Searches for 8-byte magic number pattern in the trace buffer and advances
 * read pointer to next valid record. Uses sliding window pattern matching
 * and handles 32-bit wraparound. Empties buffer if magic number not found.
 */
static int nvgpu_multimedia_trace_goto_next_record(struct gk20a *g, u32	magic_hi,
			u32 magic_lo, struct nvgpu_multimedia_trace_buf *trace_buf_ptr)
{
	u32 old_read_ptr, offset, has_size;
	u64 skip_size;
	u32 i = 0;
	union {
		struct {
			u32 lo;
			u32 hi;
		};
		u64 val64bits;
	} magic;

	if (unlikely(!trace_buf_ptr)) {
		return 0;
	}

	old_read_ptr = trace_buf_ptr->read_ptr;
	offset = old_read_ptr;
	has_size = nvgpu_multimedia_trace_get_data_size(trace_buf_ptr);

	if (has_size < 8) {
		/* Not enough size for record magic, clear this */
		offset += has_size;
	} else {
		/* Sliding Window Pattern Matching logic, to find the magic patern */
		magic.hi = (u32)(
			(trace_buf_ptr->data[(offset + 7) % trace_buf_ptr->buffer_size] << 24) +
			(trace_buf_ptr->data[(offset + 6) % trace_buf_ptr->buffer_size] << 16) +
			(trace_buf_ptr->data[(offset + 5) % trace_buf_ptr->buffer_size] << 8) +
			(trace_buf_ptr->data[(offset + 4) % trace_buf_ptr->buffer_size]));
		magic.lo = (u32)(
			(trace_buf_ptr->data[(offset + 3) % trace_buf_ptr->buffer_size] << 24) +
			(trace_buf_ptr->data[(offset + 2) % trace_buf_ptr->buffer_size] << 16) +
			(trace_buf_ptr->data[(offset + 1) % trace_buf_ptr->buffer_size] << 8) +
			(trace_buf_ptr->data[offset % trace_buf_ptr->buffer_size]));

		for (i = 0; i < has_size - 8; i++) {
			if ((magic.lo == magic_lo) && (magic.hi == magic_hi)) {
				break;
			}

			magic.val64bits = (magic.val64bits >> 8) |
				(((u64)(trace_buf_ptr->data[(offset + i + 8) % trace_buf_ptr->buffer_size])) << 56);
		}

		offset += i;
		if (i == (has_size - 8)) {
			/* Did not find magic. Empty the buffer */
			offset += 8;
		}
	}

	if (offset != old_read_ptr) {
		skip_size = 0;
		if (offset < trace_buf_ptr->read_ptr) {
			/* 32-bit turn over */
			skip_size = offset + (U32_MAX - old_read_ptr) + 1UL;
		} else {
			skip_size = offset - old_read_ptr;
		}
		nvgpu_log(g, gpu_dbg_mme, "multimedia trace skip size %llu", skip_size);
		nvgpu_multimedia_trace_make_space(old_read_ptr, (u32)skip_size, trace_buf_ptr);
	}

	return 0;
}

/* This function is called to get one record from a trace buffer. */
static u32 nvgpu_multimedia_trace_get_record(struct gk20a *g,
			struct nvgpu_multimedia_trace_buf *trace_buf_ptr,
			MULTIMEDIA_EVENT_RECORD *record_ptr, u32 magic_hi, u32 magic_lo)
{
	u32 size = 0;
	u32 data_size = 0;

	if (unlikely(record_ptr == NULL || trace_buf_ptr == NULL)) {
		return 0;
	}

	nvgpu_multimedia_trace_goto_next_record(g, magic_hi, magic_lo, trace_buf_ptr);

	size = nvgpu_multimedia_trace_get_data((u8*)record_ptr,
					sizeof(MULTIMEDIA_EVENT_RECORD), trace_buf_ptr);

	if (size != sizeof(MULTIMEDIA_EVENT_RECORD)) {
		return 0;
	}

	if (record_ptr->event_id == MULTIMEDIA_EVENT_ID_LOG_DATA) {
		if (record_ptr->event_log_data.size >
				MULTIMEDIA_ENGINE_TRACE_MAX_VARIABLE_DATA_SIZE) {
			/* Corrupted size */
			return 0;
		}

		/*
		 * TODO: Read the log data similar to trace. Support will be added
		 * to dump from userspace test.
		 * Currently, a static buffer is used as a placeholder.
		 * Per-engine buffer can be used.
		 */
		data_size = nvgpu_multimedia_trace_get_data(trace_variable_data,
					record_ptr->event_log_data.size, trace_buf_ptr);
		if (data_size != record_ptr->event_log_data.size) {
			/* Corrupted data */
			return 0;
		}
		size += data_size;
	}

	/* Got a record and return total size of the data */
	return size;
}

int nvgpu_multimedia_trace_ctrl_ring_read(struct gk20a *g, const struct nvgpu_device *dev,
					u32 multimedia_id)
{
	u32 magic_hi = MULTIMEDIA_ENGINE_TRACE_EVENT_MAGIC_HI;
	u32 magic_lo = MULTIMEDIA_ENGINE_TRACE_EVENT_MAGIC_LO;
	struct nvgpu_multimedia_trace_buf *trace_buf_ptr;
	MULTIMEDIA_EVENT_RECORD event_record;
	u32 old_read_ptr, record_size;
	struct nvgpu_mem *trace_mem;
	int count = 0;
	struct nvgpu_multimedia_trace_entry_internal entry = { };

	trace_mem = nvgpu_multimedia_get_trace_mem(g, dev);
	if (unlikely((trace_mem == NULL) || !nvgpu_mem_is_valid(trace_mem))) {
		nvgpu_err(g, "no valid multimedia (dev type %u inst %u) trace buffer",
					dev->type, dev->inst_id);
		return -ENOMEM;
	}

	trace_buf_ptr = trace_mem->cpu_va;
	if (unlikely(!trace_buf_ptr)) {
		nvgpu_err(g, "no valid multimedia (dev type %u inst %u) trace buffer ptr",
					dev->type, dev->inst_id);
		return -ENOMEM;
	}

	while (nvgpu_multimedia_trace_get_data_size(trace_buf_ptr) >= sizeof(MULTIMEDIA_EVENT_RECORD))
	{
		old_read_ptr = trace_buf_ptr->read_ptr;

		record_size = nvgpu_multimedia_trace_get_record(g,
					  trace_buf_ptr,
					  &event_record,
					  magic_hi,
					  magic_lo);

		if (record_size == 0) {
			continue;
		}

		/* If the read pointer was not moved by us, this record may be invalid */
		if ((event_record.event_id != MULTIMEDIA_EVENT_ID_LOG_DATA) &&
			((old_read_ptr + sizeof(MULTIMEDIA_EVENT_RECORD)) !=
							trace_buf_ptr->read_ptr)) {
			continue;
		}

		nvgpu_log(g, gpu_dbg_mme, "Got a multimedia (dev type %u inst %u) event record of type %u, size %u",
				dev->type, dev->inst_id, event_record.event_id, record_size);
		nvgpu_log(g, gpu_dbg_mme, "seq_no %u timestamp 0x%llx event_id %u context_id %u pid %u gfid %u",
				event_record.seq_no, event_record.ts, event_record.event_id,
				event_record.context_id, event_record.pid, event_record.gfid);

		/* Fill according to userspace entry format */
		entry.tag = nvgpu_multimedia_event_id_to_tag(event_record.event_id);
		entry.vmid = event_record.gfid;
		entry.seqno = event_record.seq_no;
		entry.context_id = event_record.context_id;
		entry.pid = event_record.pid;
		entry.timestamp = event_record.ts;

		nvgpu_multimedia_trace_write_entry(g, multimedia_id, &entry);
		count++;
	}

	/* Wake up the thread that is waiting for a trace event */
	nvgpu_multimedia_trace_wake_up(g, multimedia_id);
	return count;
}
