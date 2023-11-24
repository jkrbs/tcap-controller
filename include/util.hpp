#pragma once
#include <cstdint>

namespace util {
    struct IpAddress {
        uint8_t address[4];
        uint8_t netmask[4];
        uint16_t port;
    };

    /*
    Stolen from https://stackoverflow.com/questions/29242/off-the-shelf-c-hex-dump-code
    */
    static void hexdump(void *ptr, int buflen) {
        unsigned char *buf = (unsigned char*)ptr;
        int i, j;
        for (i=0; i<buflen; i+=16) {
            printf("%06x: ", i);
            for (j=0; j<16; j++) 
            if (i+j < buflen)
                printf("%02x ", buf[i+j]);
            else
                printf("   ");
            printf(" ");
            for (j=0; j<16; j++) 
            if (i+j < buflen)
                printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
            printf("\n");
        }
    }

}