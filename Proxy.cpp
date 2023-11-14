#include "Proxy.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <netinet/ip.h>
#include <arpa/inet.h>

Proxy::Proxy(const std::string& receiverIP, int receiverPort, int proxyPort) {
    initializeProxySocket(proxyPort);

    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(receiverPort);
    receiverAddr.sin_addr.s_addr = inet_addr(receiverIP.c_str());

    iph = reinterpret_cast<struct iphdr *>(buffer);
    tcph = reinterpret_cast<struct tcphdr *>(buffer + sizeof(struct iphdr));
}

Proxy::~Proxy() {
    close(proxySocket);
}

void Proxy::initializeProxySocket(int proxyPort) {
    proxySocket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);

    if (proxySocket == -1) {
        std::cerr << "Error creating proxy socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(proxyPort);
    proxyAddr.sin_addr.s_addr = INADDR_ANY;
}

void Proxy::receivePacket(int expectedPort) {
    while (true) {
        std::memset(buffer, 0, sizeof(buffer));
        ssize_t dataSize = recv(proxySocket, buffer, sizeof(buffer), 0);

        if (dataSize < 0) {
            std::cerr << "Error receiving packet from sender: " << strerror(errno) << std::endl;
            exit(1);
        }

        int sourcePort = ntohs(tcph->source);

        if (sourcePort == expectedPort) {
            forwardPacketToReceiver(buffer, dataSize);
            break;
        }
    }
}

void Proxy::forwardPacketToReceiver(char* packet, ssize_t dataSize) {
    if (sendto(proxySocket, packet, dataSize, 0,
               reinterpret_cast<struct sockaddr*>(&receiverAddr), sizeof(receiverAddr)) < 0) {
        std::cerr << "Error forwarding packet to receiver: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        std::cout << "Packet forwarded to receiver. Length: " << dataSize << std::endl;
    }
}