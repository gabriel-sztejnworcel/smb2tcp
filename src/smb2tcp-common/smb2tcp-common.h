#pragma once

#include <cinttypes>
#include <string>
#include <Windows.h>

enum class TunnelMessageType : uint32_t
{
    CONNECT,
    DISCONNECT,
    DATA
};

struct TunnelMessageHeader
{
    TunnelMessageType type;
    uint32_t client_id;
    uint64_t len;
};

#define CHANNEL_BUFFER_SIZE 4096

struct TunnelMessage
{
    TunnelMessageHeader header;
    char buffer[CHANNEL_BUFFER_SIZE];
};

#define TUNNEL_BUFFER_SIZE sizeof(TunnelMessage)
#define TUNNEL_ENCRYPTED_BUFFER_SIZE TUNNEL_BUFFER_SIZE + 32

std::wstring str_to_wstr(const std::string& str);
void print_hex_dump(BYTE* buffer, DWORD length);
