#pragma once
#include "ipc/UnixSocketServer.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "Driver.h"
#include "openskydimo/CommandDispatcher/CommandGroup.h"

class Server : public UnixSocketServer
{
public:
    Server(std::string socketPath, int backlogSize, int bufferSize);

    void OnMessageReceived(int clientFd, const std::string& message) final;

    void OnClientConnected(int clientFd) final;
    void OnClientDisconnected(int clientFd) final;

    void OnFailedToReceive(int clientFd) final;
    void OnFailedToSend(int clientFd, const std::string& messageSent) final;
    void OnFailedClientConnection() final;

private:
    std::shared_ptr<spdlog::logger> m_logger = spdlog::stdout_color_mt("Server");
    Driver m_driver;
    CommandGroup m_rootCommandGroup;
};
