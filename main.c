// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>

#include "debug.h"
#include "aura-gpu-i2c.h"

static struct i2c_adapter *adapter = NULL;

static int __init aura_module_init (
    void
){
    adapter = gpu_adapter_create();
    if (IS_ERR(adapter)) {
        CLEAR_ERR(adapter);
    }

    return 0;
}

static void __exit aura_module_exit (
    void
){
    if (adapter)
        gpu_adapter_destroy(adapter);
}

module_init(aura_module_init);
module_exit(aura_module_exit);


MODULE_AUTHOR("Owen Parry <waldermort@gmail.com>");
MODULE_DESCRIPTION("ASUS AURA SMBus driver");
MODULE_LICENSE("GPL");
