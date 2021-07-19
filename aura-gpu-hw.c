// SPDX-License-Identifier: GPL-2.0
#include <linux/i2c.h>

#include "debug.h"
#include "pci_ids.h"
#include "aura-gpu-hw.h"
#include "aura-gpu-bios.h"
#include "aura-gpu-reg.h"
#include "atom/atom.h"

struct ATOM_MASTER_LIST_OF_COMMAND_TABLES {
    uint16_t ASIC_Init;                              //Function Table, used by various SW components,latest version 1.1
    uint16_t GetDisplaySurfaceSize;                  //Atomic Table,  Used by Bios when enabling HW ICON
    uint16_t ASIC_RegistersInit;                     //Atomic Table,  indirectly used by various SW components,called from ASIC_Init
    uint16_t VRAM_BlockVenderDetection;              //Atomic Table,  used only by Bios
    uint16_t DIGxEncoderControl;                     //Only used by Bios
    uint16_t MemoryControllerInit;                   //Atomic Table,  indirectly used by various SW components,called from ASIC_Init
    uint16_t EnableCRTCMemReq;                       //Function Table,directly used by various SW components,latest version 2.1
    uint16_t MemoryParamAdjust;                      //Atomic Table,  indirectly used by various SW components,called from SetMemoryClock if needed
    uint16_t DVOEncoderControl;                      //Function Table,directly used by various SW components,latest version 1.2
    uint16_t GPIOPinControl;                         //Atomic Table,  only used by Bios
    uint16_t SetEngineClock;                         //Function Table,directly used by various SW components,latest version 1.1
    uint16_t SetMemoryClock;                         //Function Table,directly used by various SW components,latest version 1.1
    uint16_t SetPixelClock;                          //Function Table,directly used by various SW components,latest version 1.2
    uint16_t EnableDispPowerGating;                  //Atomic Table,  indirectly used by various SW components,called from ASIC_Init
    uint16_t ResetMemoryDLL;                         //Atomic Table,  indirectly used by various SW components,called from SetMemoryClock
    uint16_t ResetMemoryDevice;                      //Atomic Table,  indirectly used by various SW components,called from SetMemoryClock
    uint16_t MemoryPLLInit;                          //Atomic Table,  used only by Bios
    uint16_t AdjustDisplayPll;                       //Atomic Table,  used by various SW componentes.
    uint16_t AdjustMemoryController;                 //Atomic Table,  indirectly used by various SW components,called from SetMemoryClock
    uint16_t EnableASIC_StaticPwrMgt;                //Atomic Table,  only used by Bios
    uint16_t SetUniphyInstance;                      //Atomic Table,  only used by Bios
    uint16_t DAC_LoadDetection;                      //Atomic Table,  directly used by various SW components,latest version 1.2
    uint16_t LVTMAEncoderControl;                    //Atomic Table,directly used by various SW components,latest version 1.3
    uint16_t HW_Misc_Operation;                      //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t DAC1EncoderControl;                     //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t DAC2EncoderControl;                     //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t DVOOutputControl;                       //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t CV1OutputControl;                       //Atomic Table,  Atomic Table,  Obsolete from Ry6xx, use DAC2 Output instead
    uint16_t GetConditionalGoldenSetting;            //Only used by Bios
    uint16_t SMC_Init;                               //Function Table,directly used by various SW components,latest version 1.1
    uint16_t PatchMCSetting;                         //only used by BIOS
    uint16_t MC_SEQ_Control;                         //only used by BIOS
    uint16_t Gfx_Harvesting;                         //Atomic Table,  Obsolete from Ry6xx, Now only used by BIOS for GFX harvesting
    uint16_t EnableScaler;                           //Atomic Table,  used only by Bios
    uint16_t BlankCRTC;                              //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t EnableCRTC;                             //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t GetPixelClock;                          //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t EnableVGA_Render;                       //Function Table,directly used by various SW components,latest version 1.1
    uint16_t GetSCLKOverMCLKRatio;                   //Atomic Table,  only used by Bios
    uint16_t SetCRTC_Timing;                         //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t SetCRTC_OverScan;                       //Atomic Table,  used by various SW components,latest version 1.1
    uint16_t GetSMUClockInfo;                         //Atomic Table,  used only by Bios
    uint16_t SelectCRTC_Source;                      //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t EnableGraphSurfaces;                    //Atomic Table,  used only by Bios
    uint16_t UpdateCRTC_DoubleBufferRegisters;       //Atomic Table,  used only by Bios
    uint16_t LUT_AutoFill;                           //Atomic Table,  only used by Bios
    uint16_t SetDCEClock;                            //Atomic Table,  start from DCE11.1, shared by driver and VBIOS, change DISPCLK and DPREFCLK
    uint16_t GetMemoryClock;                         //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t GetEngineClock;                         //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t SetCRTC_UsingDTDTiming;                 //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t ExternalEncoderControl;                 //Atomic Table,  directly used by various SW components,latest version 2.1
    uint16_t LVTMAOutputControl;                     //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t VRAM_BlockDetectionByStrap;             //Atomic Table,  used only by Bios
    uint16_t MemoryCleanUp;                          //Atomic Table,  only used by Bios
    uint16_t ProcessI2cChannelTransaction;           //Function Table,only used by Bios
    uint16_t WriteOneByteToHWAssistedI2C;            //Function Table,indirectly used by various SW components
    uint16_t ReadHWAssistedI2CStatus;                //Atomic Table,  indirectly used by various SW components
    uint16_t SpeedFanControl;                        //Function Table,indirectly used by various SW components,called from ASIC_Init
    uint16_t PowerConnectorDetection;                //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t MC_Synchronization;                     //Atomic Table,  indirectly used by various SW components,called from SetMemoryClock
    uint16_t ComputeMemoryEnginePLL;                 //Atomic Table,  indirectly used by various SW components,called from SetMemory/EngineClock
    uint16_t Gfx_Init;                               //Atomic Table,  indirectly used by various SW components,called from SetMemory or SetEngineClock
    uint16_t VRAM_GetCurrentInfoBlock;               //Atomic Table,  used only by Bios
    uint16_t DynamicMemorySettings;                  //Atomic Table,  indirectly used by various SW components,called from SetMemoryClock
    uint16_t MemoryTraining;                         //Atomic Table,  used only by Bios
    uint16_t EnableSpreadSpectrumOnPPLL;             //Atomic Table,  directly used by various SW components,latest version 1.2
    uint16_t TMDSAOutputControl;                     //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t SetVoltage;                             //Function Table,directly and/or indirectly used by various SW components,latest version 1.1
    uint16_t DAC1OutputControl;                      //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t ReadEfuseValue;                         //Atomic Table,  directly used by various SW components,latest version 1.1
    uint16_t ComputeMemoryClockParam;                //Function Table,only used by Bios, obsolete soon.Switch to use "ReadEDIDFromHWAssistedI2C"
    uint16_t ClockSource;                            //Atomic Table,  indirectly used by various SW components,called from ASIC_Init
    uint16_t MemoryDeviceInit;                       //Atomic Table,  indirectly used by various SW components,called from SetMemoryClock
    uint16_t GetDispObjectInfo;                      //Atomic Table,  indirectly used by various SW components,called from EnableVGARender
    uint16_t DIG1EncoderControl;                     //Atomic Table,directly used by various SW components,latest version 1.1
    uint16_t DIG2EncoderControl;                     //Atomic Table,directly used by various SW components,latest version 1.1
    uint16_t DIG1TransmitterControl;                 //Atomic Table,directly used by various SW components,latest version 1.1
    uint16_t DIG2TransmitterControl;                 //Atomic Table,directly used by various SW components,latest version 1.1
    uint16_t ProcessAuxChannelTransaction;           //Function Table,only used by Bios
    uint16_t DPEncoderService;                       //Function Table,only used by Bios
    uint16_t GetVoltageInfo;                         //Function Table,only used by Bios since SI
};

