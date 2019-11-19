#include "aura-gpu-bios.h"

#include "debug.h"

#define ATOM_BIOS_MAGIC             0xAA55
#define ATOM_ATI_MAGIC_PTR          0x30
#define ATOM_ATI_MAGIC              " 761295520"

#define ATOM_ROM_HEADER_PTR         0x48
#define ATOM_ROM_PART_NUMBER_PTR    0x6E

#define ATOM_ROM_MAGIC              "ATOM"
#define ATOM_ROM_MAGIC_PTR          4

#define ATOM_IMAGE_SIZE_PTR         2
#define ATOM_IMAGE_SIZE_UNIT        512

enum bp_result {
    BP_RESULT_OK            = 0, /* There was no error */
    BP_RESULT_BADINPUT      = -EINVAL, /*Bad input parameter */
    BP_RESULT_BADBIOSTABLE  = -EBADFD, /* Bad BIOS table */
    BP_RESULT_UNSUPPORTED   = -ENOSYS, /* BIOS Table is not supported */
    BP_RESULT_NORECORD      = -ENOMSG, /* Record can't be found */
    BP_RESULT_FAILURE       = -EFAULT
};

/* atom_gpio_pin_assignment.gpio_id definition */
enum atom_gpio_pin_assignment_gpio_id {
    I2C_HW_LANE_MUX                 = 0x0f, /* only valid when bit7=1 */
    I2C_HW_ENGINE_ID_MASK           = 0x70, /* only valid when bit7=1 */
    I2C_HW_CAP                      = 0x80, /*only when the I2C_HW_CAP is set, the pin ID is assigned to an I2C pin pair, otherwise, it's an generic GPIO pin */

    /* gpio_id pre-define id for multiple usage */
    /* GPIO use to control PCIE_VDDC in certain SLT board */
    PCIE_VDDC_CONTROL_GPIO_PINID    = 56,
    /* if PP_AC_DC_SWITCH_GPIO_PINID in Gpio_Pin_LutTable, AC/DC swithing feature is enable */
    PP_AC_DC_SWITCH_GPIO_PINID      = 60,
    /* VDDC_REGULATOR_VRHOT_GPIO_PINID in Gpio_Pin_LutTable, VRHot feature is enable */
    VDDC_VRHOT_GPIO_PINID           = 61,
    /*if VDDC_PCC_GPIO_PINID in GPIO_LUTable, Peak Current Control feature is enabled */
    VDDC_PCC_GPIO_PINID             = 62,
    /* Only used on certain SLT/PA board to allow utility to cut Efuse. */
    EFUSE_CUT_ENABLE_GPIO_PINID     = 63,
    /* ucGPIO=DRAM_SELF_REFRESH_GPIO_PIND uses  for memory self refresh (ucGPIO=0, DRAM self-refresh; ucGPIO= */
    DRAM_SELF_REFRESH_GPIO_PINID    = 64,
    /* Thermal interrupt output->system thermal chip GPIO pin */
    THERMAL_INT_OUTPUT_GPIO_PINID   = 65,
};

enum atom_object_record_type_id {
    ATOM_I2C_RECORD_TYPE                        = 1,
    ATOM_HPD_INT_RECORD_TYPE                    = 2,
    ATOM_CONNECTOR_DEVICE_TAG_RECORD_TYPE       = 4,
    ATOM_OBJECT_GPIO_CNTL_RECORD_TYPE           = 9,
    ATOM_CONNECTOR_HPDPIN_LUT_RECORD_TYPE       = 16,
    ATOM_CONNECTOR_AUXDDC_LUT_RECORD_TYPE       = 17,
    ATOM_ENCODER_CAP_RECORD_TYPE                = 20,
    ATOM_BRACKET_LAYOUT_RECORD_TYPE             = 21,
    ATOM_CONNECTOR_FORCED_TMDS_CAP_RECORD_TYPE  = 22,
    ATOM_RECORD_END_TYPE                        = 0xFF,
};

#pragma pack(1)

struct atom_common_table_header {
    uint16_t structuresize;
    uint8_t  format_revision;   //mainly used for a hw function, when the parser is not backward compatible
    uint8_t  content_revision;  //change it when a data table has a structure change, or a hw function has a input/output parameter change
};

struct atom_rom_header_v1_1 {
    struct atom_common_table_header table_header;
    uint8_t  atom_bios_string[4];        //enum atom_string_def atom_bios_string;     //Signature to distinguish between Atombios and non-atombios,
    uint16_t bios_segment_address;
    uint16_t protectedmodeoffset;
    uint16_t configfilenameoffset;
    uint16_t crc_block_offset;
    uint16_t vbios_bootupmessageoffset;
    uint16_t int10_offset;
    uint16_t pcibusdevinitcode;
    uint16_t iobaseaddress;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint16_t pci_info_offset;
    uint16_t masterhwfunction_offset;      //Offest for SW to get all command function offsets, Don't change the position
    uint16_t masterdatatable_offset;       //Offest for SW to get all data table offsets, Don't change the position
    uint8_t  ucExtendedFunctionCode;
    uint8_t  ucReserved;
};

struct atom_rom_header_v2_2 {
    struct atom_common_table_header table_header;
    uint8_t  atom_bios_string[4];        //enum atom_string_def atom_bios_string;     //Signature to distinguish between Atombios and non-atombios,
    uint16_t bios_segment_address;
    uint16_t protectedmodeoffset;
    uint16_t configfilenameoffset;
    uint16_t crc_block_offset;
    uint16_t vbios_bootupmessageoffset;
    uint16_t int10_offset;
    uint16_t pcibusdevinitcode;
    uint16_t iobaseaddress;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint16_t pci_info_offset;
    uint16_t masterhwfunction_offset;      //Offest for SW to get all command function offsets, Don't change the position
    uint16_t masterdatatable_offset;       //Offest for SW to get all data table offsets, Don't change the position
    uint16_t reserved;
    uint32_t pspdirtableoffset;
};

struct atom_display_object_path_v2 {
    uint16_t display_objid;                  //Connector Object ID or Misc Object ID
    uint16_t disp_recordoffset;
    uint16_t encoderobjid;                   //first encoder closer to the connector, could be either an external or intenal encoder
    uint16_t extencoderobjid;                //2nd encoder after the first encoder, from the connector point of view;
    uint16_t encoder_recordoffset;
    uint16_t extencoder_recordoffset;
    uint16_t device_tag;                     //a supported device vector, each display path starts with this.the paths are enumerated in the way of priority, a path appears first
    uint8_t  priority_id;
    uint8_t  reserved;
};

struct display_object_info_table_v1_1 {
    struct    atom_common_table_header table_header;
    uint16_t  usDeviceSupport;
    uint16_t  usConnectorObjectTableOffset;
    uint16_t  usRouterObjectTableOffset;
    uint16_t  usEncoderObjectTableOffset;
    uint16_t  usProtectionObjectTableOffset; //only available when Protection block is independent.
    uint16_t  usDisplayPathTableOffset;
};

struct atom_object {
    uint16_t usObjectID;
    uint16_t usSrcDstTableOffset;
    uint16_t usRecordOffset;                     //this pointing to a bunch of records defined below
    uint16_t usReserved;
};

struct atom_object_table {
    uint8_t            ucNumberOfObjects;
    uint8_t            ucPadding[3];
    struct atom_object asObjects[1];
};

struct display_object_info_table_v1_3 {
    struct    atom_common_table_header table_header;
    uint16_t  usDeviceSupport;
    uint16_t  usConnectorObjectTableOffset;
    uint16_t  usRouterObjectTableOffset;
    uint16_t  usEncoderObjectTableOffset;
    uint16_t  usProtectionObjectTableOffset; //only available when Protection block is independent.
    uint16_t  usDisplayPathTableOffset;
    uint16_t  usMiscObjectTableOffset;
};

struct display_object_info_table_v1_4 {
    struct    atom_common_table_header  table_header;
    uint16_t  supporteddevices;
    uint8_t   number_of_path;
    uint8_t   reserved;
    struct    atom_display_object_path_v2 display_path[8];   //the real number of this included in the structure is calculated by using the (whole structure size - the header size- number_of_path)/size of atom_display_object_path
};

struct atom_master_list_of_data_tables_v2_1 {
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
    uint16_t indirectioaccess;              /* used as an internal one */
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
};

struct atom_master_data_table_v2_1 {
    struct atom_common_table_header               table_header;
    struct atom_master_list_of_data_tables_v2_1   list_of_datatables;
};

struct atom_master_list_of_data_tables_v1_1 {
    uint16_t utilitypipeline;
    uint16_t multimediacapabilityinfo;
    uint16_t multimediaconfiginfo;
    uint16_t standardvesa_timing;
    uint16_t firmwareinfo;
    uint16_t palettedata;
    uint16_t lcd_info;
    uint16_t digtransmitterinfo;
    uint16_t smu_info;
    uint16_t supporteddevicesinfo;
    uint16_t gpio_i2c_info;
    uint16_t vram_usagebyfirmware;
    uint16_t gpio_pin_lut;
    uint16_t vesa_tointernalmodelut;
    uint16_t gfx_info;
    uint16_t powerplayinfo;
    uint16_t gpuvirtualizationinfo;
    uint16_t saverestoreinfo;
    uint16_t ppll_ss_info;
    uint16_t oeminfo;
    uint16_t xtmds_info;
    uint16_t mclkss_info;
    uint16_t object_header;
    uint16_t indirectioaccess;
    uint16_t mc_initparameter;
    uint16_t asic_vddc_info;
    uint16_t asic_internalss_info;
    uint16_t tv_videomode;
    uint16_t vram_info;
    uint16_t memorytraininginfo;
    uint16_t integratedsysteminfo;
    uint16_t asic_profiling_info;
    uint16_t voltageobject_info;
    uint16_t powersourceinfo;
    uint16_t serviceinfo;
};

struct atom_master_data_table_v1_1 {
    struct atom_common_table_header               table_header;
    struct atom_master_list_of_data_tables_v1_1   list_of_datatables;
};

struct atom_common_record_header {
    uint8_t record_type;                      //An emun to indicate the record type
    uint8_t record_size;                      //The size of the whole record in byte
};

struct atom_i2c_record {
    struct atom_common_record_header record_header;   //record_type = ATOM_I2C_RECORD_TYPE
    uint8_t i2c_id;
    uint8_t i2c_slave_addr;                   //The slave address, it's 0 when the record is attached to connector for DDC
};

