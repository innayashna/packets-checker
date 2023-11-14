#include "Sender.h"
#include "Receiver.h"
#include "Proxy.h"
#include <iostream>

int main() {
    Sender sender;
    Receiver receiver;
    Proxy proxy("127.0.0.1", 8082, 8081);

    std::string payload;
    std::cout << "Enter the payload data: ";
    std::getline(std::cin, payload);
    sender.setPayload(payload);

    sender.setSourceIP("127.0.0.1");
    sender.setDestinationIP("127.0.0.1");
    sender.setDestinationPort(8081);

    sender.sendPacket();
    proxy.receivePacket(8080);
    receiver.receivePacket(8081);

    return 0;
}