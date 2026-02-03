#pragma once

#include <atomic>
#include <thread>

#include "SkydimoDriver.h"
#include "spdlog/sinks/stdout_color_sinks-inl.h"
#include "spdlog/spdlog.h"

class CommandsListener
{
public:
    CommandsListener(std::string socketPath, SkydimoDriver& driver);
    ~CommandsListener();

    void Start();
    void Stop();
    [[nodiscard]] bool ShouldStop() const;

    void ListenLoop() const;

    void HandleClient(int clientFd) const;

    [[nodiscard]] std::string ExecuteCommand(const std::string& command) const;

private:
    std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("CommandsListener");

    std::string m_socketPath;
    SkydimoDriver& m_driver;

    int m_serverFd;
    std::atomic<bool> m_isServerRunning;
    std::thread m_listenerThread;
};