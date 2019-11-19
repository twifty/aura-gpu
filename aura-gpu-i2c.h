/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _UAPI_AURA_GPU_I2C_H
#define _UAPI_AURA_GPU_I2C_H

#include "aura-gpu-reg.h"
#include "asic/asic-types.h"

struct i2c_adapter *gpu_adapter_create (
    void
);

int gpu_adapters_create (
    struct i2c_adapter *i2c_adapters[2],
    uint8_t count
);

void gpu_adapter_destroy (
    struct i2c_adapter *i2c_adapter
);

#endif