struct atom_i2c_id_config {
#if defined(__BIG_ENDIAN)
    uint8_t   bfHW_Capable:1;
    uint8_t   bfHW_EngineID:3;
    uint8_t   bfI2C_LineMux:4;
#else
    uint8_t   bfI2C_LineMux:4;
    uint8_t   bfHW_EngineID:3;
    uint8_t   bfHW_Capable:1;
#endif
};

union atom_i2c_id_config_access {
    struct atom_i2c_id_config sbfAccess;
    uint8_t                   ucAccess;
};

struct atom_gpio_i2c_assigment {
    uint16_t  usClkMaskRegisterIndex;
    uint16_t  usClkEnRegisterIndex;
    uint16_t  usClkY_RegisterIndex;
    uint16_t  usClkA_RegisterIndex;
    uint16_t  usDataMaskRegisterIndex;
    uint16_t  usDataEnRegisterIndex;
    uint16_t  usDataY_RegisterIndex;
    uint16_t  usDataA_RegisterIndex;
    union atom_i2c_id_config_access   sucI2cId;
    uint8_t  ucClkMaskShift;
    uint8_t  ucClkEnShift;
    uint8_t  ucClkY_Shift;
    uint8_t  ucClkA_Shift;
    uint8_t  ucDataMaskShift;
    uint8_t  ucDataEnShift;
    uint8_t  ucDataY_Shift;
    uint8_t  ucDataA_Shift;
    uint8_t  ucReserved1;
    uint8_t  ucReserved2;
};

#define ATOM_MAX_SUPPORTED_DEVICE   0x10
struct atom_gpio_i2c_info {
    struct atom_common_table_header   table_header;
    struct atom_gpio_i2c_assigment    asGPIO_Info[ATOM_MAX_SUPPORTED_DEVICE];
};

struct atom_gpio_pin_assignment {
    uint32_t data_a_reg_index;
    uint8_t  gpio_bitshift;
    uint8_t  gpio_mask_bitshift;
    uint8_t  gpio_id;
    uint8_t  reserved;
};

struct atom_gpio_pin_lut_v2_1 {
    struct  atom_common_table_header  table_header;
    /*the real number of this included in the structure is calcualted by using the (whole structure size - the header size)/size of atom_gpio_pin_lut  */
    struct  atom_gpio_pin_assignment  gpio_pin[8];
};

struct atom_pplib_thermalcontroller {
    uint8_t ucType;           // one of ATOM_PP_THERMALCONTROLLER_*
    uint8_t ucI2cLine;        // as interpreted by DAL I2C
    uint8_t ucI2cAddress;
    uint8_t ucFanParameters;  // Fan Control Parameters.
    uint8_t ucFanMinRPM;      // Fan Minimum RPM (hundreds) -- for display purposes only.
    uint8_t ucFanMaxRPM;      // Fan Maximum RPM (hundreds) -- for display purposes only.
    uint8_t ucReserved;       // ----
    uint8_t ucFlags;          // to be defined
};

struct atom_pplib_powerplaytable {
    struct  atom_common_table_header  table_header;

    uint8_t ucDataRevision;

    uint8_t ucNumStates;
    uint8_t ucStateEntrySize;
    uint8_t ucClockInfoSize;
    uint8_t ucNonClockSize;

    // offset from start of this table to array of ucNumStates ATOM_PPLIB_STATE structures
    uint16_t usStateArrayOffset;

    // offset from start of this table to array of ASIC-specific structures,
    // currently ATOM_PPLIB_CLOCK_INFO.
    uint16_t usClockInfoArrayOffset;

    // offset from start of this table to array of ATOM_PPLIB_NONCLOCK_INFO
    uint16_t usNonClockInfoArrayOffset;

    uint16_t usBackbiasTime;    // in microseconds
    uint16_t usVoltageTime;     // in microseconds
    uint16_t usTableSize;       //the size of this structure, or the extended structure

    uint32_t ulPlatformCaps;            // See ATOM_PPLIB_CAPS_*

    struct atom_pplib_thermalcontroller    sThermalController;

    uint16_t usBootClockInfoOffset;
    uint16_t usBootNonClockInfoOffset;
};


struct atom_display_controller_info_v4_1 {
    struct  atom_common_table_header  table_header;
    uint32_t display_caps;
    uint32_t bootup_dispclk_10khz;
    uint16_t dce_refclk_10khz;
    uint16_t i2c_engine_refclk_10khz;
    uint16_t dvi_ss_percentage;       // in unit of 0.001%
    uint16_t dvi_ss_rate_10hz;
    uint16_t hdmi_ss_percentage;      // in unit of 0.001%
    uint16_t hdmi_ss_rate_10hz;
    uint16_t dp_ss_percentage;        // in unit of 0.001%
    uint16_t dp_ss_rate_10hz;
    uint8_t  dvi_ss_mode;             // enum of atom_spread_spectrum_mode
    uint8_t  hdmi_ss_mode;            // enum of atom_spread_spectrum_mode
    uint8_t  dp_ss_mode;              // enum of atom_spread_spectrum_mode
    uint8_t  ss_reserved;
    uint8_t  hardcode_mode_num;       // a hardcode mode number defined in StandardVESA_TimingTable when a CRT or DFP EDID is not available
    uint8_t  reserved1[3];
    uint16_t dpphy_refclk_10khz;
    uint16_t reserved2;
    uint8_t  dceip_min_ver;
    uint8_t  dceip_max_ver;
    uint8_t  max_disp_pipe_num;
    uint8_t  max_vbios_active_disp_pipe_num;
    uint8_t  max_ppll_num;
    uint8_t  max_disp_phy_num;
    uint8_t  max_aux_pairs;
    uint8_t  remotedisplayconfig;
    uint8_t  reserved3[8];
};

struct atom_display_controller_info_v4_2 {
    struct  atom_common_table_header  table_header;
    uint32_t display_caps;
    uint32_t bootup_dispclk_10khz;
    uint16_t dce_refclk_10khz;
    uint16_t i2c_engine_refclk_10khz;
    uint16_t dvi_ss_percentage;       // in unit of 0.001%
    uint16_t dvi_ss_rate_10hz;
    uint16_t hdmi_ss_percentage;      // in unit of 0.001%
    uint16_t hdmi_ss_rate_10hz;
    uint16_t dp_ss_percentage;        // in unit of 0.001%
    uint16_t dp_ss_rate_10hz;
    uint8_t  dvi_ss_mode;             // enum of atom_spread_spectrum_mode
    uint8_t  hdmi_ss_mode;            // enum of atom_spread_spectrum_mode
    uint8_t  dp_ss_mode;              // enum of atom_spread_spectrum_mode
    uint8_t  ss_reserved;
    uint8_t  dfp_hardcode_mode_num;   // DFP hardcode mode number defined in StandardVESA_TimingTable when EDID is not available
    uint8_t  dfp_hardcode_refreshrate;// DFP hardcode mode refreshrate defined in StandardVESA_TimingTable when EDID is not available
    uint8_t  vga_hardcode_mode_num;   // VGA hardcode mode number defined in StandardVESA_TimingTable when EDID is not avablable
    uint8_t  vga_hardcode_refreshrate;// VGA hardcode mode number defined in StandardVESA_TimingTable when EDID is not avablable
    uint16_t dpphy_refclk_10khz;
    uint16_t reserved2;
    uint8_t  dcnip_min_ver;
    uint8_t  dcnip_max_ver;
    uint8_t  max_disp_pipe_num;
    uint8_t  max_vbios_active_disp_pipe_num;
    uint8_t  max_ppll_num;
    uint8_t  max_disp_phy_num;
    uint8_t  max_aux_pairs;
    uint8_t  remotedisplayconfig;
    uint8_t  reserved3[8];
};

struct atom_firmware_info_v1_4 {
    struct  atom_common_table_header  table_header;
    uint32_t  ulFirmwareRevision;
    uint32_t  ulDefaultEngineClock;       //In 10Khz unit
    uint32_t  ulDefaultMemoryClock;       //In 10Khz unit
    uint32_t  ulDriverTargetEngineClock;  //In 10Khz unit
    uint32_t  ulDriverTargetMemoryClock;  //In 10Khz unit
    uint32_t  ulMaxEngineClockPLL_Output; //In 10Khz unit
    uint32_t  ulMaxMemoryClockPLL_Output; //In 10Khz unit
    uint32_t  ulMaxPixelClockPLL_Output;  //In 10Khz unit
    uint32_t  ulASICMaxEngineClock;       //In 10Khz unit
    uint32_t  ulASICMaxMemoryClock;       //In 10Khz unit
    uint8_t   ucASICMaxTemperature;
    uint8_t   ucMinAllowedBL_Level;
    uint16_t  usBootUpVDDCVoltage;        //In MV unit
    uint16_t  usLcdMinPixelClockPLL_Output; // In MHz unit
    uint16_t  usLcdMaxPixelClockPLL_Output; // In MHz unit
    uint32_t  ul3DAccelerationEngineClock;//In 10Khz unit
    uint32_t  ulMinPixelClockPLL_Output;  //In 10Khz unit
    uint16_t  usMinEngineClockPLL_Input;  //In 10Khz unit
    uint16_t  usMaxEngineClockPLL_Input;  //In 10Khz unit
    uint16_t  usMinEngineClockPLL_Output; //In 10Khz unit
    uint16_t  usMinMemoryClockPLL_Input;  //In 10Khz unit
    uint16_t  usMaxMemoryClockPLL_Input;  //In 10Khz unit
    uint16_t  usMinMemoryClockPLL_Output; //In 10Khz unit
    uint16_t  usMaxPixelClock;            //In 10Khz unit, Max.  Pclk
    uint16_t  usMinPixelClockPLL_Input;   //In 10Khz unit
    uint16_t  usMaxPixelClockPLL_Input;   //In 10Khz unit
    uint16_t  usMinPixelClockPLL_Output;  //In 10Khz unit - lower 16bit of ulMinPixelClockPLL_Output
    uint16_t  usFirmwareCapability;
    uint16_t  usReferenceClock;           //In 10Khz unit
    uint16_t  usPM_RTS_Location;          //RTS PM4 starting location in ROM in 1Kb unit
    uint8_t   ucPM_RTS_StreamSize;        //RTS PM4 packets in Kb unit
    uint8_t   ucDesign_ID;                //Indicate what is the board design
    uint8_t   ucMemoryModule_ID;          //Indicate what is the board design
};

