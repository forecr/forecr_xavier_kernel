/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_GB10B_H
#define NVGPU_PBDMA_GB10B_H

struct gk20a;
struct nvgpu_debug_context;
struct nvgpu_device;

#define PBDMA_METHOD_SUBCH_FIVE		5U
#define PBDMA_METHOD_SUBCH_SIX		6U
#define PBDMA_METHOD_SUBCH_SEVEN	7U

#define INTR_SIZE			0U
#define INTR_SET_SIZE			1U
#define INTR_CLEAR_SIZE			2U

#define PBDMA_SUBDEVICE_ID	1U

void gb10b_pbdma_dump_status(struct gk20a *g, struct nvgpu_debug_context *o);

void gb10b_pbdma_reset_method(struct gk20a *g, u32 pbdma_id,
			u32 pbdma_method_index);
u32 gb10b_pbdma_get_num_of_pbdmas(void);
u32 gb10b_pbdma_read_data(struct gk20a *g, u32 pbdma_id);

u32 gb10b_pbdma_get_fc_pb_header(void);
u32 gb10b_pbdma_set_clear_intr_offsets(struct gk20a *g,
				       u32 set_clear_size);
u32 gb10b_pbdma_set_intr_notify(u32 eng_intr_vector);
u32 gb10b_pbdma_get_gp_base_hi(u64 gpfifo_base, u32 gpfifo_entry);
u32 gb10b_pbdma_get_fc_target(const struct nvgpu_device *dev);
u32 gb10b_pbdma_get_config_auth_level_privileged(void);
u32 gb10b_pbdma_get_fc_subdevice(void);
u32 gb10b_pbdma_intr_0_en_set_tree_mask(void);
u32 gb10b_pbdma_intr_0_en_clear_tree_mask(void);
u32 gb10b_pbdma_channel_fatal_0_intr_descs(void);
void gb10b_pbdma_report_error(struct gk20a *g, u32 pbdma_id,
		u32 pbdma_intr_0);
void gb10b_pbdma_dump_intr_0(struct gk20a *g, u32 pbdma_id,
				u32 pbdma_intr_0);
void gb10b_pbdma_reset_header(struct gk20a *g, u32 pbdma_id);
bool gb10b_pbdma_is_sw_method_subch(struct gk20a *g, u32 pbdma_id,
						u32 pbdma_method_index);
#endif /* NVGPU_PBDMA_GB10B_H */
