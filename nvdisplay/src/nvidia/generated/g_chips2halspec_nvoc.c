#define NVOC_CHIPS2HALSPEC_H_PRIVATE_ACCESS_ALLOWED

// Version of generated metadata structures
#ifdef NVOC_METADATA_VERSION
#undef NVOC_METADATA_VERSION
#endif
#define NVOC_METADATA_VERSION 2

#include "nvoc/runtime.h"
#include "nvoc/rtti.h"
#include "nvtypes.h"
#include "nvport/nvport.h"
#include "nvport/inline/util_valist.h"
#include "utils/nvassert.h"
#include "g_chips2halspec_nvoc.h"

void __nvoc_init_halspec_ChipHal(ChipHal *pChipHal, NvU32 arch, NvU32 impl, NvU32 hidrev)
{
    // T234D
    if(arch == 0x0 && impl == 0x0 && hidrev == 0x235)
    {
        pChipHal->__nvoc_HalVarIdx = 108;
    }
    // T264D
    else if(arch == 0x0 && impl == 0x0 && hidrev == 0x265)
    {
        pChipHal->__nvoc_HalVarIdx = 110;
    }
    // T256D
    else if(arch == 0x0 && impl == 0x0 && hidrev == 0x257)
    {
        pChipHal->__nvoc_HalVarIdx = 111;
    }
}

void __nvoc_init_halspec_TegraChipHal(TegraChipHal *pTegraChipHal, TEGRA_CHIP_TYPE tegraType)
{
    // TEGRA_PCIE
    if(tegraType == 0x0)
    {
        pTegraChipHal->__nvoc_HalVarIdx = 0;
    }
    // TEGRA_SOC
    else if(tegraType == 0x1)
    {
        pTegraChipHal->__nvoc_HalVarIdx = 1;
    }
}

void __nvoc_init_halspec_RmVariantHal(RmVariantHal *pRmVariantHal, RM_RUNTIME_VARIANT rmVariant)
{
    // PF_KERNEL_ONLY
    if(rmVariant == 0x2)
    {
        pRmVariantHal->__nvoc_HalVarIdx = 1;
    }
}

void __nvoc_init_halspec_DispIpHal(DispIpHal *pDispIpHal, NvU32 ipver)
{
    // DISPv0402
    if(ipver == 0x4020000)
    {
        pDispIpHal->__nvoc_HalVarIdx = 12;
    }
    // DISPv0501
    else if(ipver == 0x5010000)
    {
        pDispIpHal->__nvoc_HalVarIdx = 15;
    }
    // DISPv0504
    else if(ipver == 0x5040000)
    {
        pDispIpHal->__nvoc_HalVarIdx = 18;
    }
}

void __nvoc_init_halspec_DpuIpHal(DpuIpHal *pDpuIpHal, NvU32 ipver)
{
    // DPUv0000
    if(ipver == 0x0)
    {
        pDpuIpHal->__nvoc_HalVarIdx = 5;
    }
}

