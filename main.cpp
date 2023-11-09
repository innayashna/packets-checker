#include "Sender.h"
#include <iostream>

int main() {
    Sender sender;

    std::string payload;
    std::cout << "Enter the payload data: ";
    std::cin >> payload;
    sender.setPayload(payload);

    sender.setSourceIP("192.168.1.2");
    sender.setDestinationIP("1.2.3.4");
    sender.setDestinationPort(80);

    sender.sendPacket();

    return 0;
}
