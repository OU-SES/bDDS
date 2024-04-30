#include "TCPHandler.h"

TCPHandler::TCPHandler(std::string name)
{
    if (name == "broker")
    {
        client_type = 0x04;
    }
    else if (name =="DDS_publisher")
    {
        client_type = 0x01;
    }
    else if (name == "DDS_subscriber")
    {
        client_type = 0x02;
    }
    else if (name == "MQTT_subscriber")
    {
        client_type = 0x03;
    }
    else if (name == "broker_alt")
    {
        client_type = 0x06;
    }
}

int TCPHandler::connect_server(std::string ip, int port)
{
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) 
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    const char data[] = {0x01, client_type, '\0'};
    send_data(data, strlen(data));

    return 0;
}

int TCPHandler::report_read()
{
    const char data[] = { 0x02, 0x03, '\0' };
    send_data(data, strlen(data));
    return 0;
}

int TCPHandler::report_result()
{
    const char data[] = { 0x03, 0x03, '\0' };
    send_data(data, strlen(data));
    return 0;
}

int TCPHandler::report_reset()
{
    const char data[] = { 0x04, 0x03, '\0' };
    send_data(data, strlen(data));
    return 0;
}

int TCPHandler::recv_data(char* buf, int size)
{
    char data[512] = {};
    iResult = recv(ConnectSocket, buf, 512, 0);
    if (iResult > 0)
        printf("Bytes received: %d\n", iResult);
    else if (iResult == 0)
        printf("Connection closed\n");
    else
        printf("recv failed: %d\n", WSAGetLastError());
    return 0;
}

int TCPHandler::send_data(char const* buf, int size)
{
    // Send an initial buffer
    iResult = send(ConnectSocket, buf, size, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);
    return 0;
}

int TCPHandler::shut_down()
{
    shutdown(ConnectSocket, SD_SEND);
    return 0;
}
