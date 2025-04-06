#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Ws2_32.lib")

#include <tcp.h>
#include <smb2tcp-common.h>
#include <pipe.h>
#include <channel_client.h>
#include <channel_server.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "rpc_client.h"

#define PIPE_NAME_BUFFER_SIZE 1024
#define ENCRYPTION_KEY_BUFFER_SIZE 1024

struct Args
{
    std::string server;
    std::string mode;
    std::string listen_host;
    std::string listen_port;
    std::string connect_host;
    std::string connect_port;
};

void parse_args(int argc, char* argv[], Args& args);

int main(int argc, char* argv[])
{
    try
    {
        Args args;
        parse_args(argc, argv, args);

        winsock_init();

        std::wstring server = str_to_wstr(args.server);
        RpcClient rpc_client(server);
        wchar_t pipe_name[PIPE_NAME_BUFFER_SIZE] = { 0 };
        wchar_t encryption_key[ENCRYPTION_KEY_BUFFER_SIZE] = { 0 };

        if (args.mode == "local")
        {
            std::wstring connect_host = str_to_wstr(args.connect_host);

            HRESULT hr = rpc_client.create_local_port_forwarding(
                (wchar_t*)connect_host.c_str(),
                std::stoi(args.connect_port),
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
        else if (args.mode == "remote")
        {
            std::wstring listen_host = str_to_wstr(args.listen_host);

            HRESULT hr = rpc_client.create_remote_port_forwarding(
                (wchar_t*)listen_host.c_str(),
                std::stoi(args.listen_port),
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

        if (args.mode == "local")
        {
            ChannelServer channel(pipe, args.listen_host, args.listen_port);
            channel.start();
        }
        else if (args.mode == "remote")
        {
            ChannelClient channel(pipe, args.connect_host, args.connect_port);
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

void parse_args(int argc, char* argv[], Args& args)
{
    if (argc < 4)
    {
        throw std::invalid_argument("Invalid arguments");
    }

    std::string arg = argv[1];
    if (arg == "-L")
    {
        args.mode = "local";
    }
    else if (arg == "-R")
    {
        args.mode = "remote";
    }
    else
    {
        throw std::invalid_argument("Invalid mode. Use -L or -R.");
    }

    std::string address = argv[2];
    size_t colon_pos = address.find(':');
    if (colon_pos == std::string::npos)
    {
        throw std::invalid_argument("Invalid address format. Use <listen_host>:<listen_port>:<connect_host>:<connect_port> <server>.");
    }
    args.listen_host = address.substr(0, colon_pos);
    address = address.substr(colon_pos + 1);

    colon_pos = address.find(':');
    if (colon_pos == std::string::npos)
    {
        throw std::invalid_argument("Invalid address format. Use <listen_host>:<listen_port>:<connect_host>:<connect_port>.");
    }
    args.listen_port = address.substr(0, colon_pos);
    address = address.substr(colon_pos + 1);

    colon_pos = address.find(':');
    if (colon_pos == std::string::npos)
    {
        throw std::invalid_argument("Invalid address format. Use <listen_host>:<listen_port>:<connect_host>:<connect_port>.");
    }
    args.connect_host = address.substr(0, colon_pos);
    args.connect_port = address.substr(colon_pos + 1);

    args.server = argv[3];
}
