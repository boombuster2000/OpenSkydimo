#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "CLI/App.hpp"
#include "openskydimo/types.h"

namespace openskydimo::commands
{

struct Args
{
    ColorRGB fillColor{};
    std::string serialPort;
    uint8_t ledCount{};
};

inline CLI::App* AddSetCmd(CLI::App* app)
{
    return app->add_subcommand("set", "Configure LED driver settings")->require_subcommand(1);
}

inline CLI::App* AddSetPortCmd(CLI::App* setCmd, const std::function<void()>& callback, std::string& serialPort)
{
    auto* portCmd = setCmd->add_subcommand("port", "Configure the serial port for LED communication");
    portCmd->add_option("port", serialPort, "Serial port path (e.g. /dev/ttyUSB0)")->required();
    portCmd->callback(callback);

    return portCmd;
}

inline CLI::App* AddSetCountCmd(CLI::App* setCmd, const std::function<void()>& callback, uint8_t& ledCount)
{
    auto* countCmd = setCmd->add_subcommand("count", "Configure the total number of LEDs in the strip");
    countCmd->add_option("count", ledCount, "Number of LEDs (1-255)")->required()->check(CLI::Range(1, 255));
    countCmd->callback(callback);

    return countCmd;
}

inline CLI::App* AddStartCmd(CLI::App* app, const std::function<void()>& callback)
{
    auto* startCmd = app->add_subcommand("start", "Start the LED driver control loop");
    startCmd->callback(callback);

    return startCmd;
}

inline CLI::App* AddStopCmd(CLI::App* app, const std::function<void()>& callback)
{
    auto* stopCmd = app->add_subcommand("stop", "Stop the LED driver control loop");
    stopCmd->callback(callback);

    return stopCmd;
}

inline CLI::App* AddFillCmd(CLI::App* app, const std::function<void()>& callback, ColorRGB& color)
{
    const auto fillCmd = app->add_subcommand("fill", "Fill all LEDs with a solid color");

    fillCmd->add_option("r", color.r, "Red component (0-255)")->required()->check(CLI::Range(0, 255));
    fillCmd->add_option("g", color.g, "Green component (0-255)")->required()->check(CLI::Range(0, 255));
    fillCmd->add_option("b", color.b, "Blue component (0-255)")->required()->check(CLI::Range(0, 255));
    fillCmd->callback(callback);

    return fillCmd;
}

} // namespace openskydimo::commands