#define GetIndexIntoMasterTable(MasterOrData, FieldName) ( \
    (((char*)(&((struct ATOM_MASTER_LIST_OF_##MasterOrData##_TABLES*)0)->FieldName)-(char*)0)/sizeof(uint16_t)) \
)

struct hw_i2c_context {
    struct card_info        atom_card_info;
    struct atom_context     *atom_context;
    struct aura_reg_service *reg_service;
    struct atom_bios        *bios;
    struct i2c_adapter      adapter;
    bool                    registered;

    uint8_t                 scratch[20 * 1024];
};
#define context_from_adapter(ptr) ( \
    container_of(ptr, struct hw_i2c_context, adapter) \
)
#define context_from_card(ptr) ( \
    container_of(ptr, struct hw_i2c_context, atom_card_info) \
)

static uint32_t __invalid_read (
    struct card_info *info,
    uint32_t reg
){
    AURA_DBG("non MM read called");
    return 0;
}

static void __invalid_write (
    struct card_info *info,
    uint32_t reg,
    uint32_t val
){
    AURA_DBG("non MM write called");
}

static uint32_t mm_read (
    struct card_info *info,
    uint32_t reg
){
    uint32_t res;
    struct hw_i2c_context *ctx = context_from_card(info);

    res = reg_read(ctx->reg_service, reg);
    AURA_DBG("Read reg %x %x", reg, res);

    return res;
}

static void mm_write (
    struct card_info *info,
    uint32_t reg,
    uint32_t val
){
    struct hw_i2c_context *ctx = context_from_card(info);

    AURA_DBG("Writing reg %x val %x", reg, val);
    reg_write(ctx->reg_service, reg, val);
}

#define TARGET_HW_I2C_CLOCK             50
#define ATOM_MAX_HW_I2C_WRITE           3
#define ATOM_MAX_HW_I2C_READ            255
#define HW_I2C_WRITE                    1
#define HW_I2C_READ                     0
#define HW_ASSISTED_I2C_STATUS_FAILURE  2
#define HW_ASSISTED_I2C_STATUS_SUCCESS  1

struct transaction_parameters {
    uint8_t     ucI2CSpeed;
    union {
        uint8_t ucRegIndex;
        uint8_t ucStatus;
    };
    uint16_t    lpI2CDataOut;
    uint8_t     ucFlag;
    uint8_t     ucTransBytes;
    uint8_t     ucSlaveAddr;
    uint8_t     ucLineNumber;
};

static error_t aura_gpu_i2c_process_i2c_ch(
    struct hw_i2c_context *context,
    uint8_t slave_addr,
    uint8_t offset,
    uint8_t flags,
    uint8_t *buf
){
    struct transaction_parameters args;
    int index = GetIndexIntoMasterTable(COMMAND, ProcessI2cChannelTransaction);
    error_t err = 0;

    memset(&args, 0, sizeof(args));

    if (flags & HW_I2C_WRITE) {
        args.lpI2CDataOut = buf ? cpu_to_le16(*buf) : 0;
    } else if (!buf) {
        err = -EINVAL;
        goto done;
    } else {
        args.lpI2CDataOut = 0;
    }

    args.ucFlag = flags;
    args.ucI2CSpeed = TARGET_HW_I2C_CLOCK;
    args.ucTransBytes = 1;
    args.ucSlaveAddr = slave_addr << 1;
    args.ucRegIndex = offset;
    args.ucLineNumber = 6;

    AURA_DBG("Pre Transaction: addr = %x, offset = %x, rw = %s, count = %d, out = %x",
        args.ucSlaveAddr >> 1,
        offset,
        args.ucFlag == HW_I2C_READ ? "r" : "w",
        args.ucTransBytes,
        args.lpI2CDataOut
    );

    atom_execute_table(context->atom_context, index, (uint32_t *)&args);

    AURA_DBG(
        "Post Transaction: status = %x, read = %x",
        args.ucStatus,
        args.lpI2CDataOut
    );

    /* error */
    if (args.ucStatus != HW_ASSISTED_I2C_STATUS_SUCCESS) {
        AURA_DBG("hw_i2c error %x", args.ucStatus);
        err = -EIO;
        goto done;
    }

    if (!(flags & HW_I2C_WRITE)) {
        *buf = (uint8_t)args.lpI2CDataOut;
    }

done:
    return err;
}

#define dump_i2c_msg(num, msg) ({ \
    AURA_DBG( \
        "Message %d addr = %x, len = %d, flags = %x, data = %x %x", \
        (num), \
        (msg).addr, \
        (msg).len, \
        (msg).flags, \
        (msg).buf[0], \
        (msg).buf[1] \
    ); \
})

