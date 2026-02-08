#pragma once
#include "openskydimo/types.h"

#include <mutex>
#include <string>
#include <vector>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

class SkydimoDriver
{

public:
    SkydimoDriver() = default;
    ~SkydimoDriver();

    void SetSerialPort(const std::string& portName);
    void SetBaudRate(int baudRate);
    void SetLedCount(int ledCount);

    bool OpenSerialConnection();
    void CloseSerialConnection();

    [[nodiscard]] bool IsReadyToSend() const;
    void SendColors() const;
    void Fill(ColorRGB color);

private:
    void AddHeaderToBuffer();

private:
    std::shared_ptr<spdlog::logger> logger =
        spdlog::get("SkydimoDriver") ? spdlog::get("SkydimoDriver") : spdlog::stdout_color_mt("SkydimoDriver");

    // Thread synchronization - mutable to allow locking in const methods
    mutable std::mutex m_mutex;

    bool m_isInitialized = false;
    bool m_isReadyToSend = false;

    const int m_headerSize = 6;
    int m_serialPort = -1;

    std::string m_portName;
    int m_ledCount = 0;
    int m_baudRate = 115200;

    std::vector<std::byte> m_buffer;
};