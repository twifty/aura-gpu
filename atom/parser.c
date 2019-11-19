#include "atom.h"
#include "atombios.h"

#define GET_IMAGE(type, offset) ((type*)bios_get_image(bp->bios, offset, sizeof(type)))
#define DATA_TABLES(table) (bp->master_data_tbl->ListOfDataTables.table)


uint8_t *bios_get_image(struct atom_bios *bios, uint32_t offset, uint32_t size)
{
    if (offset + size < bios->size)
        return bios->data + offset;
    else
        return NULL;
}

struct atom_data_revision {
    uint32_t major;
    uint32_t minor;
};

struct object_info_table {
    struct atom_data_revision revision;
    union {
        ATOM_OBJECT_HEADER *v1_1;
        ATOM_OBJECT_HEADER_V3 *v1_3;
    };
};

struct bios_parser {
    bool valid;
    // struct dc_bios base;

    struct atom_bios *bios;

    struct object_info_table object_info_tbl;
    uint32_t object_info_tbl_offset;
    ATOM_MASTER_DATA_TABLE *master_data_tbl;

    // const struct bios_parser_helper *bios_helper;

    // const struct command_table_helper *cmd_helper;
    // struct cmd_tbl cmd_tbl;

    // bool remap_device_tags;
};

static void get_atom_data_table_revision(ATOM_COMMON_TABLE_HEADER *atom_data_tbl, struct atom_data_revision *tbl_revision)
{
	if (!tbl_revision)
		return;

	/* initialize the revision to 0 which is invalid revision */
	tbl_revision->major = 0;
	tbl_revision->minor = 0;

	if (!atom_data_tbl)
		return;

	tbl_revision->major = (uint32_t) GET_DATA_TABLE_MAJOR_REVISION(atom_data_tbl);
	tbl_revision->minor = (uint32_t) GET_DATA_TABLE_MINOR_REVISION(atom_data_tbl);
}

bool bios_parser_init(struct bios_parser *bp, struct atom_bios *bios)
{
    uint16_t *rom_header_offset = NULL;
    ATOM_ROM_HEADER *rom_header = NULL;
    ATOM_OBJECT_HEADER *object_info_tbl;
    struct atom_data_revision tbl_rev = {0};

    bp->bios = bios;

    // if (!init)
    //     return false;

    // if (!init->bios)
    //     return false;

    // bp->base.funcs = &vbios_funcs;
    // bp->base.bios = init->bios;
    // bp->base.bios_size = bp->base.bios[BIOS_IMAGE_SIZE_OFFSET] * BIOS_IMAGE_SIZE_UNIT;
    //
    // bp->base.ctx = init->ctx;
    // bp->base.bios_local_image = NULL;

    rom_header_offset = GET_IMAGE(uint16_t, OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER);
    if (!rom_header_offset)
        return false;

    rom_header = GET_IMAGE(ATOM_ROM_HEADER, *rom_header_offset);
    if (!rom_header)
        return false;

    get_atom_data_table_revision(&rom_header->sHeader, &tbl_rev);
    if (tbl_rev.major >= 2 && tbl_rev.minor >= 2)
        return false;

    bp->master_data_tbl = GET_IMAGE(ATOM_MASTER_DATA_TABLE, rom_header->usMasterDataTableOffset);
    if (!bp->master_data_tbl)
        return false;

    bp->object_info_tbl_offset = DATA_TABLES(Object_Header);
    if (!bp->object_info_tbl_offset)
        return false;

    object_info_tbl = GET_IMAGE(ATOM_OBJECT_HEADER, bp->object_info_tbl_offset);
    if (!object_info_tbl)
        return false;

    get_atom_data_table_revision(&object_info_tbl->sHeader, &bp->object_info_tbl.revision);
    if (bp->object_info_tbl.revision.major == 1 && bp->object_info_tbl.revision.minor >= 3) {
        ATOM_OBJECT_HEADER_V3 *tbl_v3;

        tbl_v3 = GET_IMAGE(ATOM_OBJECT_HEADER_V3, bp->object_info_tbl_offset);
        if (!tbl_v3)
            return false;

        bp->object_info_tbl.v1_3 = tbl_v3;
    } else if (bp->object_info_tbl.revision.major == 1
        && bp->object_info_tbl.revision.minor >= 1)
        bp->object_info_tbl.v1_1 = object_info_tbl;
    else {
        return false;
    }

    // dal_bios_parser_init_cmd_tbl(bp);
    // dal_bios_parser_init_cmd_tbl_helper(&bp->cmd_helper, dce_version);

    // bp->base.integrated_info = bios_parser_create_integrated_info(&bp->base);

    return true;
}

static uint8_t get_number_of_objects(struct bios_parser *bp, uint32_t offset)
{
    ATOM_OBJECT_TABLE *table;

    uint32_t object_table_offset = bp->object_info_tbl_offset + offset;

    table = GET_IMAGE(ATOM_OBJECT_TABLE, object_table_offset);

    if (!table)
        return 0;
    else
        return table->ucNumberOfObjects;
}

uint8_t bios_parser_get_connectors_number(struct bios_parser *bp)
{
    // struct bios_parser *bp = BP_FROM_DCB(dcb);

    return get_number_of_objects(bp, le16_to_cpu(bp->object_info_tbl.v1_1->usConnectorObjectTableOffset));
}
