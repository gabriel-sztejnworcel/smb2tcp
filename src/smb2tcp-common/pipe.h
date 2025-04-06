#pragma once

#include <Windows.h>
#include "smb2tcp-common.h"

struct PipeContext
{
    HANDLE pipe;
    char buffer[TUNNEL_BUFFER_SIZE];
    DWORD bytes;
    DWORD bytes_read;
    DWORD bytes_written;
};

HANDLE create_pipe_server(const wchar_t* pipe_name);
void wait_for_client(HANDLE pipe);
HANDLE create_pipe_client(const wchar_t* pipe_name);
void wait_for_available_pipe(const wchar_t* pipe_name, int retries, DWORD interval);
void pipe_write(PipeContext* pipe_context);
void pipe_write_async_await(PipeContext* pipe_context);
void pipe_read_async_await(PipeContext* pipe_context);
