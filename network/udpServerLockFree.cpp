#include <iostream>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <thread>
#include <vector>
#include <sys/epoll.h>
#include "concurrentqueue.h"  // moodycamel::ConcurrentQueue

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5060
#define BUFFER_SIZE 1024
#define MAX_EVENTS 10

moodycamel::ConcurrentQueue<std::string> message_queue; // Lock-free queue

// Function to set a socket as non-blocking
void setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        std::cout << "\tsetNonBlocking: ERROR, fcntl F_GETFL failed with (" 
		<< strerror(errno)  << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cout << "\tsetNonBlocking: ERROR, fcntl F_SETFL O_NONBLOCK failed with (" 
		<< strerror(errno)  << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Worker function to process messages
void workerThread(int index) {
    while (true) {
        std::string message;
        if (message_queue.try_dequeue(message)) {  // Non-blocking dequeue
            std::cout << "workerThread: [Worker " << index << "] Processed message: " 
		<< message << std::endl;
        }
    }
}

// UDP server function running in a separate thread
void udpServer() {
    std::cout << "\tudpServer: enter" << std::endl;
    int sockfd, epoll_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cout << "\tudpServer: ERROR, Socket creation failed with (" 
		<< strerror(errno)  << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set the socket to non-blocking mode
    setNonBlocking(sockfd);

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cout << "\tudpServer: ERROR, Socket Bind failed with (" 
		<< strerror(errno)  << ")" << std::endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Create epoll instance
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::cout << "\tudpServer: ERROR, epoll_create1 failed with (" 
		<< strerror(errno)  << ")" << std::endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Add socket to epoll
    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = sockfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
        std::cout << "\tudpServer: ERROR, epoll_ctl failed with (" 
		<< strerror(errno)  << ")" << std::endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    std::cout << "udpServer: UDP Server listening on " << SERVER_IP << ":" << SERVER_PORT << "...\n";

    while (true) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count < 0) {
            std::cout << "\tudpServer: ERROR, epoll_wait failed with (" 
		<< strerror(errno)  << ")" << std::endl;
            continue;
        }

        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == sockfd) {
                socklen_t client_len = sizeof(client_addr);
                memset(buffer, 0, BUFFER_SIZE);

                ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                                            (struct sockaddr*)&client_addr, &client_len);

                if (recv_len < 0) {
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
            		std::cout << "\tudpServer: ERROR, Receive failed with (" 
				<< strerror(errno)  << ")" << std::endl;
                    }
                    continue;
                }

                buffer[recv_len] = '\0'; // Ensure null termination
                std::string message(buffer);

                // Lock-free enqueue
                message_queue.enqueue(message);
            }
        }
    }

    close(sockfd);
    std::cout << "\tudpServer: exit" << std::endl;
}

int main() {
    // Start UDP server in a separate thread
    std::cout << "main: enter" << std::endl;
    std::thread server_thread(udpServer);

    // Start three worker threads
    std::vector<std::thread> worker_threads;
    for (int i = 0; i < 3; i++) {
        worker_threads.emplace_back(workerThread, i);
    }

    // Join threads (server thread runs indefinitely)
    server_thread.join();
    for (auto& worker : worker_threads) {
        worker.join();
    }

    std::cout << "main: exit" << std::endl;
    return 0;
}

