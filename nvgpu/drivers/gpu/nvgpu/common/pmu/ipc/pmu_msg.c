// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/pmu/allocator.h>
#include <nvgpu/engine_fb_queue.h>
#include <nvgpu/engine_queue.h>
#include <nvgpu/pmu/msg.h>
#include <nvgpu/string.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/pmu/lsfm.h>
#include <nvgpu/pmu/super_surface.h>
#include <nvgpu/pmu/pmu_perfmon.h>
#include <nvgpu/pmu/pmu_pg.h>
#include <nvgpu/pmu/fw.h>
#include <nvgpu/pmu/seq.h>
#include <nvgpu/pmu/pmu_model_20.h>
#include <nvgpu/pmu/pmuif/init.h>
#include <nvgpu/pmu/pmuif/cmn.h>
#include <nvgpu/pmu/pmuif/cmdmgmt.h>
#include <nvgpu/io.h>
#include <nvgpu/hw/gk20a/hw_pwr_gk20a.h>

static int pmu_payload_extract(struct nvgpu_pmu *pmu, struct pmu_sequence *seq)
{
	struct nvgpu_engine_fb_queue *fb_queue =
				nvgpu_pmu_seq_get_cmd_queue(seq);
	struct gk20a *g = pmu->g;
	struct pmu_fw_ver_ops *fw_ops = &g->pmu->fw->ops;
	u32 fbq_payload_offset = 0U;
	int err = 0;

	nvgpu_log_fn(g, " ");

	if (nvgpu_pmu_seq_get_out_payload_fb_queue(seq)) {
		fbq_payload_offset =
			nvgpu_engine_fb_queue_get_offset(fb_queue) +
			nvgpu_pmu_seq_get_fbq_out_offset(seq) +
			(nvgpu_pmu_seq_get_fbq_element_index(seq) *
			 nvgpu_engine_fb_queue_get_element_size(fb_queue));

		nvgpu_mem_rd_n(g, nvgpu_pmu_super_surface_mem(g,
			pmu, pmu->super_surface), fbq_payload_offset,
			nvgpu_pmu_seq_get_out_payload(seq),
			fw_ops->allocation_get_dmem_size(pmu,
				fw_ops->get_seq_out_alloc_ptr(seq)));

	} else {
		if (fw_ops->allocation_get_dmem_size(pmu,
				fw_ops->get_seq_out_alloc_ptr(seq)) != 0U) {
			err = nvgpu_falcon_copy_from_dmem(pmu->flcn,
					fw_ops->allocation_get_dmem_offset(pmu,
					  fw_ops->get_seq_out_alloc_ptr(seq)),
					nvgpu_pmu_seq_get_out_payload(seq),
					fw_ops->allocation_get_dmem_size(pmu,
					  fw_ops->get_seq_out_alloc_ptr(seq)),
					0);
			if (err != 0) {
				nvgpu_err(g, "PMU falcon DMEM copy failed");
				return err;
			}
		}
	}

	return err;
}

static void pmu_payload_free(struct nvgpu_pmu *pmu, struct pmu_sequence *seq)
{
	struct nvgpu_engine_fb_queue *fb_queue =
				nvgpu_pmu_seq_get_cmd_queue(seq);
	struct gk20a *g = pmu->g;
	struct pmu_fw_ver_ops *fw_ops = &g->pmu->fw->ops;
	void *seq_in_ptr = fw_ops->get_seq_in_alloc_ptr(seq);
	void *seq_out_ptr = fw_ops->get_seq_out_alloc_ptr(seq);
	int err;

	nvgpu_log_fn(g, " ");

	if (nvgpu_pmu_fb_queue_enabled(&pmu->queues)) {
		/* Check for allocator pointer and proceed */
		if (pmu->dmem.priv != NULL) {
			nvgpu_free(&pmu->dmem,
				nvgpu_pmu_seq_get_fbq_heap_offset(seq));
		}

		/*
		 * free FBQ allocated work buffer
		 * set FBQ element work buffer to NULL
		 * Clear the in use bit for the queue entry this CMD used.
		 */
		err = nvgpu_engine_fb_queue_free_element(fb_queue,
				nvgpu_pmu_seq_get_fbq_element_index(seq));
		if (err != 0) {
			nvgpu_err(g, "fb queue element free failed %d", err);
		}
	} else {
		/* free DMEM space payload*/
		if (fw_ops->allocation_get_dmem_size(pmu,
			seq_in_ptr) != 0U) {
			nvgpu_free(&pmu->dmem,
				fw_ops->allocation_get_dmem_offset(pmu,
					seq_in_ptr));

			fw_ops->allocation_set_dmem_size(pmu,
				seq_in_ptr, 0);
		}

		if (fw_ops->allocation_get_dmem_size(pmu,
			seq_out_ptr) != 0U) {
			nvgpu_free(&pmu->dmem,
				fw_ops->allocation_get_dmem_offset(pmu,
					seq_out_ptr));

			fw_ops->allocation_set_dmem_size(pmu,
				seq_out_ptr, 0);
		}
	}

	nvgpu_pmu_seq_payload_free(g, seq);
}

