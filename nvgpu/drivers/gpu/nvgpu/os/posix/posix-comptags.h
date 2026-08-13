/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *gk20a_get_comptags_fault_injection(void);
struct nvgpu_posix_fault_inj *gk20a_get_comptags_2_fault_injection(void);
struct nvgpu_posix_fault_inj *gk20a_get_comptags_3_fault_injection(void);
struct nvgpu_posix_fault_inj *gk20a_comptags_start_clear_fault_injection(void);
#endif
