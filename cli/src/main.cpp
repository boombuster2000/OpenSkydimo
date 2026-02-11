#include <cstring>

#include <cerrno>
#include <functional>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "CLI/CLI.hpp"
#include "spdlog/fmt/bundled/format.h"

#include "openskydimo/commands.hpp"

bool SendCommand(const std::string& command)
{
    const std::string socketPath = "/tmp/openskydimo.sock";

    // RAII wrapper for socket file descriptor
    struct SocketGuard
    {
        int fd;
        explicit SocketGuard(const int fd_) : fd(fd_)
        {
        }

        ~SocketGuard()
        {
            if (fd >= 0)
                close(fd);
        }

        SocketGuard(const SocketGuard&) = delete;
        SocketGuard& operator=(const SocketGuard&) = delete;
        explicit operator int() const
        {
            return fd;
        }
    };

    const SocketGuard sockFd(socket(AF_UNIX, SOCK_STREAM, 0));

    if (static_cast<int>(sockFd) < 0)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(static_cast<int>(sockFd), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        std::cerr << "Failed to connect (is daemon running?)" << std::endl;
        return false;
    }

    const std::string msg = command + "\n";
    size_t totalWritten = 0;
    const size_t msgLen = msg.length();

    while (totalWritten < msgLen)
    {
        const ssize_t bytesWritten = write(static_cast<int>(sockFd), msg.c_str() + totalWritten, msgLen - totalWritten);

        if (bytesWritten < 0)
        {
            std::cerr << "Write error: " << strerror(errno) << std::endl;
            return false;
        }

        if (bytesWritten == 0)
        {
            std::cerr << "Write returned 0 (connection closed?)" << std::endl;
            return false;
        }

        totalWritten += static_cast<size_t>(bytesWritten);
    }

    // Read response with loop to handle chunked data
    std::string response;
    char buffer[128];
    bool receivedData = false;

    while (true)
    {
        const ssize_t bytesRead = read(static_cast<int>(sockFd), buffer, sizeof(buffer) - 1);

        if (bytesRead < 0)
        {
            std::cerr << "Read error: " << strerror(errno) << std::endl;
            return false;
        }

        if (bytesRead == 0)
            break; // EOF - server closed connection

        receivedData = true;
        buffer[bytesRead] = '\0';
        response.append(buffer, static_cast<size_t>(bytesRead));

        // If response contains newline, assume message is complete
        if (response.find('\n') != std::string::npos)
            break;
    }

    if (receivedData)
    {
        std::cout << "[SERVER] - " << response;
        // Ensure output ends with newline if server didn't provide one
        if (!response.empty() && response.back() != '\n')
        {
            std::cout << std::endl;
        }
    }

    return true;
}

std::string JoinArgs(const int argc, char* argv[])
{
    std::string cmd;

    for (int i = 1; i < argc; ++i)
    {
        cmd += argv[i];

        if (i < argc - 1)
            cmd += " ";
    }

    return cmd;
}

int main(const int argc, char* argv[])
{
    using namespace openskydimo::commands;
    CLI::App app{"This program is used to communicate with the skydimo daemon and configure the LEDs."};
    argv = app.ensure_utf8(argv);

    const std::string cmd = JoinArgs(argc, argv);

    Args cmdArgs;

    AddFillCmd(&app, [&] { SendCommand(cmd); }, cmdArgs.fillColor);

    const auto setCmd = AddSetCmd(&app);
    AddSetPortCmd(setCmd, [&] { SendCommand(cmd); }, cmdArgs.serialPort);
    AddSetCountCmd(setCmd, [&] { SendCommand(cmd); }, cmdArgs.ledCount);

    AddStartCmd(&app, [&] { SendCommand(cmd); });
    AddStopCmd(&app, [&] { SendCommand(cmd); });

    CLI11_PARSE(app, argc, argv);
    return 0;
}
