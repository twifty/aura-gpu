// SPDX-License-Identifier: GPL-2.0
#include <linux/i2c.h>
#include <linux/delay.h>

#include "debug.h"
#include "pci_ids.h"
#include "aura-gpu-reg.h"
#include "aura-gpu-i2c.h"
#include "asic/asic-registers.h"

enum {
    GPU_I2C_TIMEOUT_DELAY    = 1000,
    GPU_I2C_TIMEOUT_INTERVAL = 10,
};

enum aura_i2c_result {
    I2C_CHANNEL_OPERATION_SUCCEEDED,
    I2C_CHANNEL_OPERATION_FAILED,
    I2C_CHANNEL_OPERATION_NOT_GRANTED,
    I2C_CHANNEL_OPERATION_IS_BUSY,
    I2C_CHANNEL_OPERATION_NO_HANDLE_PROVIDED,
    I2C_CHANNEL_OPERATION_CHANNEL_IN_USE,
    I2C_CHANNEL_OPERATION_CHANNEL_CLIENT_MAX_ALLOWED,
    I2C_CHANNEL_OPERATION_ENGINE_BUSY,
    I2C_CHANNEL_OPERATION_TIMEOUT,
    I2C_CHANNEL_OPERATION_NO_RESPONSE,
    I2C_CHANNEL_OPERATION_HW_REQUEST_I2C_BUS,
    I2C_CHANNEL_OPERATION_WRONG_PARAMETER,
    I2C_CHANNEL_OPERATION_OUT_NB_OF_RETRIES,
    I2C_CHANNEL_OPERATION_NOT_STARTED
};

enum aura_i2c_action {
    DCE_I2C_TRANSACTION_ACTION_I2C_WRITE = 0x00,
    DCE_I2C_TRANSACTION_ACTION_I2C_READ = 0x10,
    DCE_I2C_TRANSACTION_ACTION_I2C_STATUS_REQUEST = 0x20,

    DCE_I2C_TRANSACTION_ACTION_I2C_WRITE_MOT = 0x40,
    DCE_I2C_TRANSACTION_ACTION_I2C_READ_MOT = 0x50,
    DCE_I2C_TRANSACTION_ACTION_I2C_STATUS_REQUEST_MOT = 0x60,

    DCE_I2C_TRANSACTION_ACTION_DP_WRITE = 0x80,
    DCE_I2C_TRANSACTION_ACTION_DP_READ = 0x90
};

enum aura_i2c_status {
    DC_I2C_STATUS__DC_I2C_STATUS_IDLE,
    DC_I2C_STATUS__DC_I2C_STATUS_USED_BY_SW,
    DC_I2C_STATUS__DC_I2C_STATUS_USED_BY_HW,
    DC_I2C_REG_RW_CNTL_STATUS_DMCU_ONLY = 2,
};

struct aura_i2c_context {
    struct aura_reg_service     *reg_service;
    enum aura_asic_type         asic_type;

    uint32_t                    original_speed;
    uint32_t                    default_speed;

    uint32_t                    timeout_delay;
    uint32_t                    timeout_interval;

    const struct i2c_registers  *registers;
    const struct i2c_shift      *shifts;
    const struct i2c_mask       *masks;

    struct i2c_adapter          i2c_adapter;
    struct mutex                mutex;
};

#define context_from_adapter(ptr) ( \
    container_of(ptr, struct aura_i2c_context, i2c_adapter) \
)


struct aura_i2c_payload {
	bool       write;
	uint8_t    address;
	uint32_t   length;
	uint8_t    *data;
};

struct aura_i2c_transaction {
	enum aura_i2c_action   action;
	enum aura_i2c_result   status;
	uint8_t                address;
	uint32_t               length;
	uint8_t                *data;
};


static void clear_ack (
    struct aura_i2c_context *context
){
    struct aura_reg_service *reg = context->reg_service;

    reg_update_ex(reg, context->registers->GENERIC_I2C_INTERRUPT_CONTROL, (struct reg_fields[]){
        /*
            Acknowledge bit for GENERIC_I2C_DONE. Write 1 to
            clear interrupt.
         */
        PIN_FIELDS(context, GENERIC_I2C_DONE_ACK, 1),
    }, 1);
}

