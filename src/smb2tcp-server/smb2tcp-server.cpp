#pragma comment(lib, "Rpcrt4.lib")

#include <iostream>
#include "rpc_server.h"

int main()
{
    try
    {
        RpcServer::instance().start();
    }
    catch (const std::exception& ex)
    {
        wprintf(L"%hs\n", ex.what());
        exit(1);
    }

    exit(0);
}
