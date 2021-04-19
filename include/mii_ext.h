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

typedef struct __attribute__((__packed__)) {
    u32 MiiVersion:8;
    u32 Copyable:1;
    u32 NgWord:1;
    u32 RegionMove:2;
    u32 FontRegion:2;
    u32 Reserved0:2;
    u32 RoomIndex:4;
    u32 PositionInRoom:4;
    u32 AuthorType:4;
    u32 BirthPlatform:3;
    u32 Reserved1:1;

    u64 AuthorId;
    u8 CreateId[10];
    u16 Reserved2;

    u16 Gender:1;
    u16 BirthMonth:4;
    u16 BirthDay:5;
    u16 FavoriteColor:4;
    u16 Favorite:1;
    u16 Padding0:1;

    u8 Name[20];
    u8 Height;
    u8 Build;

    u16 Localonly:1;
    u16 FaceType:4;
    u16 FaceColor:3;
    u16 FaceTex:4;
    u16 FaceMake:4;

    u16 HairType:8;
    u16 HairColor:3;
    u16 HairFlip:1;
    u16 Padding1:4;

    u16 EyeType:6;
    u16 EyeColor:3;
    u16 EyeScale:4;
    u16 EyeAspect:3;

    u16 EyeRotate:5;
    u16 EyeX:4;
    u16 EyeY:5;
    u16 Padding2:2;

    u16 EyebrowType:5;
    u16 EyebrowColor:3;
    u16 EyebrowScale:4;
    u16 EyebrowAspect:3;
    u16 Padding3:1;

    u16 EyebrowRotate:5;
    u16 EyebrowX:4;
    u16 EyebrowY:5;
    u16 Padding4:2;

    u16 NoseType:5;
    u16 NoseScale:4;
    u16 NoseY:5;
    u16 Padding5:2;

    u16 MouthType:6;
    u16 MouthColor:3;
    u16 MouthScale:4;
    u16 MouthAspect:3;

    u16 MouthY:5;
    u16 MustacheType:3;
    u16 Padding6:8;

    u16 BeardType:3;
    u16 BeardColor:3;
    u16 BeardScale:4;
    u16 BeardY:5;
    u16 Padding7:1;

    u16 GlassType:4;
    u16 GlassColor:3;
    u16 GlassScale:4;
    u16 GlassY:5;

    u16 MoleType:1;
    u16 MoleScale:4;
    u16 MoleX:5;
    u16 MoleY:5;
    u16 Padding8:1;

    u8 CreatorName[20];
    u16 Padding9;
    u16 Crc;
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

Result miiDatabaseFindIndex(MiiDatabase *db, const MiiCreateId* id, bool idk, int* out_idx) {
    const struct {
        MiiCreateId id;
        bool idk;
    } in = {*id, idk};
    return serviceDispatchInOut(&db->s, 11, in, *out_idx);
}

Result miiDatabaseGetIndex(MiiDatabase *db, int idx, charInfo* out) {
    return serviceDispatchInOut(&db->s, 21, idx, out);
}