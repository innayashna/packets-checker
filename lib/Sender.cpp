#include "../include/Sender.h"
#include "../include/Checksum.h"
#include "../include/SocketConfigurator.h"

#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

Sender::Sender(const std::string& senderIP, int senderPort, const std::string& proxyIP, int proxyPort) {
    initializeSenderSocket(senderPort, senderIP);

    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(proxyPort);
    proxyAddr.sin_addr.s_addr = inet_addr(proxyIP.c_str());

    iph = reinterpret_cast<struct iphdr*>(datagram);
    tcph = reinterpret_cast<struct tcphdr*>(datagram + sizeof(struct iphdr));
}

Sender::~Sender() {
    close(senderSocket);
}

void Sender::initializeSenderSocket(int senderPort, const std::string& senderIP) {
    senderSocket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);

    if (senderSocket == -1) {
        std::cerr << "[SENDER] Error creating socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    senderAddr.sin_family = AF_INET;
    senderAddr.sin_port = htons(senderPort);
    senderAddr.sin_addr.s_addr = inet_addr(senderIP.c_str());

    SocketConfigurator::configureSocketOptions(senderSocket);
}

void Sender::setPayload(const std::string& payload) {
    if (payload.length() > MAX_PAYLOAD_SIZE) {
        std::cerr << "[SENDER] Payload is too long. Maximum length is 4096 bytes." << std::endl;
        exit(1);
    }
    senderPayload = payload;
}

void Sender::fillInIPHeader() {
    std::memcpy(datagram + sizeof(struct iphdr) + sizeof(struct tcphdr),
                senderPayload.c_str(), senderPayload.length());

    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + senderPayload.length();
    iph->id = htonl(54321);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;
    iph->saddr = senderAddr.sin_addr.s_addr;
    iph->daddr = proxyAddr.sin_addr.s_addr;

    iph->check = Checksum::calculateChecksum(reinterpret_cast<unsigned short*>(iph), iph->ihl * 4);
}

void Sender::fillInTCPHeader() {
    tcph->source = senderAddr.sin_port;
    tcph->dest = proxyAddr.sin_port;
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;
    tcph->fin = 0;
    tcph->syn = 1;
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(WINDOW_SIZE);
    tcph->check = 0;
    tcph->urg_ptr = 0;
    tcph->check = Checksum::fillInPseudoHeader(datagram, iph->tot_len, iph, senderPayload);
}

void Sender::sendPacket() {
    fillInIPHeader();
    fillInTCPHeader();

    if (sendto(senderSocket, datagram, iph->tot_len, 0,
               reinterpret_cast<sockaddr*>(&proxyAddr), sizeof(proxyAddr)) < 0) {
        std::cerr << "[SENDER] Error sending packet: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        std::cout << "[SENDER] Packet send. Length : " << iph->tot_len << std::endl;
    }
}
