#include "Proxy.h"
#include "Checksum.h"
#include "SocketConfigurator.h"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <netinet/ip.h>
#include <arpa/inet.h>

Proxy::Proxy(const std::string& proxyIP, int proxyPort, const std::string& receiverIP, int receiverPort) {
    initializeProxySocket(proxyPort, proxyIP);

    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(receiverPort);
    receiverAddr.sin_addr.s_addr = inet_addr(receiverIP.c_str());

    iph = reinterpret_cast<struct iphdr *>(packet);
    tcph = reinterpret_cast<struct tcphdr *>(packet + sizeof(struct iphdr));
    payload =  packet + sizeof(struct iphdr) + sizeof(struct tcphdr);
}

Proxy::~Proxy() {
    close(proxySocket);
}

void Proxy::initializeProxySocket(int proxyPort, const std::string& proxyIP) {
    proxySocket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);

    if (proxySocket == -1) {
        std::cerr << "Error creating proxy socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(proxyPort);
    proxyAddr.sin_addr.s_addr = inet_addr(proxyIP.c_str());

    SocketConfigurator::configureSocketOptions(proxySocket);
}

void Proxy::setForwardFlag(const std::string &flag) {
    forwardFlag = flag;
}

void Proxy::receivePacket(int expectedPort) {
    while (true) {
        std::memset(packet, 0, sizeof(packet));
        ssize_t dataSize = recv(proxySocket, packet, sizeof(packet), 0);

        if (dataSize < 0) {
            std::cerr << "Error receiving packet from sender: " << strerror(errno) << std::endl;
            exit(1);
        }

        int sourcePort = ntohs(tcph->source);

        if (sourcePort == expectedPort) {
            forwardPacketToReceiver(packet, dataSize);
            break;
        }
    }
}

void Proxy::forwardPacketToReceiver(char* receivedPacket, ssize_t dataSize) {
    tcph->source = proxyAddr.sin_port;
    tcph->dest = receiverAddr.sin_port;

    std::memcpy(receivedPacket + sizeof(struct iphdr), tcph, sizeof(struct tcphdr));

    tcph->check = 0;

    unsigned short newChecksum = Checksum::fillInPseudoHeader(receivedPacket, dataSize, iph, payload);
    tcph->check = newChecksum;

    if(forwardFlag == "MODIFY") {
        dataSize = modifyPayload(receivedPacket);
    }

    sendPacket(receivedPacket, dataSize);
}

ssize_t Proxy::modifyPayload(char* receivedPacket) {
    std::string payload(receivedPacket + sizeof(struct iphdr) + sizeof(struct tcphdr));
    std::string modifiedPayload = payload + " (modified)";

    std::memcpy(receivedPacket + sizeof(struct iphdr) + sizeof(struct tcphdr),
                modifiedPayload.c_str(), modifiedPayload.length());

    return static_cast<int>(sizeof(struct iphdr) + sizeof(struct tcphdr) + modifiedPayload.length());
}

void Proxy::sendPacket(char* receivedPacket, ssize_t dataSize) {
    if (sendto(proxySocket, receivedPacket, dataSize, 0,
               reinterpret_cast<struct sockaddr*>(&receiverAddr), sizeof(receiverAddr)) < 0) {
        std::cerr << "Error forwarding packet to receiver: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        std::cout << "Packet forwarded to receiver. Length: " << dataSize << std::endl;
    }
}