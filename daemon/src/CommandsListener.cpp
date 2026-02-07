#include "CommandsListener.h"

#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>

#include "openskydimo/commands.hpp"

CommandsListener::CommandsListener(std::string socketPath, SkydimoDriver& driver)
    : m_socketPath(std::move(socketPath)), m_driver(driver), m_serverFd(-1), m_isServerRunning(false)
{
    using namespace openskydimo::commands;

    AddFillCmd(&m_app, [this] { m_driver.Fill(m_cmdArgs.fillColor); }, m_cmdArgs.fillColor);

    const auto setCmd = AddSetCmd(&m_app);
    AddSetPortCmd(setCmd, [this] { m_driver.SetSerialPort(m_cmdArgs.serialPort); }, m_cmdArgs.serialPort);
    AddSetCountCmd(setCmd, [this] { m_driver.SetLedCount(m_cmdArgs.ledCount); }, m_cmdArgs.ledCount);

    AddStartCmd(&m_app, [this] { m_driver.OpenSerialConnection(); });
    AddStopCmd(&m_app, [this] { m_driver.CloseSerialConnection(); });
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

void CommandsListener::ListenLoop()
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

void CommandsListener::HandleClient(const int clientFd)
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

std::vector<std::string> CommandsListener::SplitCommand(const std::string& command)
{
    std::vector<std::string> parts;

    std::string part;
    for (int i = 0; i < command.length(); i++)
    {
        if (command[i] == ' ')
        {
            parts.insert(parts.begin(), part);
            part.clear();
            continue;
        }

        if (i == command.length() - 1)
        {
            part += command[i];
            parts.insert(parts.begin(), part);
            continue;
        }

        part += command[i];
    }

    return parts;
}

std::string CommandsListener::ExecuteCommand(const std::string& command)
{
    logger->info("Executing command {}", command);

    try
    {

        std::vector<std::string> args = SplitCommand(command);
        m_app.parse(args);

        return "OK\n";
    }
    catch (const CLI::ParseError& e)
    {
        logger->error("Error executing command: {}", e.what());
        return "ERROR: " + std::string(e.what()) + "\n";
    }
    catch (const std::exception& e)
    {
        logger->error("Error executing command: {}", e.what());
        return "ERROR: " + std::string(e.what()) + "\n";
    }
}