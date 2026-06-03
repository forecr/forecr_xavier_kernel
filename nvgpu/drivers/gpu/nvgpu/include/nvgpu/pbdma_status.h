/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_STATUS_H
#define NVGPU_PBDMA_STATUS_H

#include <nvgpu/types.h>
/**
 * @file
 *
 * Abstract interface for interpreting pbdma status info read from h/w.
 */
/**
 * H/w defined value for Channel ID type in pbdma status h/w register.
 */
#define PBDMA_STATUS_ID_TYPE_CHID 0U
/**
 * H/w defined value for Tsg ID type in pbdma status h/w register.
 */
#define PBDMA_STATUS_ID_TYPE_TSGID 1U
/**
 * S/w defined value for unknown ID type.
 */
#define PBDMA_STATUS_ID_TYPE_INVALID (~U32(0U))
/**
 * H/w defined value for next Channel ID type in pbdma status h/w register.
 */
#define PBDMA_STATUS_NEXT_ID_TYPE_CHID PBDMA_STATUS_ID_TYPE_CHID
/**
 * H/w defined value for next Tsg ID type in pbdma status h/w register.
 */
#define PBDMA_STATUS_NEXT_ID_TYPE_TSGID PBDMA_STATUS_ID_TYPE_TSGID
/**
 * S/w defined value for unknown ID type.
 */
#define PBDMA_STATUS_NEXT_ID_TYPE_INVALID PBDMA_STATUS_ID_TYPE_INVALID

/**
 * S/w defined value for unknown ID.
 */
#define PBDMA_STATUS_ID_INVALID (~U32(0U))
/**
 * S/w defined value for unknown next ID.
 */
#define PBDMA_STATUS_NEXT_ID_INVALID PBDMA_STATUS_ID_INVALID

enum nvgpu_pbdma_status_chsw_status {
	/** Channel is not loaded on pbdma. Both id and next_id are invalid. */
	NVGPU_PBDMA_CHSW_STATUS_INVALID,
	/**
	 * Channel is loaded on the pbdma. id field of pbdma_status
	 * h/w register is valid but next_id is not valid. Also host
	 * is currently not channel switching this pbdma.
	 */
	NVGPU_PBDMA_CHSW_STATUS_VALID,
	/**
	 * Host is loading a new channel and the previous channel is
	 * invalid. In this state only next_id is valid.
	 */
	NVGPU_PBDMA_CHSW_STATUS_LOAD,
	/**
	 * Host is saving the current channel and not loading a new one.
	 * In this state only id is valid.
	 */
	NVGPU_PBDMA_CHSW_STATUS_SAVE,
	/**
	 * Host is switching between two valid channels. In this state both
	 * id and next_id are valid.
	 */
	NVGPU_PBDMA_CHSW_STATUS_SWITCH,
};

struct nvgpu_pbdma_status_info {
	/** Pbdma_status h/w register's read value. */
	u32 pbdma_reg_status;
	/** Chan_status field of pbdma_status h/w register. */
	u32 pbdma_channel_status;
	/** Channel or tsg id of the context currently loaded on the pbdma. */
	u32 id;
	/** Specifies whether #id is of channel or tsg type. */
	u32 id_type;
	/** gfid for the loaded tsg */
	u32 gfid;
	/** Channel or tsg id of the next context to be loaded on the pbdma. */
	u32 next_id;
	/** Specifies whether #next_id is of channel or tsg type. */
	u32 next_id_type;
	/** gfid for the next tsg */
	u32 next_gfid;
	/** Enum for chan_status field of pbdma_status h/w register. */
	enum nvgpu_pbdma_status_chsw_status chsw_status;
};

/**
 * @brief Check if chsw_status is set to switch.
 *
 * @param pbdma_status [in]	Pointer to struct containing pbdma_status h/w
 * 				reg/field value.
 *
 * @return Interprets #pbdma_status and returns true if channel
 *         status is set to #NVGPU_PBDMA_CHSW_STATUS_SWITCH else returns false.
 */
bool nvgpu_pbdma_status_is_chsw_switch(struct nvgpu_pbdma_status_info
		*pbdma_status);
/**
 * @brief Check if chsw_status is set to load.
 *
 * @param pbdma_status [in]	Pointer to struct containing pbdma_status h/w
 * 				reg/field value.
 *
 * @return Interprets #pbdma_status and returns true if channel
 *         status is set to #NVGPU_PBDMA_CHSW_STATUS_LOAD else returns false.
 */
bool nvgpu_pbdma_status_is_chsw_load(struct nvgpu_pbdma_status_info
		*pbdma_status);
/**
 * @brief Check if chsw_status is set to save.
 *
 * @param pbdma_status [in]	Pointer to struct containing pbdma_status h/w
 * 				reg/field value.
 *
 * @return Interprets #pbdma_status and returns true if channel
 *         status is set to #NVGPU_PBDMA_CHSW_STATUS_SAVE else returns false.
 */
bool nvgpu_pbdma_status_is_chsw_save(struct nvgpu_pbdma_status_info
		*pbdma_status);
/**
 * @brief Check if chsw_status is set to valid.
 *
 * @param pbdma_status [in]	Pointer to struct containing pbdma_status h/w
 * 				reg/field value.
 *
 * @return Interprets #pbdma_status and returns true if channel
 *         status is set to #NVGPU_PBDMA_CHSW_STATUS_VALID else returns false.
 */
bool nvgpu_pbdma_status_is_chsw_valid(struct nvgpu_pbdma_status_info
		*pbdma_status);
/**
 * @brief Check if chsw_status is set to invalid.
 *
 * @param pbdma_status [in]	Pointer to struct containing pbdma_status h/w
 *				reg/field value.
 *
 * @return Interprets #pbdma_status and returns true if channel
 *         status is set to #NVGPU_PBDMA_CHSW_STATUS_INVALID else returns false.
 */
bool nvgpu_pbdma_status_ch_not_loaded(struct nvgpu_pbdma_status_info
		*pbdma_status);
/**
 * @brief Check if id_type is tsg.
 *
 * @param pbdma_status [in]	Pointer to struct containing pbdma_status h/w
 * 				reg/field value.
 *
 * @return Interprets #pbdma_status and returns true if id_type
 *         is #PBDMA_STATUS_ID_TYPE_TSGID else returns false.
 */
bool nvgpu_pbdma_status_is_id_type_tsg(struct nvgpu_pbdma_status_info
		*pbdma_status);
/**
 * @brief Check if next_id_type is tsg.
 *
 * @param pbdma_status [in]	Pointer to struct containing pbdma_status h/w
 * 				reg/field value.
 *
 * @return Interprets #pbdma_status and returns true if next_id_type
 *         is #PBDMA_STATUS_NEXT_ID_TYPE_TSGID else returns false.
 */
bool nvgpu_pbdma_status_is_next_id_type_tsg(struct nvgpu_pbdma_status_info
		*pbdma_status);

#endif /* NVGPU_PBDMA_STATUS_H */
