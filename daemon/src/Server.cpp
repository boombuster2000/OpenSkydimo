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
    Command& fillCmd = m_dispatcher.AddCommand("fill", "Fill all LEDs with a solid color");
    fillCmd.AddOption<int>("r", "Red component (0-255)",
                           std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd.AddOption<int>("g", "Green component (0-255)",
                           std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd.AddOption<int>("b", "Blue component (0-255)",
                           std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd.SetCallback([this, &fillCmd] {
        m_cmdArgs.fillColor = ColorRGB{static_cast<uint8_t>(fillCmd.GetOption<int>("r")),
                                       static_cast<uint8_t>(fillCmd.GetOption<int>("g")),
                                       static_cast<uint8_t>(fillCmd.GetOption<int>("b"))};
        m_driver.Fill(m_cmdArgs.fillColor);
        return Response{};
    });

    // --- set (command group) ---
    Command& setCmd = m_dispatcher.AddCommand("set", "Configure LED driver settings");

    Command& setPortCmd = setCmd.AddCommand("port", "Configure the serial port for LED communication");
    setPortCmd.AddOption<std::string>("port", "Serial port path (e.g. /dev/ttyUSB0)",
                                      std::function([](const std::string& value) { return !value.empty(); }));
    setPortCmd.SetCallback([this, &setPortCmd] {
        m_cmdArgs.serialPort = setPortCmd.GetOption<std::string>("port");
        m_driver.SetSerialPort(m_cmdArgs.serialPort);
        return Response{};
    });

    Command& setCountCmd = setCmd.AddCommand("count", "Configure the total number of LEDs in the strip");
    setCountCmd.AddOption<int>("count", "Number of LEDs (1-255)",
                               std::function([](const int value) { return value >= 1 && value <= 255; }));
    setCountCmd.SetCallback([this, &setCountCmd] {
        m_cmdArgs.ledCount = static_cast<uint8_t>(setCountCmd.GetOption<int>("count"));
        m_driver.SetLedCount(m_cmdArgs.ledCount);
        return Response{};
    });

    // --- start ---
    Command& startCmd = m_dispatcher.AddCommand("start", "Start the LED driver control loop");
    startCmd.SetCallback([this] {
        m_driver.OpenSerialConnection();
        return Response{};
    });

    // --- stop ---
    Command& stopCmd = m_dispatcher.AddCommand("stop", "Stop the LED driver control loop");
    stopCmd.SetCallback([this] {
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
    catch (const CLI::ParseError& e)
    {
        response.code = 1;
        response.message = std::string(e.what()) + "\n";
        m_logger->error(response.message);

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
