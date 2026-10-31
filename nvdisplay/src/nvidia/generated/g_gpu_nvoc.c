#define NVOC_GPU_H_PRIVATE_ACCESS_ALLOWED

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
#include "g_gpu_nvoc.h"


#ifdef DEBUG
char __nvoc_class_id_uniqueness_check__0x7ef3cb = 1;
#endif

extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJGPU;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_Object;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_GpuHalspecOwner;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_RmHalspecOwner;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJTRACEABLE;

// Forward declarations for OBJGPU
void __nvoc_init__Object(Object*);
void __nvoc_init__GpuHalspecOwner(GpuHalspecOwner*,
        NvU32 ChipHal_arch, NvU32 ChipHal_impl, NvU32 ChipHal_hidrev,
        TEGRA_CHIP_TYPE TegraChipHal_tegraType);
void __nvoc_init__RmHalspecOwner(RmHalspecOwner*,
        RM_RUNTIME_VARIANT RmVariantHal_rmVariant,
        NvU32 DispIpHal_ipver);
void __nvoc_init__OBJTRACEABLE(OBJTRACEABLE*);
void __nvoc_init__OBJGPU(OBJGPU*,
        NvU32 ChipHal_arch, NvU32 ChipHal_impl, NvU32 ChipHal_hidrev,
        TEGRA_CHIP_TYPE TegraChipHal_tegraType,
        RM_RUNTIME_VARIANT RmVariantHal_rmVariant,
        NvU32 DispIpHal_ipver);
void __nvoc_init_funcTable_OBJGPU(OBJGPU*);
NV_STATUS __nvoc_ctor_OBJGPU(OBJGPU*, NvU32 arg_gpuInstance, NvU32 arg_gpuId, NvUuid *arg_pUuid, struct GpuArch *arg_pGpuArch);
void __nvoc_init_dataField_OBJGPU(OBJGPU*);
void __nvoc_dtor_OBJGPU(OBJGPU*);

// Structures used within RTTI (run-time type information)
extern const struct NVOC_CASTINFO __nvoc_castinfo__OBJGPU;
extern const struct NVOC_EXPORT_INFO __nvoc_export_info__OBJGPU;

// Down-thunk(s) to bridge OBJGPU methods from ancestors (if any)

// Up-thunk(s) to bridge OBJGPU methods to ancestors (if any)

const struct NVOC_CLASS_DEF __nvoc_class_def_OBJGPU = 
{
    /*classInfo=*/ {
        /*size=*/               sizeof(OBJGPU),
        /*classId=*/            classId(OBJGPU),
        /*providerId=*/         &__nvoc_rtti_provider,
#if NV_PRINTF_STRINGS_ALLOWED
        /*name=*/               "OBJGPU",
#endif
    },
    /*objCreatefn=*/        (NVOC_DYNAMIC_OBJ_CREATE) &__nvoc_objCreateDynamic_OBJGPU,
    /*pCastInfo=*/          &__nvoc_castinfo__OBJGPU,
    /*pExportInfo=*/        &__nvoc_export_info__OBJGPU
};


// Metadata with per-class RTTI with ancestor(s)
static const struct NVOC_METADATA__OBJGPU __nvoc_metadata__OBJGPU = {
    .rtti.pClassDef = &__nvoc_class_def_OBJGPU,    // (gpu) this
    .rtti.dtor      = (NVOC_DYNAMIC_DTOR) &__nvoc_dtor_OBJGPU,
    .rtti.offset    = 0,
    .metadata__Object.rtti.pClassDef = &__nvoc_class_def_Object,    // (obj) super
    .metadata__Object.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__Object.rtti.offset    = NV_OFFSETOF(OBJGPU, __nvoc_base_Object),
    .metadata__GpuHalspecOwner.rtti.pClassDef = &__nvoc_class_def_GpuHalspecOwner,    // (gpuhalspecowner) super
    .metadata__GpuHalspecOwner.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__GpuHalspecOwner.rtti.offset    = NV_OFFSETOF(OBJGPU, __nvoc_base_GpuHalspecOwner),
    .metadata__RmHalspecOwner.rtti.pClassDef = &__nvoc_class_def_RmHalspecOwner,    // (rmhalspecowner) super
    .metadata__RmHalspecOwner.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__RmHalspecOwner.rtti.offset    = NV_OFFSETOF(OBJGPU, __nvoc_base_RmHalspecOwner),
    .metadata__OBJTRACEABLE.rtti.pClassDef = &__nvoc_class_def_OBJTRACEABLE,    // (traceable) super
    .metadata__OBJTRACEABLE.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__OBJTRACEABLE.rtti.offset    = NV_OFFSETOF(OBJGPU, __nvoc_base_OBJTRACEABLE),
};


