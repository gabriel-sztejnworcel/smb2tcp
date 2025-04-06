#include "rpc_client.h"
#include <smb2tcp_rpc.h>
#include <stdexcept>

#define PROTSEQ L"ncacn_np";
#define ENDPOINT L"\\pipe\\smb2tcp-rpc";

RpcClient::RpcClient(const std::wstring& host)
{
    create_binding(host);
}

HRESULT RpcClient::create_local_port_forwarding(
    /* [string][in] */ wchar_t* connect_host,
    /* [in] */ int connect_port,
    /* [in] */ int pipe_name_size,
    /* [string][size_is][out] */ wchar_t* pipe_name,
    /* [in] */ int encryption_key_size,
    /* [string][size_is][out] */ wchar_t* encryption_key)
{
    HRESULT hr = S_OK;

    __try
    {
        hr = c_create_local_port_forwarding(
            binding_handle_.get(),
            connect_host,
            connect_port,
            pipe_name_size,
            pipe_name,
            encryption_key_size,
            encryption_key
        );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hr = HRESULT_FROM_WIN32(GetExceptionCode());
    }

    return hr;
}

HRESULT RpcClient::create_remote_port_forwarding(
    /* [string][in] */ wchar_t* listen_host,
    /* [in] */ int listen_port,
    /* [in] */ int pipe_name_size,
    /* [string][size_is][out] */ wchar_t* pipe_name,
    /* [in] */ int encryption_key_size,
    /* [string][size_is][out] */ wchar_t* encryption_key
)
{
    HRESULT hr = S_OK;
    
    __try
    {
        hr = c_create_remote_port_forwarding(
            binding_handle_.get(),
            listen_host,
            listen_port,
            pipe_name_size,
            pipe_name,
            encryption_key_size,
            encryption_key
        );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hr = HRESULT_FROM_WIN32(GetExceptionCode());
    }

    return hr;
}

void RpcClient::create_binding(const std::wstring& host)
{
    wchar_t protseq[] = PROTSEQ;
    wchar_t endpoint[] = ENDPOINT;

    RPC_STATUS rpc_status = RPC_S_OK;
    
    wil::unique_rpc_wstr string_binding;

    rpc_status = RpcStringBindingCompose(
        nullptr,
        (RPC_WSTR)protseq,
        (RPC_WSTR)host.c_str(),
        (RPC_WSTR)endpoint,
        nullptr,
        &string_binding);

    if (rpc_status != RPC_S_OK)
    {
        std::string msg = "RpcStringBindingCompose failed: " + std::to_string(rpc_status);
        throw std::runtime_error(msg);
    }

    rpc_status = RpcBindingFromStringBinding(string_binding.get(), &binding_handle_);

    if (rpc_status != RPC_S_OK)
    {
        std::string msg = "RpcBindingFromStringBinding failed: " + std::to_string(rpc_status);
        throw std::runtime_error(msg);
    }
}
