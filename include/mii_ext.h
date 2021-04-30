#pragma once
#include <switch.h>

typedef struct {  /* charInfo */
    MiiCreateId create_id;
    char16_t nickname[11]; /* null terminated */
    u8 font_region;
    u8 favorite_color;
    u8 gender;
    u8 height;
    u8 build;
    u8 type; /* special or not */
    u8 region_move;
    u8 faceline_type;
    u8 faceline_color;
    u8 faceline_wrinkle;
    u8 faceline_make;
    u8 hair_type;
    u8 hair_color;
    u8 hair_flip;
    u8 eye_type;
    u8 eye_color;
    u8 eye_scale;
    u8 eye_aspect;
    u8 eye_rotate;
    u8 eye_x;
    u8 eye_y;
    u8 eyebrow_type;
    u8 eyebrow_color;
    u8 eyebrow_scale;
    u8 eyebrow_aspect;
    u8 eyebrow_rotate;
    u8 eyebrow_x;
    u8 eyebrow_y;
    u8 nose_type;
    u8 nose_scale;
    u8 nose_y;
    u8 mouth_type;
    u8 mouth_color;
    u8 mouth_scale;
    u8 mouth_aspect;
    u8 mouth_y;
    u8 beard_color;
    u8 beard_type;
    u8 mustache_type;
    u8 mustache_scale;
    u8 mustache_y;
    u8 glass_type;
    u8 glass_color;
    u8 glass_scale;
    u8 glass_y;
    u8 mole_type;
    u8 mole_scale;
    u8 mole_x;
    u8 mole_y;
    u8 reserved; /* always zero */
} charInfo;

typedef struct {  /* coreData */
    u8 hair_type;

    u8 height:7;
    u8 mole_type:1;

    u8 build:7;
    u8 hair_flip:1;

    u8 hair_color:7;
    u8 type:1;

    u8 eye_color:7;
    u8 gender:1;

    u8 eyebrow_color:7;
    u8 unused1:1;

    u8 mouth_color:7;
    u8 unused2:1;

    u8 beard_color:7;
    u8 unused3:1;

    u8 glass_color:7;
    u8 unused4:1;

    u8 eye_type:6;
    u8 region_move:2;

    u8 mouth_type:6;
    u8 font_region:2;

    u8 eye_y:5;
    u8 glass_scale:3;

    u8 eyebrow_type:5;
    u8 mustache_type:3;

    u8 nose_type:5;
    u8 beard_type:3;

    u8 nose_y:5;
    u8 mouth_aspect:3;

    u8 mouth_y:5;
    u8 eyebrow_aspect:3;

    u8 mustache_y:5;
    u8 eye_rotate:3;

    u8 glass_y:5;
    u8 eye_aspect:3;

    u8 mole_x:5;
    u8 eye_scale:3;

    u8 mole_y:5;
    u8 unused5:3;

    u8 glass_type:5;
    u8 unused6:3;

    u8 favorite_color:4;
    u8 faceline_type:4;

    u8 faceline_color:4;
    u8 faceline_wrinkle:4;

    u8 faceline_make:4;
    u8 eye_x:4;

    u8 eyebrow_scale:4;
    u8 eyebrow_rotate:4;

    u8 eyebrow_x:4;
    u8 eyebrow_y:4; /* value + 3 = true value */

    u8 nose_scale:4;
    u8 mouth_scale:4;

    u8 mustache_scale:4;
    u8 mole_scale:4;

    char16_t nickname[10]; /* Not null terminated */
} coreData;

typedef struct {
    coreData core_data;
    MiiCreateId create_id;
    u16 crc16;
    u16 crc16_device;
} storeData;

typedef struct {
    char magic[4]; /* NFDB */
    storeData entries[100];
    u8 version; /* always 1 */
    u8 entry_count;
    u16 crc16;
} NFDB;

typedef struct {
    char magic[4]; /* NFIF */
    coreData entries[100];
    u8 version; /* always 2 */
    u8 entry_count;
    u8 unused[2];
    u8 hmac_sha256[0x20]; /* key found in sdb main. Look at database export function */
} NFIF;

typedef struct __attribute__((__packed__)) {  /* ver3StoreData */
    u32 mii_version:8;
    u32 copyable:1;
    u32 ng_word:1;
    u32 region_move:2;
    u32 font_region:2;
    u32 reserved0:2;
    u32 room_index:4;
    u32 position_in_room:4;
    u32 author_type:4;
    u32 birth_platform:3;
    u32 reserved_1:1;

    u64 author_id;
    u8 create_id[10];
    u16 reserved_2;

    u16 gender:1;
    u16 birth_month:4;
    u16 birth_day:5;
    u16 favorite_color:4;
    u16 favorite:1;
    u16 padding0:1;

    u8 name[20];
    u8 height;
    u8 build;

    u16 localonly:1;
    u16 face_type:4;
    u16 face_color:3;
    u16 face_tex:4;
    u16 face_make:4;

    u16 hair_type:8;
    u16 hair_color:3;
    u16 hair_flip:1;
    u16 padding1:4;

    u16 eye_type:6;
    u16 eye_color:3;
    u16 eye_scale:4;
    u16 eye_aspect:3;

    u16 eye_rotate:5;
    u16 eye_x:4;
    u16 eye_y:5;
    u16 padding2:2;

    u16 eyebrow_type:5;
    u16 eyebrow_color:3;
    u16 eyebrow_scale:4;
    u16 eyebrow_aspect:3;
    u16 padding3:1;

    u16 eyebrow_rotate:5;
    u16 eyebrow_x:4;
    u16 eyebrow_y:5;
    u16 padding4:2;

    u16 nose_type:5;
    u16 nose_scale:4;
    u16 nose_y:5;
    u16 padding5:2;

    u16 mouth_type:6;
    u16 mouth_color:3;
    u16 mouth_scale:4;
    u16 mouth_aspect:3;

    u16 mouth_y:5;
    u16 mustache_type:3;
    u16 padding6:8;

    u16 beard_type:3;
    u16 beard_color:3;
    u16 beard_scale:4;
    u16 beard_y:5;
    u16 padding7:1;

    u16 glass_type:4;
    u16 glass_color:3;
    u16 glass_scale:4;
    u16 glass_y:5;

    u16 mole_type:1;
    u16 mole_scale:4;
    u16 mole_x:5;
    u16 mole_y:5;
    u16 padding8:1;

    u8 creator_name[20];
    u16 padding9;
    u16 crc;
} ver3StoreData;

Result miiDatabaseExport(MiiDatabase *db, NFIF* out_buffer) {
    return serviceDispatch(&db->s, 19,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { out_buffer, sizeof(NFIF) } },
    );
}

Result miiDatabaseImport(MiiDatabase *db, const NFIF* in_buffer) {
    return serviceDispatch(&db->s, 18,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { in_buffer, sizeof(NFIF) } },
    );
}

Result miiDatabaseAddOrReplace(MiiDatabase *db, const storeData *input) {
    return serviceDispatchIn(&db->s, 13, *input);
}

Result miiDatabaseFindIndex(MiiDatabase *db, const MiiCreateId* id, bool include_special, int* out_idx) {
    const struct {
        MiiCreateId id;
        bool include_special;
    } in = {*id, include_special};
    return serviceDispatchInOut(&db->s, 11, in, *out_idx);
}

// needs changes
Result miiDatabaseGetIndex(MiiDatabase *db, int idx, charInfo* out) {
    return serviceDispatchInOut(&db->s, 21, idx, out);
}