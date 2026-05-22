#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<thread>
#include<cstring>
#include<vector>
#include<algorithm>
#include<mutex>

using namespace std;

// Global list to track all active client sockets
vector<int> clients;
// Mutex to protect our vector from data corruption when multiple threads access it at once
mutex clients_mutex;

// Function to send a message to every client EXCEPT the one who sent it
void broadcast_message(string message, int sender_fd) {
    lock_guard<mutex> lock(clients_mutex);
    for (int client_fd : clients) {
        if (client_fd != sender_fd) {
            send(client_fd, message.c_str(), message.size(), 0);
        }
    }
}

// Thread worker function assigned to handle a single client's incoming traffic
void handle_client(int client_fd) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, 1024);
        int bytes = recv(client_fd, buffer, 1024, 0);

        if (bytes <= 0) {
            cout << "Client disconnected." << endl;
            close(client_fd);
            break;
        }

        buffer[bytes] = '\0'; // null-terminate the string
        string msg(buffer);

        cout << "[Relaying] Message from socket " << client_fd << ": " << msg << "\n";

        // Relay the message to all other connected users
        broadcast_message(msg, client_fd);
    }    

    // Clean up when the client leaves
    close(client_fd);
    
    lock_guard<mutex> lock(clients_mutex);
    clients.erase(remove(clients.begin(), clients.end(), client_fd), clients.end());

}

int main() {
    // 1. Create the Socket (The Hardware)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Socket creation failed.\n";
        return 1;
    }

    // Allow quick reuse of the port to avoid "Address already in use" errors on restart
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Prepare the Mailing Label
    struct sockaddr_in address;
    address.sin_family = AF_INET;           // IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // Any interface
    address.sin_port = htons(8080);          // port

    // 3. Bind the socket to our address/port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed.\n";
        return 1;
    }

    // 4. Start Listening
    if (listen(server_fd, 10) < 0) {
        cerr << "listen failed.\n";
        return 1;
    }

    cout << "Server is listening on port 8080..." << endl;    

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Blocks here until someone knocks, then pops out a custom conversation socket
        int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        if (new_socket < 0) {
            cerr << "Failed to accept client connection.\n";
            continue;
        }

        std::cout << "[Server] New connection accepted! Socket FD: " << new_socket << "\n";

        // Add the new client to our master tracking list safely using a lock
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(new_socket);
        }

        // Spawn a background worker thread for this client so main loop can instantly return to accept()
        std::thread client_thread(handle_client, new_socket);
        client_thread.detach(); // Detach allows the thread to manage its own lifecycle independently
    }

    close(server_fd);
    return 0;
}
