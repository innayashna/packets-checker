#ifndef PACKETS_CHECKER_SENDER_H
#define PACKETS_CHECKER_SENDER_H

#include <string>
#include <netinet/ip.h>

constexpr int SOURCE_PORT = 8080;
constexpr int WINDOW_SIZE = 5840;
constexpr int MAX_PAYLOAD_SIZE = 4096;
constexpr int MAX_DATAGRAM_SIZE = 4096;

struct pseudo_header {
    uint32_t source_address;
    uint32_t destination_address;
    uint8_t reserved_field;
    uint8_t protocol;
    uint16_t tcp_length;
};

class Sender {
public:
    Sender();
    ~Sender();

    void setPayload(const std::string& payload);
    void setSourceIP(const std::string& sourceIP);
    void setDestinationIP(const std::string& destinationIP);
    void setDestinationPort(int port);

    void sendPacket();

private:
    static unsigned short calculateChecksum(unsigned short *ptr, int numberOfBytes);

    int senderSocket;
    char datagram[4096]{};
    std::string senderPayload;
    std::string sourceIP;
    std::string destinationIP;
    int destinationPort{};
    struct iphdr* iph;
    struct tcphdr* tcph;
    sockaddr_in sin{};

    void fillInIPHeader();
    void fillInTCPHeader();
    void fillInPseudoHeader();
    void configureSocketOptions();
    void printPacketContent(const char* packet, ssize_t dataSize);
};

#endif //PACKETS_CHECKER_SENDER_H
