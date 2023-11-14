#ifndef PACKETS_CHECKER_RECEIVER_H
#define PACKETS_CHECKER_RECEIVER_H

#include "Checksum.h"

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string>

class Receiver {
public:
    Receiver();
    ~Receiver();

    void receivePacket(int expectedPort);

private:
    Checksum checksumCalculator;
    int receiverSocket;
    struct sockaddr_in sin{};
    char buffer[4096]{};
    struct iphdr *iph;
    struct tcphdr *tcph;

    void validateChecksum(char *buffer, ssize_t dataSize);
};

#endif //PACKETS_CHECKER_RECEIVER_H