static error_t open_engine (
    struct aura_i2c_context *context
){
    struct aura_reg_service *reg;

    if (IS_NULL(context))
        return -EINVAL;

    mutex_lock(&context->mutex);
    reg = context->reg_service;

    reg_update_ex(reg, context->registers->GENERIC_I2C_CONTROL, (struct reg_fields[]){
        /*

         */
        PIN_FIELDS(context, GENERIC_I2C_ENABLE, 1),
    }, 1);

    /*
        Read       reg mmDCO_MEM_PWR_CTRL                    6db6d800
        Writing    reg mmDCO_MEM_PWR_CTRL                    6db6d800           removing I2C_LIGHT_SLEEP_DIS && I2C_LIGHT_SLEEP_FORCE ??
        Writing    reg mmGLOBAL_CAPABILITIES                 12260
        Read       reg mmOUTPUT_PAYLOAD_CAPABILITY           0
        Writing    reg mmOUTPUT_PAYLOAD_CAPABILITY           0
        Read       reg mmOUTPUT_PAYLOAD_CAPABILITY           0
        Writing    reg mmOUTPUT_PAYLOAD_CAPABILITY           0
     */

    reg_update_ex(reg, context->registers->GENERIC_I2C_PIN_SELECTION, (struct reg_fields[]){
        /*
            GPIO pin selection to use for SCL, if
            GENERIC_I2C_SCL_PIN_SEL ==
            GENERIC_I2C_SDA_PIN_SEL => disable pin selectin.

            TODO: Where do these values come from and are they
                  specific to asic types?
         */
        PIN_FIELDS(context, GENERIC_I2C_SCL_PIN_SEL, 0x29),
        PIN_FIELDS(context, GENERIC_I2C_SDA_PIN_SEL, 0x28),
    }, 2);

    // set_speed(engine, 100);

    return 0;
}

static void close_engine (
    struct aura_i2c_context *context
){
    struct aura_reg_service *reg = context->reg_service;
    // struct reg_fields sw_status = PIN_FIELDS(engine, GENERIC_I2C_STATUS, 0);

    // set_speed(engine, engine->original_speed);

    reg_update_ex(reg, context->registers->GENERIC_I2C_PIN_SELECTION, (struct reg_fields[]){
        /*
            GPIO pin selection to use for SCL, if
            GENERIC_I2C_SCL_PIN_SEL ==
            GENERIC_I2C_SDA_PIN_SEL => disable pin selectin.

            TODO: Where do these values come from and are they
                  specific to asic types?
         */
        PIN_FIELDS(context, GENERIC_I2C_SCL_PIN_SEL, 0),
        PIN_FIELDS(context, GENERIC_I2C_SDA_PIN_SEL, 0),
    }, 2);

    reg_update_ex(reg, context->registers->GENERIC_I2C_CONTROL, (struct reg_fields[]){
        /*
            Reset the controller
         */
        PIN_FIELDS(context, GENERIC_I2C_ENABLE, 0),
        PIN_FIELDS(context, GENERIC_I2C_SOFT_RESET, 1),
    }, 2);

    reg_update_ex(reg, context->registers->GENERIC_I2C_CONTROL, (struct reg_fields[]){
        /*
            Clear the reset flag
         */
        PIN_FIELDS(context, GENERIC_I2C_SOFT_RESET, 0),
    }, 1);

    mutex_unlock(&context->mutex);
}


