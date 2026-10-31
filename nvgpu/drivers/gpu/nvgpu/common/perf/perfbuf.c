// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/mm.h>
#include <nvgpu/sizes.h>
#include <nvgpu/perfbuf.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/vm_area.h>
#include <nvgpu/utils.h>

int nvgpu_perfbuf_enable_locked(struct gk20a *g, u32 pma_channel_id, u64 offset,
				u32 size)
{
	int err;

	err = gk20a_busy(g);
	if (err != 0) {
		nvgpu_err(g, "failed to poweron");
		return err;
	}

	g->ops.perf.membuf_reset_streaming(g, pma_channel_id);
	g->ops.perf.enable_membuf(g, pma_channel_id, size, offset);

	gk20a_idle(g);

	return 0;
}

int nvgpu_perfbuf_disable_locked(struct gk20a *g, u32 pma_channel_id)
{
	int err = gk20a_busy(g);
	if (err != 0) {
		nvgpu_err(g, "failed to poweron");
		return err;
	}

	g->ops.perf.membuf_reset_streaming(g, pma_channel_id);
	g->ops.perf.disable_membuf(g, pma_channel_id);

	gk20a_idle(g);

	return 0;
}

int nvgpu_perfbuf_init_inst_block(struct gk20a *g, struct perfbuf *perfbuf,
				u32 pma_channel_id)
{
	u32 gfid = 0U;
	int err;
	u32 inst_blk_ptr;
	u32 aperture;

	err = nvgpu_alloc_inst_block(g, &perfbuf->inst_block);
	if (err != 0) {
		return err;
	}

	err = g->ops.mm.init_inst_block_core(&perfbuf->inst_block, perfbuf->vm, 0);
	if (err != 0) {
		nvgpu_free_inst_block(g, &perfbuf->inst_block);
		return err;
	}

	inst_blk_ptr = nvgpu_inst_block_ptr(g, &perfbuf->inst_block);
	aperture = perfbuf->inst_block.aperture;
	err = g->ops.perf.init_inst_block(g, pma_channel_id, inst_blk_ptr,
			aperture, gfid);
	if (err != 0) {
		nvgpu_free_inst_block(g, &perfbuf->inst_block);
		return err;
	}

	return 0;
}

struct perfbuf *nvgpu_perfbuf_get_perfbuf(struct gk20a *g, u32 pma_channel_id,
						u32 *perfbuf_channel_idx)
{
	u32 perfbuf_idx, channels_per_perfbuf;

	channels_per_perfbuf = g->num_pma_channels / g->mm.num_of_perfbuf;
	perfbuf_idx = pma_channel_id / channels_per_perfbuf;

	if (perfbuf_channel_idx != NULL) {
		*perfbuf_channel_idx =
				pma_channel_id % channels_per_perfbuf;
	}
	nvgpu_log(g, gpu_dbg_prof,
		  "g->mm.perfbuf[%u] is used for pma_channel_id : %u",
		  perfbuf_idx, pma_channel_id);

	return &g->mm.perfbuf[perfbuf_idx];
}

