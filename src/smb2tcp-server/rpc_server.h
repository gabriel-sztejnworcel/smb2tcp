#pragma once

#include <Windows.h>
#include <string>

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
        /* [string][size_is][out] */ wchar_t* pipe_name
    );

    HRESULT create_remote_port_forwarding(
        /* [in] */ handle_t IDL_handle,
        /* [string][in] */ wchar_t* listen_host,
        /* [in] */ int listen_port,
        /* [in] */ int pipe_name_size,
        /* [string][size_is][out] */ wchar_t* pipe_name
    );

private:
    void validate_host(const wchar_t* host);
    bool has_illegal_ip_characters(const std::wstring& ip);
    bool has_illegal_hostname_characters(const std::wstring& hostname);
    
    HRESULT create_pipe_name(wchar_t* pipe_name, int pipe_name_size);

    HRESULT run_tunnel_process(
        const std::wstring& pipe_name,
        const std::wstring& mode,
        const std::wstring& host,
        int port
    );
};
