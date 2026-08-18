#pragma once
#include "ConfigFileHandler.h"

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

#include "openskydimo/config.h"
#include "openskydimo/types/ColorRGB.h"
#include "openskydimo/types/Response.h"

class Driver
{
    using Response = openskydimo::types::Response;
    using ColorRGB = openskydimo::types::ColorRGB;

public:
    enum class Effect
    {
        FILL
    };

public:
    Driver();
    ~Driver();

    void LoadConfigAndStart();
    Response ApplyEffect(Effect effect, const nlohmann::json& params, bool saveToFile = true);

    Response SetSerialPort(const std::string& portName);
    Response SetBaudRate(int baudRate);
    Response SetLedCount(int ledCount);
    Response SetRefreshRate(int hz);

    Response OpenSerialConnection();
    Response CloseSerialConnection();

private:
    void StopAndCleanup();
    std::optional<Response> RequireStopped(const char* action) const;
    void SendColors() const;
    void SendLoop();
    void AddHeaderToBuffer();

    Response Fill(ColorRGB color);

private:
    std::shared_ptr<spdlog::logger> m_logger = spdlog::stdout_color_mt("Driver");
    std::optional<ConfigFileHandler> m_configHandler;
    nlohmann::json m_defaultConfig = {{"port", ""}, {"led-count", 0}, {"last-effect", nullptr}};

    static constexpr int m_headerSize = 6;

    int m_serialPort = -1;
    std::string m_portName;
    int m_ledCount = 0;
    int m_baudRate = 115200;
    std::chrono::microseconds m_sendInterval{1'000'000 / 30};

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

inline void to_json(nlohmann::json& j, const Driver::Effect effect)
{
    switch (effect)
    {
    case Driver::Effect::FILL:
        j = "fill";
        return;
    default:
        throw std::invalid_argument("Unhandled Effect value in to_json");
    }
}

inline void from_json(const nlohmann::json& j, Driver::Effect& effect)
{
    const std::string value = j.get<std::string>();
    if (value == "fill")
    {
        effect = Driver::Effect::FILL;
        return;
    }
    throw nlohmann::json::type_error::create(302, std::format("Unrecognized effect type: '{}'", value), &j);
}
