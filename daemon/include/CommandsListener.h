#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "CLI/CLI.hpp"
#include "spdlog/sinks/stdout_color_sinks-inl.h"
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

    void ListenLoop();

    void HandleClient(int clientFd);

    static std::vector<std::string> SplitCommand(const std::string& command);
    [[nodiscard]] std::string ExecuteCommand(const std::string& command);

private:
    std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("CommandsListener");
    CLI::App m_app;

    std::string m_socketPath;
    SkydimoDriver& m_driver;

    int m_serverFd;
    std::atomic<bool> m_isServerRunning;
    std::thread m_listenerThread;

    openskydimo::commands::Args m_cmdArgs;
};