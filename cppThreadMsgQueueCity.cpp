#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
//message queue
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <unistd.h>
#include <pthread.h>

struct CHome {
        std::string name;
        int age;
        bool sexn;
};
struct CCity {
	CCity() {
		std::cout << "\tCCity is called" << std::endl;
	}
	~CCity() {
		std::cout << "\t~CCity is called~" << std::endl;
	}
        std::string name;
        std::string zipcode;
        std::vector <std::shared_ptr<struct CHome>> chome_shared_ptr;
};
#define MSG_KEY 1234
#define MAX_MSG_SIZE 64
struct Message {
    long msg_type;
    char msg_text[MAX_MSG_SIZE];
};
void print_memory_map() {
    std::ifstream maps_file("/proc/self/maps");
    std::string line;

	std::cout << "\t\t**** print_memory_map start" << std::endl;
    while (std::getline(maps_file, line)) {
        std::cout << line << std::endl;
    }
	std::cout << "\t\t**** print_memory_map end" << std::endl;
}

void print_stack_range() {
    pthread_attr_t attr;
    void *stack_addr;
    size_t stack_size;

    pthread_attr_init(&attr);
    pthread_attr_getstack(&attr, &stack_addr, &stack_size);
    pthread_attr_destroy(&attr);

    std::cout << "CFX, Stack Range: " << stack_addr << " - " 
              << static_cast<char*>(stack_addr) + stack_size << std::endl;
}
void MQ_server(void) {
	std::cout << "\tMQ_server, Server is running..." << std::endl;
	int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
	if (msgid == -1) {
		std::cout << "MQ_server: Faild to create msgid" << std::endl;
		return;
	}
	std::cout << "\tMQ_server, while..." << std::endl;
	while (true) {
		struct Message msg; 
		// Receive message from client
		ssize_t rn = msgrcv(msgid, &msg, sizeof(msg.msg_text), 1, 0);
		if (rn == -1) {
			std::cout << "MQ_server: Faild to msgrcv" << std::endl;
			return;
		}
		std::cout << "\tMQ_server,Received: type is" << msg.msg_type  <<
			" rn = " << rn << std::endl; 
		std::shared_ptr<CCity> shared_ptr1 = nullptr;
		//strncpy(static_cast<char>(shared_ptr1), msg.msg_text, rn);
		//std::cout << "\tMQ_server,Received: name is" << shared_ptr1->name << std::endl;
		// Prepare response message
        	msg.msg_type = 2;  // Response type
		strcpy(msg.msg_text, "Message received!");

        	// Send response to client
        	if (msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1) {
			std::cout << "MQ_server: Faild to snd" << std::endl;
			return;
		}
		std::cout << "\tMQ_server,Response sent." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		break;
	}

	std::cout << "\tMQ_server, exit..." << std::endl;
    	// Cleanup (unreachable in this loop, but good practice)
    	msgctl(msgid, IPC_RMID, nullptr);
	return;
}

void MQ_client(void) {
	std::cout << "\tClient is running..." << std::endl;
	int msgid = msgget(MSG_KEY, 0666);
	if (msgid == -1) {
		std::cout << "MQ_client: Faild to msgget" << std::endl;
		return;
	}
	struct Message msg;
	std::cout << "\tMQ_client: msgid=" << msgid << "  sizeof(msg)=" << sizeof(msg) << std::endl;
	msg.msg_type = 1;  // Request type
	memset(msg.msg_text, 0, MAX_MSG_SIZE);
	//strcpy(msg.msg_text, "Hello, Server!");
	auto shared_ptr1 = std::make_shared<CCity>();
	shared_ptr1->name = "QingDao";
        shared_ptr1->zipcode = "266101";
        std::cout << "\tMQ_client: shared_ptr1.use_cnt():" << shared_ptr1.use_count() << std::endl;
        auto ptr2 = std::make_shared<CHome>();
        ptr2->name = "Cliff";
        ptr2->age = 28;
        ptr2->sexn = true;
        shared_ptr1->chome_shared_ptr.push_back(ptr2);
    	int i=6;	
        std::cout << "\tMQ_client: sizeof(i):" << sizeof(i)  <<
		" address i: " << &i << std::endl;
	//heap
	void* heap_start = sbrk(0);
        std::cout << "\tMQ_client: heap start address: " << heap_start  << std::endl;
	//print_stack_range();
	print_memory_map();
    	CCity* local_ptr = new CCity();	
        std::cout << "\tMQ_client: sizeof(local_ptr):" << sizeof(local_ptr)  <<
		" address of local_ptr: " << &local_ptr << 
		" local_ptr content: " << local_ptr << std::endl;
	delete local_ptr;
	local_ptr = nullptr;
        std::cout << "\tMQ_client: sizeof(shared_ptr1):" << sizeof(shared_ptr1)  <<
		" value of shared_ptr1: " << shared_ptr1 << std::endl;
    	//strncpy(msg.msg_text, static_cast<char>(shared_ptr1), sizeof(shared_ptr1));
    	// Send message to server
    	if (msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1) {
    	//if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(msg.msg_type), 0) == -1) {
		std::cout << "MQ_client: Faild to msgsnd" << std::endl;
        	return;
	}

	std::cout << "\tMQ_client: msgsnd done to server" << std::endl;

    	// Receive response from server
    	if (msgrcv(msgid, &msg, sizeof(msg.msg_text), 2, 0) == -1) {
		std::cout << "MQ_client: Faild to msgrcv" << std::endl;
        	return;
    	}

	std::cout << "\tMQ_client: got resp from server and exit" << std::endl;
	return;
}

int main() {
	std::cout << "main: thread enter" << std::endl;
	std::thread t1(MQ_server);
	t1.detach();
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::thread t2(MQ_client);
	t2.detach();
	std::this_thread::sleep_for(std::chrono::seconds(20));
	std::cout << "main: thread exit" << std::endl;
	return 0;
}