// Dynamic down-casting information
const struct NVOC_CASTINFO __nvoc_castinfo__OBJGPU = {
    .numRelatives = 5,
    .relatives = {
        &__nvoc_metadata__OBJGPU.rtti,    // [0]: (gpu) this
        &__nvoc_metadata__OBJGPU.metadata__Object.rtti,    // [1]: (obj) super
        &__nvoc_metadata__OBJGPU.metadata__GpuHalspecOwner.rtti,    // [2]: (gpuhalspecowner) super
        &__nvoc_metadata__OBJGPU.metadata__RmHalspecOwner.rtti,    // [3]: (rmhalspecowner) super
        &__nvoc_metadata__OBJGPU.metadata__OBJTRACEABLE.rtti,    // [4]: (traceable) super
    }
};

const struct NVOC_EXPORT_INFO __nvoc_export_info__OBJGPU = 
{
    /*numEntries=*/     0,
    /*pExportEntries=*/  0
};

void __nvoc_dtor_Object(Object*);
void __nvoc_dtor_GpuHalspecOwner(GpuHalspecOwner*);
void __nvoc_dtor_RmHalspecOwner(RmHalspecOwner*);
void __nvoc_dtor_OBJTRACEABLE(OBJTRACEABLE*);
void __nvoc_dtor_OBJGPU(OBJGPU *pThis) {
    __nvoc_gpuDestruct(pThis);
    __nvoc_dtor_Object(&pThis->__nvoc_base_Object);
    __nvoc_dtor_GpuHalspecOwner(&pThis->__nvoc_base_GpuHalspecOwner);
    __nvoc_dtor_RmHalspecOwner(&pThis->__nvoc_base_RmHalspecOwner);
    __nvoc_dtor_OBJTRACEABLE(&pThis->__nvoc_base_OBJTRACEABLE);
    PORT_UNREFERENCED_VARIABLE(pThis);
}

