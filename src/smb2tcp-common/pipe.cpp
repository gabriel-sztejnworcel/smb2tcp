#include "pipe.h"
#include <stdexcept>
#include <wil/resource.h>
#include <string>

HANDLE create_pipe_server(const wchar_t* pipe_name)
{
    wil::unique_handle pipe(CreateNamedPipe(
        pipe_name,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        TUNNEL_BUFFER_SIZE,
        TUNNEL_BUFFER_SIZE,
        0,
        nullptr
    ));

    if (pipe.get() == INVALID_HANDLE_VALUE)
    {
        std::string msg = "CreateNamedPipe failed: " + std::to_string(GetLastError());
        throw std::runtime_error(msg);
    }

    return pipe.release();
}

void wait_for_client(HANDLE pipe)
{
    if (!ConnectNamedPipe(pipe, nullptr))
    {
        DWORD last_error = GetLastError();
        if (last_error != ERROR_PIPE_CONNECTED)
        {
            std::string msg = "ConnectNamedPipe failed: " + std::to_string(last_error);
            throw std::runtime_error(msg);
        }
    }
}

HANDLE create_pipe_client(const wchar_t* pipe_name)
{
    wait_for_available_pipe(pipe_name, 20, 500);
    
    wil::unique_handle pipe(CreateFile(
        pipe_name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        nullptr
    ));

    if (pipe.get() == INVALID_HANDLE_VALUE)
    {
        std::string msg = "CreateFile failed: " + std::to_string(GetLastError());
        throw std::runtime_error(msg);
    }

    DWORD mode = PIPE_READMODE_MESSAGE;
    BOOL result = SetNamedPipeHandleState(
        pipe.get(),
        &mode,
        nullptr,
        nullptr
    );

    if (!result)
    {
        std::string msg = "SetNamedPipeHandleState failed: " + std::to_string(GetLastError());
        throw std::runtime_error(msg);
    }

    return pipe.release();
}

void wait_for_available_pipe(const wchar_t* pipe_name, int retries, DWORD interval)
{
    for (int i = 0; i < retries; ++i)
    {
        if (!WaitNamedPipe(pipe_name, NMPWAIT_NOWAIT))
        {
            Sleep(interval);
            continue;
        }

        return;
    }

    throw std::runtime_error("WaitNamedPipe failed: " + std::to_string(GetLastError()));
}

void pipe_write(PipeContext* pipe_context)
{
    BOOL result = WriteFile(
        pipe_context->pipe,
        pipe_context->buffer,
        pipe_context->bytes,
        &pipe_context->bytes_written,
        nullptr
    );

    if (!result)
    {
        throw std::runtime_error("WriteFile failed: " + std::to_string(GetLastError()));
    }
}

void pipe_write_async_await(PipeContext* pipe_context)
{
    wil::unique_event evt(CreateEvent(nullptr, TRUE, FALSE, nullptr));
    if (evt.get() == nullptr)
    {
        throw std::runtime_error("CreateEvent failed: " + std::to_string(GetLastError()));
    }

    OVERLAPPED overlapped = {};
    overlapped.hEvent = evt.get();

    BOOL result = WriteFile(
        pipe_context->pipe,
        pipe_context->buffer,
        pipe_context->bytes,
        &pipe_context->bytes_written,
        &overlapped
    );

    if (!result)
    {
        DWORD last_error = GetLastError();
        if (last_error != ERROR_IO_PENDING)
        {
            throw std::runtime_error("WriteFile failed: " + std::to_string(last_error));
        }
    }

    DWORD wait_result = WaitForSingleObject(overlapped.hEvent, INFINITE);
    if (wait_result != WAIT_OBJECT_0)
    {
        throw std::runtime_error("WaitForSingleObject failed: " + std::to_string(GetLastError()));
    }

    if (!GetOverlappedResult(pipe_context->pipe, &overlapped, &pipe_context->bytes_written, FALSE))
    {
        throw std::runtime_error("GetOverlappedResult failed: " + std::to_string(GetLastError()));
    }

    if (pipe_context->bytes_written == 0)
    {
        throw std::runtime_error("WriteFile failed: 0 bytes written");
    }
}

void pipe_read_async_await(PipeContext* pipe_context)
{
    wil::unique_event evt(CreateEvent(nullptr, TRUE, FALSE, nullptr));
    if (evt.get() == nullptr)
    {
        throw std::runtime_error("CreateEvent failed: " + std::to_string(GetLastError()));
    }
    
    OVERLAPPED overlapped = {};
    overlapped.hEvent = evt.get();

    BOOL result = ReadFile(
        pipe_context->pipe,
        pipe_context->buffer,
        TUNNEL_BUFFER_SIZE,
        &pipe_context->bytes_read,
        &overlapped
    );

    if (!result)
    {
        DWORD last_error = GetLastError();
        if (last_error != ERROR_IO_PENDING && last_error != ERROR_MORE_DATA)
        {
            throw std::runtime_error("ReadFile failed: " + std::to_string(last_error));
        }
    }

    DWORD wait_result = WaitForSingleObject(overlapped.hEvent, INFINITE);
    if (wait_result != WAIT_OBJECT_0)
    {
        throw std::runtime_error("WaitForSingleObject failed: " + std::to_string(GetLastError()));
    }

    if (!GetOverlappedResult(pipe_context->pipe, &overlapped, &pipe_context->bytes_read, FALSE))
    {
        DWORD last_error = GetLastError();
        if (last_error != ERROR_MORE_DATA)
        {
            throw std::runtime_error("GetOverlappedResult failed: " + std::to_string(last_error));
        }
    }

    if (pipe_context->bytes_read == 0)
    {
        throw std::runtime_error("ReadFile failed: 0 bytes read");
    }
}
