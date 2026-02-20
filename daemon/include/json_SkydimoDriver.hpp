#pragma once

#include <nlohmann/json.hpp>

#include "SkydimoDriver.h"

inline void to_json(nlohmann::json& j, const ColorRGB& color)
{
    j["r"] = std::to_integer<uint8_t>(color.r);
    j["g"] = std::to_integer<uint8_t>(color.g);
    j["b"] = std::to_integer<uint8_t>(color.b);
}

inline void from_json(const nlohmann::json& j, ColorRGB& color)
{
    color = ColorRGB(j.at("r").get<int>(), j.at("g").get<int>(), j.at("b").get<int>());
}

inline void to_json(nlohmann::json& j, const SkydimoDriver& driver)
{
    j["serial-port"] = driver.m_portName;
    j["led-count"] = driver.m_ledCount;
    j["baud-rate"] = driver.m_baudRate;
    j["mode"] = SkydimoDriver::ModeToString(driver.m_mode);
    j["color"] = driver.m_color;
}

inline void from_json(const nlohmann::json& j, SkydimoDriver& driver)
{
    driver.SetSerialPort(j.at("serial-port"));
    driver.SetLedCount(j.at("led-count"));
    driver.SetBaudRate(j.at("baud-rate"));
    driver.m_mode = SkydimoDriver::StringToMode(j.at("mode"));
    driver.m_color = j.at("color");

    if (driver.m_mode == SkydimoDriver::Mode::FILL)
        driver.Fill(driver.m_color);

    driver.logger->info("Loaded skydimo driver from config (port={}, baud={}, leds={})", driver.GetSerialPortName(),
                        driver.GetBaudRate(), driver.GetLedCount());

    if (!driver.OpenSerialConnection())
        driver.logger->error("Failed to start serial connection with loaded config");
}
