#include "Server.h"

#include "Driver.h"

#include <algorithm>
#include <format>
#include <utility>

#include <nlohmann/json.hpp>

#include "openskydimo/commands.hpp"
#include "openskydimo/types/Response.h"

using namespace openskydimo::types;
using namespace openskydimo::command_dispatcher;

Server::Server(std::string socketPath, const int backlogSize, const int bufferSize)
    : UnixSocketServer(std::move(socketPath), backlogSize, bufferSize),
      m_rootCommandGroup("openskydimo", "Program to control skydimo lights on linux.")
{
    using namespace openskydimo;

    // --- fill ---
    AddFillCmd(&m_rootCommandGroup, [this](const Command& cmd) {
        const ColorRGB fillColor = {(cmd.GetOption<int>("red")), (cmd.GetOption<int>("green")),
                                    (cmd.GetOption<int>("blue"))};
        return m_driver.ApplyEffect(Driver::Effect::FILL, fillColor);
    });

    // --- set (command group) ---
    CommandGroup* setCmd = AddSetCmd(&m_rootCommandGroup);
    AddSetPortCmd(setCmd, [this](const Command& cmd) {
        const auto serialPort = cmd.GetOption<std::string>("port");
        return m_driver.SetSerialPort(serialPort);
    });

    AddSetCountCmd(setCmd, [this](const Command& cmd) {
        const auto ledCount = cmd.GetOption<int>("count");
        return m_driver.SetLedCount(ledCount);
    });

    // --- start ---
    AddStartCmd(&m_rootCommandGroup, [this](const Command&) { return m_driver.OpenSerialConnection(); });

    // --- stop ---
    AddStopCmd(&m_rootCommandGroup, [this](const Command&) { return m_driver.CloseSerialConnection(); });

    m_driver.LoadConfigAndStart();
}

void Server::OnMessageReceived(const int clientFd, const std::string& message)
{
    Response response;
    try
    {
        nlohmann::json json = nlohmann::json::parse(message);
        const std::vector<std::string> args = json.at("argv").get<std::vector<std::string>>();
        response = m_rootCommandGroup.Execute(args);
    }
    catch (const nlohmann::json::exception& e)
    {
        response = MakeError(1, std::format("malformed request: {}", e.what()));
        m_logger->error("{}", response.message);
    }
    catch (const std::exception& e)
    {
        response = MakeError(1, std::format("unexpected error: {}", e.what()));
        m_logger->error("{}", response.message);
    }

    if (const ssize_t result = SendResponse(clientFd, nlohmann::json(response).dump()); result < 0)
        m_logger->error("Failed to send response.");
}

void Server::OnClientConnected(const int clientFd)
{
    m_logger->info("Client connected: " + std::to_string(clientFd));
}
void Server::OnClientDisconnected(const int clientFd)
{
    m_logger->info("Client disconnected: " + std::to_string(clientFd));
}
void Server::OnFailedToReceive(const int clientFd)
{
    m_logger->error("Failed to receive response of client " + std::to_string(clientFd));
}
void Server::OnFailedToSend(const int clientFd, const std::string& messageSent)
{
    m_logger->error("ERROR: Failed to send response to client " + std::to_string(clientFd));
}
void Server::OnFailedClientConnection()
{
    m_logger->error("ERROR: Failed client connection");
}
