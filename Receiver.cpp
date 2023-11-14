#include "Receiver.h"
#include "Checksum.h"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <memory>

Receiver::Receiver() {
    receiverSocket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);

    if (receiverSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(8080);
    sin.sin_addr.s_addr = INADDR_ANY;

    iph = reinterpret_cast<struct iphdr *>(buffer);
    tcph = reinterpret_cast<struct tcphdr *>(buffer + sizeof(struct iphdr));
}

Receiver::~Receiver() {
    close(receiverSocket);
}

void Receiver::receivePacket(int expectedPort) {
    while (true) {
        std::memset(buffer, 0, 4096);
        ssize_t dataSize = recv(receiverSocket, buffer, sizeof(buffer), 0);

        if (dataSize < 0) {
            std::cerr << "Error receiving packet: " << strerror(errno) << std::endl;
            exit(1);
        }

        int sourcePort = ntohs(tcph->source);

        if (sourcePort == expectedPort) {
            validateChecksum(buffer, dataSize);
            std::cout << "Payload: " << buffer + sizeof(struct iphdr) + sizeof(struct tcphdr) << std::endl;
            break;
        }
    }
}

void Receiver::validateChecksum(char* buffer, ssize_t dataSize) {
    unsigned short sentChecksum = tcph->check;

    pseudo_header psh{};
    psh.source_address = iph->saddr;
    psh.destination_address = iph->daddr;
    psh.reserved_field = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(dataSize - sizeof(struct iphdr));

    int payloadSize = dataSize - sizeof(struct iphdr) - sizeof(struct tcphdr);

    int pseudogramSize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + payloadSize;
    auto pseudogram = std::make_unique<char[]>(pseudogramSize);

    std::memcpy(pseudogram.get(), reinterpret_cast<char*>(&psh), sizeof(struct pseudo_header));
    std::memcpy(pseudogram.get() + sizeof(struct pseudo_header), buffer + sizeof(struct iphdr),
                sizeof(struct tcphdr) + payloadSize);

    unsigned short receivedChecksum = checksumCalculator.calculateChecksum(
            reinterpret_cast<unsigned short*>(pseudogram.get()), pseudogramSize);

    std::cout << "Sent checksum: " << sentChecksum << std::endl;
    std::cout << "Received checksum: " << receivedChecksum << std::endl;

    if (receivedChecksum == sentChecksum) {
        std::cout << "Checksum is valid. Packet was not corrupted." << std::endl;
    } else {
        std::cout << "Checksum does not match. Packet may be corrupted." << std::endl;
    }
}