void __nvoc_init_dataField_OBJGPU(OBJGPU *pThis) {
    ChipHal *chipHal = &staticCast(pThis, GpuHalspecOwner)->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    RmVariantHal *rmVariantHal = &staticCast(pThis, RmHalspecOwner)->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);
    pThis->setProperty(pThis, PDB_PROP_GPU_IS_CONNECTED, NV_TRUE);

    // NVOC Property Hal field -- PDB_PROP_GPU_KEEP_WPR_ACROSS_GC6_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_KEEP_WPR_ACROSS_GC6_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_TEGRA_SOC_NVDISPLAY
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_TEGRA_SOC_NVDISPLAY, NV_TRUE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_ATS_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_ATS_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_SC7_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_SC7_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_RTD3_RG_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_RTD3_RG_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_TRIGGER_PCIE_FLR
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_TRIGGER_PCIE_FLR, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_CLKS_IN_TEGRA_SOC
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_CLKS_IN_TEGRA_SOC, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_PREINITIALIZED_WPR_REGION
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_PREINITIALIZED_WPR_REGION, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_BUG_3007008_EMULATE_VF_MMU_TLB_INVALIDATE
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_BUG_3007008_EMULATE_VF_MMU_TLB_INVALIDATE, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_ZERO_FB
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_ZERO_FB, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_CAN_OPTIMIZE_COMPUTE_USE_CASE
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_CAN_OPTIMIZE_COMPUTE_USE_CASE, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_MIG_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_MIG_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_MIG_MIRROR_HOST_CI_ON_GUEST
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_MIG_MIRROR_HOST_CI_ON_GUEST, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_MIG_SUPPORTS_SPLIT_CE_RANGES
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_MIG_SUPPORTS_SPLIT_CE_RANGES, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_MIG_GFX_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_MIG_GFX_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_MIG_TIMESLICING_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_MIG_TIMESLICING_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_VC_CAPABILITY_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_VC_CAPABILITY_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_RESETLESS_MIG_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_RESETLESS_MIG_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_IS_COT_ENABLED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_IS_COT_ENABLED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_FW_WPR_OFFSET_SET_BY_ACR
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_FW_WPR_OFFSET_SET_BY_ACR, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_SRIOV_SYSMEM_DIRTY_PAGE_TRACKING_ENABLED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_SRIOV_SYSMEM_DIRTY_PAGE_TRACKING_ENABLED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_VGPU_OFFLOAD_CAPABLE
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_VGPU_OFFLOAD_CAPABLE, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_UNIX_DYNAMIC_POWER_SUPPORTED
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_UNIX_DYNAMIC_POWER_SUPPORTED, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_SKIP_CE_MAPPINGS_NO_NVLINK
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_SKIP_CE_MAPPINGS_NO_NVLINK, NV_TRUE);
    }
    pThis->setProperty(pThis, PDB_PROP_GPU_OPTIMUS_GOLD_CFG_SPACE_RESTORE, NV_TRUE);

    // NVOC Property Hal field -- PDB_PROP_GPU_CC_FEATURE_CAPABLE
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_CC_FEATURE_CAPABLE, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_APM_FEATURE_CAPABLE
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_APM_FEATURE_CAPABLE, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_EXTENDED_GSP_RM_INITIALIZATION_TIMEOUT_FOR_VGX
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_EXTENDED_GSP_RM_INITIALIZATION_TIMEOUT_FOR_VGX, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_CHIP_SUPPORTS_RTD3_DEF
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_CHIP_SUPPORTS_RTD3_DEF, NV_TRUE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_IS_SOC_SDM
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_IS_SOC_SDM, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_DISP_PB_REQUIRES_SMMU_BYPASS
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_DISP_PB_REQUIRES_SMMU_BYPASS, NV_TRUE);
    }
    pThis->setProperty(pThis, PDB_PROP_GPU_FASTPATH_SEQ_ENABLED, NV_FALSE);
    pThis->setProperty(pThis, PDB_PROP_GPU_RECOVERY_DRAIN_P2P_REQUIRED, NV_FALSE);

    // NVOC Property Hal field -- PDB_PROP_GPU_REUSE_INIT_CONTING_MEM
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_REUSE_INIT_CONTING_MEM, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_RUSD_POLLING_SUPPORT_MONOLITHIC
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_RUSD_POLLING_SUPPORT_MONOLITHIC, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_GPU_RUSD_DISABLE_CLK_PUBLIC_DOMAIN_INFO
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_RUSD_DISABLE_CLK_PUBLIC_DOMAIN_INFO, NV_FALSE);
    }
    pThis->setProperty(pThis, PDB_PROP_GPU_RECOVERY_REBOOT_REQUIRED, NV_FALSE);

    // NVOC Property Hal field -- PDB_PROP_GPU_ALLOC_ISO_SYS_MEM_FROM_CARVEOUT
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_GPU_ALLOC_ISO_SYS_MEM_FROM_CARVEOUT, NV_FALSE);
    }

    pThis->deviceInstance = 32;

    // Hal field -- isVirtual
    if (( ((rmVariantHal_HalVarIdx >> 5) == 0UL) && ((1UL << (rmVariantHal_HalVarIdx & 0x1f)) & 0x00000002UL) )) /* RmVariantHal: PF_KERNEL_ONLY */ 
    {
        pThis->isVirtual = NV_FALSE;
    }

    // Hal field -- isGspClient
    if (( ((rmVariantHal_HalVarIdx >> 5) == 0UL) && ((1UL << (rmVariantHal_HalVarIdx & 0x1f)) & 0x00000002UL) )) /* RmVariantHal: PF_KERNEL_ONLY */ 
    {
        if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
        {
            pThis->isGspClient = NV_FALSE;
        }
    }

    // Hal field -- isDceClient
    if (( ((rmVariantHal_HalVarIdx >> 5) == 0UL) && ((1UL << (rmVariantHal_HalVarIdx & 0x1f)) & 0x00000002UL) )) /* RmVariantHal: PF_KERNEL_ONLY */ 
    {
        if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
        {
            pThis->isDceClient = NV_TRUE;
        }
    }

    pThis->bIsDebugModeEnabled = NV_FALSE;

    pThis->numOfMclkLockRequests = 0U;

    pThis->bUseRegisterAccessMap = !(0);

    pThis->boardInfo = ((void *)0);

    // Hal field -- gpuGroupCount
    // default
    {
        pThis->gpuGroupCount = 1;
    }

    pThis->bIsMigRm = NV_FALSE;

    // Hal field -- bUnifiedMemorySpaceEnabled
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bUnifiedMemorySpaceEnabled = NV_TRUE;
    }

    // Hal field -- bWarBug200577889SriovHeavyEnabled
    pThis->bWarBug200577889SriovHeavyEnabled = NV_FALSE;

    // Hal field -- bNonPowerOf2ChannelCountSupported
    pThis->bNonPowerOf2ChannelCountSupported = NV_FALSE;

    // Hal field -- bWarBug4347206PowerCycleOnUnload
    // default
    {
        pThis->bWarBug4347206PowerCycleOnUnload = NV_FALSE;
    }

    // Hal field -- bNeed4kPageIsolation
    // default
    {
        pThis->bNeed4kPageIsolation = NV_FALSE;
    }

    // Hal field -- bInstLoc47bitPaWar
    // default
    {
        pThis->bInstLoc47bitPaWar = NV_FALSE;
    }

    // Hal field -- bIsBarPteInSysmemSupported
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bIsBarPteInSysmemSupported = NV_TRUE;
    }

    // Hal field -- bClientRmAllocatedCtxBuffer
    // default
    {
        pThis->bClientRmAllocatedCtxBuffer = NV_FALSE;
    }

    // Hal field -- bInstanceMemoryAlwaysCached
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bInstanceMemoryAlwaysCached = NV_TRUE;
    }

    // Hal field -- bAPageSizeAllocRetryEnabled
    pThis->bAPageSizeAllocRetryEnabled = NV_FALSE;

    // Hal field -- bComputePolicyTimesliceSupported
    // default
    {
        pThis->bComputePolicyTimesliceSupported = NV_FALSE;
    }

    // Hal field -- bSriovCapable
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bSriovCapable = NV_TRUE;
    }

    // Hal field -- bRecheckSliSupportAtResume
    // default
    {
        pThis->bRecheckSliSupportAtResume = NV_FALSE;
    }

    // Hal field -- bGpuNvEncAv1Supported
    // default
    {
        pThis->bGpuNvEncAv1Supported = NV_FALSE;
    }

    pThis->bIsGspOwnedFaultBuffersEnabled = NV_FALSE;

    // Hal field -- bVfResizableBAR1Supported
    // default
    {
        pThis->bVfResizableBAR1Supported = NV_FALSE;
    }

    // Hal field -- bVoltaHubIntrSupported
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bVoltaHubIntrSupported = NV_TRUE;
    }

    // Hal field -- bUsePmcDeviceEnableForHostEngine
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0000d000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bUsePmcDeviceEnableForHostEngine = NV_TRUE;
    }

    pThis->bBlockNewWorkload = NV_FALSE;
}

