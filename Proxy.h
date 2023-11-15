#ifndef PACKETS_CHECKER_PROXY_H
#define PACKETS_CHECKER_PROXY_H

#include <string>
#include <netinet/ip.h>
#include <netinet/tcp.h>

class Proxy {
public:
    Proxy(const std::string& proxyIP, int proxyPort, const std::string& receiverIP, int receiverPort);
    ~Proxy();

    void receivePacket(int expectedPort);

private:
    int proxySocket{};
    struct sockaddr_in proxyAddr{};
    struct sockaddr_in receiverAddr{};

    char packet[4096]{};

    struct iphdr *iph{};
    struct tcphdr *tcph{};

    void initializeProxySocket(int proxyPort, const std::string& proxyIP);
    void configureSocketOptions() const;
    void forwardPacketToReceiver(char* packet, ssize_t dataSize);
};

#endif //PACKETS_CHECKER_PROXY_H