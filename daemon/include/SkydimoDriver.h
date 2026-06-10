#pragma once
#include "openskydimo/types/ColorRGB.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

class SkydimoDriver
{
public:
    SkydimoDriver() = default;
    ~SkydimoDriver();

    void SetSerialPort(const std::string& portName);
    void SetBaudRate(int baudRate);
    void SetLedCount(int ledCount);

    void OpenSerialConnection();
    void CloseSerialConnection();

    [[nodiscard]] bool IsReadyToSend() const;

    void SendColors() const;

    void Fill(ColorRGB color);

private:
    void AddHeaderToBuffer();

private:
    std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("SkydimoDriver");
    mutable std::mutex m_mutex;
    bool m_isReadyToSend = false;
    static constexpr int m_headerSize = 6;
    int m_serialPort = -1;
    std::string m_portName;
    int m_ledCount = 0;
    int m_baudRate = 115200;
    std::vector<std::byte> m_buffer;
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