struct atom_firmware_info_v2_1 {
    struct  atom_common_table_header  table_header;
    uint32_t  ulFirmwareRevision;
    uint32_t  ulDefaultEngineClock;       //In 10Khz unit
    uint32_t  ulDefaultMemoryClock;       //In 10Khz unit
    uint32_t  ulReserved1;
    uint32_t  ulReserved2;
    uint32_t  ulMaxEngineClockPLL_Output; //In 10Khz unit
    uint32_t  ulMaxMemoryClockPLL_Output; //In 10Khz unit
    uint32_t  ulMaxPixelClockPLL_Output;  //In 10Khz unit
    uint32_t  ulBinaryAlteredInfo;        //Was ulASICMaxEngineClock
    uint32_t  ulDefaultDispEngineClkFreq; //In 10Khz unit
    uint8_t   ucReserved1;                //Was ucASICMaxTemperature;
    uint8_t   ucMinAllowedBL_Level;
    uint16_t  usBootUpVDDCVoltage;        //In MV unit
    uint16_t  usLcdMinPixelClockPLL_Output; // In MHz unit
    uint16_t  usLcdMaxPixelClockPLL_Output; // In MHz unit
    uint32_t  ulReserved4;                //Was ulAsicMaximumVoltage
    uint32_t  ulMinPixelClockPLL_Output;  //In 10Khz unit
    uint16_t  usMinEngineClockPLL_Input;  //In 10Khz unit
    uint16_t  usMaxEngineClockPLL_Input;  //In 10Khz unit
    uint16_t  usMinEngineClockPLL_Output; //In 10Khz unit
    uint16_t  usMinMemoryClockPLL_Input;  //In 10Khz unit
    uint16_t  usMaxMemoryClockPLL_Input;  //In 10Khz unit
    uint16_t  usMinMemoryClockPLL_Output; //In 10Khz unit
    uint16_t  usMaxPixelClock;            //In 10Khz unit, Max.  Pclk
    uint16_t  usMinPixelClockPLL_Input;   //In 10Khz unit
    uint16_t  usMaxPixelClockPLL_Input;   //In 10Khz unit
    uint16_t  usMinPixelClockPLL_Output;  //In 10Khz unit - lower 16bit of ulMinPixelClockPLL_Output
    uint16_t  usFirmwareCapability;
    uint16_t  usCoreReferenceClock;       //In 10Khz unit
    uint16_t  usMemoryReferenceClock;     //In 10Khz unit
    uint16_t  usUniphyDPModeExtClkFreq;   //In 10Khz unit, if it is 0, In DP Mode Uniphy Input clock from internal PPLL, otherwise Input clock from external Spread clock
    uint8_t   ucMemoryModule_ID;          //Indicate what is the board design
    uint8_t   ucReserved4[3];
};

struct product_branding {
    uint8_t     ucEMBEDDED_CAP:2;          // Bit[1:0] Embedded feature level
    uint8_t     ucReserved:2;              // Bit[3:2] Reserved
    uint8_t     ucBRANDING_ID:4;           // Bit[7:4] Branding ID
};

struct atom_firmware_info_v2_2 {
    struct  atom_common_table_header  table_header;
    uint32_t  ulFirmwareRevision;
    uint32_t  ulDefaultEngineClock;       //In 10Khz unit
    uint32_t  ulDefaultMemoryClock;       //In 10Khz unit
    uint32_t  ulSPLL_OutputFreq;          //In 10Khz unit
    uint32_t  ulGPUPLL_OutputFreq;        //In 10Khz unit
    uint32_t  ulReserved1;                //Was ulMaxEngineClockPLL_Output; //In 10Khz unit*
    uint32_t  ulReserved2;                //Was ulMaxMemoryClockPLL_Output; //In 10Khz unit*
    uint32_t  ulMaxPixelClockPLL_Output;  //In 10Khz unit
    uint32_t  ulBinaryAlteredInfo;        //Was ulASICMaxEngineClock  ?
    uint32_t  ulDefaultDispEngineClkFreq; //In 10Khz unit. This is the frequency before DCDTO, corresponding to usBootUpVDDCVoltage.
    uint8_t   ucReserved3;                //Was ucASICMaxTemperature;
    uint8_t   ucMinAllowedBL_Level;
    uint16_t  usBootUpVDDCVoltage;        //In MV unit
    uint16_t  usLcdMinPixelClockPLL_Output; // In MHz unit
    uint16_t  usLcdMaxPixelClockPLL_Output; // In MHz unit
    uint32_t  ulReserved4;                //Was ulAsicMaximumVoltage
    uint32_t  ulMinPixelClockPLL_Output;  //In 10Khz unit
    uint8_t   ucRemoteDisplayConfig;
    uint8_t   ucReserved5[3];             //Was usMinEngineClockPLL_Input and usMaxEngineClockPLL_Input
    uint32_t  ulReserved6;                //Was usMinEngineClockPLL_Output and usMinMemoryClockPLL_Input
    uint32_t  ulReserved7;                //Was usMaxMemoryClockPLL_Input and usMinMemoryClockPLL_Output
    uint16_t  usReserved11;               //Was usMaxPixelClock;  //In 10Khz unit, Max.  Pclk used only for DAC
    uint16_t  usMinPixelClockPLL_Input;   //In 10Khz unit
    uint16_t  usMaxPixelClockPLL_Input;   //In 10Khz unit
    uint16_t  usBootUpVDDCIVoltage;       //In unit of mv; Was usMinPixelClockPLL_Output;
    uint16_t  usFirmwareCapability;
    uint16_t  usCoreReferenceClock;       //In 10Khz unit
    uint16_t  usMemoryReferenceClock;     //In 10Khz unit
    uint16_t  usUniphyDPModeExtClkFreq;   //In 10Khz unit, if it is 0, In DP Mode Uniphy Input clock from internal PPLL, otherwise Input clock from external Spread clock
    uint8_t   ucMemoryModule_ID;          //Indicate what is the board design
    uint8_t   ucCoolingSolution_ID;       //0: Air cooling; 1: Liquid cooling ... [COOLING_SOLUTION]
    struct product_branding ucProductBranding;          // Bit[7:4]ucBRANDING_ID: Branding ID, Bit[3:2]ucReserved: Reserved, Bit[1:0]ucEMBEDDED_CAP: Embedded feature level.
    uint8_t   ucReserved9;
    uint16_t  usBootUpMVDDCVoltage;       //In unit of mv; Was usMinPixelClockPLL_Output;
    uint16_t  usBootUpVDDGFXVoltage;      //In unit of mv;
    uint32_t  ulReserved10[3];            // New added comparing to previous version
};

#pragma pack()

enum revision {
    VERSION_1_1 = 0x0101,
    VERSION_1_2 = 0x0102,
    VERSION_1_3 = 0x0103,
    VERSION_1_4 = 0x0104,
    VERSION_2_1 = 0x0201,
    VERSION_2_2 = 0x0202,
    VERSION_3_1 = 0x0301,
    VERSION_3_2 = 0x0302,
    VERSION_3_3 = 0x0303,
};
struct atom_data_revision {
    uint16_t major;
    uint16_t minor;
};

#define revision(rev)\
    ((rev.major << 8) | (uint8_t)rev.minor)
#define table_revision(table) \
    ((table.revision.major << 8) | (uint8_t)table.revision.minor)

struct object_info_table {
    struct atom_data_revision revision;
    union {
        struct display_object_info_table_v1_1 *v1_1;
        struct display_object_info_table_v1_3 *v1_3;
        struct display_object_info_table_v1_4 *v1_4;
    };
};

struct master_data_table {
    struct atom_data_revision revision;
    union {
        struct atom_master_data_table_v1_1  *v1_1;
        struct atom_master_data_table_v2_1  *v2_1;
    };
};

struct atom_context {
    struct atom_bios            bios;

    struct object_info_table    object_info_table;
    struct master_data_table    master_data_table;

    size_t                      size;
    uint8_t                     data[];
};

#define object_info_field(context, ver, field) \
    le16_to_cpu(context->object_info_table.ver->field)

#define table_list_field(context, ver, field) \
    le16_to_cpu(context->master_data_table.ver->list_of_datatables.field)

#define to_atom_context(bios_ptr) \
    container_of(bios_ptr, struct atom_context, bios)


static inline uint8_t get_u8(struct atom_context *context, uint32_t offset)
{
    WARN_ON(offset >= context->size);

    return ((unsigned char*)context->data)[offset];
}

static inline uint16_t get_u16(struct atom_context *context, uint32_t offset)
{
    return get_u8(context, offset) | (((uint16_t)get_u8(context, offset + 1)) << 8);
}

static inline uint32_t get_u32(struct atom_context *context, uint32_t offset)
{
    return get_u16(context, offset) | (((uint32_t)get_u16(context, offset + 2)) << 16);
}

static inline char *get_str(struct atom_context *context, uint32_t offset, size_t size)
{
    if (WARN_ON(offset + size >= context->size))
        return NULL;

    return &(((char *)context->data)[offset]);
}

static inline void *get_image(struct atom_context *context, uint32_t offset, size_t size)
{
    if (WARN_ON(offset + size >= context->size))
        return NULL;

    if (offset == 0)
        return NULL;

    return (((uint8_t *)context->data) + offset);
}
#define GET_IMAGE(context, type, offset) \
    ((type *)get_image(context, offset, sizeof(type)))


enum object_id_bit{
    OBJECT_ID_MASK      = 0x00FF,
    ENUM_ID_MASK        = 0x0F00,
    OBJECT_TYPE_MASK    = 0xF000,
    OBJECT_ID_SHIFT     = 0x00,
    ENUM_ID_SHIFT       = 0x08,
    OBJECT_TYPE_SHIFT   = 0x0C
};

enum object_type {
    OBJECT_TYPE_UNKNOWN  = 0,

    /* Direct ATOM BIOS translation */
    OBJECT_TYPE_GPU,
    OBJECT_TYPE_ENCODER,
    OBJECT_TYPE_CONNECTOR,
    OBJECT_TYPE_ROUTER,
    OBJECT_TYPE_GENERIC,

    /* Driver specific */
    OBJECT_TYPE_AUDIO,
    OBJECT_TYPE_CONTROLLER,
    OBJECT_TYPE_CLOCK_SOURCE,
    OBJECT_TYPE_ENGINE,