static int pmu_response_handle(struct nvgpu_pmu *pmu,
			struct pmu_msg *msg)
{
	struct gk20a *g = pmu->g;
	enum pmu_seq_state state;
	struct pmu_sequence *seq;
	int err = 0;
	u8 id;

	nvgpu_log_fn(g, " ");

	seq = nvgpu_pmu_sequences_get_seq(pmu->sequences, msg->hdr.seq_id);
	state = nvgpu_pmu_seq_get_state(seq);
	id = nvgpu_pmu_seq_get_id(seq);

	if (state != PMU_SEQ_STATE_USED) {
		nvgpu_err(g, "msg for an unknown sequence %u", (u32) id);
		err = -EINVAL;
		goto exit;
	}

	if (msg->hdr.unit_id == PMU_UNIT_RC &&
		msg->msg.rc.msg_type == PMU_RC_MSG_TYPE_UNHANDLED_CMD) {
		nvgpu_err(g, "unhandled cmd: seq %u", (u32) id);
		err = -EINVAL;
	} else {
		err = pmu_payload_extract(pmu, seq);
	}

exit:
	/*
	 * free allocated space for payload in
	 * DMEM/FB-surface/FB_QUEUE as data is
	 * copied to buffer pointed by
	 * seq->out_payload
	 */
	pmu_payload_free(pmu, seq);

	nvgpu_pmu_seq_callback(g, seq, msg, err);

	nvgpu_pmu_seq_release(g, pmu->sequences, seq);

	/* TBD: notify client waiting for available dmem */

	nvgpu_log_fn(g, "done err %d", err);

	return err;
}

static int pmu_handle_event(struct nvgpu_pmu *pmu, struct pmu_msg *msg)
{
	int err = 0;
	struct gk20a *g = pmu->g;

	nvgpu_log_fn(g, " ");
	switch (msg->hdr.unit_id) {
	case PMU_UNIT_PERFMON:
	case PMU_UNIT_PERFMON_T18X:
		err = nvgpu_pmu_perfmon_event_handler(g, pmu, (void *)msg);
		break;
	case PMU_UNIT_PERF:
		if (g->ops.pmu_perf.handle_pmu_perf_event != NULL) {
			err = g->ops.pmu_perf.handle_pmu_perf_event(g,
				(void *)&msg->hdr);
		} else {
			WARN_ON(true);
		}
		break;
	case PMU_UNIT_PG:
		if (pmu->pg->process_pg_event != NULL) {
			err = pmu->pg->process_pg_event(g, (void *)&msg->hdr);
		}
		break;
	default:
		nvgpu_log_info(g, "Received invalid PMU unit event");
		break;
	}

	return err;
}

static bool pmu_engine_mem_queue_read(struct nvgpu_pmu *pmu,
	u32 queue_id, void *data,
	u32 bytes_to_read, int *status)
{
	struct gk20a *g = pmu->g;
	u32 bytes_read;
	int err;

	err = nvgpu_pmu_queue_pop(&pmu->queues, pmu->flcn, queue_id, data,
				  bytes_to_read, &bytes_read);
	if (err != 0) {
		nvgpu_err(g, "fail to read msg: err %d", err);
		*status = err;
		return false;
	}
	if (bytes_read != bytes_to_read) {
		nvgpu_err(g, "fail to read requested bytes: 0x%x != 0x%x",
			bytes_to_read, bytes_read);
		*status = -EINVAL;
		return false;
	}

	return true;
}