int nvgpu_perfbuf_init_vm(struct gk20a *g, u32 pma_channel_id)
{
	u32 big_page_size = g->ops.mm.gmmu.get_default_big_page_size();
	u32 per_buf_space, channels_per_perfbuf;
	u64 user_size, kernel_size, base;
	u32 perfbuf_idx, i;
	struct perfbuf *perfbuf;
	int err, ref_count;

	perfbuf = nvgpu_perfbuf_get_perfbuf(g, pma_channel_id, &perfbuf_idx);
	nvgpu_mutex_acquire(&perfbuf->lock);
	channels_per_perfbuf = g->num_pma_channels / g->mm.num_of_perfbuf;

	ref_count = nvgpu_atomic_read(&perfbuf->buf_refcount);
	nvgpu_log(g, gpu_dbg_prof, "Perfbuf-%u, pma_channel_id:%u Initializing with refcount of %d",
			perfbuf_idx, pma_channel_id, ref_count);
	if (ref_count >= nvgpu_safe_cast_u32_to_s32(channels_per_perfbuf)) {
		nvgpu_err(g, "perfbuf-%u, pma_channel_id:%u All channels are in use",
				perfbuf_idx, pma_channel_id);
		err = -EBUSY;
		goto fail;
	} else if (ref_count != 0) {
		nvgpu_atomic_inc(&perfbuf->buf_refcount);
		nvgpu_log(g, gpu_dbg_prof,
			"For channel id %u, reusing perfbuf-%u with incremented refcount %d",
			pma_channel_id, perfbuf_idx, nvgpu_atomic_read(&perfbuf->buf_refcount));
		nvgpu_mutex_release(&perfbuf->lock);
		return 0;
	}

	nvgpu_log(g, gpu_dbg_prof,
			"perfbuf-%u, pma_channel_id:%u, Allocating and intializing new vm",
			perfbuf_idx, pma_channel_id);
	g->ops.mm.get_default_va_sizes(NULL, &user_size, &kernel_size);

	perfbuf->vm = nvgpu_vm_init(g, big_page_size, SZ_4K,
			nvgpu_safe_sub_u64(user_size, SZ_4K),
			kernel_size,
			0ULL,
			false, false, "perfbuf");
	if (perfbuf->vm == NULL) {
		err = -ENOMEM;
		goto fail;
	}

	/*
	 * The PMA unit can only access GPU VAs within a 4GB window which
	 * includes PMA_BUF + PMA_AVAILABLE_BYTES_BUF, hence carveout and
	 * reserved a 4GB window from the perfbuf.vm VA space and use this
	 * VA while binding the buffers.
	 */
	perfbuf->pma_buffer_gpu_va[0] = 0;

	err = nvgpu_vm_area_alloc(perfbuf->vm,
			PERFBUF_PMA_MEM_WINDOW_SIZE / SZ_4K,
			SZ_4K, &perfbuf->pma_buffer_gpu_va[0], 0);
	if (err != 0) {
		nvgpu_vm_put(perfbuf->vm);
		goto fail;
	}

	per_buf_space = nvgpu_safe_cast_u64_to_u32(PERFBUF_PMA_BUF_MAX_SIZE / channels_per_perfbuf);
	for (i = 0; i < channels_per_perfbuf; i++) {
		perfbuf->pma_buffer_gpu_va[i] =
			nvgpu_safe_add_u64(perfbuf->pma_buffer_gpu_va[0],
				nvgpu_safe_mult_u64(i, per_buf_space));
		nvgpu_log(g, gpu_dbg_prof, "perfbuf-%u: pma_buf_gpu_va[%d] : 0x%llx",
				perfbuf_idx, i, perfbuf->pma_buffer_gpu_va[i]);
	}

	per_buf_space = nvgpu_safe_cast_u64_to_u32(PERFBUF_PMA_AVAILABLE_BYTES_BUF_MAX_SIZE /
					channels_per_perfbuf);
	base = nvgpu_safe_add_u64(PERFBUF_PMA_BUF_MAX_SIZE,
				  perfbuf->pma_buffer_gpu_va[0]);

	for (i = 0; i < channels_per_perfbuf; i++) {
		perfbuf->pma_bytes_available_buffer_gpu_va[i] =
		       nvgpu_safe_add_u64(base,
				nvgpu_safe_mult_u64(i, per_buf_space));
		nvgpu_log(g, gpu_dbg_prof,
			"perfbuf-%u pma_bytes_available_buffer_gpu_va[%d] : 0x%llx",
			perfbuf_idx, i, perfbuf->pma_bytes_available_buffer_gpu_va[i]);

		if (u64_hi32(perfbuf->pma_bytes_available_buffer_gpu_va[i]) !=
			u64_hi32(perfbuf->pma_buffer_gpu_va[i])) {
			nvgpu_err(g,
				"perfbuf-%u: ch_in_cblock:%u,  0x%llx, 0x%llx, crosses 4GB boundary",
				perfbuf_idx, i, perfbuf->pma_buffer_gpu_va[i],
				perfbuf->pma_bytes_available_buffer_gpu_va[i]);

			nvgpu_vm_area_free(perfbuf->vm,
				perfbuf->pma_buffer_gpu_va[0]);
			nvgpu_vm_put(perfbuf->vm);
			err = -ENOMEM;
			goto fail;
		}
	}

	err = g->ops.perfbuf.init_inst_block(g, perfbuf, pma_channel_id);
	if (err != 0) {
		nvgpu_vm_put(perfbuf->vm);
		goto fail;
	}

	nvgpu_atomic_inc(&perfbuf->buf_refcount);

fail:
	nvgpu_mutex_release(&perfbuf->lock);
	return err;
}

