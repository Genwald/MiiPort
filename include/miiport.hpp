#include <cstdio>
#include <string>
#include <cstring>
#include <errno.h>
#include <filesystem>
namespace fs = std::filesystem;

#include <switch.h>

#include <mii_ext.h>
#include <crc.hpp>

template <typename T>
bool readFromFile(const char *path, T *out) {
    size_t size_read;
    FILE* file = fopen(path, "r");
    if(file == nullptr) {
        printf("File open error: %d", errno);
        return false;
    } 

    size_read = fread(out, 1, sizeof(T), file);
    if (size_read != sizeof(T)) return false;

    fclose(file);
    return true;
}
template <typename T>
bool writeToFile(const char *path, T *out) {
    size_t size_written;
    FILE* file = fopen(path, "w");
    if(file == nullptr) {
        printf("File open error: %d", errno);
        return false;
    }

    size_written = fwrite(out, 1, sizeof(T), file);
    if (size_written != sizeof(T)) return false;

    fclose(file);
    return true;
}

void stringToLower(std::string *str) {
    std::transform(str->begin(), str->end(), str->begin(),
        [](unsigned char c){ return std::tolower(c); });
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
    /* 
    * These two leftmost bits must be 0b10 for the ID to be valid.
    * The console may generate the ID differently to assure this,
    * but we just set the bits.
    */
    out->uuid.uuid[8] &= 0b1011'1111;
    out->uuid.uuid[8] |= 0b1000'0000;
}

Result addOrReplaceStoreData(storeData *input) {
    MiiDatabase DbService;
    Result res;
    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseAddOrReplace(&DbService, input);
    miiDatabaseClose(&DbService);
    return res;
}

Result exportNFIF(NFIF *out) {
    MiiDatabase DbService;
    Result res;
    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseExport(&DbService, out);
    miiDatabaseClose(&DbService);
    return res;
}

Result importNFIF(NFIF *input) {
    MiiDatabase DbService;
    Result res;
    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseImport(&DbService, input);
    miiDatabaseClose(&DbService);
    return res;
}

Result miiDbExportToFile(const char* file_path) {
    NFIF Db;
    Result res = exportNFIF(&Db);
    if(R_FAILED(res)) return res;
    writeToFile(file_path, &Db);
    
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
    NFIF Db;
    readFromFile(file_path, &Db);
    return importNFIF(&Db);
}

Result miiDbAddOrReplaceCoreDataFromFile(const char* file_path) {
    coreData in_data;
    storeData new_data;
    MiiCreateId id;

    readFromFile(file_path, &in_data);
    
    // get createID from file name, or use a random one
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
    
    coreDataToStoreData(&in_data, &id, &new_data);
    return addOrReplaceStoreData(&new_data);
}

Result miiDbAddOrReplaceCharInfoFromFile(const char* file_path) {
    charInfo in_data;
    coreData intermediate;
    storeData new_data;
    MiiCreateId id;

    readFromFile(file_path, &in_data);

    charInfoToCoreData(&in_data, &intermediate, &id);
    coreDataToStoreData(&intermediate, &id, &new_data);

    return addOrReplaceStoreData(&new_data);
}

Result importMiiFile(fs::path file_path) {
    std::string ext = file_path.extension().string();
    stringToLower(&ext);
    Result res = 0;
    
    if(ext == ".charinfo") {
        res = miiDbAddOrReplaceCharInfoFromFile(file_path.c_str());
    }
    else if(ext == ".nfif") {
        res = miiDbImportFromFile(file_path.c_str());
    }
    // drop coredata file support? Or feature flag? Or just keep it and don't advertise it?
    else if(ext == ".coredata") {
        res = miiDbAddOrReplaceCoreDataFromFile(file_path.c_str());
    }
    else {
        // todo: something other than fake result
        // No matching extension
        return 0xFFFFFFFF;
    }
    return res;
}