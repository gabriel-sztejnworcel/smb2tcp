#include "channel_server.h"
#include "pipe.h"
#include <thread>

ChannelServer::ChannelServer(HANDLE pipe, const std::string& host, const std::string& port)
    : ChannelBase(pipe, host, port)
{

}

void ChannelServer::start()
{
    std::thread pipe_thread(&ChannelServer::pipe_thread_fn, this);
    pipe_thread.detach();

    wil::unique_socket listen_socket(tcp_listen(host_.c_str(), port_.c_str()));
    while (true)
    {
        SOCKET client_socket = tcp_accept(listen_socket.get());
        create_channel(client_socket);
    }
}

void ChannelServer::create_channel(SOCKET client_socket)
{
    TcpContext& tcp_write_context = add_tcp_write_context(client_socket);
    PipeContext pipe_write_context = { 0 };
    pipe_write_context.pipe = pipe_.get();

    TunnelMessage* tunnel_message = (TunnelMessage*)pipe_write_context.buffer;
    ZeroMemory(tunnel_message, TUNNEL_BUFFER_SIZE);
    tunnel_message->header.type = TunnelMessageType::CONNECT;
    tunnel_message->header.client_id = tcp_write_context.client_id;
    tunnel_message->header.len = 0;

    pipe_write_context.bytes = (DWORD)sizeof(TunnelMessageHeader);
    pipe_write_async_await(&pipe_write_context);

    std::thread tcp_thread(&ChannelServer::tcp_thread_fn, this, client_socket, tcp_write_context.client_id);
    tcp_thread.detach();
}

void ChannelServer::handle_channel_connect(TunnelMessage* message)
{
    throw std::runtime_error("handle_channel_connect: Unexpected CONNECT message");
}
