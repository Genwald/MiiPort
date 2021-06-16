#include <cstring>
#include <cstdlib>

#include "switch/types.h"
#include "mii_ext.h"
#include "crc.hpp"

const u8 Ver3FacelineColorTable[6] = {0, 1, 2, 3, 4, 5};

const u8 Ver3HairColorTable[8] = {8, 1, 2, 3, 4, 5, 6, 7};

const u8 Ver3EyeColorTable[6] = {8, 9, 10, 11, 12, 13};

const u8 Ver3MouthColorTable[5] = {19, 20, 21, 22, 23};

const u8 Ver3GlassColorTable[7] = {8, 14, 15, 16, 17, 0};

const u8 ToVer3GlassTypeTable[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 1, 3, 7, 7, 6, 7, 8, 7, 7};

const u8 ToVer3HairColorTable[100] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 4, 3, 5, 4, 4, 6, 2, 0, 6, 4, 3, 2, 2, 7, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 4, 4, 0, 0, 0, 4, 4, 4, 4, 4, 4, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 5, 7, 5, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 3, 7, 7, 7, 7, 7, 0, 4, 4, 4, 4};

const u8 ToVer3EyeColorTable[100] = {0, 2, 2, 2, 1, 3, 2, 3, 0, 1, 2, 3, 4, 5, 2, 2, 4, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 4, 4, 4, 4, 4, 4, 4, 1, 0, 4, 4, 4, 4, 4, 4, 4, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1};

const u8 ToVer3MouthColorTable[100] = {4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 1, 4, 4, 4, 0, 1, 2, 3, 4, 4, 2, 3, 3, 4, 4, 4, 4, 1, 4, 4, 2, 3, 3, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 4, 4, 4, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 3, 3, 3, 3, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 4, 4, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 4, 0, 3, 3, 3, 3, 4, 3, 3, 3, 3};

const u8 ToVer3GlassColorTable[100] = {0, 1, 1, 1, 5, 1, 1, 4, 0, 5, 1, 1, 3, 5, 1, 2, 3, 4, 5, 4, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5};

const u8 ToVer3FacelineColorTable[10] = {0, 1, 2, 3, 4, 5, 0, 1, 5, 5};

void makeRandCreateId(MiiCreateId *out) {
    randomGet(out->uuid.uuid, sizeof(MiiCreateId));
    //These two leftmost bits must be 0b10 for the ID to be valid.
    out->uuid.uuid[8] &= 0b1011'1111;
    out->uuid.uuid[8] |= 0b1000'0000;
}

// crc written in last two bytes of in
void setCrc16(void* in, int size) {
    *(u16*)((u8*)in + size - sizeof(u16)) = crc16LE(in, size - sizeof(u16));
}

void setDeviceCrc16(void* in, int size) {
    Uuid device_id;
    setsysGetMiiAuthorId(&device_id);
    int device_id_crc = crc16(&device_id, sizeof(device_id));
    *(u16*)((u8*)in + size - sizeof(u16)) =  crc16LE(in, size - sizeof(u16), device_id_crc);
}

void setStoreDataCrc16(storeData* in) {
    setCrc16(in, sizeof(storeData) - sizeof(u16));
    setDeviceCrc16(in, sizeof(storeData));
}

void coreDataToStoreData(const coreData* in, const MiiCreateId* id, storeData* out) {
    out->core_data = *in;
    out->create_id = *id;
    setStoreDataCrc16(out);
}