static bool pmu_read_message(struct nvgpu_pmu *pmu, u32 queue_id,
	struct pmu_msg *msg, int *status)
{
	struct gk20a *g = pmu->g;
	u32 read_size;
	int err;

	*status = 0;

	if (nvgpu_pmu_queue_is_empty(&pmu->queues, queue_id)) {
		return false;
	}

	if (!pmu_engine_mem_queue_read(pmu, queue_id, &msg->hdr,
				       PMU_MSG_HDR_SIZE, status)) {
		nvgpu_err(g, "fail to read msg from queue %d", queue_id);
		goto clean_up;
	}

	if (msg->hdr.unit_id == PMU_UNIT_REWIND) {
		if (!nvgpu_pmu_fb_queue_enabled(&pmu->queues)) {
			err = nvgpu_pmu_queue_rewind(&pmu->queues, queue_id,
						     pmu->flcn);
			if (err != 0) {
				nvgpu_err(g, "fail to rewind queue %d",
					  queue_id);
				*status = err;
				goto clean_up;
			}
		}

		/* read again after rewind */
		if (!pmu_engine_mem_queue_read(pmu, queue_id, &msg->hdr,
				PMU_MSG_HDR_SIZE, status)) {
			nvgpu_err(g, "fail to read msg from queue %d",
				queue_id);
			goto clean_up;
		}
	}

	if (!PMU_UNIT_ID_IS_VALID(msg->hdr.unit_id)) {
		nvgpu_err(g, "read invalid unit_id %d from queue %d",
			msg->hdr.unit_id, queue_id);
			*status = -EINVAL;
			goto clean_up;
	}

	if (msg->hdr.size > PMU_MSG_HDR_SIZE) {
		read_size = U32(msg->hdr.size) - PMU_MSG_HDR_SIZE;
		if (!pmu_engine_mem_queue_read(pmu, queue_id, &msg->msg,
					       read_size, status)) {
			nvgpu_err(g, "fail to read msg from queue %d",
				queue_id);
			goto clean_up;
		}
	}

	return true;

clean_up:
	return false;
}

static void pmu_read_init_msg_fb(struct gk20a *g, struct nvgpu_pmu *pmu,
	u32 element_index, u32 size, void *buffer)
{
	u32 fbq_msg_queue_ss_offset = 0U;

	fbq_msg_queue_ss_offset =
		nvgpu_pmu_get_ss_msg_fbq_element_offset(g, pmu,
			pmu->super_surface, element_index);

	fbq_msg_queue_ss_offset = nvgpu_safe_add_u32(fbq_msg_queue_ss_offset,
			(u32)sizeof(struct nv_falcon_fbq_msgq_hdr));
	nvgpu_mem_rd_n(g, nvgpu_pmu_super_surface_mem(g,
		pmu, pmu->super_surface), fbq_msg_queue_ss_offset,
		buffer, size);
}

static int pmu_process_init_msg_fb(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct pmu_msg *msg)
{
	u32 tail = 0U;
	int err = 0;

	nvgpu_log_fn(g, " ");

	g->ops.pmu.pmu_msgq_tail(pmu, &tail, QUEUE_GET);

	pmu_read_init_msg_fb(g, pmu, tail, PMU_MSG_HDR_SIZE,
		(void *)&msg->hdr);

	if (msg->hdr.unit_id != PMU_UNIT_INIT_DGPU &&
			msg->hdr.unit_id != PMU_UNIT_CMDMGMT) {
		nvgpu_err(g, "FB MSG Q: expecting init msg");
		err = -EINVAL;
		goto exit;
	}

	pmu_read_init_msg_fb(g, pmu, tail, msg->hdr.size,
		(void *)&msg->hdr);
	if (msg->event_rpc.cmdmgmt_init.hdr.function !=
		PMU_INIT_MSG_TYPE_PMU_INIT) {
		nvgpu_err(g, "FB MSG Q: expecting pmu init msg");
		err = -EINVAL;
		goto exit;
	}

	/* Queue is not yet constructed, so inline next element code here.*/
	tail++;
	if (tail >= NV_PMU_FBQ_MSG_NUM_ELEMENTS) {
		tail = 0U;
	}

	g->ops.pmu.pmu_msgq_tail(pmu, &tail, QUEUE_SET);

exit:
	return err;
}