NV_STATUS __nvoc_ctor_Object(Object* );
NV_STATUS __nvoc_ctor_GpuHalspecOwner(GpuHalspecOwner* );
NV_STATUS __nvoc_ctor_RmHalspecOwner(RmHalspecOwner* );
NV_STATUS __nvoc_ctor_OBJTRACEABLE(OBJTRACEABLE* );
NV_STATUS __nvoc_ctor_OBJGPU(OBJGPU *pThis, NvU32 arg_gpuInstance, NvU32 arg_gpuId, NvUuid * arg_pUuid, struct GpuArch * arg_pGpuArch) {
    NV_STATUS status = NV_OK;
    status = __nvoc_ctor_Object(&pThis->__nvoc_base_Object);
    if (status != NV_OK) goto __nvoc_ctor_OBJGPU_fail_Object;
    status = __nvoc_ctor_GpuHalspecOwner(&pThis->__nvoc_base_GpuHalspecOwner);
    if (status != NV_OK) goto __nvoc_ctor_OBJGPU_fail_GpuHalspecOwner;
    status = __nvoc_ctor_RmHalspecOwner(&pThis->__nvoc_base_RmHalspecOwner);
    if (status != NV_OK) goto __nvoc_ctor_OBJGPU_fail_RmHalspecOwner;
    status = __nvoc_ctor_OBJTRACEABLE(&pThis->__nvoc_base_OBJTRACEABLE);
    if (status != NV_OK) goto __nvoc_ctor_OBJGPU_fail_OBJTRACEABLE;
    __nvoc_init_dataField_OBJGPU(pThis);

    status = __nvoc_gpuConstruct(pThis, arg_gpuInstance, arg_gpuId, arg_pUuid, arg_pGpuArch);
    if (status != NV_OK) goto __nvoc_ctor_OBJGPU_fail__init;
    goto __nvoc_ctor_OBJGPU_exit; // Success

__nvoc_ctor_OBJGPU_fail__init:
    __nvoc_dtor_OBJTRACEABLE(&pThis->__nvoc_base_OBJTRACEABLE);
__nvoc_ctor_OBJGPU_fail_OBJTRACEABLE:
    __nvoc_dtor_RmHalspecOwner(&pThis->__nvoc_base_RmHalspecOwner);
__nvoc_ctor_OBJGPU_fail_RmHalspecOwner:
    __nvoc_dtor_GpuHalspecOwner(&pThis->__nvoc_base_GpuHalspecOwner);
__nvoc_ctor_OBJGPU_fail_GpuHalspecOwner:
    __nvoc_dtor_Object(&pThis->__nvoc_base_Object);
__nvoc_ctor_OBJGPU_fail_Object:
__nvoc_ctor_OBJGPU_exit:

    return status;
}

