#include "Proxy.h"
#include "Checksum.h"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <memory>

Proxy::Proxy(const std::string& proxyIP, int proxyPort, const std::string& receiverIP, int receiverPort) {
    initializeProxySocket(proxyPort, proxyIP);

    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(receiverPort);
    receiverAddr.sin_addr.s_addr = inet_addr(receiverIP.c_str());

    iph = reinterpret_cast<struct iphdr *>(packet);
    tcph = reinterpret_cast<struct tcphdr *>(packet + sizeof(struct iphdr));
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

    configureSocketOptions();
}

void Proxy::configureSocketOptions() const {
    int one = 1;
    const int *val = &one;

    if (setsockopt(proxySocket, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        std::cerr << "Error setting IP_HDRINCL option: " << strerror(errno) << std::endl;
        exit(1);
    }
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

    unsigned short newPortsChecksum = calculateChecksumWithNewPorts(receivedPacket, dataSize);
    tcph->check = newPortsChecksum;

    if(forwardFlag == "MODIFY") {
        dataSize = modifyPayload(receivedPacket);
    }

    if (sendto(proxySocket, receivedPacket, dataSize, 0,
               reinterpret_cast<struct sockaddr*>(&receiverAddr), sizeof(receiverAddr)) < 0) {
        std::cerr << "Error forwarding packet to receiver: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        std::cout << "Packet forwarded to receiver. Length: " << dataSize << std::endl;
    }
}

unsigned short Proxy::calculateChecksumWithNewPorts(char* receivedPacket, ssize_t dataSize) {
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

    return Checksum::calculateChecksum(reinterpret_cast<unsigned short*>(pseudogram.get()), pseudogramSize);
}

ssize_t Proxy::modifyPayload(char* receivedPacket) {
    std::string payload(receivedPacket + sizeof(struct iphdr) + sizeof(struct tcphdr));
    std::string modifiedPayload = payload + " (modified)";

    std::memcpy(receivedPacket + sizeof(struct iphdr) + sizeof(struct tcphdr),
                modifiedPayload.c_str(), modifiedPayload.length());

    return static_cast<int>(sizeof(struct iphdr) + sizeof(struct tcphdr) + modifiedPayload.length());
}