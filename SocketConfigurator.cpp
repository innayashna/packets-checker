#include "SocketConfigurator.h"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <netinet/ip.h>

void SocketConfigurator::configureSocketOptions(int socket) {
    int one = 1;
    const int *val = &one;

    if (setsockopt(socket, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        std::cerr << "Error setting IP_HDRINCL option: " << strerror(errno) << std::endl;
        exit(1);
    }
}