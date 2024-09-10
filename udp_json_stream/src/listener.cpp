#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <cstring>
#include <chrono>
#include <vector>

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
std::string get_current_timestamp_filename(const std::string &relative_base_dir="") {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;

#ifdef _WIN32
    localtime_s(&now_tm, &now_c);
#else
    localtime_r(&now_c, &now_tm);
#endif

    std::ostringstream date_oss;
    date_oss << std::put_time(&now_tm, "%Y-%m-%d");
    std::ostringstream time_oss;
    time_oss << std::put_time(&now_tm, "%H;%M;%S");

    // Get the current working directory
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path full_path = cwd / relative_base_dir / date_oss.str();

    // Normalize the path (resolves ".." and ".")
    full_path = std::filesystem::weakly_canonical(full_path);
    std::filesystem::create_directories(full_path);
    full_path /= time_oss.str() + ".csv";

    return full_path.string();
}

void flatten_json(const json& j, std::string prefix, json& result) {
    for (auto& el : j.items()) {
        // concatenate the key with the parent structure key using dot
        std::string new_key = prefix.empty() ? el.key() : prefix + "." + el.key();
        if (el.value().is_object()) {
            flatten_json(el.value(), new_key, result);
        } else {
            result[new_key] = el.value();
        }
    }
}

std::string escape_csv(const std::string& str) {
    std::ostringstream oss;
    oss << '"';
    for (auto c : str) {
        if (c == '"') {
            oss << '"' << '"';
        } else {
            oss << c;
        }
    }
    oss << '"';
    return oss.str();
}

void write_csv_line(std::ofstream& file, const json& j, const std::vector<std::string>& keys) {
    for (size_t i = 0; i < keys.size(); ++i) {
        if (j.contains(keys[i])) {
            const auto& value = j[keys[i]];
            if (value.is_string()) {
                file << escape_csv(value.get<std::string>());
            } else if (value.is_number() || value.is_boolean()) {
                file << value;
            } else {
                file << escape_csv(value.dump());
            }
        } else {
            // Write an empty string if the key is not found
            file << "";
        }
        if (i < keys.size() - 1) file << ",";
    }
    file << "\n";
}

void udp_listener(const std::string& ip, int port) {
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

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR_CODE) {
        std::cerr << "Bind failed\n";
        CLOSE_SOCKET(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    std::string csv_filename = get_current_timestamp_filename("../../../logs/json_udp");
#ifdef VERBOSE
    std::cout << "Starting UDP listener on " << ip << " port " << port << "\n";
    std::cout << "CSV filename: " << csv_filename << std::endl;
#endif

    std::ofstream csv_file(csv_filename);
    if (!csv_file.is_open()) {
        std::cerr << "Failed to open " << csv_filename << "\n";
        CLOSE_SOCKET(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    char buffer[4096];
    sockaddr_in client_addr;
#ifdef _WIN32
    int client_len = sizeof(client_addr);
#else
    socklen_t client_len = sizeof(client_addr);
#endif

    std::vector<std::string> header;
    bool header_written = false;

    while (true) {
        int received = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_len);

        if (received == SOCKET_ERROR_CODE) {
            std::cerr << "Failed to receive\n";
            continue;
        }

        auto arrivalTime = std::chrono::steady_clock::now();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(arrivalTime.time_since_epoch()).count();
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        try {
            json message = json::parse(std::string(buffer, received));
#ifdef VERBOSE
            std::cout << "Received JSON message:\n" << message.dump(2) << "\n";
#endif
            json flattened;
            flatten_json(message, "", flattened);
            // manual time tag
            flattened["ArrivalTimeUs"] = micros;

            if (!header_written) {
                for (auto& item : flattened.items()) {
                    header.push_back(item.key());
                }
                for (size_t i = 0; i < header.size(); i++) {
                    csv_file << escape_csv(header[i]);
                    if (i < header.size() - 1) {
                        csv_file << ",";
                    }
                }
                csv_file << "\n";
                header_written = true;
            }

            write_csv_line(csv_file, flattened, header);
            csv_file.flush();
        }
        catch (json::parse_error&) {
            std::cerr << "Invalid JSON message\n";
        }
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

    udp_listener(ip, port);
    return 0;
}