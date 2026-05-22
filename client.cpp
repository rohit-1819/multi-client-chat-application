#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h> // Essential for inet_pton()
#include <unistd.h>
#include <thread>
#include <cstring>

using namespace std;

// Worker thread for receiving
void receive_handler(int socket_fd) {
    char buffer[1024];
    while (true) {
        // clear the buffer
        memset(buffer, 0, 1024);
        
        int bytes = recv(socket_fd, buffer, 1024, 0);

        if (bytes <= 0) {
            cout << "\n[Disconnected]" << endl;
            exit(0);
        }

        // 1. Wipe the current line and move cursor to the left
        cout << "\033[2K\r";
        cout << buffer << endl;
        cout << "You: " << flush;
    }
}

int main() {
    // 1. Create the Socket (The hardware)
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Define the Server's "Address Label"
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    
    // 3. Convert the IP address from text to binary
    // Use "127.0.0.1" for local testing, or the Public IP for remote
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // 4. Reach out and connect!
    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection Failed!" << endl;
        return -1;
    }
    // std::cout << "Connected to the server successfully!" << std::endl;

    string username;
    cout << "Enter your chat name: ";
    getline(cin, username);
    
    // Spin up the background listener
    thread listener(receive_handler, client_fd);
    listener.detach(); 
  
    // sender logic
    string typed_msg;
    while (true) {
        cout << "You: " << endl;
        getline(cin, typed_msg);

        // Combine the name and the message
        string full_msg = username + ": " + typed_msg;

        if (typed_msg == "/quit") break;

        send(client_fd, full_msg.c_str(), full_msg.length(), 0);
    }
 
    return 0;
}
