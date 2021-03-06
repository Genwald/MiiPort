#include "quirc.h"
#include "turbojpeg.h"
#include <ccm_3ds.h>
#include <mbedtls/error.h>
#include <switch/types.h>
#include <switch/crypto/crc.h>
#include "errors.h"
#include "mii_ext.h"
#include "QR-Code-generator/QrCode.cpp"
#include "scope_guard/scope_guard.hpp"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cctype>
#include <iomanip>
#include <sys/stat.h>

const char* QR_KEY_FILE_PATH = "/MiiPort/qrkey.txt";
const int QR_DATA_SIZE = 0x58;
const int CCM_TAG_LEN = 0x10;
typedef struct {
    u64 nonce;
    u8 enc_data[QR_DATA_SIZE];
    u8 ccm_mac[CCM_TAG_LEN];
} miiQrData;

const u32 MII_QR_KEY_CRC32 = 0xb1cdb267;
typedef struct {
    u8 key[0x10];
} miiQrKey;

int hex2int(char ch) {
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

// returns false if the buffer could not be filled
bool findBytesInText(const char* str, void* out, size_t size) {
    size_t i = 0;
    size_t bytes_found = 0;
    while(true) {
        if(str[i+1] == 0 || str[i] == 0) {
            return false;
        }
        if(isxdigit(str[i]) && isxdigit(str[i+1])) {
            ((u8*)out)[bytes_found] = (hex2int(str[i]) << 4) | hex2int(str[i+1]);
            bytes_found++;
            i++;
            if(bytes_found >= size) {
                return true;
            }
        }
        i++;
    }
}

bool getMiiKeyFromTxtFile(const char* path, miiQrKey* key_out) {
    std::string str;
    std::ifstream key_file(path);
    if(key_file.fail()) {
        key_file.close();
        return false;
    }
    std::getline(key_file, str);
    key_file.close();
    
    if(!findBytesInText(str.c_str(), key_out, sizeof(miiQrKey))) {
        return false;
    }

    if( !(crc32Calculate(key_out, sizeof(miiQrKey)) == MII_QR_KEY_CRC32) ) {
        return false;
    }
    return true;
}

int aesCcmDecrypt(const void* in, void* out, const int size, const void* nonce, const int nonce_size, const void* key, const int key_size) {
    int ret = 0;
    mbedtls_ccm_context ctx;
    mbedtls_ccm_init(&ctx);
    const auto ccm_guard = sg::make_scope_guard([ctx{&ctx}]() { mbedtls_ccm_free(ctx); });
    ret = mbedtls_ccm_setkey(&ctx,
        MBEDTLS_CIPHER_ID_AES,
        (unsigned char*)key,
        key_size * 8
    );
    if(ret) {
        char err[100] = {0};
        mbedtls_strerror(ret, err, 99);
        fprintf(stderr, "MbedTLS setkey: %s\n", err);
        return ret;
    }
    ret = mbedtls_ccm_auth_decrypt_3ds(
        &ctx,
        size,
        (unsigned char*)nonce,
        nonce_size,
        nullptr,
        0,
        (unsigned char*)in,
        (unsigned char*)out,
        (unsigned char*)in + size,
        CCM_TAG_LEN
    );
    if(ret) {
        char err[100] = {0};
        mbedtls_strerror(ret, err, 99);
        fprintf(stderr, "MbedTLS decrypt: %s\n", err);
        return ret;
    }
    return 0;
}

// out must be size+CCM_TAG_LEN for MAC
int aesCcmEncrypt(const void* in, void* out, const int size, const void* nonce, const int nonce_size, const void* key, const int key_size) {
    int ret = 0;
    mbedtls_ccm_context ctx;
    mbedtls_ccm_init(&ctx);
    const auto ccm_guard = sg::make_scope_guard([ctx{&ctx}]() { mbedtls_ccm_free(ctx); });
    ret = mbedtls_ccm_setkey(&ctx,
        MBEDTLS_CIPHER_ID_AES,
        (unsigned char*)key,
        key_size * 8
    );
    if(ret) {
        char err[100] = {0};
        mbedtls_strerror(ret, err, 99);
        fprintf(stderr, "MbedTLS setkey: %s\n", err);
        return ret;
    }
    ret = mbedtls_ccm_star_encrypt_and_tag_3ds(
        &ctx,
        size,
        (unsigned char*)nonce,
        nonce_size,
        nullptr,
        0,
        (unsigned char*)in,
        (unsigned char*)out,
        (unsigned char*)out + size,
        CCM_TAG_LEN
    );
    if(ret) {
        char err[100] = {0};
        mbedtls_strerror(ret, err, 99);
        fprintf(stderr, "MbedTLS encrypt: %s\n", err);
        return ret;
    }
    return 0;
}

Result decryptMiiQrData(miiQrData* data, ver3StoreData* out) {
    int err;
    u8 decrypted_data[QR_DATA_SIZE];
    const int nonce_size = 8;
    const int padded_nonce_size = 12;
    u8 nonce[padded_nonce_size];
    // pad nonce
    memcpy(nonce, &data->nonce, nonce_size);
    memset(nonce + nonce_size, 0, padded_nonce_size - nonce_size);

    miiQrKey key;
    if(!getMiiKeyFromTxtFile(QR_KEY_FILE_PATH, &key)){
        return BAD_KEY_FILE;
    }
    err = aesCcmDecrypt(&data->enc_data, decrypted_data, QR_DATA_SIZE, &nonce, padded_nonce_size, &key, sizeof(key));
    if(err != 0) {
        return AES_CCM_FAILED;
    }

    // insert nonce into data
    memcpy(out, decrypted_data, padded_nonce_size);
    memcpy((u8*)out + padded_nonce_size, nonce, nonce_size);
    memcpy((u8*)out + padded_nonce_size + nonce_size, decrypted_data + padded_nonce_size, QR_DATA_SIZE - padded_nonce_size);
    return 0;
}

Result encryptMiiQrData(ver3StoreData* in, miiQrData* out) {
    u8 unencrypted_data[QR_DATA_SIZE];
    const int nonce_size = 8;
    const int padded_nonce_size = 12;
    u8 nonce[padded_nonce_size];

    miiQrKey key;
    if(!getMiiKeyFromTxtFile(QR_KEY_FILE_PATH, &key)){
        return BAD_KEY_FILE;
    }
    // seperate nonce and rest of data
    memcpy(unencrypted_data, in, padded_nonce_size);
    memcpy(&out->nonce, (u8*)in + padded_nonce_size, nonce_size);
    memcpy(unencrypted_data + padded_nonce_size, (u8*)in + padded_nonce_size + nonce_size, QR_DATA_SIZE - padded_nonce_size);

    // pad nonce
    memcpy(nonce, &out->nonce, nonce_size);
    memset(nonce + nonce_size, 0, padded_nonce_size - nonce_size);

    int err = aesCcmEncrypt(unencrypted_data, &out->enc_data, QR_DATA_SIZE, &nonce, padded_nonce_size, &key, sizeof(key));
    if(err != 0) {
        return AES_CCM_FAILED;
    }
    return 0;
}

Result parseMiiQr(const char* path, ver3StoreData* out_mii) {
    struct quirc *qr;
    int w, h, subsamp, colorspace, err = 0;

    struct stat st;
    stat(path, &st);
    size_t jpg_size = st.st_size;
    std::unique_ptr<u8> jpg_data(new u8[jpg_size]);

    FILE* jpg_file = fopen(path, "r");
    fread(jpg_data.get(), 1, jpg_size, jpg_file);
    fclose(jpg_file);

    tjhandle handle = tjInitDecompress();
    if(handle == nullptr) {
        printf("tjInitDecompress failed\n");
        return JPEG_DECODE_FAIL;
    }
    const auto tj_init_guard = sg::make_scope_guard([handle]() { tjDestroy(handle); });

    err = tjDecompressHeader3(handle, jpg_data.get(), jpg_size, &w, &h, &subsamp, &colorspace);
    if(err != 0) {
        printf("tjDecompressHeader3 error\n");
        return JPEG_DECODE_FAIL;
    }

    qr = quirc_new();
    const auto quirc_new_guard = sg::make_scope_guard([qr]() { quirc_destroy(qr); });
    if (!qr) {
        printf("Failed to allocate memory for QR\n");
        return QR_DECODE_FAIL;
    }
    if (quirc_resize(qr, w, h) < 0) {
        printf("Failed to allocate memory for QR resize\n");
        return QR_DECODE_FAIL;
    }

    {
        int w, h;
        u8 *image = quirc_begin(qr, &w, &h);
        err = tjDecompress2(handle, jpg_data.get(), jpg_size, image, w, w, h, TJPF_GRAY, TJFLAG_ACCURATEDCT);
        quirc_end(qr);
        if(err != 0) {
            printf("tjDecompress2 error\n");
            return JPEG_DECODE_FAIL;
        }
    }

    if (quirc_count(qr) == 0) {
        return NO_QR;
    }
    else {
        struct quirc_code code;
        struct quirc_data data;
        quirc_decode_error_t err;

        quirc_extract(qr, 0, &code);

        err = quirc_decode(&code, &data);
        if (err != 0) {
            printf("QR decode failed: %s\n", quirc_strerror(err));
            return QR_DECODE_FAIL;
        }
        else {        
            Result res = decryptMiiQrData((miiQrData*)data.payload, out_mii);
            if(R_FAILED(res)) {
                return res;
            }
        }
    }

    return 0;
}

std::unique_ptr<u32[]> generateQrRGBA(u8 *data, size_t data_size, u32 scale, int* out_width) {
    using qrcodegen::QrCode;
    std::vector<u8> data_vec(data, data+data_size);
    const QrCode qr = QrCode::encodeBinary(data_vec, QrCode::Ecc::HIGH);
    int qr_size = qr.getSize();
    const int border = 2;
    int width = border*2 + qr_size;
    *out_width = width*scale;
    size_t arr_size = *out_width * *out_width;
    std::unique_ptr<u32[]> out_data(new u32[arr_size]);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < width; y++) {
            bool black_square = qr.getModule(x-border, y-border);
            for (u32 scale_x=0; scale_x<scale; scale_x++) {
                for (u32 scale_y=0; scale_y<scale; scale_y++) {
                    size_t pos = (y*scale+scale_y)*width*scale + (x*scale+scale_x);
                    if(pos < arr_size) {
                        if(black_square) {
                            // black
                            out_data[pos] = 0xFF000000;
                        }
                        else {
                            // white
                            out_data[pos] = 0xFFFFFFFF;
                        }
                    }
                    else {
                        printf("qr tried to write out of bounds\n");
                    }
                }
            }
        }
	}
    return out_data;
}

Result generateMiiQr(ver3StoreData* in, u32 scale, int* out_width, std::unique_ptr<u32[]> &out) {
    Result ret;
    miiQrData data;
    ret = encryptMiiQrData(in, &data);
    if(R_SUCCEEDED(ret)) {
        out = generateQrRGBA((u8*)&data, sizeof(miiQrData), scale, out_width);
    }
    return ret;
}