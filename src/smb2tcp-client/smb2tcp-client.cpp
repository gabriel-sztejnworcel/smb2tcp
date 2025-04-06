#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Ws2_32.lib")

#include <tcp.h>
#include <iostream>
#include <stdexcept>
#include <smb2tcp-common.h>
#include <pipe.h>
#include <channel_client.h>
#include <channel_server.h>
#include "rpc_client.h"

#define PIPE_NAME_BUFFER_SIZE 1024
#define ENCRYPTION_KEY_BUFFER_SIZE 1024

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 7)
        {
            throw std::invalid_argument("Invalid arguments");
        }

        winsock_init();

        std::wstring server = str_to_wstr(argv[1]);
        std::wstring mode = str_to_wstr(argv[2]);
        std::string host_local = argv[3];
        std::string port_local = argv[4];
        std::wstring host_server = str_to_wstr(argv[5]);
        std::wstring port_server = str_to_wstr(argv[6]);

        if (mode != L"--local" && mode != L"--remote")
        {
            throw std::invalid_argument("Invalid mode. Use --local or --remote.");
        }

        RpcClient rpc_client(server);
        wchar_t pipe_name[PIPE_NAME_BUFFER_SIZE] = { 0 };
        wchar_t encryption_key[ENCRYPTION_KEY_BUFFER_SIZE] = { 0 };

        if (mode == L"--local")
        {
            HRESULT hr = rpc_client.create_local_port_forwarding(
                (wchar_t*)host_server.c_str(),
                std::stoi(port_server),
                PIPE_NAME_BUFFER_SIZE,
                pipe_name,
                ENCRYPTION_KEY_BUFFER_SIZE,
                encryption_key
            );

            if (FAILED(hr))
            {
                std::string msg = "Failed to create local port forwarding: " + std::to_string(hr);
                throw std::runtime_error(msg);
            }
        }
        else if (mode == L"--remote")
        {
            HRESULT hr = rpc_client.create_remote_port_forwarding(
                (wchar_t*)host_server.c_str(),
                std::stoi(port_server),
                PIPE_NAME_BUFFER_SIZE,
                pipe_name,
                ENCRYPTION_KEY_BUFFER_SIZE,
                encryption_key
            );

            if (FAILED(hr))
            {
                std::string msg = "Failed to create remote port forwarding: " + std::to_string(hr);
                throw std::runtime_error(msg);
            }
        }

        std::wstring full_pipe_name = L"\\\\" + server + L"\\pipe\\" + std::wstring(pipe_name);
        HANDLE pipe = create_pipe_client(full_pipe_name.c_str());
        wprintf(L"Connected to pipe server: %s\n", full_pipe_name.c_str());

        if (mode == L"--local")
        {
            ChannelServer channel(pipe, host_local, port_local);
            channel.start();
        }
        else if (mode == L"--remote")
        {
            ChannelClient channel(pipe, host_local, port_local);
            channel.start();
        }

        exit(0);
    }
    catch (const std::exception& e)
    {
        wprintf(L"Error: %S\n", e.what());
        exit(1);
    }
}
