#include <csignal>

#include "openskydimo/config.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "Server.h"

static Server* g_server = nullptr;

static void SignalHandler(const int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
        if (g_server)
            Server::RequestStop();
}

int main()
{
    const std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("Daemon");

    Server server(openskydimo::s_socketPath, 1, 128);
    g_server = &server;

    struct sigaction signalAction{};
    signalAction.sa_handler = SignalHandler;
    sigemptyset(&signalAction.sa_mask);

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

    server.Start();
    return 0;
}
