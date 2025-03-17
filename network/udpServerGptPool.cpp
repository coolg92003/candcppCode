#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#define PORT 8080
#define BUFFER_SIZE 65507  // Max UDP payload
#define TIMEOUT_MS 100     // Poll timeout (100ms)

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "[ERROR] Socket creation failed: " << strerror(errno) << "\n";
        return 1;
    }

    // Set socket to non-blocking mode
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // Increase OS receive buffer
    int recvBufferSize = 65536; // 64 KB
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, sizeof(recvBufferSize));

    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[ERROR] Binding failed: " << strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

    std::cout << "[INFO] UDP Server is running on 127.0.0.1:" << PORT << "...\n";

    char buffer[BUFFER_SIZE];
    socklen_t clientLen = sizeof(clientAddr);

    struct pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN;

    while (true) {
        int pollRes = poll(&pfd, 1, TIMEOUT_MS);

        if (pollRes < 0) {
            std::cerr << "[ERROR] poll() failed: " << strerror(errno) << "\n";
            break;
        } else if (pollRes == 0) {
            continue;  // No data, continue polling
        } else if (pfd.revents & POLLIN) {
            memset(buffer, 0, BUFFER_SIZE);

            ssize_t bytesReceived = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                             (struct sockaddr*)&clientAddr, &clientLen);

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << "[CLIENT] " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port)
                          << " → Received " << bytesReceived << " bytes\n";

                // (Optional) Send acknowledgment
                std::string response = "ACK: Received " + std::to_string(bytesReceived) + " bytes";
                sendto(sockfd, response.c_str(), response.length(), 0,
                       (struct sockaddr*)&clientAddr, clientLen);
            } 
            else if (bytesReceived < 0) {
                // 🔹 Handle different `errno` cases
                switch (errno) {
                    case EWOULDBLOCK:
                    case EAGAIN:
                        // No data available, expected in non-blocking mode
                        break;

                    case EINTR:
                        // Interrupted by signal, retry recvfrom()
                        std::cerr << "[WARNING] recvfrom() interrupted by signal, retrying...\n";
                        continue;

                    case EMSGSIZE:
                        // Message too large for buffer
                        std::cerr << "[WARNING] recvfrom() message too large, increase buffer size!\n";
                        break;

                    case ECONNREFUSED:
                        // Destination unreachable or no listener on the port
                        std::cerr << "[WARNING] recvfrom() connection refused (client may be down).\n";
                        break;

                    case EBADF:
                    case ENOTSOCK:
                        // Invalid socket, critical error
                        std::cerr << "[FATAL] recvfrom() invalid socket, exiting...\n";
                        close(sockfd);
                        return 1;

                    case EFAULT:
                        // Bad memory address, should never happen
                        std::cerr << "[FATAL] recvfrom() invalid buffer address, exiting...\n";
                        close(sockfd);
                        return 1;

                    default:
                        std::cerr << "[ERROR] recvfrom() failed: " << strerror(errno) << "\n";
                        break;
                }
            }
        }
    }

    close(sockfd);
    return 0;
}

