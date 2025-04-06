#include "channel_client.h"
#include <thread>

ChannelClient::ChannelClient(HANDLE pipe, const std::string& host, const std::string& port)
    : ChannelBase(pipe, host, port)
{

}

void ChannelClient::start()
{
    std::thread pipe_thread(&ChannelClient::pipe_thread_fn, this);
    pipe_thread.detach();
    Sleep(INFINITE);
}

void ChannelClient::handle_channel_connect(TunnelMessage* message)
{
    SOCKET socket = tcp_connect(host_.c_str(), port_.c_str());
    TcpContext & tcp_write_context = add_tcp_write_context(socket, message->header.client_id);
    std::thread tcp_thread(&ChannelClient::tcp_thread_fn, this, socket, tcp_write_context.client_id);
    tcp_thread.detach();
}
