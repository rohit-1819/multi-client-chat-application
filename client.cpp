#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h> // Essential for inet_pton()
#include <unistd.h>
#include <thread>
#include <cstring>
#include <termios.h>
#include <mutex>

using namespace std;

string current_input = "";
mutex input_mutex;
struct termios orig_termios;

// --- Terminal Manipulation Functions ---
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);  // Ensure terminal goes back to normal when program exits

    struct termios raw = orig_termios;
    // Turn off ECHO (don't print keys automatically) and ICANON (read byte-by-byte instead of line-by-line)
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

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
        buffer[bytes] = '\0';

        // Lock the input state while we redraw the screen
        lock_guard<mutex> lock(input_mutex);

        // 1. Wipe the current line and move cursor to the left
        cout << "\033[2K\r";
        cout << buffer << endl;
        cout << "You: " << current_input << flush;
    }
}

int main() {
    // 1. Create the Socket (The hardware)
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    // so here i made some changes-> now program will ask for the server's ip which will allow a change to dynamic from static localhost
    string server_ip;
    cout << "Enter the ip of the server you want to connect: ";
    getline(cin, server_ip);

    // 2. Define the Server's "Address Label"
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    
    // 3. Convert the IP address from text to binary
    // Use "127.0.0.1" for local testing, or the Public IP for remote
    // changes made -> replacing localhost with server_ip received from user inet_pton is older so it doesn't get string(get c style array) so we convert string to array using .c_str()
    inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr);

    // 4. Reach out and connect!
    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection Failed!" << endl;
        return -1;
    }
    // std::cout << "Connected to the server successfully!" << std::endl;
    string server_ip;
    cout << "Enter the ip of the server you want to connect: ";
    getline(cin, server_ip);

    string username;
    cout << "Enter your chat name: ";
    getline(cin, username);
    
    // Spin up the background listener
    thread listener(receive_handler, client_fd);
    listener.detach(); 
  
    // Enter Raw Mode for the chat phase
    enable_raw_mode();
    cout << "You: " << flush;

    char c;
    while (true) {
        // Read ONE character at a time directly from the keyboard
        if (read(STDIN_FILENO, &c, 1) == 1) {
            lock_guard<mutex> lock(input_mutex);
            
            if (c == '\n' || c == '\r') { // Enter key pressed
                cout << "\n";
                
                string full_msg = username + ": " + current_input;
                send(client_fd, full_msg.c_str(), full_msg.length(), 0);

                if (current_input == "/quit") break;

                current_input = ""; // clear our_typed buffer
                cout << "You: " << flush;
            }
            else if (c == 127 || c == '\b') { // backspace key pressed
                if (!current_input.empty()) {
                    current_input.pop_back();
                    cout << "\033[2K\rYou: " << current_input << flush;
                }
            }
            else {
                current_input += c;
                cout << c << flush;
            }
        }
    }

    // sender logic
    /* string typed_msg;
    while (true) {
        cout << "You: " << endl;
        getline(cin, typed_msg);

        // Combine the name and the message
        string full_msg = username + ": " + typed_msg;

        if (typed_msg == "/quit") break;

        send(client_fd, full_msg.c_str(), full_msg.length(), 0);
    }
    */
 
    return 0;
}
