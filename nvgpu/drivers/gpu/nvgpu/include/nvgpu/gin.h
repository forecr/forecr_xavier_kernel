/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GIN_H
#define NVGPU_GIN_H

/**
 * Value to be passed to gin.*_unit_config to enable the interrupt.
 */
#define NVGPU_GIN_INTR_ENABLE       true

/**
 * Value to be passed to gin.*_unit_config to disable the interrupt.
 */
#define NVGPU_GIN_INTR_DISABLE      false

/**
 * Value to be passed to gin.get_intr_ctrl_msg to enable routing the interrupt
 * to the CPU.
 */
#define NVGPU_GIN_CPU_ENABLE        true

/**
 * Value to be passed to gin.get_intr_ctrl_msg to disable routing the interrupt
 * to the CPU.
 */
#define NVGPU_GIN_CPU_DISABLE       false

/**
 * Value to be passed to gin.get_intr_ctrl_msg to enable routing the interrupt
 * to the GSP.
 */
#define NVGPU_GIN_GSP_ENABLE        true

/**
 * Value to be passed to gin.get_intr_ctrl_msg to disable routing the interrupt
 * to the GSP.
 */
#define NVGPU_GIN_GSP_DISABLE       false

/**
 * GIN interrupt for the XAL-EP unit.
 */
#define NVGPU_GIN_INTR_UNIT_XAL_EP                                      0U

/**
 * GIN interrupt for the PRIV RING unit.
 */
#define NVGPU_GIN_INTR_UNIT_PRIV_RING                                   1U

/**
 * GIN interrupt for the MMU unit.
 */
#define NVGPU_GIN_INTR_UNIT_MMU                                         2U
#define NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_ECC_ERROR                       0U
#define NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_INFO_FAULT                      1U
#define NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT                2U
#define NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT            3U
#define NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT_ERROR          4U
#define NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT_ERROR      5U

/**
 * GIN interrupt for GR engines.
 */
#define NVGPU_GIN_INTR_UNIT_GR                                          3U

/**
 * GIN interrupt for CE engines.
 */
#define NVGPU_GIN_INTR_UNIT_CE                                          4U

/**
 * GIN interrupt for SEC engines.
 */
#define NVGPU_GIN_INTR_UNIT_SEC                                         5U

/**
 * GIN interrupt for NVENC engines.
 */
#define NVGPU_GIN_INTR_UNIT_NVENC                                       6U

/**
 * GIN interrupt for NVDEC engines.
 */
#define NVGPU_GIN_INTR_UNIT_NVDEC                                       7U

/**
 * GIN interrupt for NVJPG engines.
 */
#define NVGPU_GIN_INTR_UNIT_NVJPG                                       8U

/**
 * GIN interrupt for OFA engines.
 */
#define NVGPU_GIN_INTR_UNIT_OFA                                         9U

/**
 * GIN interrupt for runlist tree 0.
 */
#define NVGPU_GIN_INTR_UNIT_RUNLIST_TREE_0                              10U

/**
 * GIN interrupt for runlist tree 1.
 */
#define NVGPU_GIN_INTR_UNIT_RUNLIST_TREE_1                              11U

/**
 * GIN interrupt for the LTC unit.
 */
#define NVGPU_GIN_INTR_UNIT_LTC                                         12U

/**
 * GIN interrupt for the PMU unit.
 */
#define NVGPU_GIN_INTR_UNIT_PMU                                         13U

/**
 * GIN interrupt for the GSP unit.
 */
#define NVGPU_GIN_INTR_UNIT_GSP                                         14U

/**
 * GIN interrupt for the XAL-EP unit.
 */
#define NVGPU_GIN_INTR_UNIT_XTL_EP                                      15U

/**
 * GIN interrupt for the CTCCHI unit.
 */
#define NVGPU_GIN_INTR_UNIT_CTC_CHI                                     16U

/**
 * GIN interrupt for the LRC unit.
 */
#define NVGPU_GIN_INTR_UNIT_LRC                                         17U

/**
 * GIN interrupt for the SYSCTRL unit.
 */
#define NVGPU_GIN_INTR_UNIT_SYSCTRL                                     18U

/**
 * GIN interrupt for the FUSE unit.
 */
#define NVGPU_GIN_INTR_UNIT_FUSE                                        19U

/**
 * Total number of support GIN interrupt units.
 */
#define NVGPU_GIN_INTR_UNIT_MAX                                         20U

/**
 * Given a subtree and leaf bit, returns the interrupt vector that can be
 * used to program unit INTR_CTRL registers.
 */
