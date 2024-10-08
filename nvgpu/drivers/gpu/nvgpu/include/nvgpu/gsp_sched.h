/*
 * Copyright (c) 2022-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef GSP_SCHED_H
#define GSP_SCHED_H

#include <nvgpu/nvs.h>

struct gk20a;
struct nvgpu_gsp_sched;
struct nvgpu_runlist;
/*
 * Scheduler shall support only two engines with two runlists per domain.
 * 1. GR0
 * 2. Async CE0
 */
#define TOTAL_NO_OF_RUNLISTS 4U

struct nvgpu_gsp_runlist_info {
	/*
	 * Is this runlist valid, this field will be updated by NVGPU which tell GSP
	 * to submit this runlist or ignore for that domain.
	 */
	bool is_runlist_valid;
	/*
	 * This is don't care for KMD.
	 */
	bool is_runlist_updated;
	/*
	 * Device id to which this runlist belongs to.
	 */
	u8 device_id;
	/*
	 * Domain Id to which this runlist belongs to.
	 */
	u32 domain_id;
	/*
	 * ID contains the identifier of the runlist.
	 */
	u32 runlist_id;
	/*
	 * Indicates how many runlist entries are in the runlist.
	 */
	u32 num_entries;
	/*
	 * Indicates runlist memory aperture.
	 */
	u32 aperture;
	/*
	 *NV_RUNLIST_SUBMIT_BASE_L0 in-memory location of runlist.
	 */
	u32 runlist_base_lo;
	/*
	 *NV_RUNLIST_SUBMIT_BASE_Hi in-memory location of runlist.
	 */
	u32 runlist_base_hi;
};

struct nvgpu_gsp_domain_info {
	/*
	 * Is the current Domain Active. This is don't care for KMD.
	 */
	bool is_domain_active;
	/*
	 * Is the current Domain Valid. This is don't care for KMD.
	 */
	bool is_domain_valid;
	/*
	 * Domain Id
	 */
	u32 domain_id;
	/*
	 * Priority of the Domain for priority driven scheduling.
	 */
	u32 priority;
	/*
	 * Time-slicing of the domain for which scheduler will schedule it for.
	 */
	u32 time_slicing;
	/*
	 * Runlist info
	 */
	struct nvgpu_gsp_runlist_info runlist_info[TOTAL_NO_OF_RUNLISTS];
};

int nvgpu_gsp_sched_bootstrap_hs(struct gk20a *g);
int nvgpu_gsp_sched_sw_init(struct gk20a *g);
void nvgpu_gsp_sched_sw_deinit(struct gk20a *g);
void nvgpu_gsp_sched_suspend(struct gk20a *g, struct nvgpu_gsp_sched *gsp_sched);
void nvgpu_gsp_sched_isr(struct gk20a *g);
int nvgpu_gsp_sched_send_devices_info(struct gk20a *g);
int nvgpu_gsp_sched_domain_submit(struct gk20a *g, u32 domain_id);
int nvgpu_gsp_sched_domain_add(struct gk20a *g,
		struct nvgpu_gsp_domain_info *gsp_dom);
int nvgpu_gsp_sched_domain_update(struct gk20a *g,
		struct nvgpu_gsp_domain_info *gsp_dom);
int nvgpu_gsp_sched_runlist_update(struct gk20a *g,
		struct nvgpu_gsp_runlist_info *gsp_rl);
int nvgpu_gsp_sched_domain_delete(struct gk20a *g, u32 domain_id);
int nvgpu_gsp_sched_query_active_domain(struct gk20a *g, u32 *active_domain);
int nvgpu_gsp_sched_query_no_of_domains(struct gk20a *g, u32 *no_of_domains);
int nvgpu_gsp_sched_start(struct gk20a *g);
int nvgpu_gsp_sched_stop(struct gk20a *g);
/*  functions to get nvs scheduler and runlist domains info to gsp */
int nvgpu_gsp_nvs_add_domain(struct gk20a *g, u64 nvgpu_domain_id);
int nvgpu_gsp_nvs_delete_domain(struct gk20a *g, u64 nvgpu_domain_id);
int nvgpu_gsp_nvs_update_runlist(struct gk20a *g, const char *name,struct nvgpu_runlist *rl);
int nvgpu_gps_sched_update_runlist(struct gk20a *g,
    struct nvgpu_runlist_domain *domain, struct nvgpu_runlist *rl);
int nvgpu_gsp_sched_bind_ctx_reg(struct gk20a *g);
bool nvgpu_gsp_is_ready(struct gk20a *g);
#ifdef CONFIG_NVS_PRESENT
int nvgpu_gsp_sched_send_queue_info(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *queue,
	enum nvgpu_nvs_ctrl_queue_direction queue_direction);

int nvgpu_gsp_sched_erase_ctrl_fifo(struct gk20a *g,
	enum nvgpu_nvs_ctrl_queue_direction queue_direction);
#endif
#endif /* GSP_SCHED_H */
