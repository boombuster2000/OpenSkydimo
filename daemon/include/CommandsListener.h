#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "SkydimoDriver.h"
#include "openskydimo/commands.hpp"

class CommandsListener
{
public:
    CommandsListener(std::string socketPath, SkydimoDriver& driver);
    ~CommandsListener();

    void Start();
    void Stop();
    [[nodiscard]] bool ShouldStop() const;

private:
    void ListenLoop();

    void HandleClient(int clientFd);

    [[nodiscard]] std::string ExecuteCommand(const std::string& command);

private:
    std::shared_ptr<spdlog::logger> m_logger =
        spdlog::get("CommandsListener") ? spdlog::get("CommandsListener") : spdlog::stdout_color_mt("CommandsListener");

    CLI::App m_app;

    std::string m_socketPath;
    SkydimoDriver& m_driver;

    int m_serverFd;
    std::atomic<bool> m_isServerRunning;
    std::thread m_listenerThread;

    openskydimo::commands::Args m_cmdArgs;
};