    OBJECT_TYPE_COUNT
};
enum _object_types {
    GRAPH_OBJECT_TYPE_NONE                    = 0x0,
    GRAPH_OBJECT_TYPE_GPU                     = 0x1,
    GRAPH_OBJECT_TYPE_ENCODER                 = 0x2,
    GRAPH_OBJECT_TYPE_CONNECTOR               = 0x3,
    GRAPH_OBJECT_TYPE_ROUTER                  = 0x4,
    GRAPH_OBJECT_TYPE_DISPLAY_PATH            = 0x6  ,
    GRAPH_OBJECT_TYPE_GENERIC                 = 0x7,
};
static enum object_type object_type_from_bios_object_id(
    uint32_t bios_object_id
){
    uint32_t bios_object_type = (bios_object_id & OBJECT_TYPE_MASK) >> OBJECT_TYPE_SHIFT;

    switch (bios_object_type) {
        case GRAPH_OBJECT_TYPE_GPU:         return OBJECT_TYPE_GPU;
        case GRAPH_OBJECT_TYPE_ENCODER:     return OBJECT_TYPE_ENCODER;
        case GRAPH_OBJECT_TYPE_CONNECTOR:   return OBJECT_TYPE_CONNECTOR;
        case GRAPH_OBJECT_TYPE_ROUTER:      return OBJECT_TYPE_ROUTER;
        case GRAPH_OBJECT_TYPE_GENERIC:     return OBJECT_TYPE_GENERIC;
        default:   return OBJECT_TYPE_UNKNOWN;
    }
}

enum object_enum_id {
    ENUM_ID_UNKNOWN = 0,
    ENUM_ID_1,
    ENUM_ID_2,
    ENUM_ID_3,
    ENUM_ID_4,
    ENUM_ID_5,
    ENUM_ID_6,
    ENUM_ID_7,

    ENUM_ID_COUNT
};
enum _object_enum_ids {
    GRAPH_OBJECT_ENUM_ID1 = 0x01,
    GRAPH_OBJECT_ENUM_ID2 = 0x02,
    GRAPH_OBJECT_ENUM_ID3 = 0x03,
    GRAPH_OBJECT_ENUM_ID4 = 0x04,
    GRAPH_OBJECT_ENUM_ID5 = 0x05,
    GRAPH_OBJECT_ENUM_ID6 = 0x06,
    GRAPH_OBJECT_ENUM_ID7 = 0x07,
};
static enum object_enum_id enum_id_from_bios_object_id(
    uint32_t bios_object_id
){
    uint32_t bios_enum_id = (bios_object_id & ENUM_ID_MASK) >> ENUM_ID_SHIFT;

    switch (bios_enum_id) {
        case GRAPH_OBJECT_ENUM_ID1: return ENUM_ID_1;
        case GRAPH_OBJECT_ENUM_ID2: return ENUM_ID_2;
        case GRAPH_OBJECT_ENUM_ID3: return ENUM_ID_3;
        case GRAPH_OBJECT_ENUM_ID4: return ENUM_ID_4;
        case GRAPH_OBJECT_ENUM_ID5: return ENUM_ID_5;
        case GRAPH_OBJECT_ENUM_ID6: return ENUM_ID_6;
        case GRAPH_OBJECT_ENUM_ID7: return ENUM_ID_7;
        default:                    return ENUM_ID_UNKNOWN;
    }
}

static uint32_t gpu_id_from_bios_object_id(
    uint32_t bios_object_id
){
    return (bios_object_id & OBJECT_ID_MASK) >> OBJECT_ID_SHIFT;
}

enum encoder_id {
    ENCODER_ID_UNKNOWN = 0,

    /* Radeon Class Display Hardware */
    ENCODER_ID_INTERNAL_LVDS,
    ENCODER_ID_INTERNAL_TMDS1,
    ENCODER_ID_INTERNAL_TMDS2,
    ENCODER_ID_INTERNAL_DAC1,
    ENCODER_ID_INTERNAL_DAC2,    /* TV/CV DAC */

    /* External Third Party Encoders */
    ENCODER_ID_INTERNAL_LVTM1,    /* not used for Radeon */
    ENCODER_ID_INTERNAL_HDMI,

    /* Kaledisope (KLDSCP) Class Display Hardware */
    ENCODER_ID_INTERNAL_KLDSCP_TMDS1,
    ENCODER_ID_INTERNAL_KLDSCP_DAC1,
    ENCODER_ID_INTERNAL_KLDSCP_DAC2,    /* Shared with CV/TV and CRT */
    /* External TMDS (dual link) */
    ENCODER_ID_EXTERNAL_MVPU_FPGA,    /* MVPU FPGA chip */
    ENCODER_ID_INTERNAL_DDI,
    ENCODER_ID_INTERNAL_UNIPHY,
    ENCODER_ID_INTERNAL_KLDSCP_LVTMA,
    ENCODER_ID_INTERNAL_UNIPHY1,
    ENCODER_ID_INTERNAL_UNIPHY2,
    ENCODER_ID_EXTERNAL_NUTMEG,
    ENCODER_ID_EXTERNAL_TRAVIS,

    ENCODER_ID_INTERNAL_WIRELESS,    /* Internal wireless display encoder */
    ENCODER_ID_INTERNAL_UNIPHY3,
    ENCODER_ID_INTERNAL_VIRTUAL,
};
enum _encoder_ids {
    ENCODER_OBJECT_ID_NONE                    = 0x00,
    ENCODER_OBJECT_ID_INTERNAL_LVDS           = 0x01,
    ENCODER_OBJECT_ID_INTERNAL_TMDS1          = 0x02,
    ENCODER_OBJECT_ID_INTERNAL_TMDS2          = 0x03,
    ENCODER_OBJECT_ID_INTERNAL_DAC1           = 0x04,
    ENCODER_OBJECT_ID_INTERNAL_DAC2           = 0x05,
    ENCODER_OBJECT_ID_INTERNAL_LVTM1          = 0x0F,
    ENCODER_OBJECT_ID_HDMI_INTERNAL           = 0x12,
    ENCODER_OBJECT_ID_INTERNAL_KLDSCP_TMDS1   = 0x13,
    ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC1    = 0x15,
    ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC2    = 0x16,
    ENCODER_OBJECT_ID_MVPU_FPGA               = 0x18,
    ENCODER_OBJECT_ID_INTERNAL_DDI            = 0x19,
    ENCODER_OBJECT_ID_INTERNAL_UNIPHY         = 0x1E,
    ENCODER_OBJECT_ID_INTERNAL_KLDSCP_LVTMA   = 0x1F,
    ENCODER_OBJECT_ID_INTERNAL_UNIPHY1        = 0x20,
    ENCODER_OBJECT_ID_INTERNAL_UNIPHY2        = 0x21,
    ENCODER_OBJECT_ID_ALMOND                  = 0x22,
    ENCODER_OBJECT_ID_TRAVIS                  = 0x23,
    ENCODER_OBJECT_ID_INTERNAL_UNIPHY3        = 0x25,
};
static enum encoder_id encoder_id_from_bios_object_id(
    uint32_t bios_object_id
){
    uint32_t bios_encoder_id = gpu_id_from_bios_object_id(bios_object_id);

    switch (bios_encoder_id) {
    case ENCODER_OBJECT_ID_INTERNAL_LVDS:           return ENCODER_ID_INTERNAL_LVDS;
    case ENCODER_OBJECT_ID_INTERNAL_TMDS1:          return ENCODER_ID_INTERNAL_TMDS1;
    case ENCODER_OBJECT_ID_INTERNAL_TMDS2:          return ENCODER_ID_INTERNAL_TMDS2;
    case ENCODER_OBJECT_ID_INTERNAL_DAC1:           return ENCODER_ID_INTERNAL_DAC1;
    case ENCODER_OBJECT_ID_INTERNAL_DAC2:           return ENCODER_ID_INTERNAL_DAC2;
    case ENCODER_OBJECT_ID_INTERNAL_LVTM1:          return ENCODER_ID_INTERNAL_LVTM1;
    case ENCODER_OBJECT_ID_HDMI_INTERNAL:           return ENCODER_ID_INTERNAL_HDMI;
    case ENCODER_OBJECT_ID_INTERNAL_KLDSCP_TMDS1:   return ENCODER_ID_INTERNAL_KLDSCP_TMDS1;
    case ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC1:    return ENCODER_ID_INTERNAL_KLDSCP_DAC1;
    case ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC2:    return ENCODER_ID_INTERNAL_KLDSCP_DAC2;
    case ENCODER_OBJECT_ID_MVPU_FPGA:               return ENCODER_ID_EXTERNAL_MVPU_FPGA;
    case ENCODER_OBJECT_ID_INTERNAL_DDI:            return ENCODER_ID_INTERNAL_DDI;
    case ENCODER_OBJECT_ID_INTERNAL_UNIPHY:         return ENCODER_ID_INTERNAL_UNIPHY;
    case ENCODER_OBJECT_ID_INTERNAL_KLDSCP_LVTMA:   return ENCODER_ID_INTERNAL_KLDSCP_LVTMA;
    case ENCODER_OBJECT_ID_INTERNAL_UNIPHY1:        return ENCODER_ID_INTERNAL_UNIPHY1;
    case ENCODER_OBJECT_ID_INTERNAL_UNIPHY2:        return ENCODER_ID_INTERNAL_UNIPHY2;
    case ENCODER_OBJECT_ID_ALMOND:                  return ENCODER_ID_EXTERNAL_NUTMEG;
    case ENCODER_OBJECT_ID_TRAVIS:                  return ENCODER_ID_EXTERNAL_TRAVIS;
    case ENCODER_OBJECT_ID_INTERNAL_UNIPHY3:        return ENCODER_ID_INTERNAL_UNIPHY3;
    default:               return ENCODER_ID_UNKNOWN;
    }
}

enum connector_id {
    CONNECTOR_ID_UNKNOWN = 0,
    CONNECTOR_ID_SINGLE_LINK_DVII = 1,
    CONNECTOR_ID_DUAL_LINK_DVII = 2,
    CONNECTOR_ID_SINGLE_LINK_DVID = 3,
    CONNECTOR_ID_DUAL_LINK_DVID = 4,
    CONNECTOR_ID_VGA = 5,
    CONNECTOR_ID_HDMI_TYPE_A = 12,
    CONNECTOR_ID_LVDS = 14,
    CONNECTOR_ID_PCIE = 16,
    CONNECTOR_ID_HARDCODE_DVI = 18,
    CONNECTOR_ID_DISPLAY_PORT = 19,
    CONNECTOR_ID_EDP = 20,
    CONNECTOR_ID_MXM = 21,
    CONNECTOR_ID_WIRELESS = 22,
    CONNECTOR_ID_MIRACAST = 23,