static s32 pmu_response_handle_ptcb(struct nvgpu_pmu *pmu, struct pmu_msg *msg,
		struct pmu_nvgpu_rpc_struct *pmu_rpc)
{
	struct gk20a *g = pmu->g;
	enum pmu_seq_state state;
	struct pmu_sequence *seq;
	struct nv_pmu_rpc_header *rpc_hdr = &(pmu_rpc->event_rpc.hdr);
	u16 size_rpc;
	s32 err = 0;
	u8 id;

	nvgpu_log_fn(g, " ");
	seq = nvgpu_pmu_sequences_get_seq(pmu->sequences, rpc_hdr->seq_num_id);
	state = nvgpu_pmu_seq_get_state(seq);
	id = nvgpu_pmu_seq_get_id(seq);

	if (state != PMU_SEQ_STATE_USED) {
		nvgpu_err(g, "msg for an unknown sequence %u", (u32) id);
		err = -EINVAL;
		return err;
	}
	/* extract RPC response payload */
	size_rpc = nvgpu_pmu_seq_get_buffer_size(seq);
	nvgpu_memcpy((u8 *)nvgpu_pmu_seq_get_out_payload(seq),
			(u8 *)rpc_hdr, size_rpc);

	/*
	 * msg parameter is passed just to
	 * maintain the definition of seq_callback
	 * to-do: clean-up the unused param
	 */
	nvgpu_pmu_seq_callback(g, seq, msg, err);
	nvgpu_pmu_seq_release(g, pmu->sequences, seq);
	return 0;
}

static s32 pmu_handle_event_ptcb(struct nvgpu_pmu *pmu, struct pmu_msg *msg,
		struct pmu_nvgpu_rpc_struct *pmu_rpc)
{
	s32 err = 0;
	struct gk20a *g = pmu->g;

	nvgpu_log_fn(g, " ");

	switch (pmu_rpc->event_rpc.hdr.unit_id) {
	case PMU_UNIT_PERFMON:
	case PMU_UNIT_PERFMON_T18X:
		err = nvgpu_pmu_perfmon_event_handler(g, pmu, (void *)pmu_rpc);
		break;
	case PMU_UNIT_PERF:
		if (g->ops.pmu_perf.handle_pmu_perf_event != NULL) {
			err = g->ops.pmu_perf.handle_pmu_perf_event(g,
				(void *)&msg->hdr);
		} else {
			WARN_ON(true);
		}
		break;
	case PMU_UNIT_PG:
		if (pmu->pg->process_pg_event != NULL) {
			err = pmu->pg->process_pg_event(g, (void *)pmu_rpc);
		}
		break;
	case PMU_UNIT_CMDMGMT:
		nvgpu_pmu_dbg(g, "Received CMDMGMT EVENT, function %d",
						pmu_rpc->event_rpc.hdr.function);
		if (pmu_rpc->event_rpc.hdr.function ==
			(u8)PMU_INIT_MSG_TYPE_DRIVER_REGISTER_EVENT_RESPONSE) {
			nvgpu_pmu_dbg(g, "register_event_response %d",
pmu_rpc->event_rpc.cmdmgmt_driver_register_event_response.register_event_response);
		}
		break;
	default:
		nvgpu_log_info(g, "Received invalid PMU unit event");
		break;
	}

	return err;
}

static void pmu_read_msg_ptcb(struct gk20a *g, struct nvgpu_pmu *pmu,
	u32 element_index, u32 size, void *buffer)
{
	u32 fbq_msg_queue_ss_offset = 0U;

	size = nvgpu_safe_cast_u64_to_u32(sizeof(struct nv_pmu_rpc_header));

	fbq_msg_queue_ss_offset =
		nvgpu_pmu_get_fbq_ptcb_ss_offset(g, pmu,
			pmu->super_surface, 1, element_index);

	nvgpu_mem_rd_n(g, nvgpu_pmu_super_surface_mem(g,
		pmu, pmu->super_surface), fbq_msg_queue_ss_offset,
		buffer, size);

	size = nvgpu_safe_cast_u64_to_u32(
			((struct nv_pmu_rpc_header *)buffer)->rpc_size -
				sizeof(struct nv_pmu_rpc_header));

	fbq_msg_queue_ss_offset = nvgpu_safe_add_u32(fbq_msg_queue_ss_offset,
			(u32)sizeof(struct nv_pmu_rpc_header));

	nvgpu_mem_rd_n(g, nvgpu_pmu_super_surface_mem(g,
		pmu, pmu->super_surface), fbq_msg_queue_ss_offset,
		(u8 *)buffer + sizeof(struct nv_pmu_rpc_header), size);
}

