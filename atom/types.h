#ifndef _UAPI_AURA_ATOM_TYPES_H
#define _UAPI_AURA_ATOM_TYPES_H

#include <linux/ctype.h>

// #define AMD_VBIOS_SIGNATURE " 761295520"
// #define AMD_VBIOS_SIGNATURE_OFFSET 0x30
// #define AMD_VBIOS_SIGNATURE_SIZE sizeof(AMD_VBIOS_SIGNATURE)
// #define AMD_VBIOS_SIGNATURE_END (AMD_VBIOS_SIGNATURE_OFFSET + AMD_VBIOS_SIGNATURE_SIZE)
// #define AMD_IS_VALID_VBIOS(p) ((p)[0] == 0x55 && (p)[1] == 0xAA)
// #define AMD_VBIOS_LENGTH(p) ((p)[2] << 9)
//
// #define ATOM_ROM_PART_NUMBER_PTR	0x6E
// #define ATOM_ROM_TABLE_PTR	        0x48
// #define ATOM_ROM_MSG_PTR	        0x10
// #define ATOM_ROM_CMD_PTR	        0x1E
// #define ATOM_ROM_DATA_PTR	        0x20

// #define ATOM_DEVICE_RESERVEDC_INDEX                       0x0000000C
// #define ATOM_DEVICE_RESERVEDD_INDEX                       0x0000000D
// #define ATOM_DEVICE_RESERVEDE_INDEX                       0x0000000E
#define ATOM_DEVICE_RESERVEDF_INDEX                       0x0000000F
// #define ATOM_MAX_SUPPORTED_DEVICE_INFO                    (ATOM_DEVICE_DFP3_INDEX+1)
// #define ATOM_MAX_SUPPORTED_DEVICE_INFO_2                  ATOM_MAX_SUPPORTED_DEVICE_INFO
// #define ATOM_MAX_SUPPORTED_DEVICE_INFO_3                  (ATOM_DEVICE_DFP5_INDEX + 1 )

#define   ATOM_VRAM_BLOCK_SRIOV_MSG_SHARE_RESERVATION 0x2
#define ATOM_MAX_SUPPORTED_DEVICE                         (ATOM_DEVICE_RESERVEDF_INDEX+1)

#define  ATOM_VRAM_OPERATION_FLAGS_MASK         0xC0000000L
#define  ATOM_VRAM_OPERATION_FLAGS_SHIFT        30
// #define   ATOM_VRAM_BLOCK_NEEDS_NO_RESERVATION   0x1
// #define   ATOM_VRAM_BLOCK_NEEDS_RESERVATION      0x0
#define   ATOM_VRAM_BLOCK_SRIOV_MSG_SHARE_RESERVATION 0x2

#define  ATOM_MAX_FIRMWARE_VRAM_USAGE_INFO         1

#define MasterDataTableIndexOf(FieldName) (((char*)(&((ATOM_MASTER_LIST_OF_DATA_TABLES*)0)->FieldName)-(char*)0)/sizeof(uint16_t))
#define MasterCommandTableIndexOf(FieldName) (((char*)(&((ATOM_MASTER_LIST_OF_COMMAND_TABLES*)0)->FieldName)-(char*)0)/sizeof(uint16_t))
#define MasterTableIndexOf(master_table, table_name) (offsetof(master_table, table_name) / sizeof(uint16_t))