#define NVGPU_GIN_VECTOR(subtree, leaf) ((64U * subtree) + leaf)

/**
 * Returns the CPU_INTR_TOP index that contains the given subtree.
 */
#define NVGPU_GIN_SUBTREE_TO_TOP_IDX(subtree) (subtree / 32U)

/**
 * Returns the CPU_INTR_TOP bit for the given subtree.
 */
#define NVGPU_GIN_SUBTREE_TO_TOP_BIT(subtree) (subtree % 32U)

/**
 * Returns the CPU_INTR_LEAF index for LEAF0 of the given subtree.
 */
#define NVGPU_GIN_SUBTREE_TO_LEAF0(subtree) (nvgpu_safe_add_u32(nvgpu_safe_mult_u32(2U, subtree), 0U))

/**
 * Returns the CPU_INTR_LEAF index for LEAF1 of the given subtree.
 */
#define NVGPU_GIN_SUBTREE_TO_LEAF1(subtree) (nvgpu_safe_add_u32(nvgpu_safe_mult_u32(2U, subtree), 1U))

/**
 * Returns the subtree that contains the given interrupt vector.
 */
#define NVGPU_GIN_VECTOR_TO_SUBTREE(vector) (vector / 64U)

/**
 * Returns the leaf bit that corresponds to the given vector.
 */
#define NVGPU_GIN_VECTOR_TO_LEAF_BIT(vector) (vector % 64U)

/**
 * Function pointer type for GIN interrupt stall handlers. Accepts an
 * optional cookie for use in identifying source or providing unit context.
 */
typedef void (*nvgpu_gin_stall_handler)(struct gk20a *g, u64 cookie);

/**
 * Function pointer type for GIN interrupt nonstall handlers. Accepts an
 * optional cookie for use in identifying source or providing unit context.
 */
typedef u32 (*nvgpu_gin_nonstall_handler)(struct gk20a *g, u64 cookie);

/**
 * The max number of stall and nonstall interrupt vectors that can be configured
 * per GIN interrupt unit.
 */
#define NVGPU_GIN_MAX_VECTORS_PER_UNIT									16U

#define NVGPU_LRC_UE_VECTOR_OFFSET					0U

#define NVGPU_LTC_LEGACY_VECTOR_OFFSET					0U
#define NVGPU_LTC_UE_VECTOR_OFFSET					1U
#define NVGPU_LTC_CE_VECTOR_OFFSET					2U

#define NVGPU_XTL_UE_VECTOR_OFFSET					0U

#define NVGPU_XTL_UE_TREE_OFFSET					0U

#define NVGPU_SYSCTRL_UE_VECTOR_OFFSET				0U

#define NVGPU_OFA_LEGACY_VECTOR_OFFSET					0U
#define NVGPU_OFA_UE_VECTOR_OFFSET					1U
#define NVGPU_OFA_CE_VECTOR_OFFSET					2U

#define NVGPU_XAL_UE_VECTOR_OFFSET					0U
#define NVGPU_XAL_CE_VECTOR_OFFSET					1U

#define NVGPU_FUSE_UE_VECTOR_OFFSET					0U

#define NVGPU_GSP_LEGACY_VECTOR_OFFSET					0U
#define NVGPU_GSP_UE_VECTOR_OFFSET					1U
#define NVGPU_GSP_CE_VECTOR_OFFSET					2U

#define NVGPU_CTC_CHI_UE_VECTOR_OFFSET				0U

#define NVGPU_NVENC_LEGACY_VECTOR_OFFSET_0				0U
#define NVGPU_NVENC_LEGACY_VECTOR_OFFSET_1				1U
#define NVGPU_NVENC_UE_VECTOR_OFFSET_0					2U
#define NVGPU_NVENC_UE_VECTOR_OFFSET_1					3U
#define NVGPU_NVENC_CE_VECTOR_OFFSET_0					4U
#define NVGPU_NVENC_CE_VECTOR_OFFSET_1					5U

struct nvgpu_gin_vector_config {
	u32 num_stall_vectors;
	u32 num_nonstall_vectors;
	u32 stall_vectors[NVGPU_GIN_MAX_VECTORS_PER_UNIT];
	u32 nonstall_vectors[NVGPU_GIN_MAX_VECTORS_PER_UNIT];
};

struct nvgpu_gin_config {
	u64 stall_intr_top_mask;
	u64 nonstall_intr_top_mask;
	struct nvgpu_gin_vector_config vector_configs[NVGPU_GIN_INTR_UNIT_MAX];
};

struct nvgpu_gin_handler {
	union {
		nvgpu_gin_stall_handler stall_handler;
		nvgpu_gin_nonstall_handler nonstall_handler;
	};
	u64 cookie;
};

