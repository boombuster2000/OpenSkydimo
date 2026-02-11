#pragma once

#include <nlohmann/json.hpp>

#include "SkydimoDriver.h"

using json = nlohmann::json;

inline void to_json(json& j, const SkydimoDriver& driver)
{
    j["serial-port"] = driver.m_portName;
    j["led-count"] = driver.m_ledCount;
    j["baud-rate"] = driver.m_baudRate;
}

inline void from_json(const json& j, SkydimoDriver& driver)
{
    driver.m_portName = j["serial-port"].get<std::string>();
    driver.m_ledCount = j["led-count"].get<int>();
    driver.m_baudRate = j["baud-rate"].get<int>();
}