static bool process_transaction (
    const struct aura_i2c_context *context,
    struct aura_i2c_transaction *request
){
    struct aura_reg_service *reg = context->reg_service;
    uint32_t length = request->length;
    uint8_t *buffer = request->data;
    uint32_t value = 0;

    /*
        Configure the transaction register
     */
    reg_update_ex(reg, context->registers->GENERIC_I2C_TRANSACTION, (struct reg_fields[]){
        /*
            Read/write indicator for second transaction - set to 0 for
            write, 1 for read. This bit only controls DC_I2C behaviour -
            the R/W bit in the transaction is programmed into the I2C
            buffer as the LSB of the address byte.
             0=WRITE
             1=READ
            MASK == 0x1
         */
        PIN_FIELDS(context, GENERIC_I2C_RW, 0 != (request->action & DCE_I2C_TRANSACTION_ACTION_I2C_READ)),
        /*
            Determines whether the current transfer will stop if a NACK
            is received during the transaction (current transaction
            always stops).
             0=STOP CURRENT TRANSACTION, GO TO NEXT
            TRANSACTION
             1=STOP ALL TRANSACTIONS, SEND STOP BIT
            MASK == 0x100
         */
        PIN_FIELDS(context, GENERIC_I2C_STOP_ON_NACK, 1),
        /*
            Determines whether hardware will send an ACK after the
            last byte on a read in the second transaction.
             0=Send NACK
             1=Send ACK
            MASK == 0x200
         */
        PIN_FIELDS(context, GENERIC_I2C_ACK_ON_READ, 0),
        /*
            Determines whether a start bit will be sent before the
            second transaction
             0=NO START
             1=START
            MASK == 0x1000
         */
        PIN_FIELDS(context, GENERIC_I2C_START, 1),
        /*
            Determines whether a stop bit will be sent after the second
            transaction
             0=NO STOP
             1=STOP
            MASK == 0x2000
         */
        PIN_FIELDS(context, GENERIC_I2C_STOP, true ? 1 : 0),
        /*
            Byte count for the transaction (excluding the first byte,
            which is usually the address).
           MASK == 0xf0000
         */
        PIN_FIELDS(context, GENERIC_I2C_COUNT, length),
    }, 6);

    /* Write the I2C address and I2C data
     * into the hardware circular buffer, one byte per entry.
     * As an example, the 7-bit I2C slave address for CRT monitor
     * for reading DDC/EDID information is 0b1010001.
     * For an I2C send operation, the LSB must be programmed to 0;
     * for I2C receive operation, the LSB must be programmed to 1.
     *
     */
    value = reg_set_ex(reg, context->registers->GENERIC_I2C_DATA, 0, (struct reg_fields[]){
        /*
            Select whether buffer access will be a read or write. For
            writes, address auto-increments on write to DC_I2C_DATA.
            For reads, address auto-increments on reads to
            GENERIC_I2C_DATA.
             0=Write
             1=Read
            MASK == 0x1
         */
        PIN_FIELDS(context, GENERIC_I2C_DATA_RW, false),
        /*
            Use to fill or read the generic I2C buffer
            MASK == 0xff00
         */
        PIN_FIELDS(context, GENERIC_I2C_DATA, request->address),
        /*
            Use to set index into I2C buffer for next read or current
            write, or to read index of current read or next write. Writable
            only when GENERIC_I2C_INDEX_WRITE=1.
            MASK == 0xf0000
         */
        PIN_FIELDS(context, GENERIC_I2C_INDEX, 0),
        /*
            To write index field, set this bit to 1 while writing
            GENERIC_I2C_DATA
            MASK == 0x80000000
         */
        PIN_FIELDS(context, GENERIC_I2C_INDEX_WRITE, 1),
    }, 4);

    if (!(request->action & DCE_I2C_TRANSACTION_ACTION_I2C_READ)) {
        // TODO this should auto increment
        uint8_t index = 1;
        while (length) {
            reg_set_ex(reg, context->registers->GENERIC_I2C_DATA, 0, (struct reg_fields[]){
                PIN_FIELDS(context, GENERIC_I2C_INDEX, index),
                PIN_FIELDS(context, GENERIC_I2C_DATA, *buffer++),
            }, 2);

            ++index;
            --length;
        }
    }

    return true;
}

static void execute_transaction (
    const struct aura_i2c_context *context
){
    struct aura_reg_service *reg = context->reg_service;

    reg_update_ex(reg, context->registers->GENERIC_I2C_CONTROL, (struct reg_fields[]){
        /*
            Write 1 to start I2C transfer
         */
        PIN_FIELDS(context, GENERIC_I2C_GO, 1),
    }, 1);
}

