#include "Receiver.h"
#include "Checksum.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <memory>

Receiver::Receiver(const std::string& receiverIP, int receiverPort) {
    receiverSocket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);

    if (receiverSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(receiverPort);
    receiverAddr.sin_addr.s_addr = inet_addr(receiverIP.c_str());

    iph = reinterpret_cast<struct iphdr *>(packet);
    tcph = reinterpret_cast<struct tcphdr *>(packet + sizeof(struct iphdr));
}

Receiver::~Receiver() {
    close(receiverSocket);
}

void Receiver::receivePacket(int expectedPort) {
    while (true) {
        std::memset(packet, 0, sizeof(packet));
        ssize_t dataSize = recv(receiverSocket, packet, sizeof(packet), 0);

        if (dataSize < 0) {
            std::cerr << "Error receiving packet: " << strerror(errno) << std::endl;
            exit(1);
        }

        int sourcePort = ntohs(tcph->source);
        std::cout << "Source port: " << sourcePort << std::endl;

        if (sourcePort == expectedPort) {
            std::cout << "Received data size: " << dataSize << std::endl;
            validateChecksum(packet, dataSize);
            std::cout << "Payload: " << packet + sizeof(struct iphdr) + sizeof(struct tcphdr) << std::endl;
            break;
        }
    }
}

void Receiver::validateChecksum(char* receivedPacket, ssize_t dataSize) {
    unsigned short sentChecksum = tcph->check;
    tcph->check = 0;

    unsigned short receivedChecksum = calculatePseudoHeaderChecksum(receivedPacket, dataSize);
    tcph->check = receivedChecksum;

    if (receivedChecksum == sentChecksum) {
        std::cout << "Checksum is valid. Packet was not corrupted." << std::endl;
    } else {
        std::cout << "Checksum does not match. Packet may be corrupted." << std::endl;
    }
}

unsigned short Receiver::calculatePseudoHeaderChecksum(char* receivedPacket, ssize_t dataSize) {
    pseudo_header psh{};
    psh.source_address = iph->saddr;
    psh.destination_address = iph->daddr;
    psh.reserved_field = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(dataSize - sizeof(struct iphdr));

    int payloadSize = static_cast<int>(dataSize - sizeof(struct iphdr) - sizeof(struct tcphdr));

    int pseudogramSize = static_cast<int>(sizeof(struct pseudo_header) + sizeof(struct tcphdr) + payloadSize);
    auto pseudogram = std::make_unique<char[]>(pseudogramSize);

    std::memcpy(pseudogram.get(), reinterpret_cast<char*>(&psh), sizeof(struct pseudo_header));
    std::memcpy(pseudogram.get() + sizeof(struct pseudo_header), receivedPacket + sizeof(struct iphdr),
                sizeof(struct tcphdr) + payloadSize);

    return checksumCalculator.calculateChecksum(reinterpret_cast<unsigned short*>(pseudogram.get()), pseudogramSize);
}