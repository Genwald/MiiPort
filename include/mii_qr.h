#include "quirc.h"
#include "jpeglib.h"
#include "turbojpeg.h"
#include <mbedtls/config.h>
#include <mbedtls/cipher.h>
#include <mbedtls/ccm.h>
#include <mbedtls/error.h>
#include <switch/types.h>


#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sys/stat.h>

const u32 MII_QR_KEY_SIZE = 0x10;
const u8 MII_QR_KEY[MII_QR_KEY_SIZE] = {REMOVED};

void aesCcmDecrypt(const void* in, void* out, int size, const void* nonce, int nonce_size) {
    int ret = 0;
    mbedtls_ccm_context ctx;
    mbedtls_ccm_init(&ctx);
    ret = mbedtls_ccm_setkey(&ctx,
        MBEDTLS_CIPHER_ID_AES,
        MII_QR_KEY,
        MII_QR_KEY_SIZE * 8
    );
    if (ret) {
        char err[100] = {0};
        mbedtls_strerror(ret, err, 99);
        fprintf(stderr, "MbedTLS setkey: %s\n", err);
        return;
    }
    ret = mbedtls_ccm_star_auth_decrypt(&ctx,
        size,
        (unsigned char*)nonce,
        nonce_size,
        nullptr,
        0,
        (unsigned char*)in,
        (unsigned char*)out,
        nullptr,
        0
    );
    if (ret) {
        char err[100] = {0};
        mbedtls_strerror(ret, err, 99);
        fprintf(stderr, "MbedTLS decrypt: %s\n", err);
    }
}

const int QR_DATA_SIZE = 0x58;
const int CCM_MAC_SIZE = 0x10;
typedef struct {
    u64 nonce;
    u8 enc_data[QR_DATA_SIZE];
    u8 ccm_mac[CCM_MAC_SIZE];
} miiQrData;

void decyptMiiQrData(miiQrData* data, ver3StoreData* out) {
    
    u8 decrypted_data[QR_DATA_SIZE];
    const int nonce_size = 8;
    const int padded_nonce_size = 12;
    u8 nonce[padded_nonce_size];
    // pad nonce
    memcpy(nonce, &data->nonce, nonce_size);
    memset(nonce + nonce_size, 0, padded_nonce_size - nonce_size);

    aesCcmDecrypt(&data->enc_data, decrypted_data, QR_DATA_SIZE, &nonce, padded_nonce_size);

    // insert nonce into data
    memcpy(out, decrypted_data, padded_nonce_size);
    memcpy((u8*)out + padded_nonce_size, nonce, nonce_size);
    memcpy((u8*)out + padded_nonce_size + nonce_size, decrypted_data + padded_nonce_size, QR_DATA_SIZE - padded_nonce_size);
}

void parseMiiQr(const char* path, ver3StoreData* out_mii) {
    struct quirc *qr;
    int w, h, subsamp, colorspace, err = 0;

    struct stat st;
    stat(path, &st);
    size_t jpg_size = st.st_size;
    u8 *jpg_data = new u8[jpg_size];

    FILE* jpg_file = fopen(path, "r");
    fread(jpg_data, 1, jpg_size, jpg_file);

    tjhandle handle = tjInitDecompress();
    if(handle == nullptr) {

    }
    err = tjDecompressHeader3(handle, jpg_data, jpg_size, &w, &h, &subsamp, &colorspace);

    qr = quirc_new();
    if (!qr) {
        printf("Failed to allocate memory");
        return;
    }
    if (quirc_resize(qr, w, h) < 0) {
        printf("Failed to allocate memory");
        return;
    }

    {
        int w, h;

        u8 *image = quirc_begin(qr, &w, &h);

        err = tjDecompress2(handle, jpg_data, jpg_size, image, w, w, h, TJPF_GRAY, TJFLAG_ACCURATEDCT);

        quirc_end(qr);
    }

    delete[] jpg_data;
    err = tjDestroy(handle);

    if (quirc_count(qr) > 0) {
        struct quirc_code code;
        struct quirc_data data;
        quirc_decode_error_t err;

        quirc_extract(qr, 0, &code);

        err = quirc_decode(&code, &data);
        if (err != 0) {
            printf("DECODE FAILED: %s\n", quirc_strerror(err));
        }
        else {
            decyptMiiQrData((miiQrData*)data.payload, out_mii);
        }
    }

    quirc_destroy(qr);
}