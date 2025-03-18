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
//exp and trace
#include <exception>
#include <execinfo.h>

#include "concurrentqueue.h"  // moodycamel::ConcurrentQueue
#include "rapidjson/document.h"
#include "trace2.pb.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8089
#define BUFFER_SIZE 4096
#define STACK_BUFFER_SIZE 1024
#define MAX_EVENTS 10
#define MAX_WORKER_COUNT 1
//PM
unsigned long udp_server_msg_rcv_cnt = 0;
unsigned long udp_server_msg_enqueue_cnt = 0;
unsigned long udp_worker_msg_denqueue_cnt[MAX_WORKER_COUNT] {0};
moodycamel::ConcurrentQueue<std::string> message_queue[MAX_WORKER_COUNT]; // Lock-free queue
void printStackTrace() {
	void* call_stack[STACK_BUFFER_SIZE]{0};
	int frames = backtrace(call_stack, 128);
	char ** symbols = backtrace_symbols(call_stack, frames);
	std::cout << "printStackTrace: debugging stack:\n";
	for (int i=0; i < frames; ++i) {
		std::cout << symbols[i] << std::endl;
	}
	free(symbols);
	return;
}
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

// json
void decodeJson( [[maybe_unused]] const char* pJsonStr) {
	// try to analysis
	std::cout << "\tdecodeJson: enter\n";
	try {
		rapidjson::Document doc; 
		doc.Parse(pJsonStr);
		if (doc.HasMember("socket_streamed_trace_segment") &&
			doc["socket_streamed_trace_segment"].HasMember("event")) {
			std::cout << "\t\tseconds: " << doc["socket_streamed_trace_segment"]["event"]["timestamp"]["seconds"].GetInt() << std::endl;
			if (doc["socket_streamed_trace_segment"]["event"].HasMember("read")) {
				std::cout << "\t\tread: as_bytes: " << doc["socket_streamed_trace_segment"]["event"]["read"]["data"]["as_bytes"].GetString() << std::endl;
			} 
			else { 
				std::cout << "\t\twrite: as_bytes: " << doc["socket_streamed_trace_segment"]["event"]["write"]["data"]["as_bytes"].GetString() << std::endl;
			}
			std::cout << "\t\tlocal ip: " << doc["socket_streamed_trace_segment"]["event"]["connection"]["local_address"]["socket_address"]["address"].GetString() << std::endl;
			std::cout << "\t\tlocal port: " << doc["socket_streamed_trace_segment"]["event"]["connection"]["local_address"]["socket_address"]["port_value"].GetInt() << std::endl;
			std::cout << "\t\tremote ip: " << doc["socket_streamed_trace_segment"]["event"]["connection"]["remote_address"]["socket_address"]["address"].GetString() << std::endl;
			std::cout << "\t\tremote port: " << doc["socket_streamed_trace_segment"]["event"]["connection"]["remote_address"]["socket_address"]["port_value"].GetInt() << std::endl;
		}
	} catch (const std::exception& e) {
		std::cout << "Exception: " << e.what() << std::endl;
		printStackTrace();
	}
	return;
}
void decodeByProto(std::string& pMsg) {
	std::cout << "\tdecodeByProto: enter, pMsg.size(): " << pMsg.size() << " pMsg.capacity(): " << pMsg.capacity() << std::endl;
	try {
		//trace2
		trace2::TraceWrapper new_segment;
		// output the binary
		/*
		for (size_t i=0; i < pMsg.size(); ++i) {
			//printf("%02X ",static_cast<unsigned char>(pMsg[i]));
		} 
		std::cout << std::endl;
		*/

		if (!new_segment.ParseFromArray(&pMsg[0], pMsg.capacity())) {
			std::cout << "\tdecodeByProto: ERROR Failed to parseFromArray\n";
			// output as string
			/*std::string serialized_data;
			if (!new_segment.SerializeToString(&serialized_data)) {
    				std::cout << "\tdecodeByProto: ERROR  Failed to string" << std::endl;
				return;
			}
    			std::cout << "\tdecodeByProto: string: " << serialized_data << std::endl; */
			return;
		} else {
			std::cout << "\tdecodeByProto: SUCC Done from parseFromArray for recv Msg, size is:" <<  pMsg.size() << std::endl;
		}
 		std::cout << "===== Parsed Protobuf Data =====" << std::endl;
    		std::cout << "Timestamp: " << new_segment.socket_streamed_trace_segment().event().timestamp().seconds() 
              		<< "s, " << new_segment.socket_streamed_trace_segment().event().timestamp().nanos() << "ns" << std::endl;

    		std::cout << "Local Address: " << new_segment.socket_streamed_trace_segment().event().connection_per_event().local_address().socket_address().address()              << ":" << new_segment.socket_streamed_trace_segment().event().connection_per_event().local_address().socket_address().port_value() << std::endl;

    		std::cout << "Remote Address: " << new_segment.socket_streamed_trace_segment().event().connection_per_event().remote_address().socket_address().address()              << ":" << new_segment.socket_streamed_trace_segment().event().connection_per_event().remote_address().socket_address().port_value() << std::endl;

    		std::string as_bytes = new_segment.socket_streamed_trace_segment().event().read().data().as_bytes();
    		if (as_bytes.size() == 0) {
			as_bytes = new_segment.socket_streamed_trace_segment().event().write().data().as_bytes();
		}
		std::cout << "Read Data:size(): " << as_bytes.size() << " as_bytes.capacity(): " << as_bytes.capacity() << std::endl;
    		std::cout << "Read Data (as_bytes): ";
    		for (unsigned char c : as_bytes) {
        		printf("\\x%02x", c);  // Print bytes in hex format
    		}
 		std::cout << "\n\n";

	} catch (const std::exception& e) {
		std::cout << "Exception: " << e.what() << std::endl;
		printStackTrace();
	}
	return;
}
// Worker function to process messages
// thread index is work queue id
void workerThread(int index) {
   std::cout << "\n\tworkerThread: " << index << std::endl; 
    while (true) {
        std::string message;
        if (message_queue[index].try_dequeue(message)) {  // Non-blocking dequeue
            //std::cout << "workerThread: [Worker " << index << "] Processed message: " << message << std::endl;
            std::cout << "\t\tworkerThread: [Worker " << index << "] Processed message and size is:" << message.size() << std::endl;
            std::cout << "\t\tworkerThread: [Worker " << index << "] Processed message and size is:" << message.capacity() << std::endl;
	    //decode by json s
            //const char* l_json_str = message.c_str();
            //decodeJson(l_json_str);
	    //decode by json e
	    //decode by protobuffer s
		decodeByProto(message);
	    //decode by protobuffer e
            udp_worker_msg_denqueue_cnt[index]++;
        }
        //std::cout << "workerThread: [Worker " << index << " yield()" << std::endl; 
        std::this_thread::yield(); //Reduce CPU usage
    }
}
void printcnt(int worker_index) {
   std::cout << "\n\n\tprintcnt: work index: (" << worker_index << ")" << std::endl; 
   std::cout << "\tprintcnt: UDP server PM" << std::endl; 
   std::cout << "\t\tudp_server_msg_rcv_cnt:" << udp_server_msg_rcv_cnt << std::endl; 
   std::cout << "\t\tudp_server_msg_enqueue_cnt:" << udp_server_msg_rcv_cnt << std::endl; 
   std::cout << "\tprintcnt: UDP worker PM" << std::endl; 
   for (int i=0; i < MAX_WORKER_COUNT; i++) {
   	std::cout << "\t\t work_index:" << i << 
		". udp_worker_msg_denqueue_cnt:" << udp_worker_msg_denqueue_cnt[i] << std::endl; 
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
    int worker_index = 0;
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

   		std::cout << "\n\n\tudpServer: msg size: " << recv_len << std::endl; 
                buffer[recv_len] = '\0'; // Ensure null termination
                //std::string message(buffer);
                /*
		for (size_t i=0; i < recv_len; ++i) {
			printf("%02X ",static_cast<unsigned char>(buffer[i]));
		}*/
                std::string message;
		message.resize(recv_len);
		memcpy(&message[0], buffer, recv_len);

                // Lock-free enqueue
		udp_server_msg_rcv_cnt++;
                message_queue[worker_index].enqueue(message);
                udp_server_msg_enqueue_cnt++;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
                //printcnt(worker_index);
                worker_index = (worker_index + 1) % MAX_WORKER_COUNT; 
                //message_queue.enqueue(message);
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
    for (int i = 0; i < MAX_WORKER_COUNT; i++) {
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