struct nvgpu_gin {
	const struct nvgpu_gin_config *config;
	bool is_present;
	struct nvgpu_gin_handler *handlers;
	struct nvgpu_mutex intr_thread_mutex;

	/** Lock to access the interrupt en/clear registers. */
	struct nvgpu_spinlock intr_lock;
};

/**
 * @brief Initialize the GIN unit.
 *
 * @param g [in] - The GPU driver struct.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_gin_init(struct gk20a *g);

/**
 * @brief Retrieve the assigned stall interrupt vector for a given
 *        unit/subunit combination.
 *
 * @param g [in]       - The GPU driver struct.
 * @param unit [in]    - The GIN interrupt unit.
 * @param subunit [in] - The GIN interrupt subunit within \a unit.
 *
 * @return the unit's stall interrupt vector.
 */
u32 nvgpu_gin_get_unit_stall_vector(struct gk20a *g, u32 unit, u32 subunit);

/**
 * @brief Retrieve the assigned nonstall interrupt vector for a given
 *        unit/subunit combination.
 *
 * @param g [in]       - The GPU driver struct.
 * @param unit [in]    - The GIN interrupt unit.
 * @param subunit [in] - The GIN interrupt subunit within \a unit.
 *
 * @return the unit's nonstall interrupt vector.
 */
u32 nvgpu_gin_get_unit_nonstall_vector(struct gk20a *g, u32 unit, u32 subunit);

/**
 * @brief Retrieve a value that may be programmed into a unit's INTR_CTRL
 *        register to configure the interrupt vector and routing.
 *
 * @param g [in]          - The GPU driver struct.
 * @param vector [in]     - The interrupt vector to be programmed into the
 *                          INTR_CTRL register.
 * @param cpu_enable [in] - Indicates whether or not the interrupt should be
 *                          routed to the CPU.
 * @param gsp_enable [in] - Indicates whether or not the interrupt should be
 *                          routed to the GSP.
 *
 * @return the value that may written to an INTR_CTRL register.
 */
u32 nvgpu_gin_get_intr_ctrl_msg(struct gk20a *g, u32 vector,
		bool cpu_enable, bool gsp_enable);

/**
 * @brief Retrieve the interrupt vector from a value that is formatted for
 *        programming into an INTR_CTRL register.
 *
 * @param g [in]             - The GPU driver struct.
 * @param intr_ctrl_msg [in] - The value that is formatted for programming into an
 *                             INTR_CTRL register.
 *
 * @return the interrupt vector.
 */
u32 nvgpu_gin_get_vector_from_intr_ctrl_msg(struct gk20a *g, u32 intr_ctrl_msg);

/**
 * @brief Set the stall interrupt handler for a given interrupt vector.
 *
 * @param g [in]       - The GPU driver struct.
 * @param vector [in]  - The interrupt vector to configure.
 * @param handler [in] - The stall interrupt handler that will be called when
 *                       \a vector is triggered.
 * @param cookie [in]  - Optional cookie that will be passed to the handler.
 */
void nvgpu_gin_set_stall_handler(struct gk20a *g, u32 vector,
		nvgpu_gin_stall_handler handler, u64 cookie);

/**
 * @brief Set the nonstall interrupt handler for a given interrupt vector.
 *
 * @param g [in]       - The GPU driver struct.
 * @param vector [in]  - The interrupt vector to configure.
 * @param handler [in] - The nonstall interrupt handler that will be called when
 *                       \a vector is active.
 * @param cookie [in]  - Optional cookie that will be passed to the handler.
 */
void nvgpu_gin_set_nonstall_handler(struct gk20a *g, u32 vector,
		nvgpu_gin_nonstall_handler handler, u64 cookie);

/**
 * @brief Enable/disable the stall interrupt for a given unit/subunit
 *        combination.
 *
 * @param g [in]       - The GPU driver struct.
 * @param unit [in]    - The GIN interrupt unit to be configured.
 * @param subunit [in] - The GIN interrupt subunit within \a unit to be
 *                       configured.
 * @param enable [in]  - Indicates whether the interrupt should be enabled or
 *                       disabled.
 */
void nvgpu_gin_stall_unit_config(struct gk20a *g, u32 unit, u32 subunit, bool enable);

/**
 * @brief Enable/disable the nonstall interrupt for a given unit/subunit
 *        combination.
 *
 * @param g [in]       - The GPU driver struct.
 * @param unit [in]    - The GIN interrupt unit to be configured.
 * @param subunit [in] - The GIN interrupt subunit within \a unit to be
 *                       configured.
 * @param enable [in]  - Indicates whether the interrupt should be enabled or
 *                       disabled.
 */
