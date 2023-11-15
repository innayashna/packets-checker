#ifndef PACKETS_CHECKER_CHECKSUM_H
#define PACKETS_CHECKER_CHECKSUM_H

#include <cstdint>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string>

struct pseudo_header {
    uint32_t source_address;
    uint32_t destination_address;
    uint8_t reserved_field;
    uint8_t protocol;
    uint16_t tcp_length;
};

class Checksum {
public:
    static unsigned short calculateChecksum(unsigned short *ptr, unsigned int count);
    static unsigned short fillInPseudoHeader(char* packet, ssize_t dataSize, iphdr* iph, const std::string& payload);
};

#endif //PACKETS_CHECKER_CHECKSUM_H