static bool pmu_read_message_ptcb(struct nvgpu_pmu *pmu,
	struct pmu_nvgpu_rpc_struct *pmu_rpc, int *status)
{
	struct gk20a *g = pmu->g;
	u32 tail, next_tail, head;
	bool ret;

	*status = 0;

	nvgpu_mutex_acquire(&pmu->ptcb_queue_mutex);
	head = pwr_pmu_msgq_head_val_v(
			g->ops.pmu.get_pmu_msgq_head(g));

	g->ops.pmu.pmu_msgq_tail(pmu, &tail, QUEUE_GET);

	if (head == tail) {
		nvgpu_pmu_dbg(g, "queue empty");
		ret = false;
		goto exit;
	}

	pmu_read_msg_ptcb(g, pmu, tail,
		nvgpu_safe_cast_u64_to_u32(sizeof(pmu_rpc->event_rpc)),
		(void *)&pmu_rpc->event_rpc);

	next_tail = nvgpu_pmu_get_next_head(tail);

	g->ops.pmu.pmu_msgq_tail(pmu, &next_tail, QUEUE_SET);

	ret = true;

exit:
	nvgpu_mutex_release(&pmu->ptcb_queue_mutex);
	return ret;
}

static s32 pmu_process_init_msg_ptcb(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct pmu_nvgpu_rpc_struct *pmu_rpc)
{
	u32 tail = 0U;
	s32 err = 0;

	nvgpu_log_fn(g, " ");

	g->ops.pmu.pmu_msgq_tail(pmu, &tail, QUEUE_GET);

	pmu_read_msg_ptcb(g, pmu, tail,
		nvgpu_safe_cast_u64_to_u32(sizeof(pmu_rpc->event_rpc)),
		(void *)&pmu_rpc->event_rpc);

	if ((pmu_rpc->event_rpc.hdr.unit_id != PMU_UNIT_CMDMGMT) ||
			(pmu_rpc->event_rpc.hdr.function != PMU_INIT_MSG_TYPE_PMU_INIT)) {
		nvgpu_err(g, "FB MSG Q: expecting pmu init msg");
		err = -EINVAL;
		goto exit;
	}

	tail = nvgpu_pmu_get_next_head(tail);
	g->ops.pmu.pmu_msgq_tail(pmu, &tail, QUEUE_SET);

exit:
	return err;
}

static int pmu_process_init_msg_dmem(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct pmu_msg *msg)
{
	u32 tail = 0U;
	int err = 0;

	nvgpu_log_fn(g, " ");

	g->ops.pmu.pmu_msgq_tail(pmu, &tail, QUEUE_GET);

	err = nvgpu_falcon_copy_from_dmem(pmu->flcn, tail,
		(u8 *)&msg->hdr, PMU_MSG_HDR_SIZE, 0);
	if (err != 0) {
		nvgpu_err(g, "PMU falcon DMEM copy failed");
		goto exit;
	}
	if (msg->hdr.unit_id != PMU_UNIT_INIT) {
		nvgpu_err(g, "expecting init msg");
		err = -EINVAL;
		goto exit;
	}

	err = nvgpu_falcon_copy_from_dmem(pmu->flcn, tail + PMU_MSG_HDR_SIZE,
		(u8 *)&msg->msg, (u32)msg->hdr.size - PMU_MSG_HDR_SIZE, 0);
	if (err != 0) {
		nvgpu_err(g, "PMU falcon DMEM copy failed");
		goto exit;
	}

	if (msg->msg.init.msg_type != PMU_INIT_MSG_TYPE_PMU_INIT) {
		nvgpu_err(g, "expecting pmu init msg");
		err = -EINVAL;
		goto exit;
	}

	tail += NVGPU_ALIGN(U32(msg->hdr.size), PMU_DMEM_ALIGNMENT);
	g->ops.pmu.pmu_msgq_tail(pmu, &tail, QUEUE_SET);

exit:
	return err;
}

