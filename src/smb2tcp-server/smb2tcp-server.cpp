#pragma comment(lib, "Rpcrt4.lib")

#include <iostream>
#include "rpc_server.h"

int main()
{
    try
    {
        RpcServer::instance().start();
    }
    catch (const std::exception& e)
    {
        wprintf(L"Error: %S\n", e.what());
        exit(1);
    }

    exit(0);
}
