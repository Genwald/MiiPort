
int crc16(void *ptr, size_t len, int crc) {
    char *addr = (char*)ptr;
    const int poly = 0x1021;
    int i;
    for (; len>0; len--) {
        crc = crc ^ (*addr++ << 8);
        for (i=0; i<8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ poly;
            }
            else {
                crc = crc << 1;
            }
        }
    } 
    return(crc);
}

int crc16(void *ptr, size_t len) {
    return crc16(ptr, len, 0);
}

int crc16LE(void *ptr, size_t len, int crc) {
    return __builtin_bswap16(crc16(ptr, len, crc));
}

int crc16LE(void *ptr, size_t len) {
    return __builtin_bswap16(crc16(ptr, len, 0));
}