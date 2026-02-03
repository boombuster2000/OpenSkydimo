#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

bool SendCommand(const std::string& socketPath, const std::string& command)
{
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
    std::string msg = command + "\n";
    write(sockFd, msg.c_str(), msg.length());

    // Wait for response
    char buffer[128];
    ssize_t bytesRead = read(sockFd, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        std::cout << buffer;
    }
    close(sockFd);
    return true;
}

void PrintUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " <command> [args...]\n"
              << "\nCommands:\n"
              << "  fill <r,g,b>        Fill all LEDs with color (e.g., fill 255,255,255)\n"
              << "  brightness <0-255>  Set brightness level\n"
              << "  pattern <name>      Set pattern (e.g., rainbow, pulse)\n"
              << "  off                 Turn off all LEDs\n"
              << "\nExamples:\n"
              << "  " << programName << " fill 255,0,0\n"
              << "  " << programName << " brightness 128\n"
              << "  " << programName << " pattern rainbow\n";
}

int main(const int argc, char* argv[])
{
    const std::string socketPath = "/tmp/openskydimo.sock";

    if (argc < 2)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    const std::string cmd = argv[1];
    std::string command;

    if (cmd == "fill" && argc == 3)
    {
        command = "FILL " + std::string(argv[2]);
    }
    else if (cmd == "brightness" && argc == 3)
    {
        command = "BRIGHTNESS " + std::string(argv[2]);
    }
    else if (cmd == "pattern" && argc == 3)
    {
        command = "PATTERN " + std::string(argv[2]);
    }
    else if (cmd == "off" && argc == 2)
    {
        command = "OFF";
    }
    else
    {
        std::cerr << "Invalid command or arguments\n";
        PrintUsage(argv[0]);
        return 1;
    }

    if (!SendCommand(socketPath, command))
    {
        return 1;
    }

    return 0;
}