    CONNECTOR_ID_VIRTUAL = 100
};
enum _connector_ids {
    CONNECTOR_OBJECT_ID_NONE                  = 0x00,
    CONNECTOR_OBJECT_ID_SINGLE_LINK_DVI_I     = 0x01,
    CONNECTOR_OBJECT_ID_DUAL_LINK_DVI_I       = 0x02,
    CONNECTOR_OBJECT_ID_SINGLE_LINK_DVI_D     = 0x03,
    CONNECTOR_OBJECT_ID_DUAL_LINK_DVI_D       = 0x04,
    CONNECTOR_OBJECT_ID_VGA                   = 0x05,
    CONNECTOR_OBJECT_ID_COMPOSITE             = 0x06,
    CONNECTOR_OBJECT_ID_SVIDEO                = 0x07,
    CONNECTOR_OBJECT_ID_YPbPr                 = 0x08,
    CONNECTOR_OBJECT_ID_D_CONNECTOR           = 0x09,
    CONNECTOR_OBJECT_ID_9PIN_DIN              = 0x0A,
    CONNECTOR_OBJECT_ID_SCART                 = 0x0B,
    CONNECTOR_OBJECT_ID_HDMI_TYPE_A           = 0x0C,
    CONNECTOR_OBJECT_ID_HDMI_TYPE_B           = 0x0D,
    CONNECTOR_OBJECT_ID_LVDS                  = 0x0E,
    CONNECTOR_OBJECT_ID_7PIN_DIN              = 0x0F,
    CONNECTOR_OBJECT_ID_PCIE_CONNECTOR        = 0x10,
    CONNECTOR_OBJECT_ID_CROSSFIRE             = 0x11,
    CONNECTOR_OBJECT_ID_HARDCODE_DVI          = 0x12,
    CONNECTOR_OBJECT_ID_DISPLAYPORT           = 0x13,
    CONNECTOR_OBJECT_ID_eDP                   = 0x14,
    CONNECTOR_OBJECT_ID_MXM                   = 0x15,
    CONNECTOR_OBJECT_ID_LVDS_eDP              = 0x16,
};
static enum connector_id connector_id_from_bios_object_id(
    uint32_t bios_object_id
){
    uint32_t bios_connector_id = gpu_id_from_bios_object_id(bios_object_id);

    switch (bios_connector_id) {
    case CONNECTOR_OBJECT_ID_SINGLE_LINK_DVI_I: return CONNECTOR_ID_SINGLE_LINK_DVII;
    case CONNECTOR_OBJECT_ID_DUAL_LINK_DVI_I:   return CONNECTOR_ID_DUAL_LINK_DVII;
    case CONNECTOR_OBJECT_ID_SINGLE_LINK_DVI_D: return CONNECTOR_ID_SINGLE_LINK_DVID;
    case CONNECTOR_OBJECT_ID_DUAL_LINK_DVI_D:   return CONNECTOR_ID_DUAL_LINK_DVID;
    case CONNECTOR_OBJECT_ID_VGA:               return CONNECTOR_ID_VGA;
    case CONNECTOR_OBJECT_ID_HDMI_TYPE_A:       return CONNECTOR_ID_HDMI_TYPE_A;
    case CONNECTOR_OBJECT_ID_LVDS:              return CONNECTOR_ID_LVDS;
    case CONNECTOR_OBJECT_ID_PCIE_CONNECTOR:    return CONNECTOR_ID_PCIE;
    case CONNECTOR_OBJECT_ID_HARDCODE_DVI:      return CONNECTOR_ID_HARDCODE_DVI;
    case CONNECTOR_OBJECT_ID_DISPLAYPORT:       return CONNECTOR_ID_DISPLAY_PORT;
    case CONNECTOR_OBJECT_ID_eDP:               return CONNECTOR_ID_EDP;
    case CONNECTOR_OBJECT_ID_MXM:               return CONNECTOR_ID_MXM;
    default:           return CONNECTOR_ID_UNKNOWN;
    }
}

enum generic_id {
    GENERIC_ID_UNKNOWN = 0,
    GENERIC_ID_MXM_OPM,
    GENERIC_ID_GLSYNC,
    GENERIC_ID_STEREO,

    GENERIC_ID_COUNT
};
enum _generic_ids {
    GENERIC_OBJECT_ID_NONE                    = 0x00,
    GENERIC_OBJECT_ID_GLSYNC                  = 0x01,
    GENERIC_OBJECT_ID_PX2_NON_DRIVABLE        = 0x02,
    GENERIC_OBJECT_ID_MXM_OPM                 = 0x03,
    GENERIC_OBJECT_ID_STEREO_PIN              = 0x04,
    GENERIC_OBJECT_ID_BRACKET_LAYOUT          = 0x05,
};
static enum generic_id generic_id_from_bios_object_id(
    uint32_t bios_object_id
){
    uint32_t bios_generic_id = gpu_id_from_bios_object_id(bios_object_id);

    switch (bios_generic_id) {
    case GENERIC_OBJECT_ID_MXM_OPM:     return GENERIC_ID_MXM_OPM;
    case GENERIC_OBJECT_ID_GLSYNC:      return GENERIC_ID_GLSYNC;
    case GENERIC_OBJECT_ID_STEREO_PIN:  return GENERIC_ID_STEREO;
    default:   return GENERIC_ID_UNKNOWN;
    }
}

static uint32_t id_from_bios_object_id(
    enum object_type type,
    uint32_t bios_object_id
){
    switch (type) {
        case OBJECT_TYPE_GPU:
            return gpu_id_from_bios_object_id(bios_object_id);
        case OBJECT_TYPE_ENCODER:
            return (uint32_t)encoder_id_from_bios_object_id(bios_object_id);
        case OBJECT_TYPE_CONNECTOR:
            return (uint32_t)connector_id_from_bios_object_id(
                    bios_object_id);
        case OBJECT_TYPE_GENERIC:
            return generic_id_from_bios_object_id(bios_object_id);
        default:
            return 0;
    }
}

static inline struct graphics_object_id dal_graphics_object_id_init(
    uint32_t id,
    enum object_enum_id enum_id,
    enum object_type type
){
    struct graphics_object_id result = {
        id, enum_id, type, 0
    };

    return result;
}

static bool object_id_from_bios_object_id(
    uint32_t bios_object_id,
    struct graphics_object_id *object_id
){
    enum object_type type;
    enum object_enum_id enum_id;

    type = object_type_from_bios_object_id(bios_object_id);

    if (OBJECT_TYPE_UNKNOWN == type)
        return false;

    enum_id = enum_id_from_bios_object_id(bios_object_id);

    if (ENUM_ID_UNKNOWN == enum_id)
        return false;

    *object_id = dal_graphics_object_id_init(id_from_bios_object_id(type, bios_object_id), enum_id, type);

    return true;
}

static struct atom_object *get_bios_object_v1_1(
    struct atom_context *context,
    struct graphics_object_id id
){
    struct atom_object_table *table;
    struct graphics_object_id obj_id;
    uint32_t offset, i;

    offset = table_list_field(context, v1_1, object_header);

    switch (id.type) {
    case OBJECT_TYPE_ENCODER:
        offset += object_info_field(context, v1_1, usEncoderObjectTableOffset);
        break;

    case OBJECT_TYPE_CONNECTOR:
        offset += object_info_field(context, v1_1, usConnectorObjectTableOffset);
        break;

    case OBJECT_TYPE_ROUTER:
        offset += object_info_field(context, v1_1, usRouterObjectTableOffset);
        break;

    case OBJECT_TYPE_GENERIC:
        if (context->object_info_table.revision.minor < 3)
            return NULL;
        offset += object_info_field(context, v1_3, usMiscObjectTableOffset);
        break;

    default:
        return NULL;
    }

    table = GET_IMAGE(context, struct atom_object_table, offset);
    if (!table)
        return NULL;

    for (i = 0; i < table->ucNumberOfObjects; i++) {
        if (!object_id_from_bios_object_id(le16_to_cpu(table->asObjects[i].usObjectID), &obj_id))
            continue;

        if (id.id == obj_id.id && id.enum_id == obj_id.enum_id && id.type == obj_id.type)
            return &table->asObjects[i];
    }

    return NULL;
}

static struct atom_display_object_path_v2 *get_bios_object_v2_1(
    struct atom_context *context,
    struct graphics_object_id id
){
    struct display_object_info_table_v1_4 *table;
    struct graphics_object_id obj_id = {0};
    unsigned int i;

    table = context->object_info_table.v1_4;

    switch (id.type) {
    case OBJECT_TYPE_ENCODER:
        for (i = 0; i < table->number_of_path; i++) {
            if (!object_id_from_bios_object_id(table->display_path[i].encoderobjid, &obj_id))
                continue;

            if (id.type == obj_id.type && id.id == obj_id.id && id.enum_id == obj_id.enum_id)
                return &table->display_path[i];
        }
        /* fall through */
    case OBJECT_TYPE_CONNECTOR:
    case OBJECT_TYPE_GENERIC:
        /* Both Generic and Connector Object ID
         * will be stored on display_objid
         */
        for (i = 0; i < table->number_of_path; i++) {
            if (!object_id_from_bios_object_id(table->display_path[i].display_objid, &obj_id))
                continue;

            if (id.type == obj_id.type && id.id == obj_id.id && id.enum_id == obj_id.enum_id)
                return &table->display_path[i];
        }
        /* fall through */
    default:
        return NULL;
    }
}

#define equal_bits(v1, v2, bit) \
    (((v1) & bit) == ((v2) & bit))

