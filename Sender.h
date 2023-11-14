#ifndef PACKETS_CHECKER_SENDER_H
#define PACKETS_CHECKER_SENDER_H

#include "Checksum.h"

#include <string>
#include <netinet/ip.h>

constexpr int SOURCE_PORT = 8080;
constexpr int WINDOW_SIZE = 5840;
constexpr int MAX_PAYLOAD_SIZE = 4096;
constexpr int MAX_DATAGRAM_SIZE = 4096;

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
    Checksum checksumCalculator;
    int senderSocket;
    char datagram[4096]{};
    std::string senderPayload;
    std::string sourceIP;
    std::string destinationIP;
    int destinationPort{};
    struct iphdr* iph;
    struct tcphdr* tcph;
    struct sockaddr_in senderAddr;
    sockaddr_in receiverAddr{};

    void fillInIPHeader();
    void fillInTCPHeader();
    void fillInPseudoHeader();
    void configureSocketOptions();
    void printPacketContent(const char* packet, ssize_t dataSize);
};

#endif //PACKETS_CHECKER_SENDER_H