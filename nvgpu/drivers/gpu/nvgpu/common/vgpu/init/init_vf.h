/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef INIT_VF_H
#define INIT_VF_H

struct gk20a;

int vf_enable_engine_nonstall_intrs(struct gk20a *g);

#endif
