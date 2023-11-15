#include "Sender.h"
#include "Proxy.h"
#include "Receiver.h"

#include <iostream>

int main() {
    constexpr int SENDER_PORT = 8080;
    constexpr int PROXY_PORT = 8081;
    constexpr int RECEIVER_PORT = 8082;
    constexpr const char* LOCAL_IP = "127.0.0.1";

    Sender sender(LOCAL_IP, SENDER_PORT, LOCAL_IP, PROXY_PORT);
    Proxy proxy(LOCAL_IP, PROXY_PORT, LOCAL_IP, RECEIVER_PORT);
    Receiver receiver(LOCAL_IP, RECEIVER_PORT);

    std::string payload;
    std::string forwardFlag;

    std::cout << "Choose proxy forwarding option: " << std::endl;
    std::cout << "1. Proxy just forwards data to the receiver - FORWARD" << std::endl;
    std::cout << "2. Proxy does modifications in payload - MODIFY" << std::endl;
    std::cout << "Type your option here >> ";

    std::getline(std::cin, forwardFlag);
    proxy.setForwardFlag(forwardFlag);

    std::cout << "Enter the payload data: ";
    std::getline(std::cin, payload);
    sender.setPayload(payload);

    sender.sendPacket();
    proxy.receivePacket(SENDER_PORT);
    receiver.receivePacket(PROXY_PORT);

    return 0;
}