void ver3StoreDataToStoreData(const ver3StoreData* in, storeData* out) {
    out->core_data.font_region = in->font_region;
    out->core_data.favorite_color = in->favorite_color;
    out->core_data.gender = in->gender;
    out->core_data.height = in->height;
    out->core_data.build = in->build;
    // todo: type is derived from create id
    out->core_data.type = 0;
    out->core_data.region_move = in->region_move;
    out->core_data.faceline_type = in->face_type;
    out->core_data.faceline_color = Ver3FacelineColorTable[in->face_color];
    out->core_data.faceline_wrinkle = in->face_tex;
    out->core_data.faceline_make = in->face_make;
    out->core_data.hair_type = in->hair_type;
    out->core_data.hair_color = Ver3HairColorTable[in->hair_color];
    out->core_data.hair_flip = in->hair_flip;
    out->core_data.eye_type = in->eye_type;
    out->core_data.eye_color = Ver3EyeColorTable[in->eye_color];
    out->core_data.eye_scale = in->eye_scale;
    out->core_data.eye_aspect = in->eye_aspect;
    out->core_data.eye_rotate = in->eye_rotate;
    out->core_data.eye_x = in->eye_x;
    out->core_data.eye_y = in->eye_y;
    out->core_data.eyebrow_type = in->eyebrow_type;
    out->core_data.eyebrow_color = in->eyebrow_color;
    out->core_data.eyebrow_scale = in->eyebrow_scale;
    out->core_data.eyebrow_aspect = in->eyebrow_aspect;
    out->core_data.eyebrow_rotate = in->eyebrow_rotate;
    out->core_data.eyebrow_x = in->eyebrow_x;
    out->core_data.eyebrow_y = in->eyebrow_y - 3;
    out->core_data.nose_type = in->nose_type;
    out->core_data.nose_scale = in->nose_scale;
    out->core_data.nose_y = in->nose_y;
    out->core_data.mouth_type = in->mouth_type;
    out->core_data.mouth_color = Ver3MouthColorTable[in->mouth_color];
    out->core_data.mouth_scale = in->mouth_scale;
    out->core_data.mouth_aspect = in->mouth_aspect;
    out->core_data.mouth_y = in->mouth_y;
    out->core_data.beard_color = in->beard_color;
    out->core_data.beard_type = in->beard_type;
    out->core_data.mustache_type = in->mustache_type;
    out->core_data.mustache_scale = in->beard_scale;
    out->core_data.mustache_y = in->beard_y;
    out->core_data.glass_type = in->glass_type;
    out->core_data.glass_color = Ver3GlassColorTable[in->glass_color];
    out->core_data.glass_scale = in->glass_scale;
    out->core_data.glass_y = in->glass_y;
    out->core_data.mole_type = in->mole_type;
    out->core_data.mole_scale = in->mole_scale;
    out->core_data.mole_x = in->mole_x;
    out->core_data.mole_y = in->mole_y;
    memcpy(out->core_data.nickname, in->name, 10 * sizeof(char16_t));
    makeRandCreateId(&out->create_id);
    coreDataToStoreData(&out->core_data, &out->create_id, out);
}

