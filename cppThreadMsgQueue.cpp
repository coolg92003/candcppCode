#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
//message queue
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <unistd.h>

#define MSG_KEY 1234
struct Message {
    long msg_type;
    char msg_text[512];
};
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
		if (msgrcv(msgid, &msg, sizeof(msg.msg_text), 1, 0) == -1) {
			std::cout << "MQ_server: Faild to msgrcv" << std::endl;
			return;
		}
		std::cout << "\tMQ_server,Received: type is " << msg.msg_type << std::endl; 
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
	std::cout << "\tMQ_client: msgid=" << msgid << "sizeof(msg)=" << sizeof(msg) << std::endl;
	msg.msg_type = 1;  // Request type
	strcpy(msg.msg_text, "Hello, Server!");

    	// Send message to server
	std::cout << "\tMQ_client: msgsnd, sizeof(msg.msg_text)=" << sizeof(msg.msg_text) << "leng=" << strlen(msg.msg_text) << std::endl;
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
	std::this_thread::sleep_for(std::chrono::seconds(10));
	std::thread t2(MQ_client);
	t2.detach();
	std::cout << "main: thread exit" << std::endl;
	return 0;
}
