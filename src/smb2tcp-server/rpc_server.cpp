#include "rpc_server.h"
#include <smb2tcp_rpc.h>
#include <iostream>
#include <stdexcept>
#include <Windows.h>

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

    if (FAILED(create_pipe_name(pipe_name, pipe_name_size)))
    {
        return E_FAIL;
    }

    std::wstring full_pipe_name = L"\\\\.\\pipe\\" + std::wstring(pipe_name);

    wcscpy_s(encryption_key, encryption_key_size, L"");

    if (FAILED(run_tunnel_process(full_pipe_name, L"--connect", connect_host, connect_port)))
    {
        return E_FAIL;
    }

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

    if (FAILED(create_pipe_name(pipe_name, pipe_name_size)))
    {
        return E_FAIL;
    }

    std::wstring full_pipe_name = L"\\\\.\\pipe\\" + std::wstring(pipe_name);

    wcscpy_s(encryption_key, encryption_key_size, L"");

    if (FAILED(run_tunnel_process(full_pipe_name, L"--listen", listen_host, listen_port)))
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT RpcServer::create_pipe_name(wchar_t* pipe_name, int pipe_name_size)
{
    if (pipe_name_size < 40)
    {
        return E_INVALIDARG;
    }

    GUID guid;
    if (FAILED(CoCreateGuid(&guid)))
    {
        return E_FAIL;
    }

    wchar_t guidString[40];
    if (StringFromGUID2(guid, guidString, 40) == 0)
    {
        return E_FAIL;
    }

    wcscpy_s(pipe_name, pipe_name_size, guidString);
    return S_OK;
}

HRESULT RpcServer::run_tunnel_process(
    const std::wstring& pipe_name,
    const std::wstring& mode,
    const std::wstring& host,
    int port)
{
    // TODO: Check for command injection
    std::wstring command = L"smb2tcp-tunnel.exe " + pipe_name
        + L" " + mode + L" " + host + L" " + std::to_wstring(port);

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    if (!CreateProcess(nullptr, const_cast<LPWSTR>(command.c_str()), nullptr,
        nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi))
    {
        return E_FAIL;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

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
