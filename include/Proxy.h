#ifndef PACKETS_CHECKER_PROXY_H
#define PACKETS_CHECKER_PROXY_H

#include <string>
#include <netinet/ip.h>
#include <netinet/tcp.h>

typedef enum {
    FORWARD = 1,
    MODIFY
} InterceptionOption;

class Proxy {
public:
    Proxy(const std::string& proxyIP, int proxyPort, const std::string& receiverIP, int receiverPort);
    ~Proxy();

    void setInterceptionOption(InterceptionOption option);
    void receivePacket(int expectedPort);

private:
    int proxySocket{};
    struct sockaddr_in proxyAddr{};
    struct sockaddr_in receiverAddr{};

    char packet[4096]{};
    struct iphdr *iph{};

    struct tcphdr *tcph{};
    std::string payload;

    InterceptionOption interceptionOption;

    void initializeProxySocket(int proxyPort, const std::string& proxyIP);
    void forwardPacketToReceiver(char* packet, ssize_t dataSize);
    void sendPacket(char* receivedPacket, ssize_t dataSize);
    static ssize_t modifyPayload(char* receivedPacket);
};

#endif //PACKETS_CHECKER_PROXY_H