void nvgpu_perfbuf_deinit_inst_block(struct gk20a *g,
		struct nvgpu_mem *inst_block, u32 pma_channel_id)
{
	g->ops.perf.deinit_inst_block(g, pma_channel_id);
	nvgpu_free_inst_block(g, inst_block);
}

void nvgpu_perfbuf_deinit_vm(struct gk20a *g, u32 pma_channel_id)
{
	struct perfbuf *perfbuf;
	u32 perfbuf_idx;
	int ref_count;

	perfbuf = nvgpu_perfbuf_get_perfbuf(g, pma_channel_id, &perfbuf_idx);

	nvgpu_mutex_acquire(&perfbuf->lock);
	ref_count = nvgpu_atomic_read(&perfbuf->buf_refcount);
	nvgpu_log(g, gpu_dbg_prof, "Deinit of perfbuf-%u pma_channel_id: %u, with refcount %d",
			perfbuf_idx, pma_channel_id, ref_count);
	nvgpu_assert(ref_count != 0);

	if (!nvgpu_atomic_dec_and_test(&perfbuf->buf_refcount)) {
		nvgpu_log(g, gpu_dbg_prof, "perfbuf-%u Vm left active for another channel", perfbuf_idx);
		nvgpu_mutex_release(&perfbuf->lock);
		return;
	}

	g->ops.perfbuf.deinit_inst_block(g, &perfbuf->inst_block,
					pma_channel_id);

	nvgpu_vm_area_free(perfbuf->vm, perfbuf->pma_buffer_gpu_va[0]);
	nvgpu_vm_area_free(perfbuf->vm, perfbuf->pma_bytes_available_buffer_gpu_va[0]);
	nvgpu_vm_put(perfbuf->vm);
	nvgpu_mutex_release(&perfbuf->lock);
}

int nvgpu_perfbuf_update_get_put(struct gk20a *g, u32 pma_channel_id,
				 u64 bytes_consumed, u64 *bytes_available,
				 void *cpuva, bool wait, u64 *put_ptr,
				 bool *overflowed)
{
	struct nvgpu_timeout timeout;
	int err;
	bool update_available_bytes = (bytes_available == NULL) ? false : true;
	volatile u32 *available_bytes_va = (u32 *)cpuva;

	if (update_available_bytes && available_bytes_va != NULL) {
		*available_bytes_va = 0xffffffff;
	}

	err = g->ops.perf.update_get_put(g, pma_channel_id, bytes_consumed,
			update_available_bytes, put_ptr, overflowed);
	if (err != 0) {
		return err;
	}

	if (update_available_bytes && wait && available_bytes_va != NULL) {
		nvgpu_timeout_init_cpu_timer(g, &timeout, 10000);

		do {
			if (*available_bytes_va != 0xffffffff) {
				break;
			}

			nvgpu_msleep(10);
		} while (nvgpu_timeout_expired(&timeout) == 0);

		if (*available_bytes_va == 0xffffffff) {
			nvgpu_err(g, "perfbuf update get put timed out");
			return -ETIMEDOUT;
		}

		*bytes_available = *available_bytes_va;
	}

	return 0;
}
