// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/errata.h>
#include <nvgpu/bitops.h>
#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/utils.h>

#if defined(CONFIG_NVGPU_NON_FUSA)
/**
 * Array of flag names
 */
#define DEFINE_ERRATA(flag, chip, desc) [flag] = nvgpu_stringify(flag)
static const char *errata_flag_names[NVGPU_MAX_ERRATA_BITS + 1U] = {
	ERRATA_FLAGS_NEXT
	ERRATA_FLAGS
};
NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 20_5), "SWE-NVGPU-109-SWSADR.docx")
#undef DEFINE_ERRATA

/**
 * Array of flag descriptions
 */
#define DEFINE_ERRATA(flag, chip, desc) [flag] = desc
static const char *errata_flag_desc[NVGPU_MAX_ERRATA_BITS + 1U] = {
	ERRATA_FLAGS_NEXT
	ERRATA_FLAGS
};
NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 20_5), "SWE-NVGPU-109-SWSADR.docx")
#undef DEFINE_ERRATA

/**
 * Array of chips where errata was first discovered
 */
#define DEFINE_ERRATA(flag, chip, desc) [flag] = chip
static const char *errata_flag_chip[NVGPU_MAX_ERRATA_BITS + 1U] = {
	ERRATA_FLAGS_NEXT
	ERRATA_FLAGS
};
NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 20_5), "SWE-NVGPU-109-SWSADR.docx")
#undef DEFINE_ERRATA

void nvgpu_print_errata_flags(struct gk20a *g)
{
	u32 i;

	nvgpu_log(g, gpu_dbg_info, "NVGPU Erratas present in chip");
	nvgpu_log(g, gpu_dbg_info, "%-55.55s %-5.5s %s",
			"Flag", "Chip", "Description");
	nvgpu_log(g, gpu_dbg_info, "%-55.55s %-5.5s %s",
			"----", "-----", "-----------");

	for (i = 0U; i < U32(NVGPU_MAX_ERRATA_BITS); i++) {
		/* Only print erratas present in chip */
		if (nvgpu_is_errata_present(g, i)) {
			nvgpu_log(g, gpu_dbg_info, "%-55.55s %-5.5s %s",
			errata_flag_names[i],
			errata_flag_chip[i],
			errata_flag_desc[i]);
		}
	}
}
#endif

int nvgpu_init_errata_flags(struct gk20a *g)
{
	/*
	 * Zero all flags initially. Flags that should be set to non-zero states
	 * can be done so during hal init.
	 */
	g->errata_flags = nvgpu_kzalloc(g,
				BITS_TO_LONGS(U32(NVGPU_MAX_ERRATA_BITS)) *
				sizeof(unsigned long));
	if (g->errata_flags == NULL) {
		return -ENOMEM;
	}

	return 0;
}

/*
 * Call this on driver shutdown!
 */
void nvgpu_free_errata_flags(struct gk20a *g)
{
	nvgpu_kfree(g, g->errata_flags);
}

bool nvgpu_is_errata_present(struct gk20a *g, u32 flag)
{
	if (flag < NVGPU_MAX_ERRATA_BITS) {
		return nvgpu_test_bit(flag, g->errata_flags);
	} else {
		return 0;
	}
}

void nvgpu_set_errata(struct gk20a *g, u32 flag, bool state)
{
	if (flag >= NVGPU_MAX_ERRATA_BITS) {
		return;
	}

	if (state) {
		nvgpu_set_bit(flag, g->errata_flags);
	} else {
		nvgpu_clear_bit(flag, g->errata_flags);
	}
}
