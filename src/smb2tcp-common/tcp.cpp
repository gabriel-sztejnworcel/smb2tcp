#include "tcp.h"
#include <stdexcept>
#include <string>
#include <wil/resource.h>

void winsock_init()
{
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0)
    {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
    }
}

void winsock_cleanup()
{
    int result = WSACleanup();
    if (result != 0)
    {
        throw std::runtime_error("WSACleanup failed: " + std::to_string(result));
    }
}

SOCKET tcp_connect(const char* host, const char* port)
{
    addrinfo hints = {}, * addr_info;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int result = getaddrinfo(host, port, &hints, &addr_info);
    if (result != 0)
    {
        throw std::runtime_error("getaddrinfo failed: " + std::to_string(result));
    }

    auto freeaddrinfo_scoped = wil::scope_exit([&addr_info]()
    {   
        freeaddrinfo(addr_info);
    });

    wil::unique_socket client_socket(socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol));
    if (client_socket.get() == INVALID_SOCKET)
    {
        throw std::runtime_error("socket failed: " + std::to_string(WSAGetLastError()));
    }

    result = connect(client_socket.get(), addr_info->ai_addr, (int)addr_info->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        throw std::runtime_error("connect failed: " + std::to_string(WSAGetLastError()));
    }

    return client_socket.release();
}

SOCKET tcp_listen(const char* host, const char* port)
{
    addrinfo hints = {}, * addr_info;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int result = getaddrinfo(host, port, &hints, &addr_info);
    if (result != 0)
    {
        throw std::runtime_error("getaddrinfo failed: " + std::to_string(result));
    }

    auto freeaddrinfo_scoped = wil::scope_exit([&addr_info]()
    {
        freeaddrinfo(addr_info);
    });

    wil::unique_socket listen_socket(socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol));
    if (listen_socket.get() == INVALID_SOCKET)
    {
        throw std::runtime_error("socket failed: " + std::to_string(WSAGetLastError()));
    }

    result = bind(listen_socket.get(), addr_info->ai_addr, (int)addr_info->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        throw std::runtime_error("bind failed: " + std::to_string(WSAGetLastError()));
    }

    result = listen(listen_socket.get(), SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        throw std::runtime_error("listen failed: " + std::to_string(WSAGetLastError()));
    }

    return listen_socket.release();
}

SOCKET tcp_accept(SOCKET listen_socket)
{
    SOCKET client_socket = accept(listen_socket, nullptr, nullptr);
    if (client_socket == INVALID_SOCKET)
    {
        throw std::runtime_error("accept failed: " + std::to_string(WSAGetLastError()));
    }

    return client_socket;
}

void tcp_send(TcpContext* tcp_context)
{
    int result = send(tcp_context->socket, tcp_context->buffer, tcp_context->bytes, 0);
    if (result == SOCKET_ERROR)
    {
        throw std::runtime_error("send failed: " + std::to_string(WSAGetLastError()));
    }

    tcp_context->bytes_written = result;
}

void tcp_recv_async_await(TcpContext* tcp_context)
{
    WSABUF buffer = { 0 };
    buffer.buf = tcp_context->buffer;
    buffer.len = CHANNEL_BUFFER_SIZE;

    DWORD flags = 0;
    WSAOVERLAPPED overlapped = {};
    overlapped.hEvent = WSACreateEvent();
    if (overlapped.hEvent == WSA_INVALID_EVENT)
    {
        throw std::runtime_error("WSACreateEvent failed: " + std::to_string(WSAGetLastError()));
    }

    auto wsacloseevent_scoped = wil::scope_exit([&overlapped]()
    {
        WSACloseEvent(overlapped.hEvent);
    });

    int result = WSARecv(
        tcp_context->socket,
        &buffer,
        1,
        nullptr,
        &flags,
        &overlapped,
        nullptr
    );

    if (result == SOCKET_ERROR)
    {
        DWORD last_error = WSAGetLastError();
        if (last_error != WSA_IO_PENDING)
        {
            throw std::runtime_error("WSARecv failed: " + std::to_string(WSAGetLastError()));
        }
    }

    DWORD wait_result = WSAWaitForMultipleEvents(
        1,
        &overlapped.hEvent,
        TRUE,
        WSA_INFINITE,
        TRUE
    );

    if (wait_result != WSA_WAIT_EVENT_0)
    {
        throw std::runtime_error("WSAWaitForMultipleEvents failed: " + std::to_string(WSAGetLastError()));
    }

    DWORD bytes_received = 0;
    result = WSAGetOverlappedResult(tcp_context->socket, &overlapped, &bytes_received, FALSE, &flags);
    if (result == SOCKET_ERROR)
    {
        throw std::runtime_error("WSAGetOverlappedResult failed: " + std::to_string(WSAGetLastError()));
    }

    if (bytes_received == 0)
    {
        throw std::runtime_error("WSARecv failed: 0 bytes received");
    }

    tcp_context->bytes_read = bytes_received;
}