void nvgpu_gin_nonstall_unit_config(struct gk20a *g, u32 unit, u32 subunit, bool enable);

/**
 * @brief Check if GIN is present and supported.
 *
 * @param g [in] - The GPU driver struct.
 *
 * @return true if GIN is present and supported, false otherwise.
 */
bool nvgpu_gin_is_present(struct gk20a *g);

/**
 * @brief Mask all interrupts.
 *
 * @param g [in] - The GPU driver struct.
 */
void nvgpu_gin_intr_mask_all(struct gk20a *g);

/**
 * @brief Pause all stall interrupts.
 *
 * @param g [in] - The GPU driver struct.
 * @param intr_mask [in] - The mask of interrupts to be disabled.
 */
void nvgpu_gin_intr_stall_pause(struct gk20a *g, u64 intr_mask);

/**
 * @brief Resume all stall interrupts.
 *
 * @param g [in] - The GPU driver struct.
 * @param intr_mask [in] - The mask of interrupts to be enabled.
 */
void nvgpu_gin_intr_stall_resume(struct gk20a *g, u64 intr_mask);

/**
 * @brief Pause all nonstall interrupts.
 *
 * @param g [in] - The GPU driver struct.
 */
void nvgpu_gin_intr_nonstall_pause(struct gk20a *g);

/**
 * @brief Resume all nonstall interrupts.
 *
 * @param g [in] - The GPU driver struct.
 */
void nvgpu_gin_intr_nonstall_resume(struct gk20a *g);

/**
 * @brief Check for pending stall interrupts. Runs in interrupt context.
 *
 * @param g [in]             - The GPU driver struct.
 * @param intr_top_mask [in] - Mask of top bits that should be checked by
 *                             this ISR.
 *
 * @return mask of pending top bits.
 */
u64 nvgpu_gin_stall_isr(struct gk20a *g, u64 intr_top_mask);

void nvgpu_gin_stall_isr_handle_subtree(struct gk20a *g, u32 subtree, u64 leaf_mask);

bool nvgpu_gin_handle_gr_illegal_method(struct gk20a *g, u64 top_pending);

bool nvgpu_gin_handle_nvdec_swgen0(struct gk20a *g, u64 top_pending);

/**
 * @brief Handle pending stall interrupts. Runs in thread context.
 *
 * @param g [in]             - The GPU driver struct.
 * @param intr_top_mask [in] - Mask of top bits that should be handled by
 *                             this ISR.
 */
void nvgpu_gin_stall_isr_thread(struct gk20a *g, u64 intr_top_mask);

/**
 * @brief Check for pending nonstall interrupts. Runs in interrupt context.
 *
 * @param g [in]             - The GPU driver struct.
 * @param intr_top_mask [in] - Mask of top bits that should be checked by
 *                             this ISR.
 *
 * @return mask of pending top bits.
 */
u64 nvgpu_gin_nonstall_isr(struct gk20a *g, u64 intr_top_mask);

/**
 * @brief Handle pending nonstall interrupts. Runs in thread context.
 *
 * @param g [in]             - The GPU driver struct.
 * @param intr_top_mask [in] - Mask of top bits that should be handled by
 *                             this ISR.
 *
 * @return bitwise OR of nonstall operations that need to be handled.
 */
u32 nvgpu_gin_nonstall_isr_thread(struct gk20a *g, u64 intr_top_mask);

/**
 * @brief Check if an MSI/MSI-X is a stall interrupt.
 *
 * @param g [in]  - The GPU driver struct.
 * @param id [in] - The MSI/MSI-X id.
 *
 * @return true if the MSI/MSI-X is a stall interrupt, false otherwise.
 */
bool nvgpu_gin_msi_is_stall(struct gk20a *g, u32 id);

/**
 * @brief Check if an MSI/MSI-X is a nonstall interrupt.
 *
 * @param g [in]  - The GPU driver struct.
 * @param id [in] - The MSI/MSI-X id.
 *
 * @return true if the MSI/MSI-X is a nonstall interrupt, false otherwise.
 */
bool nvgpu_gin_msi_is_nonstall(struct gk20a *g, u32 id);

/**
 * @brief Clear pending interrupt, if any, for the specified vector.
 *
 * @param g [in]      - The GPU driver struct.
 * @param vector [in] - The interrupt vector to clear.
 */
void nvgpu_gin_clear_pending_intr(struct gk20a *g, u32 vector);

#endif /* NVGPU_GIN_H */
