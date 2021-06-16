#include <cstdio>
#include <string>
#include <cstring>
#include <filesystem>
namespace fs = std::filesystem;

#include <switch.h>

#include "mii_ext.h"
#include "convert_mii.h"
#include "mii_qr.hpp"
#include "errors.h"

void errorCodeNotify(Result res) {
    std::stringstream ss;
    ss << "Import error: 0x" << std::hex << res;
    brls::Application::notify(ss.str());
}

void errorNotify(Result res, std::string success_message = "Imported!") {
    switch(res) {
        case 0: {
            brls::Application::notify(success_message);
            break;
        }
        case 0xa7e: {
            brls::Application::notify("Mii database is full");
            break;
        }
        // bad storedata format
        case 0xda7e:
        // bad nfif format
        case 0xe07e: {
            brls::Application::notify("Improper file format");
            break;
        }
        // debug only
        case 0x1987e: {
            brls::Application::notify("Must enable DB testing mode");
            break;
        }
        case SHOWING_POPUP: {
            break;
        }
        case UNSUPPORTED_EXT: {
            brls::Application::notify("File extension not recognized");
            break;
        }
        case NO_QR: {
            brls::Application::notify("No QR code found");
            break;
        }
        case QR_DECODE_FAIL: {
            brls::Application::notify("QR decoding failed");
            break;
        }
        case AES_CCM_FAILED: {
            brls::Application::notify("Mii data decryption failed");
            break;
        }
        case JPEG_DECODE_FAIL: {
            brls::Application::notify("Jpeg decoding failed");
            break;
        }
        case BAD_KEY_FILE: {
            brls::Application::notify("Incorrect Mii QR key.\nSee \"About\" tab.");
            break;
        }
        default: {
            errorCodeNotify(res);
            break;
        }
    }
}

template <typename T>
bool readFromFile(const char *path, T *out) {
    size_t size_read;
    FILE* file = fopen(path, "r");
    if(file == nullptr) {
        printf("File open error: %d\n", errno);
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
        printf("File open error: %d\n", errno);
        return false;
    }

    size_written = fwrite(out, 1, sizeof(T), file);
    if (size_written != sizeof(T)) return false;

    fclose(file);
    return true;
}

template <typename T>
std::string getHexStr(T *data) {
    std::stringstream ss;
    ss << std::uppercase << std::hex;
    for(u64 i = 0; i<sizeof(T); i++) {
        ss << std::setw(2) << std::setfill('0') << (u32)((u8*)data)[i];
    }
    return ss.str();
}

