#pragma once
#include "openskydimo/types/ColorRGB.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

class Driver
{
public:
    Driver() = default;
    ~Driver();

    void SetSerialPort(const std::string& portName);
    void SetBaudRate(int baudRate);
    void SetLedCount(int ledCount);
    void SetRefreshRate(int hz);

    void OpenSerialConnection();
    void CloseSerialConnection();

    void Fill(ColorRGB color);

private:
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
    std::atomic<bool> m_running{false};
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
