#pragma once
#include "ipc/UnixSocketServer.h"

#include "CLI/App.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "SkydimoDriver.h"
#include "openskydimo/commands.hpp"

class OpenSkydimoServer : public UnixSocketServer
{
public:
    OpenSkydimoServer(std::string socketPath, int backlogSize, int bufferSize);

    void OnMessageReceived(int clientFd, const std::string& message) final;

    void OnClientConnected(int clientFd) final;
    void OnClientDisconnected(int clientFd) final;

    void OnFailedToReceive(int clientFd) final;
    void OnFailedToSend(int clientFd, const std::string& messageSent) final;
    void OnFailedClientConnection() final;

private:
    std::shared_ptr<spdlog::logger> m_logger = spdlog::stdout_color_mt("SkydimoServer");
    CLI::App m_app;
    SkydimoDriver m_driver;
    openskydimo::commands::Args m_cmdArgs;
};
