#pragma once
#include "LoggerFactory.h"
#include "openskydimo/types.h"

#include <mutex>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

class SkydimoDriver
{
public:
    enum class Mode
    {
        FILL,
        UNKNOWN
    };

public:
    SkydimoDriver() = default;
    ~SkydimoDriver();

    SkydimoDriver(SkydimoDriver&& other) noexcept;

    SkydimoDriver(const SkydimoDriver&) = delete;
    SkydimoDriver& operator=(const SkydimoDriver&) = delete;

    std::string GetSerialPortName() const;
    void SetSerialPort(const std::string& portName);

    int GetBaudRate() const;
    void SetBaudRate(int baudRate);

    int GetLedCount() const;
    void SetLedCount(int ledCount);

    bool OpenSerialConnection();
    void CloseSerialConnection();

    [[nodiscard]] bool IsReadyToSend() const;
    void SendColors() const;
    void Fill(ColorRGB color);

    static std::string ModeToString(const Mode mode)
    {
        switch (mode)
        {
        case Mode::FILL:
            return "fill";
        default:
            return "unknown";
        }
    }

    static Mode StringToMode(const std::string& mode)
    {
        if (mode == "fill")
            return Mode::FILL;

        return Mode::UNKNOWN;
    }

private:
    void AddHeaderToBuffer();

    friend void to_json(nlohmann::json& j, const SkydimoDriver& driver);
    friend void from_json(const nlohmann::json& j, SkydimoDriver& driver);

private:
    std::shared_ptr<spdlog::logger> logger = LoggerFactory::Create("SkydimoDriver");

    // Thread synchronization - mutable to allow locking in const methods
    mutable std::mutex m_mutex;

    bool m_isReadyToSend = false;

    static constexpr int m_headerSize = 6;
    int m_serialPort = -1;

    std::string m_portName;
    int m_ledCount = 0;
    int m_baudRate = 115200;

    Mode m_mode = Mode::FILL;
    ColorRGB m_color;

    std::vector<std::byte> m_buffer;
};

template <>
struct fmt::formatter<SkydimoDriver::Mode> : formatter<std::string>
{
    auto format(const SkydimoDriver::Mode& my, format_context& ctx) const -> decltype(ctx.out())
    {
        const std::string result = SkydimoDriver::ModeToString(my);
        return formatter<std::string>::format(result, ctx);
    }
};