static void process_reply (
    struct aura_i2c_context *context,
    struct aura_i2c_payload *reply
){
    struct aura_reg_service *reg = context->reg_service;
    uint32_t length = reply->length;
    uint8_t *buffer = reply->data;
    uint8_t index = 1;
    struct reg_fields data[] = {
        PIN_FIELDS(context, GENERIC_I2C_INDEX, 1),
        PIN_FIELDS(context, GENERIC_I2C_DATA, 0)
    };

    // AURA_DBG("process_reply");

    reg_set_ex(reg, context->registers->GENERIC_I2C_DATA, 0, (struct reg_fields[]){
        /*
            Select whether buffer access will be a read or write. For
            writes, address auto-increments on write to DC_I2C_DATA.
            For reads, address auto-increments on reads to
            DC_I2C_DATA.
             0=Write
             1=Read
         */
        PIN_FIELDS(context, GENERIC_I2C_DATA_RW, 1),
        /*
            Use to set index into I2C buffer for next read or current
            write, or to read index of current read or next write. Writable
            only when GENERIC_I2C_INDEX_WRITE=1.
            MASK == 0xf0000

            Note, the byte at index 0 is the slave_address
         */
        PIN_FIELDS(context, GENERIC_I2C_INDEX, 1),
        /*
            To write index field, set this bit to 1 while writing
            GENERIC_I2C_DATA
            MASK == 0x80000000
         */
        PIN_FIELDS(context, GENERIC_I2C_INDEX_WRITE, 1),
    }, 3);

    /*
        NOTE: Some controllers, this IR3567B in particular, will repeat the
        first byte when trying to read multiple.
     */
    while (length) {
        data[0].value = ++index;
        /*
            The register buffer auto increments. To get its current index
            use DC_I2C_INDEX above
         */
        reg_get_ex(reg, context->registers->GENERIC_I2C_DATA, data, 2);
        *buffer++ = data[1].value;

        --length;
    }

    // AURA_DBG("Reading %d bytes: 0x%02x", reply->length, reply->data[0]);

    clear_ack(context);
}

static enum aura_i2c_result get_channel_status (
    const struct aura_i2c_context *context
){
    struct aura_reg_service *reg = context->reg_service;
    struct reg_fields status = PIN_FIELDS(context, GENERIC_I2C_STATUS, 0);
    uint32_t value = reg_get_ex(reg, context->registers->GENERIC_I2C_STATUS, &status, 1);

    if (status.value || value == 0) {
        // AURA_DBG("I2C_CHANNEL_OPERATION_ENGINE_BUSY");
        return I2C_CHANNEL_OPERATION_ENGINE_BUSY;
    }

    if (value & context->masks->GENERIC_I2C_STOPPED_ON_NACK) {
        // AURA_DBG("I2C_CHANNEL_OPERATION_NO_RESPONSE");
        return I2C_CHANNEL_OPERATION_NO_RESPONSE;
    }

    if (value & context->masks->GENERIC_I2C_TIMEOUT) {
        // AURA_DBG("I2C_CHANNEL_OPERATION_TIMEOUT");
        return I2C_CHANNEL_OPERATION_TIMEOUT;
    }

    if (value & context->masks->GENERIC_I2C_ABORTED) {
        // AURA_DBG("I2C_CHANNEL_OPERATION_FAILED");
        return I2C_CHANNEL_OPERATION_FAILED;
    }

    if (value & context->masks->GENERIC_I2C_DONE) {
        // AURA_DBG("I2C_CHANNEL_OPERATION_SUCCEEDED");
        return I2C_CHANNEL_OPERATION_SUCCEEDED;
    }

    if (value & context->masks->GENERIC_I2C_NACK) {
        // AURA_DBG("I2C_CHANNEL_OPERATION_NO_RESPONSE");
        return I2C_CHANNEL_OPERATION_NO_RESPONSE;
    }

    return I2C_CHANNEL_OPERATION_SUCCEEDED;
}

enum aura_i2c_result poll_engine (
    struct aura_i2c_context *context
){
    enum aura_i2c_result result;
    uint32_t timeout = context->timeout_interval;

    do {
        result = get_channel_status(context);
        if (result != I2C_CHANNEL_OPERATION_ENGINE_BUSY)
            break;

        udelay(context->timeout_delay);
    } while (timeout--);

    clear_ack(context);

    return timeout != 0 ? result : I2C_CHANNEL_OPERATION_TIMEOUT;
}


static void submit_transaction (
    struct aura_i2c_context *context,
    struct aura_i2c_transaction *request
){
    request->status = I2C_CHANNEL_OPERATION_SUCCEEDED;

    if (!process_transaction(context, request)) {
        AURA_DBG("Failed to process transaction");
        return;
    }

    execute_transaction(context);
}

