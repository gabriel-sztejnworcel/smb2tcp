#pragma once

#include <Windows.h>

class RpcServer
{
public:
    static RpcServer& instance();

    void start();

    HRESULT create_local_port_forwarding(
        /* [in] */ handle_t IDL_handle,
        /* [string][in] */ wchar_t* connect_host,
        /* [in] */ int connect_port,
        /* [in] */ int pipe_name_size,
        /* [string][size_is][out] */ wchar_t* pipe_name,
        /* [in] */ int encryption_key_size,
        /* [string][size_is][out] */ wchar_t* encryption_key
    );

    HRESULT create_remote_port_forwarding(
        /* [in] */ handle_t IDL_handle,
        /* [string][in] */ wchar_t* listen_host,
        /* [in] */ int listen_port,
        /* [in] */ int pipe_name_size,
        /* [string][size_is][out] */ wchar_t* pipe_name,
        /* [in] */ int encryption_key_size,
        /* [string][size_is][out] */ wchar_t* encryption_key
    );

private:
};
