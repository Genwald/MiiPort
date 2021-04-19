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

void makeRandCreateId(MiiCreateId *out) {
    for(u64 i = 0; i<sizeof(MiiCreateId); i++) {
        out->uuid.uuid[i] = rand()%0xFF;
    }
    /* 
    * These two leftmost bits must be 0b10 for the ID to be valid.
    * The console may generate the ID differently to assure this,
    * but we just set the bits.
    */
    out->uuid.uuid[8] &= 0b1011'1111;
    out->uuid.uuid[8] |= 0b1000'0000;
}

void coreDataToStoreData(const coreData* in, const MiiCreateId* id, storeData* out) {
    out->core_data = *in;
    out->create_id = *id;
    out->crc16 = crc16LE(out, sizeof(storeData) - (sizeof(u16)*2));

    Uuid device_id;
    setsysGetMiiAuthorId(&device_id);
    int device_id_crc = crc16(&device_id, sizeof(device_id));
    out->crc16_device =  crc16LE(out, sizeof(storeData) - sizeof(u16), device_id_crc);
}

// double check all these conversions
void ver3StoreDataToStoreData(const ver3StoreData* in, storeData* out) {
    out->core_data.font_region = in->FontRegion;
    out->core_data.favorite_color = in->FavoriteColor;
    out->core_data.gender = in->Gender;
    out->core_data.height = in->Height;
    out->core_data.build = in->Build;
    // todo: type is derived from create id
    out->core_data.type = 0;
    out->core_data.region_move = in->RegionMove;
    out->core_data.faceline_type = in->FaceType;
    out->core_data.faceline_color = Ver3FacelineColorTable[in->FaceColor];
    out->core_data.faceline_wrinkle = in->FaceTex;
    out->core_data.faceline_make = in->FaceMake;
    out->core_data.hair_type = in->HairType;
    out->core_data.hair_color = Ver3HairColorTable[in->HairColor];
    out->core_data.hair_flip = in->HairFlip;
    out->core_data.eye_type = in->EyeType;
    out->core_data.eye_color = Ver3EyeColorTable[in->EyeColor];
    out->core_data.eye_scale = in->EyeScale;
    out->core_data.eye_aspect = in->EyeAspect;
    out->core_data.eye_rotate = in->EyeRotate;
    out->core_data.eye_x = in->EyeX;
    out->core_data.eye_y = in->EyeY;
    out->core_data.eyebrow_type = in->EyebrowType;
    out->core_data.eyebrow_color = in->EyebrowColor;
    out->core_data.eyebrow_scale = in->EyebrowScale;
    out->core_data.eyebrow_aspect = in->EyebrowAspect;
    out->core_data.eyebrow_rotate = in->EyebrowRotate;
    out->core_data.eyebrow_x = in->EyebrowX;
    out->core_data.eyebrow_y = in->EyebrowY - 3;
    out->core_data.nose_type = in->NoseType;
    out->core_data.nose_scale = in->NoseScale;
    out->core_data.nose_y = in->NoseY;
    out->core_data.mouth_type = in->MouthType;
    out->core_data.mouth_color = Ver3MouthColorTable[in->MouthColor];
    out->core_data.mouth_scale = in->MouthScale;
    out->core_data.mouth_aspect = in->MouthAspect;
    out->core_data.mouth_y = in->MouthY;
    out->core_data.beard_color = in->BeardColor;
    out->core_data.beard_type = in->BeardType;
    out->core_data.mustache_type = in->MustacheType;
    out->core_data.mustache_scale = in->BeardScale;
    out->core_data.mustache_y = in->BeardY;
    out->core_data.glass_type = in->GlassType;
    out->core_data.glass_color = Ver3GlassColorTable[in->GlassColor];
    out->core_data.glass_scale = in->GlassScale;
    out->core_data.glass_y = in->GlassY;
    out->core_data.mole_type = in->MoleType;
    out->core_data.mole_scale = in->MoleScale;
    out->core_data.mole_x = in->MoleX;
    out->core_data.mole_y = in->MoleY;
    memcpy(out->core_data.nickname, in->Name, 10 * sizeof(char16_t));
    makeRandCreateId(&out->create_id);
    coreDataToStoreData(&out->core_data, &out->create_id, out);
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