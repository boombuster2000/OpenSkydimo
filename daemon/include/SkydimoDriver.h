#pragma once
#include "openskydimo/types.h"

#include <string>
#include <vector>

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
    const int m_headerSize = 6;
    int m_serialPort = -1;

    std::string m_portName;
    int m_ledCount;
    int m_baudRate;

    std::vector<std::byte> m_buffer;
};