// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pbdma.h>

/**
 * @brief Initialize PBDMA interrupt descriptors.
 *
 * This function sets up the interrupt descriptors for the PBDMA (Push Buffer DMA).
 * The steps involved are:
 * -# Obtain a pointer to the FIFO (First In, First Out) structure from the GPU driver struct.
 * -# If the function pointer for gops_pbdma.device_fatal_0_intr_descs is not NULL, call the
 *    function to get the device fatal interrupt descriptors and store them in the FIFO's PBDMA
 *    interrupt structure.
 * -# If the function pointer for gops_pbdma.channel_fatal_0_intr_descs is not NULL, call the
 *    function to get the channel fatal interrupt descriptors and store them in the FIFO's PBDMA
 *    interrupt structure.
 * -# If the function pointer for gops_pbdma.restartable_0_intr_descs is not NULL, call the function
 *    to get the restartable interrupt descriptors and store them in the FIFO's PBDMA
 *    interrupt structure.
 *
 * @param [in] g  The GPU driver struct for which to initialize PBDMA interrupt descriptors.
 */
static void nvgpu_pbdma_init_intr_descs(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;

	if (g->ops.pbdma.device_fatal_0_intr_descs != NULL) {
		f->intr.pbdma.device_fatal_0 =
			g->ops.pbdma.device_fatal_0_intr_descs();
	}

	if (g->ops.pbdma.channel_fatal_0_intr_descs != NULL) {
		f->intr.pbdma.channel_fatal_0 =
			g->ops.pbdma.channel_fatal_0_intr_descs();
	}
	if (g->ops.pbdma.restartable_0_intr_descs != NULL) {
		f->intr.pbdma.restartable_0 =
			g->ops.pbdma.restartable_0_intr_descs();
	}
}

int nvgpu_pbdma_setup_sw(struct gk20a *g)
{
	nvgpu_pbdma_init_intr_descs(g);

	return 0;
}

void nvgpu_pbdma_cleanup_sw(struct gk20a *g)
{
	(void)g;
	return;
}
