#include <iostream>
#include <cstring>      // memset()
#include <netdb.h>      // getaddrinfo(), freeaddrinfo()
#include <arpa/inet.h>  // inet_ntop()
#include <sys/types.h>  // socket types
#include <sys/socket.h> // socket functions
#include <unistd.h>     // close()
#include <errno.h>      // errno, strerror()

void check_server_reachability(const std::string& hostname, const std::string& port) {
    struct addrinfo hints, *res, *p;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // Support both IPv4 & IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP (Change to SOCK_DGRAM for UDP)

    // Step 1: DNS Resolution
    int status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &res);
    if (status != 0) {
        std::cerr << "[DNS ERROR] getaddrinfo() failed: " << gai_strerror(status) << std::endl;
        
        if (status == EAI_NONAME)   std::cerr << "  → No such hostname found. Check /etc/resolv.conf.\n";
        if (status == EAI_AGAIN)    std::cerr << "  → Temporary DNS failure. Retry later.\n";
        if (status == EAI_FAIL)     std::cerr << "  → Permanent DNS failure. Check network settings.\n";
        
        return;
    }

    std::cout << "[DNS SUCCESS] Resolved IP addresses for " << hostname << " on port " << port << ":\n";

    for (p = res; p != nullptr; p = p->ai_next) {
        void *addr;
        std::string ipVersion;
        
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipVersion = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipVersion = "IPv6";
        }

        if (inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr)) == nullptr) {
            std::cerr << "  [ERROR] IP conversion failed\n";
            continue;
        }

        std::cout << "  [" << ipVersion << "] " << ipstr << std::endl;
/*
        // Step 2: Attempt to Connect to the Server
        int sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) {
            std::cerr << "  [SOCKET ERROR] Could not create socket: " << strerror(errno) << std::endl;
            continue;
        }

        std::cout << "  → Trying to connect to " << ipstr << ":" << port << "...\n";
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            std::cerr << "  [CONNECT ERROR] " << strerror(errno) << std::endl;

            if (errno == ECONNREFUSED) std::cerr << "    → Connection refused. Server might be down.\n";
            if (errno == ETIMEDOUT)    std::cerr << "    → Connection timed out. Network issue or firewall blocking.\n";
            if (errno == EHOSTUNREACH) std::cerr << "    → No route to host. Check internet connection.\n";

            close(sockfd);
            continue;
        }

        std::cout << "  [SUCCESS] Connected to " << ipstr << ":" << port << "!\n";
        close(sockfd);
*/
        break; // Exit loop after first successful connection
    }

    freeaddrinfo(res);
}

int main() {
    std::string hostname, port;
    std::cout << "Enter hostname (e.g., example.com): ";
    std::cin >> hostname;
    std::cout << "Enter port (e.g., 80, 443, 53): ";
    std::cin >> port;

    check_server_reachability(hostname, port);
    return 0;
}

