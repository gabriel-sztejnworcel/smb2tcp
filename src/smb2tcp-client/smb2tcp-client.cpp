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
        std::wstring host_local = str_to_wstr(argv[3]);
        std::wstring port_local = str_to_wstr(argv[4]);
        std::wstring host_server = str_to_wstr(argv[5]);
        std::wstring port_server = str_to_wstr(argv[6]);

        if (mode != L"--local" && mode != L"--remote")
        {
            throw std::invalid_argument("Invalid mode. Use --local or --remote.");
        }

        RpcClient rpc_client(server);

        if (mode == L"--local")
        {
            HRESULT hr = rpc_client.create_local_port_forwarding(
                (wchar_t*)host_local.c_str(),
                std::stoi(port_local),
                0,
                nullptr,
                0,
                nullptr
            );
        }
        else if (mode == L"--remote")
        {

        }

        //HANDLE pipe = create_pipe_client(pipe_name.c_str());
        //wprintf(L"Connected to pipe server: %s\n", pipe_name.c_str());

        //if (mode == "--listen")
        //{
        //    ChannelServer channel(pipe, host, port);
        //    channel.start();
        //}
        //else if (mode == "--connect")
        //{
        //    ChannelClient channel(pipe, host, port);
        //    channel.start();
        //}

        exit(0);
    }
    catch (const std::exception& e)
    {
        wprintf(L"Error: %S\n", e.what());
        exit(1);
    }
}
