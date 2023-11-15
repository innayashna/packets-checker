#ifndef PACKETS_CHECKER_SENDER_H
#define PACKETS_CHECKER_SENDER_H

#include <string>
#include <netinet/ip.h>

constexpr int WINDOW_SIZE = 5840;
constexpr int MAX_PAYLOAD_SIZE = 4096;

class Sender {
public:
    Sender(const std::string& senderIP, int senderPort, const std::string& proxyIP, int proxyPort);
    ~Sender();

    void setPayload(const std::string& payload);
    void sendPacket();

private:
    int senderSocket{};
    sockaddr_in senderAddr{};
    sockaddr_in proxyAddr{};

    std::string senderPayload;
    char datagram[4096]{};

    struct iphdr* iph;
    struct tcphdr* tcph;

    void initializeSenderSocket(int senderPort, const std::string& senderIP);
    void fillInIPHeader();
    void fillInTCPHeader();
};

#endif //PACKETS_CHECKER_SENDER_H