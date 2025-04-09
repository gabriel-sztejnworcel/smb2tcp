#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")

#include <tcp.h>
#include <iostream>
#include <stdexcept>
#include <smb2tcp-common.h>
#include <pipe.h>
#include <channel_client.h>
#include <channel_server.h>
#include <wil/resource.h>

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 5)
        {
            throw std::invalid_argument("Invalid arguments");
        }

        winsock_init();

        auto winsock_cleanip_scope =
            wil::scope_exit([]() { winsock_cleanup(); });

        std::wstring pipe_name = str_to_wstr(argv[1]);
        std::string mode = argv[2];
        std::string host = argv[3];
        std::string port = argv[4];

        if (mode != "--listen" && mode != "--connect")
        {
            throw std::invalid_argument("Invalid mode. Use --listen or --connect.");
        }

        wprintf(L"Creating tunnel server: pipe_name=%s, mode=%s, host=%s, port=%s\n",
            pipe_name.c_str(), str_to_wstr(mode).c_str(), str_to_wstr(host).c_str(), str_to_wstr(port).c_str());

        HANDLE pipe = create_pipe_server(pipe_name.c_str());
        wait_for_client(pipe);
        wprintf(L"Client connected\n");

        std::string encryption_key = "11111111111111111111111111111111";
        
        if (mode == "--listen")
        {
            ChannelServer channel_server(pipe, host, port, (const BYTE*)encryption_key.c_str(), (ULONG)encryption_key.length());
            channel_server.start();
        }
        else if (mode == "--connect")
        {
            ChannelClient channel_client(pipe, host, port, (const BYTE*)encryption_key.c_str(), (ULONG)encryption_key.length());
            channel_client.start();
        }

        exit(0);
    }
    catch (const std::exception& e)
    {
        wprintf(L"Error: %S\n", e.what());
        exit(1);
    }
}
