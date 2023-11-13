#include "Receiver.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

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
            processPacket(buffer, dataSize);
            break;
        }
    }
}

void Receiver::processPacket(char* buffer, ssize_t dataSize) {
    std::cout << "Payload: " << buffer + sizeof(struct iphdr) + sizeof(struct tcphdr) << std::endl;
}

unsigned short Receiver::calculateChecksum(unsigned short *ptr, int numberOfBytes) {
    long sum = 0;
    unsigned short oddByte;
    short answer;

    while (numberOfBytes > 1) {
        sum += *ptr++;
        numberOfBytes -= 2;
    }

    if (numberOfBytes == 1) {
        oddByte = 0;
        *reinterpret_cast<uint8_t*>(&oddByte) = *reinterpret_cast<uint8_t*>(ptr);
        sum += oddByte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = static_cast<short>(~sum);

    return htons(answer);
}



