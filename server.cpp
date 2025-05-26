#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm>

#define PORT 12345
#define BUFFER_SIZE 1024

std::vector<int> clients;
std::map<int, std::string> usernames;

void broadcast(const std::string& message, int sender) {
    for (int client : clients) {
        if (client != sender) {
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    usernames[client_socket] = buffer;
    std::string join_msg = usernames[client_socket] + " has joined the chat.\n";
    broadcast(join_msg, client_socket);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::string leave_msg = usernames[client_socket] + " has left the chat.\n";
            broadcast(leave_msg, client_socket);
            close(client_socket);

            // In the code:
            clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());

            usernames.erase(client_socket);
            break;
        }
        std::string message = usernames[client_socket] + ": " + buffer;
        broadcast(message, client_socket);
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (sockaddr*)&client_addr, &addr_len);
        clients.push_back(client_socket);

        send(client_socket, "Enter your username: ", 21, 0);
        std::thread(handle_client, client_socket).detach();
    }

    close(server_fd);
    return 0;
}
