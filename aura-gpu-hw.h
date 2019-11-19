#ifndef _UAPI_AURA_GPU_HW_I2C_H
#define _UAPI_AURA_GPU_HW_I2C_H

#include <linux/i2c.h>

struct i2c_adapter *aura_i2c_bios_create (
    void
);

void aura_i2c_bios_destroy (
    struct i2c_adapter *i2c_adapter
);

#endif