static int pmu_gid_info_dmem_read(struct nvgpu_pmu *pmu,
	union pmu_init_msg_pmu *init)
{
	struct pmu_fw_ver_ops *fw_ops = &pmu->fw->ops;
	struct pmu_sha1_gid *gid_info = &pmu->gid_info;
	struct pmu_sha1_gid_data gid_data;
	int err = 0;

	if (!gid_info->valid) {
		err = nvgpu_falcon_copy_from_dmem(pmu->flcn,
				fw_ops->get_init_msg_sw_mngd_area_off(init),
				(u8 *)&gid_data,
				(u32)sizeof(struct pmu_sha1_gid_data), 0);
		if (err != 0) {
			nvgpu_err(pmu->g, "PMU falcon DMEM copy failed");
			goto exit;
		}

		gid_info->valid =
			(gid_data.signature == PMU_SHA1_GID_SIGNATURE);

		if (gid_info->valid) {
			if (sizeof(gid_info->gid) !=
				sizeof(gid_data.gid)) {
				WARN_ON(1);
			}

			nvgpu_memcpy((u8 *)gid_info->gid, (u8 *)gid_data.gid,
				sizeof(gid_info->gid));
		}
	}

exit:
	return err;
}

static int pmu_process_init_msg(struct nvgpu_pmu *pmu,
		struct pmu_msg *msg, struct pmu_nvgpu_rpc_struct *pmu_rpc)
{
	struct gk20a *g = pmu->g;
	struct pmu_fw_ver_ops *fw_ops = &g->pmu->fw->ops;
	union pmu_init_msg_pmu *init;
	int err = 0;

	nvgpu_log_fn(g, " ");

	nvgpu_pmu_dbg(g, "init received\n");

	(void)memset((void *)msg, 0x0, sizeof(struct pmu_msg));

	init = fw_ops->get_init_msg_ptr(&(msg->msg.init));

	err = nvgpu_pmu_queues_init(g, init, &pmu->queues,
			nvgpu_pmu_super_surface_mem(g, pmu,
			pmu->super_surface));
	if (err != 0) {
		goto exit;
	}

	nvgpu_pmu_allocator_dmem_init(g, pmu, &pmu->dmem, init);

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_PMU_SUPER_SURFACE)) {
		err = nvgpu_pmu_ss_create_ssmd_lookup_table(g,
			pmu, pmu->super_surface);
		if (err != 0) {
			goto exit;
		}
	}

	/* For PMU 2.0 no isr from riscv. Start driver register */
	if (pmu->is_pmu_ucode_model_20) {
		err = nvgpu_pmu_wait_fw_driver_register_ready(g);
		if (err != 0) {
			nvgpu_err(g, "PMU not moved to register_ready state");
			goto exit;
		}

		nvgpu_pmu_set_fw_ready(g, pmu, true);

		err = nvgpu_pmu_driver_register_send(g, register_start);
		if (err != 0) {
			nvgpu_err(g, "Failed sending driver register start");
			goto exit;
		}

		nvgpu_pmu_fw_state_change(g, pmu, PMU_FW_STATE_INIT_RECEIVED, true);
		/* In case pmu_pg_task not running, send driver register complete */
		if (!g->can_elpg) {
			err = nvgpu_pmu_driver_register_send(g, register_complete);
			if (err != 0) {
				nvgpu_err(g, "Failed sending driver register complete");
				goto exit;
			}
		}
	} else {
		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_PMU_RTOS_FBQ)) {
			if (pmu->is_fbq_ptcb_enabled) {
				/* Process init msg through PTCB method */
				err = pmu_process_init_msg_ptcb(g, pmu, pmu_rpc);
				if (err != 0) {
					goto exit;
				}

			} else {
				err = pmu_process_init_msg_fb(g, pmu, msg);
				if (err != 0) {
					goto exit;
				}
			}
		} else {
			err = pmu_process_init_msg_dmem(g, pmu, msg);
			if (err != 0) {
				goto exit;
			}

			err = pmu_gid_info_dmem_read(pmu, init);
			if (err != 0) {
				goto exit;
			}
		}

		nvgpu_pmu_set_fw_ready(g, pmu, true);

		nvgpu_pmu_fw_state_change(g, pmu, PMU_FW_STATE_INIT_RECEIVED, true);

	}