static enum bp_result get_gpio_i2c_info_v1_1(
    struct atom_context *context,
    struct atom_i2c_record *record,
    struct i2c_info *info
){
    struct atom_gpio_i2c_info *header;
    struct atom_gpio_i2c_assigment *pin;
    uint32_t table_count;
    uint32_t i2c_table_offset;

    if (WARN_ON(info == NULL))
        return BP_RESULT_BADINPUT;

    i2c_table_offset = table_list_field(context, v1_1, gpio_i2c_info);
    header = GET_IMAGE(context, struct atom_gpio_i2c_info, i2c_table_offset);
    if (!header)
        return BP_RESULT_BADBIOSTABLE;

    if (sizeof(struct atom_common_table_header) +
        sizeof(struct atom_gpio_i2c_assigment) >
        le16_to_cpu(header->table_header.structuresize)
    ){
        return BP_RESULT_BADBIOSTABLE;
    }

    if (1 != header->table_header.content_revision)
        return BP_RESULT_UNSUPPORTED;

    /* get data count */
    table_count = (le16_to_cpu(header->table_header.structuresize) -
                   sizeof(struct atom_common_table_header)) /
                   sizeof(struct atom_gpio_i2c_assigment);

    if (table_count < (record->i2c_id & I2C_HW_LANE_MUX))
        return BP_RESULT_BADBIOSTABLE;

    // Lines 4, 6 and 7 are not being used
    // for (i = 0; i < table_count; i++) {
    //     pin = &header->asGPIO_Info[i];
    //
    //     AURA_DBG("line: %d, hw: %s, eng: %d",
    //         header->asGPIO_Info[i].sucI2cId.sbfAccess.bfI2C_LineMux,
    //         header->asGPIO_Info[i].sucI2cId.sbfAccess.bfHW_Capable ? "true" : "false",
    //         header->asGPIO_Info[i].sucI2cId.sbfAccess.bfHW_EngineID
    //     );
    //     AURA_DBG("    clk_mask %x, clk_en %x, clk_y %x, clk_a %x, data_mask %x, data_en %x, data_y %x, data_a %x",
    //         le16_to_cpu(pin->usClkMaskRegisterIndex),
    //         le16_to_cpu(pin->usClkEnRegisterIndex),
    //         le16_to_cpu(pin->usClkY_RegisterIndex),
    //         le16_to_cpu(pin->usClkA_RegisterIndex),
    //         le16_to_cpu(pin->usDataMaskRegisterIndex),
    //         le16_to_cpu(pin->usDataEnRegisterIndex),
    //         le16_to_cpu(pin->usDataY_RegisterIndex),
    //         le16_to_cpu(pin->usDataA_RegisterIndex)
    //     );
    // }

    /* get the GPIO_I2C_INFO */
    info->hw_assist     = (record->i2c_id & I2C_HW_CAP) ? true : false;
    info->line          = record->i2c_id & I2C_HW_LANE_MUX;
    info->engine_id     = (record->i2c_id & I2C_HW_ENGINE_ID_MASK) >> 4;
    info->slave_address = record->i2c_slave_addr;

    pin = &header->asGPIO_Info[info->line];

    info->gpio_info.clk_mask_register_index  = le16_to_cpu(pin->usClkMaskRegisterIndex);
    info->gpio_info.clk_en_register_index    = le16_to_cpu(pin->usClkEnRegisterIndex);
    info->gpio_info.clk_y_register_index     = le16_to_cpu(pin->usClkY_RegisterIndex);
    info->gpio_info.clk_a_register_index     = le16_to_cpu(pin->usClkA_RegisterIndex);
    info->gpio_info.data_mask_register_index = le16_to_cpu(pin->usDataMaskRegisterIndex);
    info->gpio_info.data_en_register_index   = le16_to_cpu(pin->usDataEnRegisterIndex);
    info->gpio_info.data_y_register_index    = le16_to_cpu(pin->usDataY_RegisterIndex);
    info->gpio_info.data_a_register_index    = le16_to_cpu(pin->usDataA_RegisterIndex);

    info->gpio_info.clk_mask_shift  = pin->ucClkMaskShift;
    info->gpio_info.clk_en_shift    = pin->ucClkEnShift;
    info->gpio_info.clk_y_shift     = pin->ucClkY_Shift;
    info->gpio_info.clk_a_shift     = pin->ucClkA_Shift;
    info->gpio_info.data_mask_shift = pin->ucDataMaskShift;
    info->gpio_info.data_en_shift   = pin->ucDataEnShift;
    info->gpio_info.data_y_shift    = pin->ucDataY_Shift;
    info->gpio_info.data_a_shift    = pin->ucDataA_Shift;

    info->gpio_info.clk_mask_mask   = (1 << pin->ucClkMaskShift);
    info->gpio_info.clk_en_mask     = (1 << pin->ucClkEnShift);
    info->gpio_info.clk_y_mask      = (1 << pin->ucClkY_Shift);
    info->gpio_info.clk_a_mask      = (1 << pin->ucClkA_Shift);
    info->gpio_info.data_mask_mask  = (1 << pin->ucDataMaskShift);
    info->gpio_info.data_en_mask    = (1 << pin->ucDataEnShift);
    info->gpio_info.data_y_mask     = (1 << pin->ucDataY_Shift);
    info->gpio_info.data_a_mask     = (1 << pin->ucDataA_Shift);

    return BP_RESULT_OK;
}

static enum bp_result get_gpio_i2c_info_v2_1(
    struct atom_context *context,
    struct atom_i2c_record *record,
    struct i2c_info *info
){
    struct atom_gpio_pin_lut_v2_1 *header;
    struct atom_gpio_pin_assignment *pin;
    uint32_t table_count, table_index;
    uint16_t gpio_pin_lut_offset;
    // unsigned int table_index = 0;
    bool table_found;

    if (WARN_ON(info == NULL))
        return BP_RESULT_BADINPUT;

    /* get the GPIO_I2C info */
    gpio_pin_lut_offset = table_list_field(context, v2_1, gpio_pin_lut);
    header = GET_IMAGE(context, struct atom_gpio_pin_lut_v2_1, gpio_pin_lut_offset);
    if (!header)
        return BP_RESULT_BADBIOSTABLE;

    if (sizeof(struct atom_common_table_header) +
        sizeof(struct atom_gpio_pin_assignment) >
        le16_to_cpu(header->table_header.structuresize)
    ){
        return BP_RESULT_BADBIOSTABLE;
    }

    /* TODO: is version change? */
    if (header->table_header.content_revision != 1)
        return BP_RESULT_UNSUPPORTED;

    /* get data count */
    table_count = (le16_to_cpu(header->table_header.structuresize) -
                   sizeof(struct atom_common_table_header)) /
                   sizeof(struct atom_gpio_pin_assignment);

    table_found = false;
    for (table_index = 0; table_index < table_count; table_index++) {
        pin = &header->gpio_pin[table_index];

        if (!equal_bits(record->i2c_id, pin->gpio_id, I2C_HW_CAP))
            continue;

        if (!equal_bits(record->i2c_id, pin->gpio_id, I2C_HW_ENGINE_ID_MASK))
            continue;

        if (!equal_bits(record->i2c_id, pin->gpio_id, I2C_HW_LANE_MUX))
            continue;

        table_found = true;
        break;
    }

    if (!table_found)
        return BP_RESULT_BADBIOSTABLE;

    /* get the GPIO_I2C_INFO */
    info->hw_assist     = (record->i2c_id & I2C_HW_CAP) ? true : false;
    info->line          = record->i2c_id & I2C_HW_LANE_MUX;
    info->engine_id     = (record->i2c_id & I2C_HW_ENGINE_ID_MASK) >> 4;
    info->slave_address = record->i2c_slave_addr;

    /* TODO: check how to get register offset for en, Y, etc. */
    info->gpio_info.clk_a_register_index = le16_to_cpu(pin->data_a_reg_index);
    info->gpio_info.clk_a_shift          = pin->gpio_bitshift;

    return BP_RESULT_OK;
}

static void _aura_gpu_bios_read_name(
    struct atom_context *context
){
    int name_offset = get_u16(context, ATOM_ROM_PART_NUMBER_PTR);
    const char *name_str;

    if (name_offset == 0)
        name_offset = 0x80;

    name_str = get_str(context, name_offset, sizeof(context->bios.name) - 1);
    if (*name_str != '\0')
        strlcpy(context->bios.name, name_str, sizeof(context->bios.name));
}

static bool _aura_gpu_bios_is_atom(
    struct atom_context *context
){
    uint16_t base;
    const char *magic;

    if (get_u16(context, 0) != ATOM_BIOS_MAGIC)
        return false;

    magic = get_str(context, ATOM_ATI_MAGIC_PTR, sizeof(ATOM_ATI_MAGIC) - 1);
    if (!magic || strncmp(magic, ATOM_ATI_MAGIC, sizeof(ATOM_ATI_MAGIC) - 1))
        return false;

    base = get_u16(context, ATOM_ROM_HEADER_PTR);
    magic = get_str(context, (base + ATOM_ROM_MAGIC_PTR), sizeof(ATOM_ROM_MAGIC) - 1);

    if (!magic || strncmp(magic, ATOM_ROM_MAGIC, sizeof(ATOM_ROM_MAGIC) - 1))
        return false;

    return true;
}

static void _aura_gpu_bios_data_revision_init(
    struct atom_data_revision *tbl_revision,
    struct atom_common_table_header *atom_data_tbl
){
    tbl_revision->major = (uint32_t)atom_data_tbl->format_revision & 0x3f;
    tbl_revision->minor = (uint32_t)atom_data_tbl->content_revision & 0x3f;
}

