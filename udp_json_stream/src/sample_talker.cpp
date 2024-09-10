#include <thread>
#include <iostream>
#include <sstream>
#include <chrono>
#include <string>
#include <cstring>

#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")

    typedef SOCKET SocketType;
    #define INVALID_SOCK INVALID_SOCKET
    #define CLOSE_SOCKET closesocket
    #define SOCKET_ERROR_CODE SOCKET_ERROR
#else // _WIN32, UNIX-like system: i.e. Linux
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/stat.h>
    #include <unistd.h>

    typedef int SocketType;
    #define INVALID_SOCK -1
    #define CLOSE_SOCKET close
    #define SOCKET_ERROR_CODE -1
#endif

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#define VERBOSE
// #undef VERBOSE
json create_dummy_message() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;

#ifdef _WIN32
    localtime_s(&now_tm, &now_c);
#else
    localtime_r(&now_c, &now_tm);
#endif

    std::ostringstream timestamp_ss;
    timestamp_ss << std::put_time(&now_tm, "%Y-%m-%d_%H:%M:%S");

    return json{
        {"message", "Hello, world!"},
        {"number", 37},
        {"timestamp", timestamp_ss.str()},
        {"nested", {
            {"nestedness", true},
            {"data", {1, 2, 3, 4, 5}}
        }}
    };
}

void udp_sample_talker(const std::string& ip, int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }
#endif

    SocketType sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCK) {
        std::cerr << "Failed to create socket\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

#ifdef _WIN32
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
#else
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
#endif

    while (true) {
        json message = create_dummy_message();
        std::string json_message = message.dump();

        int sent = sendto(sock, json_message.c_str(), json_message.length(), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        if (sent == SOCKET_ERROR_CODE) {
            std::cerr << "Failed to send\n";
        }
#ifdef VERBOSE
        else {
            std::cout << "Sent " << sent << " bytes to " << ip << ":" << port << "\n";
        }
#endif

        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }

    CLOSE_SOCKET(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
        return 1;
    }

    std::string ip = argv[1];
    int port = std::stoi(argv[2]);

    udp_sample_talker(ip, port);
    return 0;
}