#ifndef _UAPI_AURA_GPU_BIOS_H
#define _UAPI_AURA_GPU_BIOS_H

#include <linux/pci.h>
#include "include/types.h"

struct atom_bios {
    char name[20];
    size_t size; // TODO
    const char* data;
};

struct gpio_info {
    uint32_t clk_mask_register_index;
    uint32_t clk_en_register_index;
    uint32_t clk_y_register_index;
    uint32_t clk_a_register_index;
    uint32_t data_mask_register_index;
    uint32_t data_en_register_index;
    uint32_t data_y_register_index;
    uint32_t data_a_register_index;

    uint8_t clk_mask_shift;
    uint8_t clk_en_shift;
    uint8_t clk_y_shift;
    uint8_t clk_a_shift;
    uint8_t data_mask_shift;
    uint8_t data_en_shift;
    uint8_t data_y_shift;
    uint8_t data_a_shift;

    uint32_t clk_mask_mask;
    uint32_t clk_en_mask;
    uint32_t clk_y_mask;
    uint32_t clk_a_mask;
    uint32_t data_mask_mask;
    uint32_t data_en_mask;
    uint32_t data_y_mask;
    uint32_t data_a_mask;
};

struct i2c_info {
    struct gpio_info gpio_info;

    bool hw_assist;

    uint32_t line;
    uint32_t engine_id;
    uint32_t slave_address;
};

// #define aura_i2c_info graphics_object_i2c_info

struct gpio_pin_info {
	uint32_t offset;
	uint32_t offset_y;
	uint32_t offset_en;
	uint32_t offset_mask;

	uint32_t mask;
	uint32_t mask_y;
	uint32_t mask_en;
	uint32_t mask_mask;
};

struct graphics_object_id {
    uint32_t  id:8;
    uint32_t  enum_id:4;
    uint32_t  type:4;
    uint32_t  reserved:16; /* for padding. total size should be u32 */
};

error_t
atom_bios_get_gpio_info (struct atom_bios *bios, uint8_t index, struct i2c_info *info);

uint8_t
atom_bios_get_connectors_number (struct atom_bios *bios);

bool
atom_bios_get_connector_id (struct atom_bios *bios, uint8_t index, struct graphics_object_id *object_id);

error_t
atom_bios_get_i2c_info (struct atom_bios *bios, struct graphics_object_id *object_id, struct i2c_info *info);

error_t
atom_bios_get_crystal_frequency (struct atom_bios* bios, uint32_t *frequency);

struct atom_bios*
atom_bios_create (struct pci_dev *pci_dev);

void
atom_bios_release (struct atom_bios* bios);

#endif
