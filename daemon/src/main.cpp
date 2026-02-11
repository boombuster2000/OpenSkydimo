#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <thread>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "openskydimo/config.h"

#include "CommandsListener.h"
#include "SkydimoDriver.h"
#include "json_SkydimoDriver.hpp"
#include <nlohmann/json.hpp>

static std::atomic s_shutdown_requested{false};

static const std::shared_ptr<spdlog::logger> s_logger =
    spdlog::get("Daemon") ? spdlog::get("Daemon") : spdlog::stdout_color_mt("Daemon");

void SignalHandler(const int signal)
{
    if (signal == SIGINT)
        s_shutdown_requested.store(true, std::memory_order_release);

    if (signal == SIGTERM)
        s_shutdown_requested.store(true, std::memory_order_release);
}

std::filesystem::path GetConfigFilePath()
{
    namespace fs = std::filesystem;

    const char* homeDir = std::getenv("HOME");
    if (!homeDir)
        throw std::runtime_error("HOME environment variable not set");

    const fs::path configDir = fs::path(homeDir) / ".config" / "openskydimo";

    if (!fs::exists(configDir))
        fs::create_directories(configDir);

    return configDir / "config.json";
}

SkydimoDriver LoadDriver(const std::filesystem::path& configFilePath)
{
    using json = nlohmann::json;
    if (!std::filesystem::exists(configFilePath))
    {
        s_logger->info("Config file not found, using default settings");
        return {};
    }

    try
    {
        std::ifstream configFile(configFilePath);
        const json config = json::parse(configFile);
        return config.get<SkydimoDriver>();
    }
    catch (const std::exception& e)
    {
        s_logger->warn("Failed to parse config file, using defaults: {}", e.what());
    }

    return {};
}

void SaveDriver(const SkydimoDriver& driver, const std::filesystem::path& configFilePath)
{
    const nlohmann::json config = driver;
    std::ofstream configFile(configFilePath);
    configFile << config.dump(4);
}

int main()
{
    const auto configFilePath = GetConfigFilePath();
    SkydimoDriver driver = LoadDriver(configFilePath);
    CommandsListener listener(s_socketPath, driver);

    struct sigaction signalAction{};
    signalAction.sa_handler = SignalHandler;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &signalAction, nullptr) == -1)
    {
        s_logger->error("sigaction SIGINT");
        return 1;
    }

    if (sigaction(SIGTERM, &signalAction, nullptr) == -1)
    {
        s_logger->error("sigaction SIGTERM");
        return 1;
    }

    listener.Start();

    while (!listener.ShouldStop() && !s_shutdown_requested.load(std::memory_order_acquire))
    {
        if (driver.IsReadyToSend())
            driver.SendColors();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Trigger graceful shutdown if signal was received
    if (s_shutdown_requested.load(std::memory_order_acquire))
    {
        listener.Stop();
        SaveDriver(driver, configFilePath);
    }

    return 0;
}
