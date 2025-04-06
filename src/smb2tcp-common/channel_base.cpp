#include "channel_base.h"
#include "pipe.h"
#include <stdexcept>
#include <mutex>

ChannelBase::ChannelBase(HANDLE pipe, const std::string& host, const std::string& port)
    : pipe_(pipe), host_(host), port_(port)
{

}

void ChannelBase::pipe_thread_fn()
{
    try
    {
        PipeContext pipe_read_context;

        while (true)
        {
            ZeroMemory(&pipe_read_context, sizeof(PipeContext));
            pipe_read_context.pipe = pipe_.get();

            pipe_read_async_await(&pipe_read_context);
            TunnelMessage* message = (TunnelMessage*)pipe_read_context.buffer;
            switch (message->header.type)
            {
            case TunnelMessageType::CONNECT:
                handle_channel_connect(message);
                break;

            case TunnelMessageType::DISCONNECT:
                handle_channel_disconnect(message);
                break;

            case TunnelMessageType::DATA:
                handle_channel_data(message);
                break;

            default:
                throw std::runtime_error("Unknown message type");
                break;
            }
        }
    }
    catch (std::exception& e)
    {
        wprintf(L"Error: %S\n", e.what());
        exit(1);
    }

    exit(1);
}

void ChannelBase::tcp_thread_fn(SOCKET client_socket, uint32_t client_id)
{
    try
    {
        TcpContext tcp_read_context;

        while (true)
        {
            ZeroMemory(&tcp_read_context, sizeof(TcpContext));
            tcp_read_context.socket = client_socket;
            tcp_read_context.client_id = client_id;

            tcp_recv_async_await(&tcp_read_context);

            PipeContext pipe_write_context = { 0 };
            pipe_write_context.pipe = pipe_.get();

            TunnelMessage* tunnel_message = (TunnelMessage*)pipe_write_context.buffer;
            ZeroMemory(tunnel_message, TUNNEL_BUFFER_SIZE);
            tunnel_message->header.type = TunnelMessageType::DATA;
            tunnel_message->header.client_id = tcp_read_context.client_id;
            tunnel_message->header.len = tcp_read_context.bytes_read;
            memcpy(tunnel_message->buffer, tcp_read_context.buffer, tunnel_message->header.len);

            pipe_write_context.bytes = (DWORD)(sizeof(TunnelMessageHeader) + tunnel_message->header.len);
            pipe_write_async_await(&pipe_write_context);
        }
    }
    catch (std::exception& e)
    {
        wprintf(L"Error: %S\n", e.what());
    }

    destroy_channel(client_socket, client_id, true);
}

void ChannelBase::handle_channel_disconnect(TunnelMessage* message)
{
    destroy_channel(message->header.client_id, message->header.client_id, false);
}

void ChannelBase::handle_channel_data(TunnelMessage* message)
{
    try
    {
        TcpContext& tcp_write_context = get_tcp_write_context(message->header.client_id);
        memcpy(tcp_write_context.buffer, message->buffer, message->header.len);
        tcp_write_context.bytes = (DWORD)message->header.len;
        tcp_send(&tcp_write_context);
    }
    catch (std::exception& e)
    {
        wprintf(L"Error: %S\n", e.what());
        destroy_channel(message->header.client_id, message->header.client_id, true);
    }
}

void ChannelBase::destroy_channel(SOCKET client_socket, uint32_t client_id, bool send_disconnect_message)
{
    if (send_disconnect_message)
    {
        PipeContext pipe_write_context = { 0 };
        pipe_write_context.pipe = pipe_.get();

        TunnelMessage* tunnel_message = (TunnelMessage*)pipe_write_context.buffer;
        ZeroMemory(tunnel_message, TUNNEL_BUFFER_SIZE);
        tunnel_message->header.type = TunnelMessageType::DISCONNECT;
        tunnel_message->header.client_id = client_id;
        tunnel_message->header.len = 0;

        pipe_write_context.bytes = (DWORD)sizeof(TunnelMessageHeader);
        pipe_write_async_await(&pipe_write_context);
    }
    
    closesocket(client_socket);
    remove_tcp_write_context(client_id);
}

TcpContext& ChannelBase::add_tcp_write_context(SOCKET client_socket)
{
    std::lock_guard<std::mutex> lock(tcp_write_contexts_mutex_);
    TcpContext context = { 0 };
    context.client_id = (uint32_t)tcp_write_contexts_.size();
    context.socket = client_socket;
    tcp_write_contexts_[context.client_id] = context;
    return tcp_write_contexts_[context.client_id];
}

TcpContext& ChannelBase::add_tcp_write_context(SOCKET client_socket, uint32_t client_id)
{
    std::lock_guard<std::mutex> lock(tcp_write_contexts_mutex_);
    TcpContext context = { 0 };
    context.client_id = client_id;
    context.socket = client_socket;
    tcp_write_contexts_[context.client_id] = context;
    return tcp_write_contexts_[context.client_id];
}

TcpContext& ChannelBase::get_tcp_write_context(uint32_t client_id)
{
    std::lock_guard<std::mutex> lock(tcp_write_contexts_mutex_);
    auto it = tcp_write_contexts_.find(client_id);
    if (it == tcp_write_contexts_.end())
    {
        throw std::runtime_error("Client ID not found");
    }
    return it->second;
}

void ChannelBase::remove_tcp_write_context(uint32_t client_id)
{
    std::lock_guard<std::mutex> lock(tcp_write_contexts_mutex_);
    auto it = tcp_write_contexts_.find(client_id);
    if (it != tcp_write_contexts_.end())
    {
        closesocket(it->second.socket);
        tcp_write_contexts_.erase(it);
    }
}