typedef struct _ATOM_MASTER_LIST_OF_DATA_TABLES_V2_1 {
    uint16_t utilitypipeline;               /* Offest for the utility to get parser info,Don't change this position!*/
    uint16_t multimedia_info;
    uint16_t smc_dpm_info;
    uint16_t sw_datatable3;
    uint16_t firmwareinfo;                  /* Shared by various SW components */
    uint16_t sw_datatable5;
    uint16_t lcd_info;                      /* Shared by various SW components */
    uint16_t sw_datatable7;
    uint16_t smu_info;
    uint16_t sw_datatable9;
    uint16_t sw_datatable10;
    uint16_t vram_usagebyfirmware;          /* Shared by various SW components */
    uint16_t gpio_pin_lut;                  /* Shared by various SW components */
    uint16_t sw_datatable13;
    uint16_t gfx_info;
    uint16_t powerplayinfo;                 /* Shared by various SW components */
    uint16_t sw_datatable16;
    uint16_t sw_datatable17;
    uint16_t sw_datatable18;
    uint16_t sw_datatable19;
    uint16_t sw_datatable20;
    uint16_t sw_datatable21;
    uint16_t displayobjectinfo;             /* Shared by various SW components */
    uint16_t indirectioaccess;			  /* used as an internal one */
    uint16_t umc_info;                      /* Shared by various SW components */
    uint16_t sw_datatable25;
    uint16_t sw_datatable26;
    uint16_t dce_info;                      /* Shared by various SW components */
    uint16_t vram_info;                     /* Shared by various SW components */
    uint16_t sw_datatable29;
    uint16_t integratedsysteminfo;          /* Shared by various SW components */
    uint16_t asic_profiling_info;           /* Shared by various SW components */
    uint16_t voltageobject_info;            /* shared by various SW components */
    uint16_t sw_datatable33;
    uint16_t sw_datatable34;
} ATOM_MASTER_LIST_OF_DATA_TABLES_V2_1;

typedef struct _ATOM_MASTER_LIST_OF_DATA_TABLES {
    uint16_t        UtilityPipeLine;          // Offest for the utility to get parser info,Don't change this position!
    uint16_t        MultimediaCapabilityInfo; // Only used by MM Lib,latest version 1.1, not configuable from Bios, need to include the table to build Bios
    uint16_t        MultimediaConfigInfo;     // Only used by MM Lib,latest version 2.1, not configuable from Bios, need to include the table to build Bios
    uint16_t        StandardVESA_Timing;      // Only used by Bios
    uint16_t        FirmwareInfo;             // Shared by various SW components,latest version 1.4
    uint16_t        PaletteData;              // Only used by BIOS
    uint16_t        LCD_Info;                 // Shared by various SW components,latest version 1.3, was called LVDS_Info
    uint16_t        DIGTransmitterInfo;       // Internal used by VBIOS only version 3.1
    uint16_t        SMU_Info;                 // Shared by various SW components,latest version 1.1
    uint16_t        SupportedDevicesInfo;     // Will be obsolete from R600
    uint16_t        GPIO_I2C_Info;            // Shared by various SW components,latest version 1.2 will be used from R600
    uint16_t        VRAM_UsageByFirmware;     // Shared by various SW components,latest version 1.3 will be used from R600
    uint16_t        GPIO_Pin_LUT;             // Shared by various SW components,latest version 1.1
    uint16_t        VESA_ToInternalModeLUT;   // Only used by Bios
    uint16_t        GFX_Info;                 // Shared by various SW components,latest version 2.1 will be used from R600
    uint16_t        PowerPlayInfo;            // Shared by various SW components,latest version 2.1,new design from R600
    uint16_t        GPUVirtualizationInfo;    // Will be obsolete from R600
    uint16_t        SaveRestoreInfo;          // Only used by Bios
    uint16_t        PPLL_SS_Info;             // Shared by various SW components,latest version 1.2, used to call SS_Info, change to new name because of int ASIC SS info
    uint16_t        OemInfo;                  // Defined and used by external SW, should be obsolete soon
    uint16_t        XTMDS_Info;               // Will be obsolete from R600
    uint16_t        MclkSS_Info;              // Shared by various SW components,latest version 1.1, only enabled when ext SS chip is used
    uint16_t        Object_Header;            // Shared by various SW components,latest version 1.1
    uint16_t        IndirectIOAccess;         // Only used by Bios,this table position can't change at all!!
    uint16_t        MC_InitParameter;         // Only used by command table
    uint16_t        ASIC_VDDC_Info;           // Will be obsolete from R600
    uint16_t        ASIC_InternalSS_Info;     // New tabel name from R600, used to be called "ASIC_MVDDC_Info"
    uint16_t        TV_VideoMode;             // Only used by command table
    uint16_t        VRAM_Info;                // Only used by command table, latest version 1.3
    uint16_t        MemoryTrainingInfo;       // Used for VBIOS and Diag utility for memory training purpose since R600. the new table rev start from 2.1
    uint16_t        IntegratedSystemInfo;     // Shared by various SW components
    uint16_t        ASIC_ProfilingInfo;       // New table name from R600, used to be called "ASIC_VDDCI_Info" for pre-R600
    uint16_t        VoltageObjectInfo;        // Shared by various SW components, latest version 1.1
    uint16_t        PowerSourceInfo;          // Shared by various SW components, latest versoin 1.1
    uint16_t        ServiceInfo;
} ATOM_MASTER_LIST_OF_DATA_TABLES;