static int aura_gpu_i2c_xfer(
    struct i2c_adapter *i2c_adap,
    struct i2c_msg *msgs,
    int num
){
    struct hw_i2c_context *context = i2c_get_adapdata(i2c_adap);
    int remaining, buffer_offset;
    uint8_t flags;
    uint8_t offset;
    uint8_t address;
    error_t err;
    int i;

    /* check for bus probe */
    if ((num == 1) && (msgs[0].len == 0)) {
        if (msgs[0].len == 0) {
            // AURA_DBG("Bus probe detected %x", msgs[0].addr);
            err = aura_gpu_i2c_process_i2c_ch(
                context,
                msgs[0].addr,
                0,
                HW_I2C_WRITE,
                NULL
            );
        }

        return err ? err : num;
    } else if ((num == 1) && (msgs[0].len > 1) && !(msgs[0].flags & I2C_M_RD)) {
        // The first data byte should be the offset, remainder are the values
        offset = msgs[0].buf[0];
        address = msgs[0].addr;
        remaining = msgs[0].len - 1;
        buffer_offset = 1;

        while (remaining) {
            err = aura_gpu_i2c_process_i2c_ch(
                context,
                address,
                offset,
                HW_I2C_WRITE,
                &msgs[0].buf[buffer_offset]
            );

            if (err)
                return err;

            offset++;
            remaining--;
            buffer_offset++;
        }
    } else if (num == 2) {
        // The first message should be to set the offset
        if (!(msgs[0].flags & I2C_M_RD) && msgs[0].len == 1) {
            offset = msgs[0].buf[0];
            address = msgs[0].addr;
        } else {
            return -EIO;
        }

        // The second message should describe how many bytes to read/write
        remaining = msgs[1].len;
        buffer_offset = 0;
        flags = (msgs[1].flags & I2C_M_RD) ? HW_I2C_READ : HW_I2C_WRITE;

        while (remaining) {
            err = aura_gpu_i2c_process_i2c_ch(
                context,
                address,
                offset,
                flags,
                &msgs[1].buf[buffer_offset]
            );

            if (err)
                return err;

            offset++;
            remaining--;
            buffer_offset++;
        }
    } else {
        AURA_DBG("Failed to process messages");
        for (i = 0; i < num; i++)
            dump_i2c_msg(i, msgs[i]);

        return -EIO;
    }

    return num;
}

