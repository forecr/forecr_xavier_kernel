/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_HAL_GA10B_H
#define NVGPU_HAL_GA10B_H

struct gk20a;

/**
 * @brief Initializes the HAL (Hardware Abstraction Layer) for the GA10B GPU.
 *
 * This function sets up the function pointers for all the operations that the
 * GPU can perform. It configures the GPU based on the current platform and
 * capabilities. It also sets various errata and enables features based on
 * the GPU configuration and platform.
 *
 * The steps performed by the function are as follows:
 * -# Assign the ACR (Access Control Region) operations.
 * -# Assign the generic functions for GPU operations.
 * -# Conditionally assign the BIOS operations if DGPU is enabled.
 * -# Assign the ECC (Error-Correcting Code) operations.
 * -# Assign the LTC operations and its interrupt handlers.
 * -# Conditionally assign the CBC (Compressible Bitmap Cache) operations if
 *    compression is enabled.
 * -# Assign the CE (Copy Engine) operations.
 * -# Assign the GR (Graphics) operations, including ECC, context switch program,
 *    configuration, and setup operations.
 * -# Conditionally assign the FECS trace operations if enabled.
 * -# Conditionally assign the ZBC (Zero Bandwidth Clear) and ZCULL operations if
 *    graphics support is enabled.
 * -# Conditionally assign the HWPM (Hardware Performance Monitoring) map operations
 *    if the debugger is enabled.
 * -# Assign the GR initialization and interrupt handling operations.
 * -# Assign the GR falcon operations.
 * -# Assign the GPU class operations.
 * -# Assign the FB (Frame Buffer) operations, including ECC and interrupt handlers.
 * -# Conditionally assign the VAB (Video Attribute Buffer) operations if HAL non-FUSA
 *    is enabled.
 * -# Assign the CG (Clock Gating) operations.
 * -# Assign the FIFO (First In, First Out) operations.
 * -# Assign the engine operations.
 * -# Assign the PBDMA (Push Buffer Direct Memory Access) operations.
 * -# Assign the synchronization operations.
 * -# Assign the engine status operations.
 * -# Assign the PBDMA status operations.
 * -# Assign the RAMFC (RAM Flow Control) operations.
 * -# Assign the RAMIN (RAM Input) operations.
 * -# Assign the runlist operations.
 * -# Assign the userd (User Daemon) operations.
 * -# Assign the channel operations.
 * -# Assign the TSG (Timeslice Group) operations.
 * -# Assign the usermode operations.
 * -# Assign the netlist operations.
 * -# Assign the MM (Memory Management) operations, including MMU fault, cache,
 *    and GMMU (Graphics Memory Management Unit) operations.
 * -# Assign the therm (Thermal) operations.
 * -# Assign the PMU (Power Management Unit) operations.
 * -# Conditionally assign the clock arbitration operations if enabled.
 * -# Conditionally assign the debugger operations if enabled.
 * -# Assign the MC (Memory Controller) operations.
 * -# Assign the debug operations.
 * -# Assign the bus operations.
 * -# Assign the PTIMER (Precision Timer) operations.
 * -# Conditionally assign the cyclestats operations if enabled.
 * -# Assign the falcon operations.
 * -# Assign the GSP operations.
 * -# Assign the priv_ring operations.
 * -# Assign the fuse operations.
 * -# Assign the top operations.
 * -# Conditionally assign the TPC (Texture Processing Cluster) power-gating operations
 *    if static power-gate is enabled.
 * -# Assign the GR manager operations.
 * -# Assign the CIC monitor operations.
 * -# Assign the function to initialize GPU characteristics.
 * -# Assign the function to get litter values.
 * -# Assign the function to wake up the semaphore.
 * -# Set errata using nvgpu_set_errata() based on the platform and GPU configuration.
 * -# Enable SM TTU feature using nvgpu_set_enabled() if the fuse option is set or if FMODEL
 *    is enabled.
 * -# Enable ROP in GPC feature using nvgpu_set_enabled().
 * -# Check using gops_fuse.check_priv_security() whether the GPU needs to boot in secure
 *    or non-secure mode based on the fuse settings.
 * -# If the GPU is set to boot in secure mode, assign the function to load secure
 *    context switch microcode.
 * -# If the GPU is set to boot in non-secure mode and LS PMU is enabled, assign the
 *    function to set up PMU apertures.
 * -# Enable FECS trace virtual address feature if FECS trace is enabled.
 * -# Enable FECS trace feature control if FECS trace is enabled.
 * -# Enable FECS context switch trace if FECS trace is enabled.
 * -# Enable profiler device feature if profiler is enabled.
 * -# Enable profiler context feature if profiler is enabled.
 * -# Enable multiple WPR feature if supported.
 * -# Enable ZBC stencil feature if graphics is enabled.
 * -# Enable preemption GFXP feature if GFXP is enabled.
 * -# Enable platform atomic feature.
 * -# Enable setting context MMU debug mode feature.
 * -# Enable VAB feature if profiler is enabled.
 * -# Enable SMPC global mode feature if profiler is enabled.
 * -# Enable L2 max ways evict last feature if debugger is enabled.
 * -# Enable scheduler exit wait for error barrier feature if debugger is enabled.
 * -# Enable PMU RTOS FBQ feature if LS PMU is enabled and PMU is supported.
 * -# Enable PMU super surface feature if LS PMU is enabled and PMU is supported.
 * -# Enable GSP scheduler feature if GSP scheduler is enabled.
 * -# Enable GSP STRESS test feature if GSP scheduler is enabled.
 * -# Enable GSP VM feature.
 * -# Enable bypassing IOMMU feature.
 * -# Enable SM diversity feature if build configuration is not safety.
 * -# Set the maximum SM diversity configuration count based on the build configuration.
 *    #NVGPU_MAX_SM_DIVERSITY_CONFIG_COUNT for not safety configuration and for safety
 *    set the maximum SM diversity configuration count to NVGPU_DEFAULT_SM_DIVERSITY_CONFIG_COUNT.
 * -# If compression configuration is defined then enable compression feature if platform is silicon.
 * -# If compression configuration is defined then enable post L2 compression feature if
 *    compression is enabled.
 * -# Enable clock arbitration feature if platform is silicon and clock arbitration is enabled.
 * -# Initialize SIM support for GA10B if SIM is enabled.
 * -# Assign the MSS NVLink operations if HAL non-FUSA is enabled.
 * -# Enable emulate mode feature.
 * -# Enable PES FS feature.
 * -# Set the GPU name identifier to "ga10b".
 *
 * @param [in] g  The GPU structure for which the HAL is being initialized.
 *
 * @return 0 if the HAL was successfully initialized.
 * @return -EINVAL if the GPU should not boot due to security settings.
 */
int ga10b_init_hal(struct gk20a *g);

#endif /* NVGPU_HAL_GA10B_H */