void stringToLower(std::string *str) {
    std::transform(str->begin(), str->end(), str->begin(),
        [](unsigned char c){ return std::tolower(c); });
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

Result addOrReplaceStoreData(const storeData *input) {
    MiiDatabase DbService;
    Result res;
    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseAddOrReplace(&DbService, input);
    miiDatabaseClose(&DbService);
    return res;
}

void showDupeCreateIDPopup(storeData *input){
    brls::Dialog* dialog = new brls::Dialog("A Mii with the same Mii ID already exists on your switch.");

    brls::GenericEvent::Callback repalceCallback = [dialog, input{*input}](brls::View* view) {
        Result res = addOrReplaceStoreData(&input);

        errorNotify(res);
        dialog->close();
    };
    brls::GenericEvent::Callback randomCallback = [dialog, input{*input}](brls::View* view) mutable {
        makeRandCreateId(&input.create_id);
        // changed data, so re-generate storedata hashes
        setStoreDataCrc16(&input);
        Result res = addOrReplaceStoreData(&input);

        errorNotify(res);
        dialog->close();
    };

    dialog->addButton("Replace", repalceCallback);
    dialog->addButton("Use Random Mii ID", randomCallback);

    dialog->setCancelable(true);

    dialog->open();
}

Result addOrReplaceStoreDataWithPrompt(storeData *input) {
    MiiDatabase DbService;
    Result res;
    int idx;
    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseFindIndex(&DbService, &input->create_id, true, &idx);
    if(R_FAILED(res)) return res;
    // duplicate create ID found
    if(idx != -1) {
        showDupeCreateIDPopup(input);
        res = SHOWING_POPUP;
    }
    else {
        res = miiDatabaseAddOrReplace(&DbService, input);
    }
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

Result getCharInfos(charInfo *out_array, int size, int *out_size) {
    MiiDatabase DbService;
    Result res;
    res = miiOpenDatabase(&DbService, MiiSpecialKeyCode_Special);
    if(R_FAILED(res)) return res;
    res = miiDatabaseGet1(&DbService, MiiSourceFlag_Database, (MiiCharInfo*)out_array, size, out_size);
    miiDatabaseClose(&DbService);
    return res;
}

Result miiDbExportToFile(const char* file_path) {
    NFIF Db;
    Result res = exportNFIF(&Db);
    if(R_FAILED(res)) return res;
    writeToFile(file_path, &Db);    
    return 0;
}

Result miiDbImportFromFile(const char* file_path) {
    NFIF Db;
    readFromFile(file_path, &Db);
    return importNFIF(&Db);
}

Result miiDbAddOrReplaceStoreDataFromFile(const char* file_path) {
    storeData in_data;
    readFromFile(file_path, &in_data);
    // run this to regenerate checksums
    setStoreDataCrc16(&in_data);
    return addOrReplaceStoreDataWithPrompt(&in_data);
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
    return addOrReplaceStoreDataWithPrompt(&new_data);
}

Result miiDbAddOrReplaceCharInfoFromFile(const char* file_path) {
    charInfo in_data;
    coreData intermediate;
    storeData new_data;
    MiiCreateId id;

    readFromFile(file_path, &in_data);

    charInfoToCoreData(&in_data, &intermediate, &id);
    coreDataToStoreData(&intermediate, &id, &new_data);

    return addOrReplaceStoreDataWithPrompt(&new_data);
}

Result showQrPopup(ver3StoreData* data, std::string name) {
    int qr_width = 0;
    std::unique_ptr<u32[]> qr_RGBA;
    Result res = generateMiiQr(data, 8, &qr_width, qr_RGBA);
    if(R_FAILED(res)) {
        return res;
    }
    brls::Image *qr_image = new brls::Image;
    qr_image->setScaleType(brls::ImageScaleType::NO_RESIZE);
    qr_image->setImageRGBA((u8*)qr_RGBA.get(), qr_width, qr_width);
    brls::AppletFrame* frame = new brls::AppletFrame(0,0);
    frame->setContentView(qr_image);
    brls::PopupFrame::open("QR Code", frame, "", name);
    return 0;
}

Result importMiiQr(const char* path) {
    ver3StoreData ver3mii;
    storeData mii;
    Result res;
    res = parseMiiQr(path, &ver3mii);
    if(R_FAILED(res)) {
        return res;
    }
    ver3StoreDataToStoreData(&ver3mii, &mii);
    return addOrReplaceStoreData(&mii);
}

Result importMiiFile(fs::path file_path) {
    std::string ext = file_path.extension().string();
    stringToLower(&ext);
    Result res = 0;
    
    if(ext == ".charinfo" || ext == ".bin") {
        res = miiDbAddOrReplaceCharInfoFromFile(file_path.c_str());
    }
    else if(ext == ".nfif" || ext == ".dat") {
        res = miiDbImportFromFile(file_path.c_str());
    }
    else if(ext == ".coredata") {
        res = miiDbAddOrReplaceCoreDataFromFile(file_path.c_str());
    }
    else if(ext == ".storedata") {
        res = miiDbAddOrReplaceStoreDataFromFile(file_path.c_str());
    }
    else if(ext == ".jpg" || ext == ".jpeg") {
        res = importMiiQr(file_path.c_str());
    }
    else {
        return UNSUPPORTED_EXT;
    }
    return res;
}