static uint32_t aura_gpu_i2c_func(
    struct i2c_adapter *adap
){
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm aura_gpu_i2c_algo = {
    .master_xfer = aura_gpu_i2c_xfer,
    .functionality = aura_gpu_i2c_func,
};


static void aura_gpu_i2c_destroy (
    struct hw_i2c_context *context
){
    if (context->bios)
        atom_bios_release(context->bios);

    if (context->reg_service)
        aura_gpu_reg_destroy(context->reg_service);

    if (context->atom_context)
        atom_destroy(context->atom_context);

    if (context->registered) {
        i2c_set_adapdata(&context->adapter, NULL);
        i2c_del_adapter(&context->adapter);
    }

    kfree(context);
}

static struct hw_i2c_context *aura_gpu_i2c_create (
    struct pci_dev *pci_dev
){
    error_t err;
    struct hw_i2c_context *context = kzalloc(sizeof(*context), GFP_KERNEL);

    if (!context)
        return ERR_PTR(-ENOMEM);

    context->bios = atom_bios_create(pci_dev);
    if (IS_ERR_OR_NULL(context->bios)) {
        err = CLEAR_ERR(context->bios);
        goto error_free_all;
    }

    context->reg_service = aura_gpu_reg_create(pci_dev);
    if (IS_ERR_OR_NULL(context->reg_service)) {
        err = CLEAR_ERR(context->reg_service);
        goto error_free_all;
    }

    context->atom_card_info.reg_read    = mm_read;
    context->atom_card_info.reg_write   = mm_write;

    context->atom_card_info.ioreg_read  = __invalid_read;
    context->atom_card_info.ioreg_write = __invalid_write;
    context->atom_card_info.mc_read     = __invalid_read;
    context->atom_card_info.mc_write    = __invalid_write;
    context->atom_card_info.pll_read    = __invalid_read;
    context->atom_card_info.pll_write   = __invalid_write;

    context->atom_context = atom_parse(&context->atom_card_info, context->bios->data);
    if (!context->atom_context) {
        kfree(context);
        return ERR_PTR(-ENOMEM);
    }

    mutex_init(&context->atom_context->mutex);
    context->atom_context->scratch = (uint32_t*)context->scratch;
    context->atom_context->scratch_size_bytes = sizeof(context->scratch);
    context->adapter.owner = THIS_MODULE;
    context->adapter.class = I2C_CLASS_DDC;
    context->adapter.dev.parent = &pci_dev->dev;

    i2c_set_adapdata(&context->adapter, context);

    snprintf(context->adapter.name, sizeof(context->adapter.name), "AURA GPU adapter");
    context->adapter.algo = &aura_gpu_i2c_algo;

    err = i2c_add_adapter(&context->adapter);
    if (err)
        goto error_free_all;

    context->registered = true;

    return context;

error_free_all:
    aura_gpu_i2c_destroy(context);

    return ERR_PTR(err);
}

static struct pci_dev *find_pci_dev (
    void
){
    struct pci_dev *pci_dev = NULL;
    const struct pci_device_id *match;

    while (NULL != (pci_dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pci_dev))) {
        match = pci_match_id(pciidlist, pci_dev);
        if (match)
            return pci_dev;
    }

    return NULL;
}

struct i2c_adapter *aura_i2c_bios_create (
    void
){
    struct pci_dev *pci_dev = find_pci_dev();
    struct hw_i2c_context *context;

    if (!pci_dev) {
        AURA_ERR("Failed to find a valid pci device");
        return NULL;
    }

    context = aura_gpu_i2c_create(pci_dev);
    if (IS_ERR_OR_NULL(context))
        return ERR_PTR(CLEAR_ERR(context));

    return &context->adapter;
}

void aura_i2c_bios_destroy (
    struct i2c_adapter *i2c_adapter
){
    struct hw_i2c_context *context = context_from_adapter(i2c_adapter);

    if (IS_NULL(i2c_adapter))
        return;

    aura_gpu_i2c_destroy(context);
}
