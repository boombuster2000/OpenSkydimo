#include <cstring>

#include <functional>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "CLI/CLI.hpp"
#include "openskydimo/types.h"
#include "spdlog/fmt/bundled/format.h"

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
        std::cout << buffer;
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
    CLI::App app{"This program is used to communicate with the skydimo daemon and configure the LEDs."};
    argv = app.ensure_utf8(argv);

    const auto fill_cmd = app.add_subcommand("fill", "Sets all the LEDs to a single color");
    int r, g, b;
    fill_cmd->add_option("r", r, "Red (0-255)")->required()->check(CLI::Range(0, 255));
    fill_cmd->add_option("g", g, "Green (0-255)")->required()->check(CLI::Range(0, 255));
    fill_cmd->add_option("b", b, "Blue (0-255)")->required()->check(CLI::Range(0, 255));
    fill_cmd->callback([&] { FillCommand(ColorRGB(r, g, b)); });

    CLI11_PARSE(app, argc, argv);
    return 0;
}