#ifdef CONFIG_NVGPU_FALCON_DEBUG
	/*
	 * set the flag true after PMU is initialised.
	 * This will be used to print debug buffer data
	 * when any error on PMU occurs
	 */
	nvgpu_falcon_dbg_error_print_enable(pmu->flcn, true);
#endif

exit:
	nvgpu_pmu_dbg(g, "init receive end, err %x", err);
	return err;
}

int nvgpu_pmu_process_message(struct nvgpu_pmu *pmu)
{
	struct pmu_msg msg;
	int status;
	struct gk20a *g = pmu->g;
	struct pmu_nvgpu_rpc_struct *pmu_rpc;
	struct nv_pmu_rpc_header *rpc_header;
	int err;

	pmu_rpc = (struct pmu_nvgpu_rpc_struct *)
		nvgpu_kzalloc(g, sizeof(struct pmu_nvgpu_rpc_struct));
	if (pmu_rpc == NULL) {
		err = -ENOMEM;
		return err;
	}

	rpc_header = &(pmu_rpc->event_rpc.hdr);

	if (nvgpu_can_busy(g) == 0) {
		err = 0;
		goto exit;
	}

	if (unlikely(!nvgpu_pmu_get_fw_ready(g, pmu))) {
		err = pmu_process_init_msg(pmu, &msg, pmu_rpc);
		if (err != 0) {
			goto exit;
		}

		if (nvgpu_is_enabled(g, NVGPU_PMU_PERFMON)) {
			err = nvgpu_pmu_perfmon_initialization(g, pmu,
							pmu->pmu_perfmon);
			if (err != 0) {
				goto exit;
			}
		}

		err = 0;
		goto exit;
	}

	if (pmu->is_fbq_ptcb_enabled) {
		/* Process other message using PTCB method */
		while (pmu_read_message_ptcb(pmu, pmu_rpc, &status)) {

			if (nvgpu_can_busy(g) == 0) {
				err = 0;
				goto exit;
			}

			if (rpc_header->flags == NV_RM_PMU_RPC_FLAGS_PMU_RESPONSE_YES) {
				err = pmu_response_handle_ptcb(pmu, &msg, pmu_rpc);
			} else {
				err = pmu_handle_event_ptcb(pmu, &msg, pmu_rpc);
			}

			if (err != 0) {
				goto exit;
			}
		}
	} else {
		/* non-ptcb */
		while (pmu_read_message(pmu, PMU_MESSAGE_QUEUE, &msg, &status)) {

			if (nvgpu_can_busy(g) == 0) {
				err = 0;
				goto exit;
			}

			nvgpu_pmu_dbg(g, "read msg hdr: ");
			nvgpu_pmu_dbg(g, "unit_id = 0x%08x, size = 0x%08x",
				msg.hdr.unit_id, msg.hdr.size);
			nvgpu_pmu_dbg(g, "ctrl_flags = 0x%08x, seq_id = 0x%08x",
				msg.hdr.ctrl_flags, msg.hdr.seq_id);

			msg.hdr.ctrl_flags &= (u8)(~PMU_CMD_FLAGS_PMU_MASK);

			if ((msg.hdr.ctrl_flags == PMU_CMD_FLAGS_EVENT) ||
				(msg.hdr.ctrl_flags == PMU_CMD_FLAGS_RPC_EVENT)) {
				err = pmu_handle_event(pmu, &msg);
			} else {
				err = pmu_response_handle(pmu, &msg);
			}

			if (err != 0) {
				goto exit;
			}
		}
	}
exit:
	nvgpu_kfree(g, pmu_rpc);
	rpc_header = NULL;
	return err;
}

static void pmu_rpc_handler(struct gk20a *g, struct nv_pmu_rpc_header rpc,
			    struct rpc_handler_payload *rpc_payload)
{
	struct nvgpu_pmu *pmu = g->pmu;

