#pragma once
#include "CLI/App.hpp"
#include "types.h"

namespace openskydimo::commands
{

inline CLI::App* AddSetCmd(CLI::App* app)
{
    return app->add_subcommand("set", "Set options for LED driver")->require_subcommand(1);
}

inline CLI::App* AddSetPortCmd(CLI::App* setCmd, const std::function<void()>& callback, std::string& serialPort)
{
    auto* portCmd = setCmd->add_subcommand("port", "Set serial port");
    portCmd->add_option("port", serialPort, "Serial port")->required();
    portCmd->callback(callback);

    return portCmd;
}

inline CLI::App* AddSetCountCmd(CLI::App* setCmd, const std::function<void()>& callback, uint8_t& ledCount)
{
    auto* countCmd = setCmd->add_subcommand("count", "Set number of LEDs");
    countCmd->add_option("count", ledCount, "Number of LEDs")->required()->check(CLI::Range(1, 255));
    countCmd->callback(callback);

    return countCmd;
}

inline CLI::App* AddStartCmd(CLI::App* app, const std::function<void()>& callback)
{
    auto* startCmd = app->add_subcommand("start", "Starts LED driver loop");
    startCmd->callback(callback);

    return startCmd;
}

inline CLI::App* AddFillCmd(CLI::App* app, const std::function<void()>& callback, ColorRGB& color)
{
    const auto fillCmd = app->add_subcommand("fill", "Sets all the LEDs to a single color");

    fillCmd->add_option("r", color.r, "Red (0-255)")->required()->check(CLI::Range(0, 255));
    fillCmd->add_option("g", color.g, "Green (0-255)")->required()->check(CLI::Range(0, 255));
    fillCmd->add_option("b", color.b, "Blue (0-255)")->required()->check(CLI::Range(0, 255));
    fillCmd->callback(callback);

    return fillCmd;
}

} // namespace openskydimo::commands