typedef struct _ATOM_MASTER_LIST_OF_COMMAND_TABLES {
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
} ATOM_MASTER_LIST_OF_COMMAND_TABLES;

typedef struct _ATOM_COMMON_TABLE_HEADER {
    uint16_t usStructureSize;
    uint8_t  ucTableFormatRevision;   //Change it when the Parser is not backward compatible
    uint8_t  ucTableContentRevision;  //Change it only when the table needs to change but the firmware
                                      //Image can't be updated, while Driver needs to carry the new table!
} ATOM_COMMON_TABLE_HEADER;

typedef struct _ATOM_OBJECT_HEADER
{
    ATOM_COMMON_TABLE_HEADER   sHeader;
    uint16_t                    usDeviceSupport;
    uint16_t                    usConnectorObjectTableOffset;
    uint16_t                    usRouterObjectTableOffset;
    uint16_t                    usEncoderObjectTableOffset;
    uint16_t                    usProtectionObjectTableOffset; //only available when Protection block is independent.
    uint16_t                    usDisplayPathTableOffset;
}ATOM_OBJECT_HEADER;

typedef struct _ATOM_I2C_ID_CONFIG {
#if defined(__BIG_ENDIAN)
  uint8_t   bfHW_Capable:1;
  uint8_t   bfHW_EngineID:3;
  uint8_t   bfI2C_LineMux:4;
#else
  uint8_t   bfI2C_LineMux:4;
  uint8_t   bfHW_EngineID:3;
  uint8_t   bfHW_Capable:1;
#endif
} ATOM_I2C_ID_CONFIG;

typedef union _ATOM_I2C_ID_CONFIG_ACCESS {
  ATOM_I2C_ID_CONFIG  sbfAccess;
  uint8_t             ucAccess;
} ATOM_I2C_ID_CONFIG_ACCESS;

typedef struct _ATOM_GPIO_I2C_ASSIGMENT {
    uint16_t                    usClkMaskRegisterIndex;
    uint16_t                    usClkEnRegisterIndex;
    uint16_t                    usClkY_RegisterIndex;
    uint16_t                    usClkA_RegisterIndex;
    uint16_t                    usDataMaskRegisterIndex;
    uint16_t                    usDataEnRegisterIndex;
    uint16_t                    usDataY_RegisterIndex;
    uint16_t                    usDataA_RegisterIndex;
    ATOM_I2C_ID_CONFIG_ACCESS   sucI2cId;
    uint8_t                     ucClkMaskShift;
    uint8_t                     ucClkEnShift;
    uint8_t                     ucClkY_Shift;
    uint8_t                     ucClkA_Shift;
    uint8_t                     ucDataMaskShift;
    uint8_t                     ucDataEnShift;
    uint8_t                     ucDataY_Shift;
    uint8_t                     ucDataA_Shift;
    uint8_t                     ucReserved1;
    uint8_t                     ucReserved2;
} ATOM_GPIO_I2C_ASSIGMENT;

typedef struct _ATOM_GPIO_I2C_INFO {
    ATOM_COMMON_TABLE_HEADER   sHeader;
    ATOM_GPIO_I2C_ASSIGMENT    asGPIO_Info[ATOM_MAX_SUPPORTED_DEVICE];
} ATOM_GPIO_I2C_INFO;

