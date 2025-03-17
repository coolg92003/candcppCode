#include <iostream>
#include <cstring>      // memset()
#include <sys/socket.h> // socket(), bind(), recvfrom(), sendto()
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_ntoa()
#include <unistd.h>     // close()
#include <fcntl.h>      // fcntl()
#include <errno.h>      // errno

#define PORT 8080        // Server port
#define BUFFER_SIZE 65536 // Maximum receive buffer

int main() {
    // 1️⃣ Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "[ERROR] Socket creation failed: " << strerror(errno) << "\n";
        return 1;
    }

    // 2️⃣ Set the socket to NON-BLOCKING mode
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "[ERROR] Failed to set non-blocking mode: " << strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

	// Increase receive buffer size for better performance
    int recvBufferSize = 65536; // 64 KB
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, sizeof(recvBufferSize));

    // 3️⃣ Configure server address
    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    //serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);

    // 4️⃣ Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[ERROR] Binding failed: " << strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

    std::cout << "[INFO] Non-blocking UDP Server is running on port " << PORT << "...\n";

    // 5️⃣ Loop to handle incoming data
    char buffer[BUFFER_SIZE];
    socklen_t clientLen = sizeof(clientAddr);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        // Non-blocking recvfrom()
        ssize_t bytesReceived = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                         (struct sockaddr*)&clientAddr, &clientLen);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate received data

            // Print client details and message
            std::cout << "[CLIENT] " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port)
                      << " → " << buffer << std::endl;

            // (Optional) Send response back to client
            std::string response = "Message received: " + std::string(buffer);
            ssize_t bytesSent = sendto(sockfd, response.c_str(), response.length(), 0,
                                       (struct sockaddr*)&clientAddr, clientLen);

            if (bytesSent < 0) {
                std::cerr << "[ERROR] Failed to send response: " << strerror(errno) << "\n";
            }
        } 
        else if (bytesReceived == 0) {
            // This case is unlikely for UDP, as it doesn't establish a connection
            std::cerr << "[WARNING] Received empty message.\n";
        } 
        else if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // No data available, this is normal in non-blocking mode
        } 
        else {
            std::cerr << "[ERROR] recvfrom() failed: " << strerror(errno) << "\n";
        }

        // 🌟 Non-blocking mode: Prevents 100% CPU usage
        usleep(100000); // Sleep for 100ms
    }

    // 6️⃣ Close the socket (never reached in this loop)
    close(sockfd);
    return 0;
}

