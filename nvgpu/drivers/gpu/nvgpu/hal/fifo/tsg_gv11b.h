/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TSG_GV11B_H
#define NVGPU_TSG_GV11B_H

struct nvgpu_tsg;
struct nvgpu_channel;
struct nvgpu_channel_hw_state;

void gv11b_tsg_enable(struct nvgpu_tsg *tsg);
void gv11b_tsg_disable(struct nvgpu_tsg *tsg);
void gv11b_tsg_unbind_channel_check_eng_faulted(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch,
		struct nvgpu_channel_hw_state *hw_state);
int gv11b_tsg_init_eng_method_buffers(struct gk20a *g,
		struct nvgpu_tsg *tsg);
void gv11b_tsg_deinit_eng_method_buffers(struct gk20a *g,
		struct nvgpu_tsg *tsg);
void gv11b_tsg_bind_channel_eng_method_buffers(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch);

/**
 * @brief Initialize subcontext PDB map and valid mask for a TSG.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param tsg [in]	Pointer to TSG struct.
 *
 * - If subcontexts are enabled:
 *   - Allocate array of PDB configuration values for maximum supported
 *     subcontexts.
 *   - Initialize the array by calling g->ops.ramin.init_subctx_pdb_map.
 *   - Allocate valid subcontexts bitmask.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int gv11b_tsg_init_subctx_state(struct gk20a *g, struct nvgpu_tsg *tsg);

/**
 * @brief Deinitialize subcontext PDB map and valid mask for a TSG.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param tsg [in]	Pointer to TSG struct.
 *
 * - If subcontexts are enabled:
 *   - Free array of PDB configuration values.
 *   - Free valid subcontexts bitmask.
 */
void gv11b_tsg_deinit_subctx_state(struct gk20a *g, struct nvgpu_tsg *tsg);

/**
 * @brief Add a subctx channel to TSG.
 *
 * @param ch [in]		Pointer to Channel struct.
 * @param replayable [in]	replayable state of the channel.
 *
 * - If subcontexts are enabled:
 *   - Update subcontext info in TSG members if this is first channel
 *     of a subcontext and update instance blocks of all channels
 *     in the TSG with this information.
 *   -  If this is a channel in existing subcontext then:
 *      - Update the channel instance block with subcontext info.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int gv11b_tsg_add_subctx_channel_hw(struct nvgpu_channel *ch, bool replayable);

/**
 * @brief Remove a subctx channel from TSG.
 *
 * @param ch [in]			Pointer to Channel struct.
 *
 * - If subcontexts are enabled:
 *   - Update subcontext info in TSG members as this is the last channel
 *     of a subcontext and update instance blocks of all channels
 *     in the TSG with this information.
 */
void gv11b_tsg_remove_subctx_channel_hw(struct nvgpu_channel *ch);

#endif /* NVGPU_TSG_GV11B_H */
