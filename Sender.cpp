#include "Sender.h"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <memory>

Sender::Sender() {
    senderSocket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);

    if (senderSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    std::memset(datagram, 0, MAX_DATAGRAM_SIZE);
    iph = reinterpret_cast<struct iphdr*>(datagram);
    tcph = reinterpret_cast<struct tcphdr*>(datagram + sizeof(struct iphdr));
}

Sender::~Sender() {
    close(senderSocket);
}

void Sender::setPayload(const std::string& payload) {
    if (payload.length() > MAX_PAYLOAD_SIZE) {
        std::cerr << "Payload is too long. Maximum length is 4096 bytes." << std::endl;
        exit(1);
    }
    senderPayload = payload;
}

void Sender::setSourceIP(const std::string& newSourceIP) {
    sourceIP = newSourceIP;
}

void Sender::setDestinationIP(const std::string& newDestinationIP) {
    destinationIP = newDestinationIP;
}

void Sender::setDestinationPort(int port) {
    destinationPort = port;
}

unsigned short Sender::calculateChecksum(unsigned short *ptr, int numberOfBytes) {
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

void Sender::fillInIPHeader() {
    std::memcpy(datagram + sizeof(struct iphdr) + sizeof(struct tcphdr),
                senderPayload.c_str(), senderPayload.length());

    sin.sin_family = AF_INET;
    sin.sin_port = htons(destinationPort);
    sin.sin_addr.s_addr = inet_addr(destinationIP.c_str());

    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + senderPayload.length();
    iph->id = htonl(54321);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;
    iph->saddr = inet_addr(sourceIP.c_str());
    iph->daddr = sin.sin_addr.s_addr;

    iph->check = calculateChecksum(reinterpret_cast<unsigned short*>(datagram), iph->tot_len);
}

void Sender::fillInTCPHeader() {
    tcph->source = htons(SOURCE_PORT);
    tcph->dest = htons(destinationPort);
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
}

void Sender::fillInPseudoHeader() {
    pseudo_header psh{};
    psh.source_address = inet_addr(sourceIP.c_str());
    psh.destination_address = inet_addr(destinationIP.c_str());
    psh.reserved_field = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + senderPayload.length());

    int pseudogramSize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + senderPayload.length();
    auto pseudogram = std::make_unique<char[]>(pseudogramSize);

    std::memcpy(pseudogram.get(), reinterpret_cast<char*>(&psh), sizeof(struct pseudo_header));
    std::memcpy(pseudogram.get() + sizeof(struct pseudo_header), datagram + sizeof(struct iphdr),
            sizeof(struct tcphdr) + senderPayload.length());

    tcph->check = calculateChecksum(reinterpret_cast<unsigned short*>(pseudogram.get()), pseudogramSize);
}

void Sender::sendPacket() {
    fillInIPHeader();
    fillInTCPHeader();
    fillInPseudoHeader();

    configureSocketOptions();

    if (sendto(senderSocket, datagram, iph->tot_len, 0,
               reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) < 0) {
        std::cerr << "Error sending packet: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        std::cout << "Packet Send. Length : " << iph->tot_len << std::endl;
    }
}

void Sender::configureSocketOptions() {
    int one = 1;
    const int *val = &one;

    if (setsockopt(senderSocket, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        std::cerr << "Error setting IP_HDRINCL option: " << strerror(errno) << std::endl;
        exit(1);
    }
}



