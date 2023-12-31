#ifndef PACKETS_CHECKER_RECEIVER_H
#define PACKETS_CHECKER_RECEIVER_H

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string>

class Receiver {
public:
    Receiver(const std::string& proxyIP, int proxyPort);
    ~Receiver();

    void receivePacket(int expectedPort);

private:
    int receiverSocket;
    struct sockaddr_in receiverAddr{};

    char packet[4096]{};

    struct iphdr *iph;
    struct tcphdr *tcph;
    std::string payload;

    void validateChecksum(char *receivedPacket, ssize_t dataSize);
};

#endif //PACKETS_CHECKER_RECEIVER_H