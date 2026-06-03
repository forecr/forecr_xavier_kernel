/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_WATCHDOG_H
#define NVGPU_WATCHDOG_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel_wdt;

struct nvgpu_channel_wdt_state {
	u64 gp_get;
	u64 pb_get;
};

#ifdef CONFIG_NVGPU_CHANNEL_WDT

struct nvgpu_channel_wdt *nvgpu_channel_wdt_alloc(struct gk20a *g);
void nvgpu_channel_wdt_destroy(struct nvgpu_channel_wdt *wdt);

void nvgpu_channel_wdt_enable(struct nvgpu_channel_wdt *wdt);
void nvgpu_channel_wdt_disable(struct nvgpu_channel_wdt *wdt);
bool nvgpu_channel_wdt_enabled(struct nvgpu_channel_wdt *wdt);

void nvgpu_channel_wdt_set_limit(struct nvgpu_channel_wdt *wdt, u32 limit_ms);
u32 nvgpu_channel_wdt_limit(struct nvgpu_channel_wdt *wdt);

void nvgpu_channel_wdt_start(struct nvgpu_channel_wdt *wdt,
		struct nvgpu_channel_wdt_state *state);
bool nvgpu_channel_wdt_stop(struct nvgpu_channel_wdt *wdt);
void nvgpu_channel_wdt_continue(struct nvgpu_channel_wdt *wdt);
void nvgpu_channel_wdt_rewind(struct nvgpu_channel_wdt *wdt,
		struct nvgpu_channel_wdt_state *state);
bool nvgpu_channel_wdt_running(struct nvgpu_channel_wdt *wdt);
bool nvgpu_channel_wdt_check(struct nvgpu_channel_wdt *wdt,
		struct nvgpu_channel_wdt_state *state);

#else /* CONFIG_NVGPU_CHANNEL_WDT */

/**
 * @brief This function does nothing on safety build.
 * @return returns NULL on safety build
 */
static inline struct nvgpu_channel_wdt *nvgpu_channel_wdt_alloc(
		struct gk20a *g)
{
	(void)g;
	return NULL;
}

/**
 * @brief This function does nothing on safety build.
 */
static inline void nvgpu_channel_wdt_destroy(struct nvgpu_channel_wdt *wdt)
{
	(void)wdt;
}

/**
 * @brief This function does nothing on safety build.
 */
static inline void nvgpu_channel_wdt_enable(struct nvgpu_channel_wdt *wdt)
{
	(void)wdt;
}

/**
 * @brief This function does nothing on safety build.
 */
static inline void nvgpu_channel_wdt_disable(struct nvgpu_channel_wdt *wdt)
{
	(void)wdt;
}

/**
 * @brief This function does nothing on safety build.
 * @return returns false on safety build
 */
static inline bool nvgpu_channel_wdt_enabled(struct nvgpu_channel_wdt *wdt)
{
	(void)wdt;
	return false;
}

/**
 * @brief This function does nothing on safety build.
 */
static inline void nvgpu_channel_wdt_set_limit(struct nvgpu_channel_wdt *wdt,
		u32 limit_ms)
{
	(void)wdt;
	(void)limit_ms;
}

/**
 * @brief This function does nothing on safety build.
 * @return returns 0 on safety build
 */
static inline u32 nvgpu_channel_wdt_limit(struct nvgpu_channel_wdt *wdt)
{
	(void)wdt;
	return 0U;
}

/**
 * @brief This function does nothing on safety build.
 */
static inline void nvgpu_channel_wdt_start(struct nvgpu_channel_wdt *wdt,
		struct nvgpu_channel_wdt_state *state)
{
	(void)wdt;
	(void)state;
}

/**
 * @brief This function does nothing on safety build.
 * @return returns false on safety build
 */
static inline bool nvgpu_channel_wdt_stop(struct nvgpu_channel_wdt *wdt)
{
	(void)wdt;
	return false;
}
/**
 * @brief This function does nothing on safety build.
 */
static inline void nvgpu_channel_wdt_continue(struct nvgpu_channel_wdt *wdt)
{
	(void)wdt;
}
/**
 * @brief This function does nothing on safety build.
 */
static inline void nvgpu_channel_wdt_rewind(struct nvgpu_channel_wdt *wdt,
		struct nvgpu_channel_wdt_state *state)
{
	(void)wdt;
	(void)state;
}
/**
 * @brief This function does nothing on safety build.
 * @return returns false on safety build
 */
static inline bool nvgpu_channel_wdt_check(struct nvgpu_channel_wdt *wdt,
		struct nvgpu_channel_wdt_state *state) {
	(void)wdt;
	(void)state;
	return false;
}

#endif /* CONFIG_NVGPU_CHANNEL_WDT */

#endif
