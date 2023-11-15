#include "../include/Receiver.h"
#include "../include/Checksum.h"

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
        std::cerr << "[RECEIVER] Error creating socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(receiverPort);
    receiverAddr.sin_addr.s_addr = inet_addr(receiverIP.c_str());

    iph = reinterpret_cast<struct iphdr *>(packet);
    tcph = reinterpret_cast<struct tcphdr *>(packet + sizeof(struct iphdr));
    payload =  packet + sizeof(struct iphdr) + sizeof(struct tcphdr);
}

Receiver::~Receiver() {
    close(receiverSocket);
}

void Receiver::receivePacket(int expectedPort) {
    while (true) {
        std::memset(packet, 0, sizeof(packet));
        ssize_t dataSize = recv(receiverSocket, packet, sizeof(packet), 0);

        if (dataSize < 0) {
            std::cerr << "[RECEIVER] Error receiving packet: " << strerror(errno) << std::endl;
            exit(1);
        }

        int sourcePort = ntohs(tcph->source);
        int destinationPort = ntohs(tcph->dest);

        if (sourcePort == expectedPort && destinationPort == ntohs(receiverAddr.sin_port)) {
            std::cout << "[RECEIVER] Received data size: " << dataSize << std::endl;
            validateChecksum(packet, dataSize);
            std::cout << "[RECEIVER] Payload: " << packet + sizeof(struct iphdr) + sizeof(struct tcphdr) << std::endl;
            break;
        }
    }
}

void Receiver::validateChecksum(char* receivedPacket, ssize_t dataSize) {
    unsigned short receivedChecksum = tcph->check;
    tcph->check = 0;

    unsigned short recalculatedChecksum = Checksum::fillInPseudoHeader(receivedPacket, dataSize, iph, payload);
    tcph->check = recalculatedChecksum;

    if (receivedChecksum == recalculatedChecksum) {
        std::cout << "[RECEIVER] Checksum is valid. Payload was not changed by proxy." << std::endl;
    } else {
        std::cout << "[RECEIVER] Checksum does not match. Payload was changed by proxy." << std::endl;
    }
}
