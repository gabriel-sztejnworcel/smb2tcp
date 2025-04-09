#pragma once

#include "tcp.h"
#include <Windows.h>
#include <wil/resource.h>
#include <string>
#include <unordered_map>
#include <mutex>
#include <cinttypes>
#include "smb2tcp-common.h"

class ChannelBase
{
protected:
    ChannelBase(HANDLE pipe, const std::string& host, const std::string& port, const BYTE* key, ULONG key_len);
    virtual ~ChannelBase() = default;

    void pipe_thread_fn();
    void tcp_thread_fn(SOCKET client_socket, uint32_t client_id);

    virtual void handle_channel_connect(TunnelMessage* message) = 0;
    void handle_channel_disconnect(TunnelMessage* message);
    void handle_channel_data(TunnelMessage* message);

    void destroy_channel(SOCKET client_socket, uint32_t client_id, bool send_disconnect_message);

    TcpContext& add_tcp_write_context(SOCKET client_socket);
    TcpContext& add_tcp_write_context(SOCKET client_socket, uint32_t client_id);
    TcpContext& get_tcp_write_context(uint32_t client_id);
    void remove_tcp_write_context(uint32_t client_id);

    wil::unique_handle pipe_;
    std::string host_;
    std::string port_;
    wil::unique_bcrypt_key encryption_key_;
    wil::unique_bcrypt_algorithm alg_handle_;

private:
    std::unordered_map<uint32_t, TcpContext> tcp_write_contexts_;
    std::mutex tcp_write_contexts_mutex_;
};
