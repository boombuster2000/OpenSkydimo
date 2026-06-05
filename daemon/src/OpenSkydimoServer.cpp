#include "OpenSkydimoServer.h"

#include <algorithm>
#include <utility>

#include <nlohmann/json.hpp>

#include "openskydimo/commands.hpp"
#include "openskydimo/types/Response.h"

OpenSkydimoServer::OpenSkydimoServer(std::string socketPath, const int backlogSize, const int bufferSize)
    : UnixSocketServer(std::move(socketPath), backlogSize, bufferSize)
{
    using namespace openskydimo::commands;
    AddFillCmd(&m_app, [this] { m_driver.Fill(m_cmdArgs.fillColor); }, m_cmdArgs.fillColor);

    const auto setCmd = AddSetCmd(&m_app);
    AddSetPortCmd(setCmd, [this] { m_driver.SetSerialPort(m_cmdArgs.serialPort); }, m_cmdArgs.serialPort);
    AddSetCountCmd(setCmd, [this] { m_driver.SetLedCount(m_cmdArgs.ledCount); }, m_cmdArgs.ledCount);

    AddStartCmd(&m_app, [this] { m_driver.OpenSerialConnection(); });
    AddStopCmd(&m_app, [this] { m_driver.CloseSerialConnection(); });
}

void OpenSkydimoServer::OnMessageReceived(const int clientFd, const std::string& message)
{
    Response response;
    try
    {
        nlohmann::json json = nlohmann::json::parse(message);
        std::vector<std::string> args = json["argv"];
        std::ranges::reverse(args);
        m_app.parse(args);
    }
    catch (const CLI::ParseError& e)
    {
        response.code = 1;
        response.message = "ERROR: " + std::string(e.what()) + "\n";
        m_logger->error(response.message);

        if (const ssize_t result = SendResponse(clientFd, response.message); result < 0)
            m_logger->error("ERROR: Failed to send response.");
    }
    catch (const std::exception& e)
    {
        response.code = 1;
        response.message = "ERROR: " + std::string(e.what()) + "\n";
        m_logger->error(response.message);

        if (const ssize_t result = SendResponse(clientFd, response.message); result < 0)
            m_logger->error("ERROR: Failed to send response.");
    }
}
void OpenSkydimoServer::OnClientConnected(const int clientFd)
{
    m_logger->info("INFO: Client connected: " + std::to_string(clientFd));
}
void OpenSkydimoServer::OnClientDisconnected(const int clientFd)
{
    m_logger->info("INFO: Client disconnected: " + std::to_string(clientFd));
}
void OpenSkydimoServer::OnFailedToReceive(const int clientFd)
{
    m_logger->error("ERROR: Failed to receive response of client " + std::to_string(clientFd));
}
void OpenSkydimoServer::OnFailedToSend(const int clientFd, const std::string& messageSent)
{
    m_logger->error("ERROR: Failed to send response to client " + std::to_string(clientFd));
}
void OpenSkydimoServer::OnFailedClientConnection()
{
    m_logger->error("ERROR: Failed client connection");
}
