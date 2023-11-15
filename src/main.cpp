#include "../include/Sender.h"
#include "../include/Proxy.h"
#include "../include/Receiver.h"

#include <iostream>

constexpr int SENDER_PORT = 8080;
constexpr int PROXY_PORT = 8081;
constexpr int RECEIVER_PORT = 8082;
constexpr const char* LOCAL_IP = "127.0.0.1";

typedef struct {
    std::string payload;
    InterceptionOption option;
} UserInput;

UserInput getUserInput();

int main() {
    Sender sender(LOCAL_IP, SENDER_PORT, LOCAL_IP, PROXY_PORT);
    Proxy proxy(LOCAL_IP, PROXY_PORT, LOCAL_IP, RECEIVER_PORT);
    Receiver receiver(LOCAL_IP, RECEIVER_PORT);

    UserInput input = getUserInput();

    proxy.setInterceptionOption(InterceptionOption(input.option));
    sender.setPayload(input.payload);

    sender.sendPacket();
    proxy.receivePacket(SENDER_PORT);
    receiver.receivePacket(PROXY_PORT);

    return 0;
}

UserInput getUserInput() {
    std::string payload;
    std::string optionInput;

    std::cout << "Choose proxy forwarding option: " << std::endl;
    std::cout << "1. Proxy just forwards data to the receiver" << std::endl;
    std::cout << "2. Proxy does modifications in payload" << std::endl;
    std::cout << "Type your option here (enter corresponding number) >> ";

    std::getline(std::cin, optionInput);

    int option = atoi(optionInput.c_str());
    if (option == 0) {
        std::cerr << "Error option format: " << optionInput << std::endl;
        exit(1);
    }

    std::cout << "Enter the payload data: ";
    std::getline(std::cin, payload);

    return UserInput{payload, InterceptionOption(option)};
}
