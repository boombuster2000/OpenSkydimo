#include "Server.h"

#include <algorithm>
#include <utility>

#include <nlohmann/json.hpp>

#include "openskydimo/commands.hpp"
#include "openskydimo/types/Response.h"

Server::Server(std::string socketPath, const int backlogSize, const int bufferSize)
    : UnixSocketServer(std::move(socketPath), backlogSize, bufferSize)
{
    using namespace openskydimo::commands;

    // --- fill ---
    AddFillCmd(m_dispatcher, [this](const Command& cmd) {
        const ColorRGB fillColor = {(cmd.GetOption<int>("r")), (cmd.GetOption<int>("g")), (cmd.GetOption<int>("b"))};

        m_driver.Fill(fillColor);
        return Response{};
    });

    // --- set (command group) ---
    Command& setCmd = AddSetCmd(m_dispatcher);
    AddSetPortCmd(setCmd, [this](const Command& cmd) {
        const auto serialPort = cmd.GetOption<std::string>("port");
        m_driver.SetSerialPort(serialPort);
        return Response{};
    });

    AddSetCountCmd(setCmd, [this](const Command& cmd) {
        const auto ledCount = cmd.GetOption<int>("count");
        m_driver.SetLedCount(ledCount);
        return Response{};
    });

    // --- start ---
    AddStartCmd(m_dispatcher, [this](const Command&) {
        m_driver.OpenSerialConnection();
        return Response{};
    });

    // --- stop ---
    AddStopCmd(m_dispatcher, [this](const Command&) {
        m_driver.CloseSerialConnection();
        return Response{};
    });
}

void Server::OnMessageReceived(const int clientFd, const std::string& message)
{
    Response response;
    try
    {
        nlohmann::json json = nlohmann::json::parse(message);
        const std::vector<std::string> args = json["argv"];
        m_dispatcher.Dispatch(args);
        response.code = 0;
        response.message = "OK";

        if (const ssize_t result = SendResponse(clientFd, nlohmann::json(response).dump()); result < 0)
            m_logger->error("Failed to send response.");
    }
    catch (const std::exception& e)
    {
        response.code = 1;
        response.message = std::string(e.what()) + "\n";
        m_logger->error(response.message);

        if (const ssize_t result = SendResponse(clientFd, nlohmann::json(response).dump()); result < 0)
            m_logger->error("Failed to send response.");
    }
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
