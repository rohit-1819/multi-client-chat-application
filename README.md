# 🖥️ Multi-Client TCP Chat Application in C++

This is a simple multi-client chat application implemented in C++ using TCP sockets and threading. It allows multiple clients to connect to a central server and exchange messages in real-time over a network.

---

## 🌐 Features

✅ Multi-client support (each client handled in a separate thread)  
✅ Usernames for clients  
✅ Message broadcasting to all clients  
✅ Simple CLI interface for clients  
✅ TCP-based socket communication

---

## 📦 Technologies Used

- C++ (Sockets, Threads, STL)
- TCP Networking (`<sys/socket.h>`, `<arpa/inet.h>`)
- Linux/Unix environment

---

## 🔧 Setup & Usage

### 1️⃣ Clone the Repository

```bash
git clone https://github.com/your-username/cpp-multi-client-chat.git
cd cpp-multi-client-chat
