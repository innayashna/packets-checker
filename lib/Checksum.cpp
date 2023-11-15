#include "../include/Checksum.h"

#include <cstring>
#include <memory>
#include <netinet/tcp.h>
#include <netinet/ip.h>

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

unsigned short Checksum::fillInPseudoHeader(char* packet, ssize_t dataSize, iphdr* iph, const std::string& payload) {
    pseudo_header psh{};
    psh.source_address = iph->saddr;
    psh.destination_address = iph->daddr;
    psh.reserved_field = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(dataSize - sizeof(struct iphdr));

    int pseudogramSize = static_cast<int>(sizeof(struct pseudo_header) + sizeof(struct tcphdr) + payload.length());
    auto pseudogram = std::make_unique<char[]>(pseudogramSize);

    std::memcpy(pseudogram.get(), reinterpret_cast<char*>(&psh), sizeof(struct pseudo_header));
    std::memcpy(pseudogram.get() + sizeof(struct pseudo_header), packet + sizeof(struct iphdr),
                sizeof(struct tcphdr) + payload.length());

    return Checksum::calculateChecksum(reinterpret_cast<unsigned short*>(pseudogram.get()), pseudogramSize);
}