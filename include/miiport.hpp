#include <cstdio>
#include <string>
#include <cstring>
#include <filesystem>
namespace fs = std::filesystem;

#include <switch.h>

#include <mii_ext.h>
#include <crc.hpp>

Result miiDbExportToFile(const char* file_path) {
    MiiDatabase DbService;
    NFIF Db;
    Result res;
    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseExport(&DbService, &Db);
    miiDatabaseClose(&DbService);
    if(R_FAILED(res)) return res;

    FILE* out_file = fopen(file_path, "w");
    fwrite(&Db, 1, sizeof(Db), out_file);
    fclose(out_file);
    
    /*
    for(int i=0; i < Db.entry_count; i++) {
        printf("%s\n", Db.entries[i].type?"Special":"Normal");
        wprintf(L"%.5ls", Db.entries[i].Nickname);
        printf("\n");
    }
    */
    
    return 0;
}

Result miiDbImportFromFile(const char* file_path) {
    MiiDatabase DbService;
    NFIF Db;
    Result res;
    FILE* in_file = fopen(file_path, "r");
    fread(&Db, 1, sizeof(Db), in_file);
    fclose(in_file);
    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseImport(&DbService, &Db);
    miiDatabaseClose(&DbService);
    return res;
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
    out->eyebrow_y = in->eyebrow_y - 3;
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

int strToCreateId(const std::string& hex, MiiCreateId *id) {
    for (unsigned int i = 0; i < sizeof(MiiCreateId); i++) {
        const char* byteStr = hex.substr(i*2, 2).c_str();
        char* end = nullptr;
        u8 byte = (u8) strtol(byteStr, &end, 16);
        if (byteStr == end) return false;
        id->uuid.uuid[i] = byte;
    }
    return true;
}

void makeRandCreateId(MiiCreateId *out) {
    for(u64 i = 0; i<sizeof(MiiCreateId); i++) {
        out->uuid.uuid[i] = rand()%0xFF;
    }
    out->uuid.uuid[8] &= 0b1011'1111;
    out->uuid.uuid[8] |= 0b1000'0000;
}

Result miiDbAddOrReplaceCoreDataFromFile(const char* file_path) {
    MiiDatabase DbService;
    coreData in_data;
    Result res;
    FILE* in_file = fopen(file_path, "r");
    fread(&in_data, 1, sizeof(in_data), in_file);
    fclose(in_file);

    MiiCreateId id;
    std::string filename = fs::path(file_path).filename().string();
    if(!strToCreateId(filename, &id)) {
        printf("using random ID\n");
        makeRandCreateId(&id);
    }

    printf("Create ID: ");
    for(u64 i = 0; i<sizeof(MiiCreateId); i++) {
        printf("%02X", id.uuid.uuid[i]);
    }
    printf("\n");

    storeData new_data;
    coreDataToStoreData(&in_data, &id, &new_data);

    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseAddOrReplace(&DbService, &new_data);
    miiDatabaseClose(&DbService);
    return res;
}

Result miiDbAddOrReplaceCharInfoFromFile(const char* file_path) {
    MiiDatabase DbService;
    charInfo in_data;
    coreData intermediate;
    storeData new_data;
    MiiCreateId id;
    Result res;
    FILE* in_file = fopen(file_path, "r");
    fread(&in_data, 1, sizeof(in_data), in_file);
    fclose(in_file); 
    
    charInfoToCoreData(&in_data, &intermediate, &id);
    coreDataToStoreData(&intermediate, &id, &new_data);

    printf("storedata: \n");
    for(u64 i = 0; i<sizeof(new_data); i++) {
        printf("%02X", ((u8*)&new_data)[i]);
    }
    printf("\n");

    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseAddOrReplace(&DbService, &new_data);
    miiDatabaseClose(&DbService);
    return res;
}

void stringToLower(std::string *str) {
    std::transform(str->begin(), str->end(), str->begin(),
        [](unsigned char c){ return std::tolower(c); });
}

Result importMiiFile(fs::path file_path) {
    std::string ext = file_path.extension().string();
    stringToLower(&ext);
    Result res = 0;
    if(ext == ".coredata") {
        res = miiDbAddOrReplaceCoreDataFromFile(file_path.c_str());
    }
    else if(ext == ".charinfo") {
        res = miiDbAddOrReplaceCharInfoFromFile(file_path.c_str());
    }
    else if(ext == ".nfif") {
        res = miiDbImportFromFile(file_path.c_str());
    }
    else {
        // todo: something other than fake result
        // No matching extension
        return 0xFFFFFFFF;
    }
    return res;
}