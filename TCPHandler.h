#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "7777"

static bool running = true;

class TCPHandler
{
public:
	TCPHandler(std::string name);
	int connect_server(std::string ip, int port);
	int report_read();
	int report_result();
	int report_reset();
	int shut_down();
	int recv_data(char* buf, int size);
private:
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	WSADATA wsaData;
	int send_data(char const* buf, int size);
	int client_type;
};

