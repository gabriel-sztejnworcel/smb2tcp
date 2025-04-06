#pragma comment(lib, "Ws2_32.lib")

#include <tcp.h>
#include <iostream>
#include <stdexcept>
#include <smb2tcp-common.h>
#include <pipe.h>
#include <channel_client.h>
#include <channel_server.h>

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 5)
        {
            throw std::invalid_argument("Invalid arguments");
        }

        winsock_init();

        std::wstring pipe_name = str_to_wstr(argv[1]);
        std::string mode = argv[2];
        std::string host = argv[3];
        std::string port = argv[4];

        if (mode != "--listen" && mode != "--connect")
        {
            throw std::invalid_argument("Invalid mode. Use --listen or --connect.");
        }

        HANDLE pipe = create_pipe_server(pipe_name.c_str());
        wait_for_client(pipe);
        wprintf(L"Client connected to pipe: %s\n", pipe_name.c_str());

        if (mode == "--listen")
        {
            ChannelServer channel(pipe, host, port);
            channel.start();
        }
        else if (mode == "--connect")
        {
            ChannelClient channel(pipe, host, port);
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
