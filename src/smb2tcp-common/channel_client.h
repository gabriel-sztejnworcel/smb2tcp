#pragma once

#include "channel_base.h"

class ChannelClient : public ChannelBase
{
public:
    ChannelClient(HANDLE pipe, const std::string& host, const std::string& port, const BYTE* key, ULONG key_len);
    void start();

protected:
    void handle_channel_connect(TunnelMessage* message) override;
};
