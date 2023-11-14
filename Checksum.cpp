#include "Checksum.h"
#include <arpa/inet.h>

unsigned short Checksum::calculateChecksum(unsigned short *ptr, unsigned int count) {
    uint32_t sum = 0;
    while (count > 1) {
        sum += *ptr++;
        count -= 2;
    }

    if (count > 0) {
        sum += ((*ptr) & htons(0xFF00));
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    sum = ~sum;
    return static_cast<unsigned short>(sum);
}