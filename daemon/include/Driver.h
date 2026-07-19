#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "openskydimo/types/ColorRGB.h"
#include "openskydimo/types/Response.h"

class Driver
{
public:
    Driver() = default;
    ~Driver();

    Response SetSerialPort(const std::string& portName);
    Response SetBaudRate(int baudRate);
    Response SetLedCount(int ledCount);
    Response SetRefreshRate(int hz);

    Response OpenSerialConnection();
    Response CloseSerialConnection();

    Response Fill(ColorRGB color);

private:
    void StopAndCleanup();
    std::optional<Response> RequireStopped(const char* action) const;
    void SendColors() const;
    void SendLoop();
    void AddHeaderToBuffer();

private:
    std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("Driver");

    static constexpr int m_headerSize = 6;

    int m_serialPort = -1;
    std::string m_portName;
    int m_ledCount = 0;
    int m_baudRate = 115200;
    std::chrono::microseconds m_sendInterval{1'000'000 / 60};

    std::vector<std::byte> m_buffer;
    mutable std::mutex m_bufferMutex;

    std::thread m_sendThread;
    std::atomic<bool> m_isConnectionOpened{false};
};

class SkydimoException : public std::runtime_error
{
public:
    explicit SkydimoException(const std::string& message) : std::runtime_error(message)
    {
    }
};
class SerialConnectionException : public SkydimoException
{
public:
    explicit SerialConnectionException(const std::string& message) : SkydimoException(message)
    {
    }
};
class SerialWriteException : public SkydimoException
{
public:
    explicit SerialWriteException(const std::string& message) : SkydimoException(message)
    {
    }
};
