#ifndef PACKETS_CHECKER_RECEIVER_H
#define PACKETS_CHECKER_RECEIVER_H

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string>

class Receiver {
public:
    Receiver();
    ~Receiver();

    void receivePacket(int expectedPort);

private:
    int receiverSocket;
    struct sockaddr_in sin{};
    char buffer[4096]{};
    struct iphdr *iph;
    struct tcphdr *tcph;

    void processPacket(char *buffer, ssize_t dataSize);
    unsigned short calculateChecksum(unsigned short *ptr, int numberOfBytes);
};

#endif //PACKETS_CHECKER_RECEIVER_H
