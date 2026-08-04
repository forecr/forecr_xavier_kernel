/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_VGPU_IVC_H
#define NVGPU_VGPU_IVC_H

#include <nvgpu/types.h>

#ifdef __KERNEL__
#include "os/linux/vgpu/vgpu_ivc.h"
#elif defined(__NVGPU_POSIX__)
#include <nvgpu/posix/vgpu_ivc.h>
#elif defined(NVGPU_HVRTOS)
/* HVRTOS does not use vgpu_ivc */
#else
#include <nvgpu/qnx/vgpu_ivc.h>
#endif

struct gk20a;
struct vgpu_ivc;

/*
 * The timeout value is used on:
 * 1. receiving message from server
 * 2. waiting for server initializes ivc
 */
#define VGPU_IVC_TIMEOUT_MS 5000

int vgpu_ivc_send(struct vgpu_ivc *ivc, void *data, u32 size);
int vgpu_ivc_recv(struct vgpu_ivc *ivc, void *data, u32 size);
int vgpu_ivc_init_support(struct gk20a *g);
void vgpu_ivc_remove_support(struct gk20a *g);

#endif /* NVGPU_VGPU_IVC_H */