// Vtable initialization
static void __nvoc_init_funcTable_OBJGPU_1(OBJGPU *pThis) {
    ChipHal *chipHal = &staticCast(pThis, GpuHalspecOwner)->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    RmVariantHal *rmVariantHal = &staticCast(pThis, RmHalspecOwner)->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);

    // gpuGetIdInfo -- halified (3 hals) body
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* ChipHal: T234D */ 
    {
        pThis->__gpuGetIdInfo__ = &gpuGetIdInfo_T234D;
    }
    else if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00004000UL) )) /* ChipHal: T264D */ 
    {
        pThis->__gpuGetIdInfo__ = &gpuGetIdInfo_T264D;
    }
    else
    {
        pThis->__gpuGetIdInfo__ = &gpuGetIdInfo_T256D;
    }

    // gpuGetChildrenOrder -- halified (2 hals)
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00008000UL) )) /* ChipHal: T256D */ 
    {
        pThis->__gpuGetChildrenOrder__ = &gpuGetChildrenOrder_T256D;
    }
    else
    {
        pThis->__gpuGetChildrenOrder__ = &gpuGetChildrenOrder_T234D;
    }

    // gpuGetChildrenPresent -- halified (2 hals)
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00008000UL) )) /* ChipHal: T256D */ 
    {
        pThis->__gpuGetChildrenPresent__ = &gpuGetChildrenPresent_T256D;
    }
    else
    {
        pThis->__gpuGetChildrenPresent__ = &gpuGetChildrenPresent_T234D;
    }

    // gpuGetEngClassDescriptorList -- halified (3 hals)
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* ChipHal: T234D */ 
    {
        pThis->__gpuGetEngClassDescriptorList__ = &gpuGetEngClassDescriptorList_T234D;
    }
    else if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00004000UL) )) /* ChipHal: T264D */ 
    {
        pThis->__gpuGetEngClassDescriptorList__ = &gpuGetEngClassDescriptorList_T264D;
    }
    else
    {
        pThis->__gpuGetEngClassDescriptorList__ = &gpuGetEngClassDescriptorList_T256D;
    }

    // gpuGetNoEngClassList -- halified (3 hals)
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* ChipHal: T234D */ 
    {
        pThis->__gpuGetNoEngClassList__ = &gpuGetNoEngClassList_T234D;
    }
    else if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00004000UL) )) /* ChipHal: T264D */ 
    {
        pThis->__gpuGetNoEngClassList__ = &gpuGetNoEngClassList_T264D;
    }
    else
    {
        pThis->__gpuGetNoEngClassList__ = &gpuGetNoEngClassList_T256D;
    }
} // End __nvoc_init_funcTable_OBJGPU_1 with approximately 13 basic block(s).