static bool _aura_gpu_bios_init(
    struct atom_context *context
){
    // Using the latest rom header since the info we need
    // is shared amongst versions.
    struct atom_rom_header_v2_2 *rom_header;
    struct atom_data_revision tbl_rev = {0};
    uint32_t object_info_table_offset;
    uint32_t rom_header_offset;

    rom_header_offset = get_u16(context, ATOM_ROM_HEADER_PTR);
    rom_header = GET_IMAGE(context, struct atom_rom_header_v2_2, rom_header_offset);
    if (!rom_header)
        return false;

    _aura_gpu_bios_data_revision_init(&tbl_rev, &rom_header->table_header);
    context->master_data_table.revision = tbl_rev;

    // Navi BIOS
    if (tbl_rev.major >= 2 && tbl_rev.minor >= 2) {
        struct display_object_info_table_v1_4 *object_info_table;

        context->master_data_table.v2_1 = GET_IMAGE(context, struct atom_master_data_table_v2_1, rom_header->masterdatatable_offset);
        if (!context->master_data_table.v2_1)
            return false;

        object_info_table_offset = table_list_field(context, v2_1, displayobjectinfo);
        // object_info_table_offset = MasterDataTableOffset(context, displayobjectinfo);
        object_info_table = GET_IMAGE(context, struct display_object_info_table_v1_4, object_info_table_offset);
        if (!object_info_table)
            return false;

        _aura_gpu_bios_data_revision_init(&tbl_rev, &object_info_table->table_header);
        if (tbl_rev.major == 1 && tbl_rev.minor >= 4) {
            context->object_info_table.revision = tbl_rev;
            context->object_info_table.v1_4 = object_info_table;
        } else {
            AURA_DBG("Unsupported object_info_table version %d.%d", tbl_rev.major, tbl_rev.minor);
            return false;
        }
    }

    // Polaris/Vega BIOS
    else {
        struct display_object_info_table_v1_1 *object_info_table;

        context->master_data_table.v1_1 = GET_IMAGE(context, struct atom_master_data_table_v1_1, rom_header->masterdatatable_offset);
        if (!context->master_data_table.v1_1)
            return false;

        object_info_table_offset = table_list_field(context, v1_1, object_header);
        // object_info_table_offset = MasterDataTableOffset(context, Object_Header);
        object_info_table = GET_IMAGE(context, struct display_object_info_table_v1_1, object_info_table_offset);
        if (!object_info_table)
            return false;

        _aura_gpu_bios_data_revision_init(&tbl_rev, &object_info_table->table_header);
        context->object_info_table.revision = tbl_rev;

        if (tbl_rev.major == 1 && tbl_rev.minor >= 3) {
            struct display_object_info_table_v1_3 *tbl_v3;

            tbl_v3 = GET_IMAGE(context, struct display_object_info_table_v1_3, object_info_table_offset);
            if (!tbl_v3)
                return false;

            context->object_info_table.v1_3 = tbl_v3;
        } else if (tbl_rev.major == 1 && tbl_rev.minor >= 1) {
            context->object_info_table.v1_1 = object_info_table;
        } else {
            AURA_DBG("Unsupported object_info_table version %d.%d", tbl_rev.major, tbl_rev.minor);
            return false;
        }
    }

    return true;
}


static uint8_t atom_get_connectors_number_v1_1(
    struct atom_context *context
){
    struct atom_object_table *table;
    uint32_t object_info_table_offset;
    uint32_t connector_table_offset;

    object_info_table_offset = table_list_field(context, v1_1, object_header);
    connector_table_offset = object_info_field(context, v1_1, usConnectorObjectTableOffset);

    table = GET_IMAGE(context, struct atom_object_table, object_info_table_offset + connector_table_offset);
    if (!table)
        return 0;

    return table->ucNumberOfObjects;
}

static uint8_t atom_get_connectors_number_v2_1(
    struct atom_context *context
){
    struct display_object_info_table_v1_4 *table;
    uint8_t count, i;

    table = context->object_info_table.v1_4;
    for (i = 0, count = 0; i < table->number_of_path; i++) {
        // Is this our missing bus?
        if (table->display_path[i].encoderobjid != 0)
            count++;
    }

    AURA_DBG("Counted %d connectors", count);

    return count;
}


static bool atom_bios_get_connector_id_v1_1(
    struct atom_context *context,
    uint8_t index,
    struct graphics_object_id *object_id
){
    struct atom_object_table *table;
    uint32_t object_info_table_offset;
    uint32_t connector_table_offset;
    uint16_t id;

    object_info_table_offset = table_list_field(context, v1_1, object_header);
    connector_table_offset = object_info_field(context, v1_1, usConnectorObjectTableOffset);

    table = GET_IMAGE(context, struct atom_object_table, object_info_table_offset + connector_table_offset);
    if (!table) {
        AURA_ERR("Can't get connector table from atom bios.");
        return false;
    }

    if (table->ucNumberOfObjects <= index) {
        AURA_ERR("Can't find connector id %d in connector table of size %d.", index, table->ucNumberOfObjects);
        return false;
    }

    id = le16_to_cpu(table->asObjects[index].usObjectID);

    if (object_id_from_bios_object_id(id, object_id))
        return true;

    return false;
}

static bool atom_bios_get_connector_id_v2_1(
    struct atom_context *context,
    uint8_t index,
    struct graphics_object_id *object_id
){
    struct display_object_info_table_v1_4 *table;
    struct atom_display_object_path_v2 *object;

    table = context->object_info_table.v1_4;
    if (table->number_of_path > index) {
        object = &table->display_path[index];
        /* If display_objid is generic object id,  the encoderObj
         * /extencoderobjId should be 0
         */
        if (object->encoderobjid != 0 && object->display_objid != 0)
            return object_id_from_bios_object_id(object->display_objid, object_id);
    }

    return false;
}


static error_t atom_bios_get_i2c_info_v1_1(
    struct atom_context *context,
    struct graphics_object_id *object_id,
    struct i2c_info *info
){
    uint32_t offset;
    struct atom_object *object;
    struct atom_common_record_header *header;
    struct atom_i2c_record *record;

    object = get_bios_object_v1_1(context, *object_id);
    if (!object)
        return BP_RESULT_BADINPUT;

    offset = table_list_field(context, v1_1, object_header);
    offset += le16_to_cpu(object->usRecordOffset);

    for (;;) {
        header = GET_IMAGE(context, struct atom_common_record_header, offset);
        if (!header)
            return BP_RESULT_BADBIOSTABLE;

        if (ATOM_RECORD_END_TYPE == header->record_type || !header->record_size)
            break;

        if (ATOM_I2C_RECORD_TYPE == header->record_type && sizeof(struct atom_i2c_record) <= header->record_size) {
            /* get the I2C info */
            record = (struct atom_i2c_record *)header;

            if (get_gpio_i2c_info_v1_1(context, record, info) == BP_RESULT_OK)
                return BP_RESULT_OK;
        }

        offset += header->record_size;
    }

    return BP_RESULT_NORECORD;
}

static error_t atom_bios_get_i2c_info_v2_1(
    struct atom_context *context,
    struct graphics_object_id *object_id,
    struct i2c_info *info
){
    struct atom_display_object_path_v2 *object;
    struct atom_common_record_header *header;
    struct atom_i2c_record *record;
    uint16_t object_info_table_offset;
    uint32_t offset;

    object = get_bios_object_v2_1(context, *object_id);
    if (!object)
        return BP_RESULT_BADINPUT;

    object_info_table_offset = table_list_field(context, v2_1, displayobjectinfo);
    offset = object->disp_recordoffset + object_info_table_offset;

    for (;;) {
        header = GET_IMAGE(context, struct atom_common_record_header, offset);
        if (!header)
            return BP_RESULT_BADBIOSTABLE;

        if (header->record_type == ATOM_RECORD_END_TYPE || !header->record_size)
            break;

        if (header->record_type == ATOM_I2C_RECORD_TYPE && sizeof(struct atom_i2c_record) <= header->record_size) {
            /* get the I2C info */
            record = (struct atom_i2c_record *)header;

            if (get_gpio_i2c_info_v2_1(context, record, info) == BP_RESULT_OK)
                return BP_RESULT_OK;
        }

        offset += header->record_size;
    }

    return BP_RESULT_NORECORD;
}


uint8_t atom_bios_get_connectors_number(
   struct atom_bios *bios
){
    struct atom_context *context = to_atom_context(bios);

    if (WARN_ON(bios == NULL))
        return 0;

    switch (table_revision(context->master_data_table)) {
        case VERSION_1_1:
        case VERSION_1_2:
        case VERSION_1_3:
        case VERSION_1_4:
            return atom_get_connectors_number_v1_1(context);
        case VERSION_2_1:
        case VERSION_2_2:
            return atom_get_connectors_number_v2_1(context);
        default:
            AURA_ERR("Unexpected master_data_table version, %x", table_revision(context->master_data_table));
            return 0;
    }
}

bool atom_bios_get_connector_id(
   struct atom_bios *bios,
   uint8_t index,
   struct graphics_object_id *object_id
){
    struct atom_context *context = to_atom_context(bios);

    if (WARN_ON(bios == NULL))
        return false;

    if (WARN_ON(object_id == NULL))
        return false;

    switch (table_revision(context->master_data_table)) {
        case VERSION_1_1:
        case VERSION_1_2:
        case VERSION_1_3:
        case VERSION_1_4:
            return atom_bios_get_connector_id_v1_1(context, index, object_id);
        case VERSION_2_1:
        case VERSION_2_2:
            return atom_bios_get_connector_id_v2_1(context, index, object_id);
        default:
            *object_id = dal_graphics_object_id_init(
                0, ENUM_ID_UNKNOWN, OBJECT_TYPE_UNKNOWN);
            AURA_ERR("Unexpected master_data_table version");
            return false;
    }
}

error_t atom_bios_get_i2c_info(
    struct atom_bios *bios,
    struct graphics_object_id *object_id,
    struct i2c_info *info
){
    struct atom_context *context = to_atom_context(bios);

    if (WARN_ON(bios == NULL))
        return -EINVAL;

    if (WARN_ON(object_id == NULL))
        return -EINVAL;

    if (WARN_ON(info == NULL))
        return -EINVAL;

    switch (table_revision(context->master_data_table)) {
        case VERSION_1_1:
        case VERSION_1_2:
        case VERSION_1_3:
        case VERSION_1_4:
            return atom_bios_get_i2c_info_v1_1(context, object_id, info);
        case VERSION_2_1:
        case VERSION_2_2:
            return atom_bios_get_i2c_info_v2_1(context, object_id, info);
        default:
            AURA_ERR("Unexpected master_data_table version");
            return false;
    }
}

struct atom_bios* atom_bios_create(
    struct pci_dev *pci_dev
){
    struct atom_context *context;
    struct atom_bios *bios;
    uint8_t __iomem *buffer;
    size_t size;
    error_t err;

    buffer = pci_map_rom(pci_dev, &size);
    if (!buffer) {
        AURA_DBG("pci_map_rom() failed");
        return ERR_PTR(-EIO);
    }

    context = kzalloc(sizeof(*context) + size, GFP_KERNEL);
    if (context == NULL) {
        pci_unmap_rom(pci_dev, buffer);
        return ERR_PTR(-ENOMEM);
    }

    bios = &context->bios;
    memcpy_fromio(context->data, buffer, size);
    pci_unmap_rom(pci_dev, buffer);

    // temporarily set size to full memory region
    context->size = size;
    if (!_aura_gpu_bios_is_atom(context)) {
        AURA_DBG("Not an ATOM bios");
        err = -EINVAL;
        goto error;
    }

    // set size according to bios header
    size = get_u8(context, ATOM_IMAGE_SIZE_PTR) * ATOM_IMAGE_SIZE_UNIT;
    if (size > context->size) {
        AURA_DBG("Bios size (%ld) is greater than memory size (%ld)", size, context->size);
        err = -EINVAL;
        goto error;
    }
    context->size = size;

    _aura_gpu_bios_read_name(context);

    AURA_DBG("Detected ATOM bios: %s", context->bios.name);

    if (!_aura_gpu_bios_init(context)) {
        err = -EINVAL;
        goto error;
    }

    bios->size = context->size;
    bios->data = context->data;

    return bios;

error:
    kfree(context);

    return ERR_PTR(err);
}

