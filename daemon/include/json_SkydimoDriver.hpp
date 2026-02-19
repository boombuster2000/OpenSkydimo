#pragma once

#include <nlohmann/json.hpp>

#include "SkydimoDriver.h"

inline void to_json(nlohmann::json& j, const SkydimoDriver& driver)
{
    j["serial-port"] = driver.m_portName;
    j["led-count"] = driver.m_ledCount;
    j["baud-rate"] = driver.m_baudRate;
}

inline void from_json(const nlohmann::json& j, SkydimoDriver& driver)
{
    driver.SetSerialPort(j.at("serial-port"));
    driver.SetLedCount(j.at("led-count"));
    driver.SetBaudRate(j.at("baud-rate"));

    driver.logger->info("Loaded skydimo driver from config (port={}, baud={}, leds={})", driver.GetSerialPortName(),
                        driver.GetBaudRate(), driver.GetLedCount());

    if (!driver.OpenSerialConnection())
        driver.logger->error("Failed to start serial connection with loaded config");
}
