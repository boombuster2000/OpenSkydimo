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
    driver.SetSerialPort(j.value("serial-port", ""));
    driver.SetLedCount(j.value("led-count", 0));
    driver.SetBaudRate(j.value("baud-rate", 115200));
}