typedef struct _ATOM_FIRMWARE_VRAM_RESERVE_INFO {
    uint32_t  ulStartAddrUsedByFirmware;
    uint16_t  usFirmwareUseInKb;
    uint16_t  usReserved;
} ATOM_FIRMWARE_VRAM_RESERVE_INFO;

typedef struct _ATOM_VRAM_USAGE_BY_FIRMWARE {
    ATOM_COMMON_TABLE_HEADER         sHeader;
    ATOM_FIRMWARE_VRAM_RESERVE_INFO  asFirmwareVramReserveInfo[ATOM_MAX_FIRMWARE_VRAM_USAGE_INFO];
} ATOM_VRAM_USAGE_BY_FIRMWARE;

typedef struct _VRAM_USAGEBYFIRMWARE_V2_1 {
    ATOM_COMMON_TABLE_HEADER  table_header;
    uint32_t                  start_address_in_kb;
    uint16_t                  used_by_firmware_in_kb;
    uint16_t                  used_by_driver_in_kb;
} VRAM_USAGEBYFIRMWARE_V2_1;

typedef struct _PROCESS_I2C_CHANNEL_TRANSACTION_PARAMETERS {
    uint8_t   ucI2CSpeed;
    union
    {
        uint8_t ucRegIndex;
        uint8_t ucStatus;
    };
    uint16_t  lpI2CDataOut;
    uint8_t   ucFlag;
    uint8_t   ucTransBytes;
    uint8_t   ucSlaveAddr;
    uint8_t   ucLineNumber;
} PROCESS_I2C_CHANNEL_TRANSACTION_PARAMETERS;

typedef struct _ATOM_FIRMWARE_INFO_V3_1 {
    ATOM_COMMON_TABLE_HEADER  table_header;
    uint32_t                  firmware_revision;
    uint32_t                  bootup_sclk_in10khz;
    uint32_t                  bootup_mclk_in10khz;
    uint32_t                  firmware_capability;             // enum atombios_firmware_capability
    uint32_t                  main_call_parser_entry;          /* direct address of main parser call in VBIOS binary. */
    uint32_t                  bios_scratch_reg_startaddr;      // 1st bios scratch register dword address
    uint16_t                  bootup_vddc_mv;
    uint16_t                  bootup_vddci_mv;
    uint16_t                  bootup_mvddc_mv;
    uint16_t                  bootup_vddgfx_mv;
    uint8_t                   mem_module_id;
    uint8_t                   coolingsolution_id;              /*0: Air cooling; 1: Liquid cooling ... */
    uint8_t                   reserved1[2];
    uint32_t                  mc_baseaddr_high;
    uint32_t                  mc_baseaddr_low;
    uint32_t                  reserved2[6];
} ATOM_FIRMWARE_INFO_V3_1;

typedef struct _PROCESS_AUX_CHANNEL_TRANSACTION_PARAMETERS {
    uint16_t  lpAuxRequest;
    uint16_t  lpDataOut;
    uint8_t   ucChannelID;
    union {
        uint8_t   ucReplyStatus;
        uint8_t   ucDelay;
    };
    uint8_t   ucDataOutLen;
    uint8_t   ucReserved;
} PROCESS_AUX_CHANNEL_TRANSACTION_PARAMETERS;

typedef struct _PROCESS_AUX_CHANNEL_TRANSACTION_PARAMETERS_V2 {
    uint16_t     lpAuxRequest;
    uint16_t     lpDataOut;
    uint8_t      ucChannelID;
    union {
        uint8_t  ucReplyStatus;
        uint8_t  ucDelay;
    };
    uint8_t      ucDataOutLen;
    uint8_t      ucHPD_ID;      //=0: HPD1, =1: HPD2, =2: HPD3, =3: HPD4, =4: HPD5, =5: HPD6
} PROCESS_AUX_CHANNEL_TRANSACTION_PARAMETERS_V2;

#define PROCESS_AUX_CHANNEL_TRANSACTION_PS_ALLOCATION         PROCESS_AUX_CHANNEL_TRANSACTION_PARAMETERS

#endif
