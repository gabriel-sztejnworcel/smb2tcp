#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include "smb2tcp-common.h"

struct TcpContext
{
    uint32_t client_id;
    SOCKET socket;
    char buffer[CHANNEL_BUFFER_SIZE];
    DWORD bytes;
    DWORD bytes_read;
    DWORD bytes_written;
};

void winsock_init();
void winsock_cleanup();

SOCKET tcp_connect(const char* host, const char* port);
SOCKET tcp_listen(const char* host, const char* port);
SOCKET tcp_accept(SOCKET listen_socket);
void tcp_send(TcpContext* tcp_context);
void tcp_recv_async_await(TcpContext* tcp_context);
