/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _UAPI_AURA_GPU_I2C_H
#define _UAPI_AURA_GPU_I2C_H

#include "aura-gpu-reg.h"
#include "asic/asic-types.h"

/*
    AMD devices require an i2c adapter to be created,
    NVIDIA devices already have the adapter loaded.
 */
static const struct pci_device_id pciidlist[] = {
    {0x1002, 0x67df, 0x1043, 0x0517, 0, 0, CHIP_POLARIS10},     // RX580 (Strix)
    // {0x1002, 0x687F, 0x1043, 0x0555, 0, 0, CHIP_VEGA10},        // Vega 56 (Strix)
    // {0x1002, 0x731f, 0x1043, 0x04e2, 0, 0, CHIP_NAVI10},     // RX5700XT (Strix)
    {0, 0, 0},
};

// struct aura_i2c_service {
//     // void *private;
//     struct i2c_adapter *adapter;
// };

/* declared in aura-gpu.h */
// enum aura_asic_type;

// struct aura_i2c_service *aura_gpu_i2c_create (
//     struct pci_dev *pci_dev,
//     enum aura_asic_type asic_type
// );

struct i2c_adapter *gpu_adapter_create (
    void
);

int gpu_adapters_create (
    struct i2c_adapter *i2c_adapters[2],
    uint8_t count
);

// void aura_gpu_i2c_destroy (
//     struct aura_i2c_service *service
// );

void gpu_adapter_destroy (
    struct i2c_adapter *i2c_adapter
);

#endif
