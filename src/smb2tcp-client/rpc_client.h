#pragma once

#include <wil/resource.h>
#include <string>

class RpcClient
{
public:
    RpcClient(const std::wstring& host);

    HRESULT create_local_port_forwarding(
        /* [string][in] */ wchar_t* connect_host,
        /* [in] */ int connect_port,
        /* [in] */ int pipe_name_size,
        /* [string][size_is][out] */ wchar_t* pipe_name,
        /* [in] */ int encryption_key_size,
        /* [string][size_is][out] */ wchar_t* encryption_key
    );

    HRESULT create_remote_port_forwarding(
        /* [string][in] */ wchar_t* listen_host,
        /* [in] */ int listen_port,
        /* [in] */ int pipe_name_size,
        /* [string][size_is][out] */ wchar_t* pipe_name,
        /* [in] */ int encryption_key_size,
        /* [string][size_is][out] */ wchar_t* encryption_key
    );

private:
    void create_binding(const std::wstring& host);
    wil::unique_rpc_binding binding_handle_;
};
