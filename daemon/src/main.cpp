#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "openskydimo/config.h"

#include "CommandsListener.h"
#include "SkydimoDriver.h"

static std::atomic shutdown_requested{false};

void SignalHandler(const int signal)
{
    if (signal == SIGINT)
        shutdown_requested.store(true, std::memory_order_release);

    if (signal == SIGTERM)
        shutdown_requested.store(true, std::memory_order_release);
}

int main()
{
    const std::shared_ptr<spdlog::logger> logger =
        spdlog::get("Daemon") ? spdlog::get("Daemon") : spdlog::stdout_color_mt("Daemon");

    SkydimoDriver driver;
    CommandsListener listener(s_socketPath, driver);

    struct sigaction signalAction{};
    signalAction.sa_handler = SignalHandler;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &signalAction, nullptr) == -1)
    {
        logger->error("sigaction SIGINT");
        return 1;
    }

    if (sigaction(SIGTERM, &signalAction, nullptr) == -1)
    {
        logger->error("sigaction SIGTERM");
        return 1;
    }

    listener.Start();

    while (!listener.ShouldStop() && !shutdown_requested.load(std::memory_order_acquire))
    {
        if (driver.IsReadyToSend())
            driver.SendColors();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Trigger graceful shutdown if signal was received
    if (shutdown_requested.load(std::memory_order_acquire))
    {
        listener.Stop();
    }

    return 0;
}