static bool submit_payload (
    struct aura_i2c_context *context,
    struct aura_i2c_payload *payload,
    bool middle_of_transaction
){
    struct aura_i2c_transaction request;
    enum aura_i2c_result operation_result;
    // uint32_t transaction_timeout;

    if (!payload->write) {
        request.action = middle_of_transaction ?
            DCE_I2C_TRANSACTION_ACTION_I2C_READ_MOT :
            DCE_I2C_TRANSACTION_ACTION_I2C_READ;
    } else {
        request.action = middle_of_transaction ?
            DCE_I2C_TRANSACTION_ACTION_I2C_WRITE_MOT :
            DCE_I2C_TRANSACTION_ACTION_I2C_WRITE;
    }

    request.address = (uint8_t) ((payload->address << 1) | !payload->write);
    request.length  = payload->length;
    request.data    = payload->data;

    /* obtain timeout value before submitting request */
    // transaction_timeout = calculate_timeout(engine, payload->length + 1);

    submit_transaction(context, &request);

    if ((request.status == I2C_CHANNEL_OPERATION_FAILED) || (request.status == I2C_CHANNEL_OPERATION_ENGINE_BUSY))
        return false;

    /* wait until transaction proceed */
    operation_result = poll_engine(context);

    /* update transaction status */
    if (operation_result == I2C_CHANNEL_OPERATION_SUCCEEDED) {
        if (!(payload->write))
            process_reply(context, payload);

        return true;
    }

    return false;
}


static int aura_gpu_i2c_xfer (
    struct i2c_adapter *i2c_adapter,
    struct i2c_msg *msgs,
    int num
){
    struct aura_i2c_context *context = i2c_get_adapdata(i2c_adapter);
    struct aura_i2c_payload payload;
    bool mot, result = true;
    int i;

    if (IS_NULL(context))
        return -EIO;

    open_engine(context);

    // for (i = 0; i < num; i++) {
    //     if (msgs[i].flags & I2C_M_RD) {
    //         AURA_DBG("Raw message: rw = 'r', addr = %x, flags = %x, len = %d",
    //                     msgs[i].addr,
    //                     msgs[i].flags,
    //                     msgs[i].len
    //         );
    //     } else {
    //         AURA_DBG("Raw message: rw = 'w', addr = %x, flags = %x, len = %d, data = %x",
    //                     msgs[i].addr,
    //                     msgs[i].flags,
    //                     msgs[i].len,
    //                     msgs[i].buf[0]
    //         );
    //     }
    // }

    /* Detect bus probe */
    // if (num == 1 && msgs[0].len == 0) {
    //     payload.
    // }

    for (i = 0; i < num; i++) {
        mot = (i != num - 1);
        payload.write   = !(msgs[i].flags & I2C_M_RD);
        payload.address = msgs[i].addr;
        payload.length  = msgs[i].len;
        payload.data    = msgs[i].buf;

        if (!submit_payload(context, &payload, mot)) {
            result = false;
            break;
        }
    }

    close_engine(context);

    return result ? num : -EIO;
}

