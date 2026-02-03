#include "CommandsListener.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>

CommandsListener::CommandsListener(std::string socketPath, SkydimoDriver& driver)
    : m_socketPath(std::move(socketPath)), m_driver(driver), m_serverFd(-1), m_isServerRunning(false)
{
}

CommandsListener::~CommandsListener()
{
    Stop();
}

void CommandsListener::Start()
{
    if (m_isServerRunning)
        return;

    m_serverFd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (m_serverFd < 0)
        logger->error("Error creating socket: {}", strerror(errno));

    unlink(m_socketPath.c_str());

    sockaddr_un address{};
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, m_socketPath.c_str(), sizeof(address.sun_path) - 1);

    if (bind(m_serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
    {
        logger->error("Error binding socket: {}", strerror(errno));
        close(m_serverFd);
        return;
    }

    if (listen(m_serverFd, 5) < 0)
    {
        logger->error("Error listening on socket: {}", strerror(errno));
        close(m_serverFd);
        unlink(m_socketPath.c_str());
        return;
    }

    logger->info("Socket listening on {}", m_socketPath);
    m_isServerRunning = true;
    m_listenerThread = std::thread(&CommandsListener::ListenLoop, this);
}

void CommandsListener::Stop()
{
    if (!m_isServerRunning)
        return;

    m_isServerRunning = false;

    // Close socket to unblock accept()
    if (m_serverFd >= 0)
    {
        shutdown(m_serverFd, SHUT_RDWR);
        close(m_serverFd);
        m_serverFd = -1;
    }
    if (m_listenerThread.joinable())
    {
        m_listenerThread.join();
    }

    unlink(m_socketPath.c_str());
}

bool CommandsListener::ShouldStop() const
{
    return !m_isServerRunning;
}

void CommandsListener::ListenLoop() const
{
    while (m_isServerRunning)
    {
        const int clientFd = accept(m_serverFd, nullptr, nullptr);

        if (clientFd < 0)
        {
            if (m_isServerRunning)
                logger->error("Error accepting client connection: {}", strerror(errno));

            break;
        }

        HandleClient(clientFd);
        close(clientFd);
    }
}

void CommandsListener::HandleClient(const int clientFd) const
{
    char buffer[1024];

    if (const ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1); bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
 std::string command(buffer);
        if (!command.empty() && command.back() == '\n')
            command.pop_back();

        const auto response = ExecuteCommand(command);

        write(clientFd, response.c_str(), strlen(response.c_str()));
    }
}

std::string CommandsListener::ExecuteCommand(const std::string& command) const
{
    std::string response = "OK\n";

    if (command.substr(0, 4) == "fill")
    {
        const std::string color = command.substr(5);

        std::istringstream iss(color);

        if (int r, g, b; iss >> r >> g >> b && r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255)
        {
            const ColorRGB rgb(r, g, b);
            m_driver.Fill(rgb);
        }
        else
        {
            response = "ERROR: Invalid RGB color format. Expected: fill <r> <g> <b> where r, g, b are integers 0-255\n";
        }
    }

    return response;
}