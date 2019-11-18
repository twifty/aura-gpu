/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _UAPI_AURA_GPU_ASIC_TYPES_H
#define _UAPI_AURA_GPU_ASIC_TYPES_H
#include <linux/types.h>

enum aura_asic_type {
    CHIP_NVIDIA = 0,
    CHIP_POLARIS10,     // AMDGPU_FAMILY_VI DCE_VERSION_11_2
    CHIP_POLARIS11,     // AMDGPU_FAMILY_VI DCE_VERSION_11_2
    CHIP_POLARIS12,     // AMDGPU_FAMILY_VI DCE_VERSION_11_2
    CHIP_VEGAM,         // AMDGPU_FAMILY_VI DCE_VERSION_11_2
    CHIP_VEGA10,        // AMDGPU_FAMILY_AI DCE_VERSION_12_0
    CHIP_VEGA12,        // AMDGPU_FAMILY_AI DCE_VERSION_12_0
    CHIP_VEGA20,        // AMDGPU_FAMILY_AI DCE_VERSION_12_1
    CHIP_NAVI10,        // AMDGPU_FAMILY_NV DCN_VERSION_2_0
    CHIP_LAST,
};

static inline bool asic_is_valid(enum aura_asic_type asic)
{
    return asic >= CHIP_NVIDIA && asic <= CHIP_LAST;
}

static inline bool asic_is_nvidia(enum aura_asic_type asic)
{
    return asic_is_valid(asic) && asic == CHIP_NVIDIA;
}

static inline bool asic_is_amd(enum aura_asic_type asic)
{
    return asic_is_valid(asic) && asic != CHIP_NVIDIA;
}

#endif