void atom_bios_release(
    struct atom_bios* bios
){
    struct atom_context *context = to_atom_context(bios);

    if (WARN_ON(bios == NULL))
        return;

    kfree(context);
}

error_t atom_bios_get_gpio_info(
    struct atom_bios *bios,
    uint8_t index,
    struct i2c_info *info
){
    struct atom_context *context = to_atom_context(bios);
    struct atom_gpio_i2c_info *header;
    struct atom_gpio_i2c_assigment *pin;
    uint32_t table_count;
    uint32_t i2c_table_offset;

    if (WARN_ON(info == NULL))
        return BP_RESULT_BADINPUT;

    i2c_table_offset = table_list_field(context, v1_1, gpio_i2c_info);
    header = GET_IMAGE(context, struct atom_gpio_i2c_info, i2c_table_offset);
    if (!header)
        return BP_RESULT_BADBIOSTABLE;

    if (sizeof(struct atom_common_table_header) +
        sizeof(struct atom_gpio_i2c_assigment) >
        le16_to_cpu(header->table_header.structuresize)
    ){
        return BP_RESULT_BADBIOSTABLE;
    }

    if (1 != header->table_header.content_revision)
        return BP_RESULT_UNSUPPORTED;

    /* get data count */
    table_count = (le16_to_cpu(header->table_header.structuresize) -
                   sizeof(struct atom_common_table_header)) /
                   sizeof(struct atom_gpio_i2c_assigment);

    if (table_count < index)
        return BP_RESULT_BADBIOSTABLE;

    pin = &header->asGPIO_Info[index];

    /* get the GPIO_I2C_INFO */
    info->hw_assist     = pin->sucI2cId.sbfAccess.bfHW_Capable ? true : false;
    info->line          = pin->sucI2cId.sbfAccess.bfI2C_LineMux;
    info->engine_id     = pin->sucI2cId.sbfAccess.bfHW_EngineID;

    // info->i2c_slave_address = record->i2c_slave_addr;

    info->gpio_info.clk_mask_register_index  = le16_to_cpu(pin->usClkMaskRegisterIndex);
    info->gpio_info.clk_en_register_index    = le16_to_cpu(pin->usClkEnRegisterIndex);
    info->gpio_info.clk_y_register_index     = le16_to_cpu(pin->usClkY_RegisterIndex);
    info->gpio_info.clk_a_register_index     = le16_to_cpu(pin->usClkA_RegisterIndex);
    info->gpio_info.data_mask_register_index = le16_to_cpu(pin->usDataMaskRegisterIndex);
    info->gpio_info.data_en_register_index   = le16_to_cpu(pin->usDataEnRegisterIndex);
    info->gpio_info.data_y_register_index    = le16_to_cpu(pin->usDataY_RegisterIndex);
    info->gpio_info.data_a_register_index    = le16_to_cpu(pin->usDataA_RegisterIndex);

    info->gpio_info.clk_mask_shift  = pin->ucClkMaskShift;
    info->gpio_info.clk_en_shift    = pin->ucClkEnShift;
    info->gpio_info.clk_y_shift     = pin->ucClkY_Shift;
    info->gpio_info.clk_a_shift     = pin->ucClkA_Shift;
    info->gpio_info.data_mask_shift = pin->ucDataMaskShift;
    info->gpio_info.data_en_shift   = pin->ucDataEnShift;
    info->gpio_info.data_y_shift    = pin->ucDataY_Shift;
    info->gpio_info.data_a_shift    = pin->ucDataA_Shift;

    info->gpio_info.clk_mask_mask   = (1 << pin->ucClkMaskShift);
    info->gpio_info.clk_en_mask     = (1 << pin->ucClkEnShift);
    info->gpio_info.clk_y_mask      = (1 << pin->ucClkY_Shift);
    info->gpio_info.clk_a_mask      = (1 << pin->ucClkA_Shift);
    info->gpio_info.data_mask_mask  = (1 << pin->ucDataMaskShift);
    info->gpio_info.data_en_mask    = (1 << pin->ucDataEnShift);
    info->gpio_info.data_y_mask     = (1 << pin->ucDataY_Shift);
    info->gpio_info.data_a_mask     = (1 << pin->ucDataA_Shift);


    // Lines 4, 6 and 7 are not being used
    // for (i = 0; i < table_count; i++) {
    //     pin = &header->asGPIO_Info[i];
    //
    //     AURA_DBG("line: %d, hw: %s, eng: %d",
    //         header->asGPIO_Info[i].sucI2cId.sbfAccess.bfI2C_LineMux,
    //         header->asGPIO_Info[i].sucI2cId.sbfAccess.bfHW_Capable ? "true" : "false",
    //         header->asGPIO_Info[i].sucI2cId.sbfAccess.bfHW_EngineID
    //     );
    //     AURA_DBG("    clk_mask %x, clk_en %x, clk_y %x, clk_a %x, data_mask %x, data_en %x, data_y %x, data_a %x",
    //         le16_to_cpu(pin->usClkMaskRegisterIndex),
    //         le16_to_cpu(pin->usClkEnRegisterIndex),
    //         le16_to_cpu(pin->usClkY_RegisterIndex),
    //         le16_to_cpu(pin->usClkA_RegisterIndex),
    //         le16_to_cpu(pin->usDataMaskRegisterIndex),
    //         le16_to_cpu(pin->usDataEnRegisterIndex),
    //         le16_to_cpu(pin->usDataY_RegisterIndex),
    //         le16_to_cpu(pin->usDataA_RegisterIndex)
    //     );
    // }

    return BP_RESULT_OK;
}




static error_t get_crystal_frequency_v1_4 (
    struct atom_context *context,
    uint32_t *frequency
){
    uint32_t offset = table_list_field(context, v1_1, firmwareinfo);
    struct atom_firmware_info_v1_4 *firmware_info = GET_IMAGE(context, struct atom_firmware_info_v1_4, offset);

    if (!firmware_info)
        return BP_RESULT_BADBIOSTABLE;

    /* Pixel clock pll information. We need to convert from 10KHz units into
     * KHz units */
    *frequency = le16_to_cpu(firmware_info->usReferenceClock) * 10;

    return 0;
}

static error_t get_crystal_frequency_v2_1 (
    struct atom_context *context,
    uint32_t *frequency
){
    uint32_t offset = table_list_field(context, v1_1, firmwareinfo);
    struct atom_firmware_info_v2_1 *firmware_info = GET_IMAGE(context, struct atom_firmware_info_v2_1, offset);

    if (!firmware_info)
        return BP_RESULT_BADBIOSTABLE;

    /* Pixel clock pll information. We need to convert from 10KHz units into
     * KHz units */
    *frequency = le16_to_cpu(firmware_info->usCoreReferenceClock) * 10;

    return 0;
}

static error_t get_crystal_frequency_v2_2 (
    struct atom_context *context,
    uint32_t *frequency
){
    uint32_t offset = table_list_field(context, v1_1, firmwareinfo);
    struct atom_firmware_info_v2_2 *firmware_info = GET_IMAGE(context, struct atom_firmware_info_v2_2, offset);

    if (!firmware_info)
        return BP_RESULT_BADBIOSTABLE;

    /* Pixel clock pll information. We need to convert from 10KHz units into
     * KHz units */
    *frequency = le16_to_cpu(firmware_info->usCoreReferenceClock) * 10;

    return 0;
}

static error_t get_crystal_frequency_v3_1 (
    struct atom_context *context,
    uint32_t *frequency
){
    uint32_t offset = table_list_field(context, v2_1, dce_info);
    struct atom_display_controller_info_v4_1 *dce_info = NULL;

    dce_info = GET_IMAGE(context, struct atom_display_controller_info_v4_1, offset);

    if (!dce_info)
        return BP_RESULT_BADBIOSTABLE;

     /* 27MHz for Vega10 & Vega12; 100MHz for Vega20 */
    *frequency = dce_info->dce_refclk_10khz * 10;

    return 0;
}

error_t atom_bios_get_crystal_frequency (
    struct atom_bios* bios,
    uint32_t *frequency
){
    struct atom_context *context = to_atom_context(bios);
    struct atom_common_table_header *header;
    struct atom_data_revision firmware_rev = {0};
    uint32_t offset;

    if (WARN_ON(bios == NULL || frequency == NULL))
        return -EINVAL;

    switch (table_revision(context->master_data_table)) {
        case VERSION_1_1:
        case VERSION_1_2:
        case VERSION_1_3:
        case VERSION_1_4:
            offset = table_list_field(context, v1_1, firmwareinfo);
            break;
        case VERSION_2_1:
        case VERSION_2_2:
            offset = table_list_field(context, v2_1, firmwareinfo);
            break;
        default:
            AURA_ERR("Unexpected master_data_table version");
            return BP_RESULT_BADBIOSTABLE;
    }

    header = GET_IMAGE(context, struct atom_common_table_header, offset);
    if (!header)
        return BP_RESULT_BADBIOSTABLE;

    _aura_gpu_bios_data_revision_init(&firmware_rev, header);

    switch (revision(firmware_rev)) {
        case VERSION_1_1:
        case VERSION_1_2:
        case VERSION_1_3:
        case VERSION_1_4:
            return get_crystal_frequency_v1_4(context, frequency);
        case VERSION_2_1:
            return get_crystal_frequency_v2_1(context, frequency);
        case VERSION_2_2:
            return get_crystal_frequency_v2_2(context, frequency);
        case VERSION_3_1:
        case VERSION_3_2:
        case VERSION_3_3:
            return get_crystal_frequency_v3_1(context, frequency);
        default:
            AURA_ERR("Unexpected master_data_table version");
            return BP_RESULT_BADBIOSTABLE;
    }
}
