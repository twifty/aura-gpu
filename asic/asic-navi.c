// SPDX-License-Identifier: GPL-2.0
#include "asic-registers.h"

#define mmGENERIC_I2C_CONTROL                                                                          0x1eb8
#define mmGENERIC_I2C_INTERRUPT_CONTROL                                                                0x1eb9
#define mmGENERIC_I2C_STATUS                                                                           0x1eba
#define mmGENERIC_I2C_SPEED                                                                            0x1ebb
#define mmGENERIC_I2C_SETUP                                                                            0x1ebc
#define mmGENERIC_I2C_TRANSACTION                                                                      0x1ebd
#define mmGENERIC_I2C_DATA                                                                             0x1ebe
#define mmGENERIC_I2C_PIN_SELECTION                                                                    0x1ebf

//GENERIC_I2C_CONTROL
#define GENERIC_I2C_CONTROL__GENERIC_I2C_GO__SHIFT                                                            0x0
#define GENERIC_I2C_CONTROL__GENERIC_I2C_SOFT_RESET__SHIFT                                                    0x1
#define GENERIC_I2C_CONTROL__GENERIC_I2C_SEND_RESET__SHIFT                                                    0x2
#define GENERIC_I2C_CONTROL__GENERIC_I2C_ENABLE__SHIFT                                                        0x3
#define GENERIC_I2C_CONTROL__GENERIC_I2C_GO_MASK                                                              0x00000001L
#define GENERIC_I2C_CONTROL__GENERIC_I2C_SOFT_RESET_MASK                                                      0x00000002L
#define GENERIC_I2C_CONTROL__GENERIC_I2C_SEND_RESET_MASK                                                      0x00000004L
#define GENERIC_I2C_CONTROL__GENERIC_I2C_ENABLE_MASK                                                          0x00000008L
//GENERIC_I2C_INTERRUPT_CONTROL
#define GENERIC_I2C_INTERRUPT_CONTROL__GENERIC_I2C_DONE_INT__SHIFT                                            0x0
#define GENERIC_I2C_INTERRUPT_CONTROL__GENERIC_I2C_DONE_ACK__SHIFT                                            0x1
#define GENERIC_I2C_INTERRUPT_CONTROL__GENERIC_I2C_DONE_MASK__SHIFT                                           0x2
#define GENERIC_I2C_INTERRUPT_CONTROL__GENERIC_I2C_DONE_INT_MASK                                              0x00000001L
#define GENERIC_I2C_INTERRUPT_CONTROL__GENERIC_I2C_DONE_ACK_MASK                                              0x00000002L
#define GENERIC_I2C_INTERRUPT_CONTROL__GENERIC_I2C_DONE_MASK_MASK                                             0x00000004L
//GENERIC_I2C_STATUS
#define GENERIC_I2C_STATUS__GENERIC_I2C_STATUS__SHIFT                                                         0x0
#define GENERIC_I2C_STATUS__GENERIC_I2C_DONE__SHIFT                                                           0x4
#define GENERIC_I2C_STATUS__GENERIC_I2C_ABORTED__SHIFT                                                        0x5
#define GENERIC_I2C_STATUS__GENERIC_I2C_TIMEOUT__SHIFT                                                        0x6
#define GENERIC_I2C_STATUS__GENERIC_I2C_STOPPED_ON_NACK__SHIFT                                                0x9
#define GENERIC_I2C_STATUS__GENERIC_I2C_NACK__SHIFT                                                           0xa
#define GENERIC_I2C_STATUS__GENERIC_I2C_STATUS_MASK                                                           0x0000000FL
#define GENERIC_I2C_STATUS__GENERIC_I2C_DONE_MASK                                                             0x00000010L
#define GENERIC_I2C_STATUS__GENERIC_I2C_ABORTED_MASK                                                          0x00000020L
#define GENERIC_I2C_STATUS__GENERIC_I2C_TIMEOUT_MASK                                                          0x00000040L
#define GENERIC_I2C_STATUS__GENERIC_I2C_STOPPED_ON_NACK_MASK                                                  0x00000200L
#define GENERIC_I2C_STATUS__GENERIC_I2C_NACK_MASK                                                             0x00000400L
//GENERIC_I2C_SPEED
#define GENERIC_I2C_SPEED__GENERIC_I2C_THRESHOLD__SHIFT                                                       0x0
#define GENERIC_I2C_SPEED__GENERIC_I2C_DISABLE_FILTER_DURING_STALL__SHIFT                                     0x4
#define GENERIC_I2C_SPEED__GENERIC_I2C_START_STOP_TIMING_CNTL__SHIFT                                          0x8
#define GENERIC_I2C_SPEED__GENERIC_I2C_PRESCALE__SHIFT                                                        0x10
#define GENERIC_I2C_SPEED__GENERIC_I2C_THRESHOLD_MASK                                                         0x00000003L
#define GENERIC_I2C_SPEED__GENERIC_I2C_DISABLE_FILTER_DURING_STALL_MASK                                       0x00000010L
#define GENERIC_I2C_SPEED__GENERIC_I2C_START_STOP_TIMING_CNTL_MASK                                            0x00000300L
#define GENERIC_I2C_SPEED__GENERIC_I2C_PRESCALE_MASK                                                          0xFFFF0000L
//GENERIC_I2C_SETUP
#define GENERIC_I2C_SETUP__GENERIC_I2C_DATA_DRIVE_EN__SHIFT                                                   0x0
#define GENERIC_I2C_SETUP__GENERIC_I2C_DATA_DRIVE_SEL__SHIFT                                                  0x1
#define GENERIC_I2C_SETUP__GENERIC_I2C_CLK_DRIVE_EN__SHIFT                                                    0x7
#define GENERIC_I2C_SETUP__GENERIC_I2C_INTRA_BYTE_DELAY__SHIFT                                                0x8
#define GENERIC_I2C_SETUP__GENERIC_I2C_TIME_LIMIT__SHIFT                                                      0x18
#define GENERIC_I2C_SETUP__GENERIC_I2C_DATA_DRIVE_EN_MASK                                                     0x00000001L
#define GENERIC_I2C_SETUP__GENERIC_I2C_DATA_DRIVE_SEL_MASK                                                    0x00000002L
#define GENERIC_I2C_SETUP__GENERIC_I2C_CLK_DRIVE_EN_MASK                                                      0x00000080L
#define GENERIC_I2C_SETUP__GENERIC_I2C_INTRA_BYTE_DELAY_MASK                                                  0x0000FF00L
#define GENERIC_I2C_SETUP__GENERIC_I2C_TIME_LIMIT_MASK                                                        0xFF000000L
//GENERIC_I2C_TRANSACTION
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_RW__SHIFT                                                        0x0
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_STOP_ON_NACK__SHIFT                                              0x8
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_ACK_ON_READ__SHIFT                                               0x9
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_START__SHIFT                                                     0xc
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_STOP__SHIFT                                                      0xd
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_COUNT__SHIFT                                                     0x10
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_RW_MASK                                                          0x00000001L
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_STOP_ON_NACK_MASK                                                0x00000100L
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_ACK_ON_READ_MASK                                                 0x00000200L
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_START_MASK                                                       0x00001000L
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_STOP_MASK                                                        0x00002000L
#define GENERIC_I2C_TRANSACTION__GENERIC_I2C_COUNT_MASK                                                       0x000F0000L
//GENERIC_I2C_DATA
#define GENERIC_I2C_DATA__GENERIC_I2C_DATA_RW__SHIFT                                                          0x0
#define GENERIC_I2C_DATA__GENERIC_I2C_DATA__SHIFT                                                             0x8
#define GENERIC_I2C_DATA__GENERIC_I2C_INDEX__SHIFT                                                            0x10
#define GENERIC_I2C_DATA__GENERIC_I2C_INDEX_WRITE__SHIFT                                                      0x1f
#define GENERIC_I2C_DATA__GENERIC_I2C_DATA_RW_MASK                                                            0x00000001L
#define GENERIC_I2C_DATA__GENERIC_I2C_DATA_MASK                                                               0x0000FF00L
#define GENERIC_I2C_DATA__GENERIC_I2C_INDEX_MASK                                                              0x000F0000L
#define GENERIC_I2C_DATA__GENERIC_I2C_INDEX_WRITE_MASK                                                        0x80000000L
//GENERIC_I2C_PIN_SELECTION
#define GENERIC_I2C_PIN_SELECTION__GENERIC_I2C_SCL_PIN_SEL__SHIFT                                             0x0
#define GENERIC_I2C_PIN_SELECTION__GENERIC_I2C_SDA_PIN_SEL__SHIFT                                             0x8
#define GENERIC_I2C_PIN_SELECTION__GENERIC_I2C_SCL_PIN_SEL_MASK                                               0x0000007FL
#define GENERIC_I2C_PIN_SELECTION__GENERIC_I2C_SDA_PIN_SEL_MASK                                               0x00007F00L

static const struct i2c_registers i2c_registers = {
    I2C_GENERIC_REG_LIST()
};

static const struct i2c_mask i2c_masks = {
    I2C_GENERIC_MASK_SH_LIST(_MASK)
};

static const struct i2c_shift i2c_shifts = {
    I2C_GENERIC_MASK_SH_LIST(__SHIFT)
};

const struct asic_context asic_context_navi = {
    .i2c_registers = &i2c_registers,
    .i2c_shifts    = &i2c_shifts,
    .i2c_masks     = &i2c_masks
};
