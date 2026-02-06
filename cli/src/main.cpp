#include <cstring>

#include <functional>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "CLI/CLI.hpp"
#include "spdlog/fmt/bundled/format.h"

#include "openskydimo/commands.hpp"
#include "openskydimo/types.h"

bool SendCommand(const std::string& command)
{
    const std::string socketPath = "/tmp/openskydimo.sock";
    const int sockFd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sockFd < 0)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    sockaddr_un addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(sockFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        std::cerr << "Failed to connect (is daemon running?)" << std::endl;
        close(sockFd);
        return false;
    }

    // Send command
    const std::string msg = command + "\n";
    write(sockFd, msg.c_str(), msg.length());

    // Wait for response
    char buffer[128];

    if (const ssize_t bytesRead = read(sockFd, buffer, sizeof(buffer) - 1); bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        std::cout << "[SERVER] - " << buffer;
    }

    close(sockFd);
    return true;
}

void FillCommand(ColorRGB color)
{
    std::cout << fmt::format("Filling LEDs with RGB{}", color) << std::endl;
    SendCommand(fmt::format("fill {} {} {}", color.r, color.g, color.b));
}

int main(const int argc, char* argv[])
{
    using namespace openskydimo::commands;

    CLI::App app{"This program is used to communicate with the skydimo daemon and configure the LEDs."};
    argv = app.ensure_utf8(argv);

    ColorRGB fillColorArgs{};
    AddFillCmd(&app, [&] { FillCommand(fillColorArgs); }, fillColorArgs);

    CLI11_PARSE(app, argc, argv);
    return 0;
}