// Initialize vtable(s) for 5 virtual method(s).
void __nvoc_init_funcTable_OBJGPU(OBJGPU *pThis) {

    // Initialize vtable(s) with 5 per-object function pointer(s).
    __nvoc_init_funcTable_OBJGPU_1(pThis);
}

// Initialize newly constructed object.
void __nvoc_init__OBJGPU(OBJGPU *pThis,
        NvU32 ChipHal_arch, NvU32 ChipHal_impl, NvU32 ChipHal_hidrev,
        TEGRA_CHIP_TYPE TegraChipHal_tegraType,
        RM_RUNTIME_VARIANT RmVariantHal_rmVariant,
        NvU32 DispIpHal_ipver) {

    // Initialize pointers to inherited data.
    pThis->__nvoc_pbase_Object = &pThis->__nvoc_base_Object;    // (obj) super
    pThis->__nvoc_pbase_GpuHalspecOwner = &pThis->__nvoc_base_GpuHalspecOwner;    // (gpuhalspecowner) super
    pThis->__nvoc_pbase_RmHalspecOwner = &pThis->__nvoc_base_RmHalspecOwner;    // (rmhalspecowner) super
    pThis->__nvoc_pbase_OBJTRACEABLE = &pThis->__nvoc_base_OBJTRACEABLE;    // (traceable) super
    pThis->__nvoc_pbase_OBJGPU = pThis;    // (gpu) this

    // Recurse to superclass initialization function(s).
    __nvoc_init__Object(&pThis->__nvoc_base_Object);
    __nvoc_init__GpuHalspecOwner(&pThis->__nvoc_base_GpuHalspecOwner, ChipHal_arch, ChipHal_impl, ChipHal_hidrev, TegraChipHal_tegraType);
    __nvoc_init__RmHalspecOwner(&pThis->__nvoc_base_RmHalspecOwner, RmVariantHal_rmVariant, DispIpHal_ipver);
    __nvoc_init__OBJTRACEABLE(&pThis->__nvoc_base_OBJTRACEABLE);

    // Pointer(s) to metadata structures(s)
    pThis->__nvoc_base_Object.__nvoc_metadata_ptr = &__nvoc_metadata__OBJGPU.metadata__Object;    // (obj) super
    pThis->__nvoc_base_GpuHalspecOwner.__nvoc_metadata_ptr = &__nvoc_metadata__OBJGPU.metadata__GpuHalspecOwner;    // (gpuhalspecowner) super
    pThis->__nvoc_base_RmHalspecOwner.__nvoc_metadata_ptr = &__nvoc_metadata__OBJGPU.metadata__RmHalspecOwner;    // (rmhalspecowner) super
    pThis->__nvoc_base_OBJTRACEABLE.__nvoc_metadata_ptr = &__nvoc_metadata__OBJGPU.metadata__OBJTRACEABLE;    // (traceable) super
    pThis->__nvoc_metadata_ptr = &__nvoc_metadata__OBJGPU;    // (gpu) this

    // Initialize per-object vtables.
    __nvoc_init_funcTable_OBJGPU(pThis);
}

