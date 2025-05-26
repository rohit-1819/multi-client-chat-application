#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024

void receive_messages(int socket_fd) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(socket_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            close(socket_fd);
            exit(0);
        }
        std::cout << buffer << std::endl;
    }
}

void send_messages(int socket_fd) {
    std::string message;
    while (true) {
        std::getline(std::cin, message);
        send(socket_fd, message.c_str(), message.size(), 0);
    }
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed!" << std::endl;
        return 1;
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    std::cout << buffer;
    std::string username;
    std::getline(std::cin, username);
    send(client_socket, username.c_str(), username.size(), 0);

    std::thread(receive_messages, client_socket).detach();
    send_messages(client_socket);

    return 0;
}
