#pragma once
#include "openskydimo/types.h"

#include <string>
#include <vector>

#include "spdlog/sinks/stdout_color_sinks-inl.h"
#include "spdlog/spdlog.h"

class SkydimoDriver
{

public:
    SkydimoDriver(std::string portName, int ledCount, int baudRate = 115200);
    ~SkydimoDriver();

    bool OpenSerialConnection();

    void CloseSerialConnection();

    void SendColors() const;

    void Fill(ColorRGB color);

private:
    void AddHeaderToBuffer();

private:
    std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("SkydimoDriver");

    const int m_headerSize = 6;
    int m_serialPort = -1;

    std::string m_portName;
    int m_ledCount;
    int m_baudRate;

    std::vector<std::byte> m_buffer;
};