#pragma once

#include "channel_base.h"

class ChannelServer : public ChannelBase
{
public:
    ChannelServer(HANDLE pipe, const std::string& host, const std::string& port, const BYTE* key, ULONG key_len);
    void start();

protected:
    void create_channel(SOCKET client_socket);
    void handle_channel_connect(TunnelMessage* message) override;
};