static u32 aura_gpu_i2c_func (
    struct i2c_adapter *adap
){
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm aura_gpu_i2c_algo = {
    .master_xfer   = aura_gpu_i2c_xfer,
    .functionality = aura_gpu_i2c_func,
};


static const struct asic_context* aura_gpu_i2c_get_ddc_context (
    enum aura_asic_type asic_type
){
    switch (asic_type) {
        case CHIP_POLARIS10:
        case CHIP_POLARIS11:
        case CHIP_POLARIS12:
        case CHIP_VEGAM:
            return &asic_context_polaris;
        case CHIP_VEGA10:
        case CHIP_VEGA12:
        case CHIP_VEGA20:
            /*
             * FIXME -
             * Vega cards do not currently work
             */
            return &asic_context_vega;
        case CHIP_NAVI10:
            /*
                TODO -
                The GENERIC_I2C_ masks and shifts are not present in the AMDGPU
                sources. Test if they are the same across asics and place their
                definitions in gpu-registers.h
            */
            return &asic_context_navi;
        default:
            return NULL;
    }
}

static struct aura_i2c_context* aura_gpu_i2c_context_create (
    struct pci_dev *pci_dev,
    enum aura_asic_type asic_type
){
    struct aura_reg_service *registry;
    struct aura_i2c_context *context;
    const struct asic_context *ddc_context;
    error_t err;

    ddc_context = aura_gpu_i2c_get_ddc_context(asic_type);
    if (!ddc_context)
        return ERR_PTR(-ENODEV);

    context = kzalloc(sizeof(*context), GFP_KERNEL);
    if (!context)
        return ERR_PTR(-ENOMEM);

    registry = aura_gpu_reg_create(pci_dev);
    if (IS_ERR(registry)) {
        err = PTR_ERR(registry);
        goto error_free_context;
    }

    mutex_init(&context->mutex);
    context->asic_type          = asic_type;
    context->reg_service        = registry;

    context->registers          = ddc_context->i2c_registers;
    context->masks              = ddc_context->i2c_masks;
    context->shifts             = ddc_context->i2c_shifts;

    context->original_speed     = 50;
    context->default_speed      = 50;
    context->timeout_delay      = GPU_I2C_TIMEOUT_DELAY;
    context->timeout_interval   = GPU_I2C_TIMEOUT_INTERVAL;

    context->i2c_adapter.owner  = THIS_MODULE;
    context->i2c_adapter.class  = I2C_CLASS_DDC;
    context->i2c_adapter.algo   = &aura_gpu_i2c_algo;

    snprintf(context->i2c_adapter.name, sizeof(context->i2c_adapter.name), "AURA GPU adapter");
    i2c_set_adapdata(&context->i2c_adapter, context);

    // TODO - Do we really need to expose this?
    err = i2c_add_adapter(&context->i2c_adapter);
    if (err)
        goto error_free_registry;

    return context;

error_free_registry:
    aura_gpu_reg_destroy(registry);
error_free_context:
    kfree(context);

    return ERR_PTR(err);
}


void gpu_adapter_destroy (
    struct i2c_adapter *i2c_adapter
){
    struct aura_i2c_context *context = context_from_adapter(i2c_adapter);

    if (IS_NULL(i2c_adapter))
        return;

    i2c_del_adapter(&context->i2c_adapter);
    aura_gpu_reg_destroy(context->reg_service);
    kfree(context);
}

struct i2c_adapter *gpu_adapter_create (
    void
){
    struct pci_dev *pci_dev;
    const struct pci_device_id *match;
    struct aura_i2c_context *context = NULL;

    /* Handle the case of mixed GPU types */
    pci_dev = NULL;

    while (NULL != (pci_dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pci_dev))) {
        match = pci_match_id(pciidlist, pci_dev);
        if (match) {
            AURA_DBG("Detected AURA capable GPU %x:%x", pci_dev->subsystem_vendor, pci_dev->subsystem_device);

            context = aura_gpu_i2c_context_create(pci_dev, match->driver_data);
            if (!IS_ERR_OR_NULL(context))
                return &context->i2c_adapter;
        }
    }

    return NULL;
}

int gpu_adapters_create (
    struct i2c_adapter *i2c_adapters[2],
    uint8_t count
){
    struct pci_dev *pci_dev;
    const struct pci_device_id *match;
    struct aura_i2c_context *context = NULL;
    error_t err;
    int found;

    if (IS_NULL(i2c_adapters) || IS_FALSE(count == 2))
        return -EINVAL;

    /* Handle the case of mixed GPU types */
    pci_dev = NULL;
    found = 0;

    while (NULL != (pci_dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pci_dev))) {
        match = pci_match_id(pciidlist, pci_dev);
        if (match) {
            AURA_DBG("Detected AURA capable GPU %x:%x", pci_dev->subsystem_vendor, pci_dev->subsystem_device);

            context = aura_gpu_i2c_context_create(pci_dev, match->driver_data);
            if (IS_ERR(context)) {
                err = CLEAR_ERR(context);
                goto error_free;
            }

            if (context) {
                i2c_adapters[found++] = &context->i2c_adapter;
                if (found >= count)
                    break;
            }
        }
    }

    return found;

error_free:
    while (found > 0) {
        found--;
        gpu_adapter_destroy(i2c_adapters[found]);
    }

    return err;
}
