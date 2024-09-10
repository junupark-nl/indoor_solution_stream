#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else // _WIN32, UNIX-like system: i.e. Linux
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

int main(int argc, char* argv[]) {
    return 0;
}