void charInfoToVer3StoreData(const charInfo* in, ver3StoreData* out) {
    memset(out, 0, sizeof(ver3StoreData));
    out->font_region = in->font_region;
    out->favorite_color = in->favorite_color;
    out->gender = in->gender;
    out->height = in->height;
    out->build = in->build;
    out->region_move = in->region_move;
    out->face_type = in->faceline_type;
    out->face_color = ToVer3FacelineColorTable[in->faceline_color];
    out->face_tex = in->faceline_wrinkle;
    out->face_make = in->faceline_make;
    out->hair_type = in->hair_type;
    out->hair_color = ToVer3HairColorTable[in->hair_color];
    out->hair_flip = in->hair_flip;
    out->eye_type = in->eye_type;
    out->eye_color = ToVer3EyeColorTable[in->eye_color];
    out->eye_scale = in->eye_scale;
    out->eye_aspect = in->eye_aspect;
    out->eye_rotate = in->eye_rotate;
    out->eye_x = in->eye_x;
    out->eye_y = in->eye_y;
    out->eyebrow_type = in->eyebrow_type;
    out->eyebrow_color = ToVer3HairColorTable[in->eyebrow_color];
    out->eyebrow_scale = in->eyebrow_scale;
    out->eyebrow_aspect = in->eyebrow_aspect;
    out->eyebrow_rotate = in->eyebrow_rotate;
    out->eyebrow_x = in->eyebrow_x;
    out->eyebrow_y = in->eyebrow_y;
    out->nose_type = in->nose_type;
    out->nose_scale = in->nose_scale;
    out->nose_y = in->nose_y;
    out->mouth_type = in->mouth_type;
    out->mouth_color = ToVer3MouthColorTable[in->mouth_color];
    out->mouth_scale = in->mouth_scale;
    out->mouth_aspect = in->mouth_aspect;
    out->mouth_y = in->mouth_y;
    out->beard_color = ToVer3HairColorTable[in->beard_color];
    out->beard_type = in->beard_type;
    out->mustache_type = in->mustache_type;
    out->beard_scale = in->mustache_scale;
    out->beard_y = in->mustache_y;
    out->glass_type = ToVer3GlassTypeTable[in->glass_type];
    out->glass_color = ToVer3GlassColorTable[in->glass_color];
    out->glass_scale = in->glass_scale;
    out->glass_y = in->glass_y;
    out->mole_type = in->mole_type;
    out->mole_scale = in->mole_scale;
    out->mole_x = in->mole_x;
    out->mole_y = in->mole_y;
    memcpy(out->name, in->nickname, 10 * sizeof(char16_t));
    out->mii_version = 3;
    out->copyable = 1;
    // The switch sets this to 4, but the 3DS rejects it if set to >3
    out->birth_platform = 3;
    out->birth_month = 4;
    out->birth_day = 20;
    randomGet(out->create_id, sizeof(out->create_id));
    /* 
     * Set 0b1101 in the 4 MSB of create_id
     * This sets non-special and some unknown flags
     * Note: the Switch's nnsdk sets 0b1101, 
     * but all other QRs I checked had 0b1001
     */
    out->create_id[0] = (out->create_id[0] & 0b0000'1111) | 0b1101'0000;
    randomGet(&out->author_id, sizeof(out->author_id));
    memcpy(out->creator_name, u"MiiPort", sizeof(u"MiiPort"));
    setCrc16(out, sizeof(ver3StoreData));
}

void charInfoToCoreData(const charInfo* in, coreData* out, MiiCreateId* id_out) {
    *id_out = in->create_id;
    memcpy(out->nickname, in->nickname, 10 * sizeof(char16_t));
    out->font_region = in->font_region;
    out->favorite_color = in->favorite_color;
    out->gender = in->gender;
    out->height = in->height;
    out->build = in->build;
    out->type = in->type;
    out->region_move = in->region_move;
    out->faceline_type = in->faceline_type;
    out->faceline_color = in->faceline_color;
    out->faceline_wrinkle = in->faceline_wrinkle;
    out->faceline_make = in->faceline_make;
    out->hair_type = in->hair_type;
    out->hair_color = in->hair_color;
    out->hair_flip = in->hair_flip;
    out->eye_type = in->eye_type;
    out->eye_color = in->eye_color;
    out->eye_scale = in->eye_scale;
    out->eye_aspect = in->eye_aspect;
    out->eye_rotate = in->eye_rotate;
    out->eye_x = in->eye_x;
    out->eye_y = in->eye_y;
    out->eyebrow_type = in->eyebrow_type;
    out->eyebrow_color = in->eyebrow_color;
    out->eyebrow_scale = in->eyebrow_scale;
    out->eyebrow_aspect = in->eyebrow_aspect;
    out->eyebrow_rotate = in->eyebrow_rotate;
    out->eyebrow_x = in->eyebrow_x;
    out->eyebrow_y = in->eyebrow_y - 3; // y in coredata is 3 less than true value
    out->nose_type = in->nose_type;
    out->nose_scale = in->nose_scale;
    out->nose_y = in->nose_y;
    out->mouth_type = in->mouth_type;
    out->mouth_color = in->mouth_color;
    out->mouth_scale = in->mouth_scale;
    out->mouth_aspect = in->mouth_aspect;
    out->mouth_y = in->mouth_y;
    out->beard_color = in->beard_color;
    out->beard_type = in->beard_type;
    out->mustache_type = in->mustache_type;
    out->mustache_scale = in->mustache_scale;
    out->mustache_y = in->mustache_y;
    out->glass_type = in->glass_type;
    out->glass_color = in->glass_color;
    out->glass_scale = in->glass_scale;
    out->glass_y = in->glass_y;
    out->mole_type = in->mole_type;
    out->mole_scale = in->mole_scale;
    out->mole_x = in->mole_x;
    out->mole_y = in->mole_y;
}