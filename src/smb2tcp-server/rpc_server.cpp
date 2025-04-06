#include "rpc_server.h"
#include <smb2tcp_rpc.h>
#include <iostream>
#include <stdexcept>
#include <string>

#define PROTSEQ L"ncacn_np";
#define ENDPOINT L"\\pipe\\smb2tcp-rpc";
#define TUNNEL_PROCESS_NAME L"smb2tcp-tunnel.exe"

RpcServer& RpcServer::instance()
{
    static RpcServer instance;
    return instance;
}

void RpcServer::start()
{
    wchar_t protseq[] = PROTSEQ;
    wchar_t endpoint[] = ENDPOINT;

    RPC_STATUS rpc_status = RPC_S_OK;

    rpc_status = RpcServerUseProtseqEp(
        (RPC_WSTR)protseq, RPC_C_LISTEN_MAX_CALLS_DEFAULT, (RPC_WSTR)endpoint, nullptr);

    if (rpc_status != RPC_S_OK)
    {
        std::string msg = "RpcServerUseProtseqEp failed: " + std::to_string(rpc_status);
        throw std::runtime_error(msg);
    }

    rpc_status = RpcServerRegisterIf(s_smb2tcp_rpc_v1_0_s_ifspec, nullptr, nullptr);
    if (rpc_status != RPC_S_OK)
    {
        std::string msg = "RpcServerRegisterIf failed: " + std::to_string(rpc_status);
        throw std::runtime_error(msg);
    }

    rpc_status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);
    if (rpc_status != RPC_S_OK)
    {
        std::string msg = "RpcServerListen failed: " + std::to_string(rpc_status);
        throw std::runtime_error(msg);
    }
}

HRESULT RpcServer::create_local_port_forwarding(
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ wchar_t* connect_host,
    /* [in] */ int connect_port,
    /* [in] */ int pipe_name_size,
    /* [string][size_is][out] */ wchar_t* pipe_name,
    /* [in] */ int encryption_key_size,
    /* [string][size_is][out] */ wchar_t* encryption_key)
{
    wprintf(L"RpcServer::create_local_port_forwarding\n");
    return S_OK;
}

HRESULT RpcServer::create_remote_port_forwarding(
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ wchar_t* listen_host,
    /* [in] */ int listen_port,
    /* [in] */ int pipe_name_size,
    /* [string][size_is][out] */ wchar_t* pipe_name,
    /* [in] */ int encryption_key_size,
    /* [string][size_is][out] */ wchar_t* encryption_key)
{
    wprintf(L"RpcServer::create_remote_port_forwarding\n");
    return S_OK;
}

HRESULT s_create_local_port_forwarding(
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ wchar_t* connect_host,
    /* [in] */ int connect_port,
    /* [in] */ int pipe_name_size,
    /* [string][size_is][out] */ wchar_t* pipe_name,
    /* [in] */ int encryption_key_size,
    /* [string][size_is][out] */ wchar_t* encryption_key)
{
    return RpcServer::instance().create_local_port_forwarding(
        IDL_handle,
        connect_host,
        connect_port,
        pipe_name_size,
        pipe_name,
        encryption_key_size,
        encryption_key
    );
}

HRESULT s_create_remote_port_forwarding(
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ wchar_t* listen_host,
    /* [in] */ int listen_port,
    /* [in] */ int pipe_name_size,
    /* [string][size_is][out] */ wchar_t* pipe_name,
    /* [in] */ int encryption_key_size,
    /* [string][size_is][out] */ wchar_t* encryption_key)
{
    return RpcServer::instance().create_remote_port_forwarding(
        IDL_handle,
        listen_host,
        listen_port,
        pipe_name_size,
        pipe_name,
        encryption_key_size,
        encryption_key
    );
}