NV_STATUS __nvoc_objCreate_OBJGPU(OBJGPU **ppThis, Dynamic *pParent, NvU32 createFlags,
        NvU32 ChipHal_arch, NvU32 ChipHal_impl, NvU32 ChipHal_hidrev,
        TEGRA_CHIP_TYPE TegraChipHal_tegraType,
        RM_RUNTIME_VARIANT RmVariantHal_rmVariant,
        NvU32 DispIpHal_ipver, NvU32 arg_gpuInstance, NvU32 arg_gpuId, NvUuid * arg_pUuid, struct GpuArch * arg_pGpuArch)
{
    NV_STATUS status;
    Object *pParentObj = NULL;
    OBJGPU *pThis;

    // Assign `pThis`, allocating memory unless suppressed by flag.
    status = __nvoc_handleObjCreateMemAlloc(createFlags, sizeof(OBJGPU), (void**)&pThis, (void**)ppThis);
    if (status != NV_OK)
        return status;

    // Zero is the initial value for everything.
    portMemSet(pThis, 0, sizeof(OBJGPU));

    pThis->__nvoc_base_Object.createFlags = createFlags;

    // Link the child into the parent if there is one unless flagged not to do so.
    if (pParent != NULL && !(createFlags & NVOC_OBJ_CREATE_FLAGS_PARENT_HALSPEC_ONLY))
    {
        pParentObj = dynamicCast(pParent, Object);
        objAddChild(pParentObj, &pThis->__nvoc_base_Object);
    }
    else
    {
        pThis->__nvoc_base_Object.pParent = NULL;
    }

    __nvoc_init__OBJGPU(pThis, ChipHal_arch, ChipHal_impl, ChipHal_hidrev, TegraChipHal_tegraType, RmVariantHal_rmVariant, DispIpHal_ipver);
    status = __nvoc_ctor_OBJGPU(pThis, arg_gpuInstance, arg_gpuId, arg_pUuid, arg_pGpuArch);
    if (status != NV_OK) goto __nvoc_objCreate_OBJGPU_cleanup;

    // Assignment has no effect if NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT is set.
    *ppThis = pThis;

    return NV_OK;

__nvoc_objCreate_OBJGPU_cleanup:

    // Unlink the child from the parent if it was linked above.
    if (pParentObj != NULL)
        objRemoveChild(pParentObj, &pThis->__nvoc_base_Object);

    // Do not call destructors here since the constructor already called them.
    if (createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(pThis, 0, sizeof(OBJGPU));
    else
    {
        portMemFree(pThis);
        *ppThis = NULL;
    }

    // coverity[leaked_storage:FALSE]
    return status;
}

NV_STATUS __nvoc_objCreateDynamic_OBJGPU(OBJGPU **ppThis, Dynamic *pParent, NvU32 createFlags, va_list args) {
    NV_STATUS status;
    NvU32 ChipHal_arch = va_arg(args, NvU32);
    NvU32 ChipHal_impl = va_arg(args, NvU32);
    NvU32 ChipHal_hidrev = va_arg(args, NvU32);
    TEGRA_CHIP_TYPE TegraChipHal_tegraType = va_arg(args, TEGRA_CHIP_TYPE);
    RM_RUNTIME_VARIANT RmVariantHal_rmVariant = va_arg(args, RM_RUNTIME_VARIANT);
    NvU32 DispIpHal_ipver = va_arg(args, NvU32);
    NvU32 arg_gpuInstance = va_arg(args, NvU32);
    NvU32 arg_gpuId = va_arg(args, NvU32);
    NvUuid * arg_pUuid = va_arg(args, NvUuid *);
    struct GpuArch * arg_pGpuArch = va_arg(args, struct GpuArch *);

    status = __nvoc_objCreate_OBJGPU(ppThis, pParent, createFlags, ChipHal_arch, ChipHal_impl, ChipHal_hidrev, TegraChipHal_tegraType, RmVariantHal_rmVariant, DispIpHal_ipver, arg_gpuInstance, arg_gpuId, arg_pUuid, arg_pGpuArch);

    return status;
}

