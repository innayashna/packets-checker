#include "Sender.h"
#include "Receiver.h"
#include <iostream>
#include <thread>

int main() {
    Sender sender;
    Receiver receiver;

    std::string payload;
    std::cout << "Enter the payload data: ";
    std::getline(std::cin, payload);
    sender.setPayload(payload);

    sender.setSourceIP("127.0.0.1");
    sender.setDestinationIP("127.0.0.1");
    sender.setDestinationPort(8080);

    sender.sendPacket();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    receiver.receivePacket();

    return 0;
}