	switch (rpc.unit_id) {
	case PMU_UNIT_ACR:
		nvgpu_pmu_lsfm_rpc_handler(g, rpc_payload);
		break;
	case PMU_UNIT_PERFMON_T18X:
	case PMU_UNIT_PERFMON:
		nvgpu_pmu_perfmon_rpc_handler(g, pmu, &rpc, rpc_payload);
		break;
	case PMU_UNIT_VOLT:
		if (pmu->volt->volt_rpc_handler != NULL) {
			pmu->volt->volt_rpc_handler(g, &rpc);
		}
		break;
	case PMU_UNIT_CLK:
		nvgpu_pmu_dbg(g, "reply PMU_UNIT_CLK");
		break;
	case PMU_UNIT_PERF:
		nvgpu_pmu_dbg(g, "reply PMU_UNIT_PERF");
		break;
	case PMU_UNIT_THERM:
		if (pmu->therm_rpc_handler != NULL) {
			pmu->therm_rpc_handler(g, pmu, &rpc);
		}
		break;
	case PMU_UNIT_PG_LOADING:
		if (pmu->pg->pg_loading_rpc_handler != NULL) {
			pmu->pg->pg_loading_rpc_handler(g, &rpc,
							rpc_payload);
		}
		break;
	case PMU_UNIT_PG:
		if (pmu->pg->pg_rpc_handler != NULL) {
			pmu->pg->pg_rpc_handler(g, pmu, &rpc, rpc_payload);
		}
		break;
	case PMU_UNIT_LPWR_LP:
		if (pmu->pg->lpwr_lp_rpc_handler != NULL) {
			pmu->pg->lpwr_lp_rpc_handler(g, pmu, &rpc, rpc_payload);
		}
		break;
	case PMU_UNIT_CMDMGMT:
		nvgpu_pmu_dbg(g, "reply PMU_UNIT_CMDMGMT, function %d", rpc.function);
		if (rpc.function == (u8)NV_PMU_RPC_ID_CMDMGMT_DRIVER_REGISTER_EVENT) {
			nvgpu_pmu_dbg(g, "registerEvent %d",
			((struct pmu_rpc_struct_driver_register_event *)
			rpc_payload->rpc_buff)->registerEvent);
		}
		break;
	default:
		nvgpu_err(g, " Invalid RPC response, stats 0x%x",
			rpc.flcn_status);
		break;
	}
}

void nvgpu_pmu_rpc_handler(struct gk20a *g, struct pmu_msg *msg,
		void *param, u32 status)
{
	struct nv_pmu_rpc_header rpc;
	struct rpc_handler_payload *rpc_payload =
		(struct rpc_handler_payload *)param;
	struct pmu_sequence *seq = NULL;
	struct nvgpu_pmu *pmu = g->pmu;

	(void)status;

	if (nvgpu_can_busy(g) == 0) {
		return;
	}

	(void) memset(&rpc, 0, sizeof(struct nv_pmu_rpc_header));
	nvgpu_memcpy((u8 *)&rpc, (u8 *)rpc_payload->rpc_buff,
		sizeof(struct nv_pmu_rpc_header));

	if (rpc.flcn_status != 0U) {
		nvgpu_err(g,
			"failed RPC response, unit-id=0x%x, func=0x%x, status=0x%x",
			rpc.unit_id, rpc.function, rpc.flcn_status);
		goto exit;
	}

	pmu_rpc_handler(g, rpc, rpc_payload);

exit:
	rpc_payload->complete = true;

	/*
	 * free allocated memory and set seq_free_status to
	 * true to sync the memory free
	 */
	if (rpc_payload->is_mem_free_set) {
		seq = nvgpu_pmu_sequences_get_seq(pmu->sequences,
				msg->hdr.seq_id);
		if (seq->seq_free_status == false) {
			nvgpu_kfree(g, rpc_payload);
			seq->seq_free_status = true;
		}
	}
}

void pmu_wait_message_cond(struct nvgpu_pmu *pmu, u32 timeout_ms,
			void *var, u8 val)
{
	struct gk20a *g = pmu->g;

	if (nvgpu_pmu_wait_fw_ack_status(g, pmu, timeout_ms, var, val) == -ETIMEDOUT) {
		nvgpu_err(g, "PMU wait timeout expired.");
		WARN_ON(true);
	}
}
