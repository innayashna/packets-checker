#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <memory>
#include <thread>

// pseudo header needed for tcp header checksum calculation
struct pseudo_header {
    uint32_t source_address;
    uint32_t destination_address;
    uint8_t reserved_field;
    uint8_t protocol;
    uint16_t tcp_length;
};

// checksum calculation
unsigned short calculateChecksum(unsigned short *ptr, int numberOfBytes) {
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

    return answer;
}

int main() {
    // create a raw socket
    int senderSocket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);

    if (senderSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    // datagram to represent the packet
    char datagram[4096], source_ip[32], *data;
    std::unique_ptr<char[]> pseudogram;

    std::memset(datagram, 0, 4096);

    auto iph = reinterpret_cast<struct iphdr*>(datagram);
    auto tcph = reinterpret_cast<struct tcphdr*>(datagram + sizeof(struct ip));

    sockaddr_in sin{};
    pseudo_header psh{};

    data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);
    std::strcpy(data, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    // address resolution
    std::strcpy(source_ip, "192.168.1.2");
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr("1.2.3.4");

    // fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + std::strlen(data);
    iph->id = htonl(54321);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;
    iph->saddr = inet_addr(source_ip);
    iph->daddr = sin.sin_addr.s_addr;

    iph->check = calculateChecksum(reinterpret_cast<unsigned short*>(datagram), iph->tot_len);

    // fill in the TCP Header
    tcph->source = htons(1234);
    tcph->dest = htons(80);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;
    tcph->fin = 0;
    tcph->syn = 1;
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(5840);
    tcph->check = 0;
    tcph->urg_ptr = 0;

    // calculate TCP checksum
    psh.source_address = inet_addr(source_ip);
    psh.destination_address = sin.sin_addr.s_addr;
    psh.reserved_field = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + std::strlen(data));

    int pseudogramSize = static_cast<int>(sizeof(struct pseudo_header) + sizeof(struct tcphdr) + std::strlen(data));
    pseudogram = std::make_unique<char[]>(pseudogramSize);

    std::memcpy(pseudogram.get(), reinterpret_cast<char*>(&psh), sizeof(struct pseudo_header));
    std::memcpy(pseudogram.get() + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr) + std::strlen(data));

    tcph->check = calculateChecksum(reinterpret_cast<unsigned short*>(pseudogram.get()), pseudogramSize);

    //IP_HDRINCL to tell the kernel that headers are included in the packet
    int one = 1;
    const int *val = &one;

    // send packet
    if (setsockopt(senderSocket, IPPROTO_IP, IP_HDRINCL, val, sizeof(one) < 0)) {
        std::cerr << "Error setting IP_HDRINCL option: " << strerror(errno) << std::endl;
        close(senderSocket);
        exit(1);
    }

    if (sendto(senderSocket, datagram, iph->tot_len, 0, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) < 0) {
        std::cerr << "Error sending packet: " << strerror(errno) << std::endl;
        close(senderSocket);
        exit(1);
    } else {
        std::cout << "Packet Send. Length : " << iph->tot_len << std::endl;
    }

    close(senderSocket);
    return 0;
}
