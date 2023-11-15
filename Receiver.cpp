#include "Receiver.h"
#include "Checksum.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

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
        int destinationPort = ntohs(tcph->dest);

        if (sourcePort == expectedPort && destinationPort == ntohs(receiverAddr.sin_port)) {
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

    unsigned short receivedChecksum = Checksum::recalculateChecksum(receivedPacket, dataSize, iph);
    tcph->check = receivedChecksum;

    if (receivedChecksum == sentChecksum) {
        std::cout << "Checksum is valid. Packet was not corrupted." << std::endl;
    } else {
        std::cout << "Checksum does not match. Packet may be corrupted." << std::endl;
    }
}
