#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "CommandDispatcher/Command.h"
#include "CommandDispatcher/Dispatcher.h"

namespace openskydimo::commands
{

inline Command& AddSetCmd(Dispatcher& dispatcher)
{
    return dispatcher.AddCommand("set", "Configure LED driver settings");
}

inline Command& AddSetPortCmd(Command& setCmd, const Callback& callback)
{
    Command& setPortCmd = setCmd.AddCommand("port", "Configure the serial port for LED communication");
    setPortCmd.AddOption<std::string>("port", "Serial port path (e.g. /dev/ttyUSB0)",
                                      std::function([](const std::string& value) { return !value.empty(); }));
    setPortCmd.SetCallback(callback);

    return setPortCmd;
}

inline Command& AddSetCountCmd(Command& setCmd, const Callback& callback)
{
    Command& setCountCmd = setCmd.AddCommand("count", "Configure the total number of LEDs in the strip");
    setCountCmd.AddOption<int>("count", "Number of LEDs (1-255)",
                               std::function([](const int value) { return value >= 1 && value <= 255; }));
    setCountCmd.SetCallback(callback);
    return setCountCmd;
}

inline Command& AddStartCmd(Dispatcher& dispatcher, const Callback& callback)
{
    Command& startCmd = dispatcher.AddCommand("start", "Start the LED driver control loop");
    startCmd.SetCallback(callback);
    return startCmd;
}

inline Command& AddStopCmd(Dispatcher& dispatcher, const Callback& callback)
{
    Command& stopCmd = dispatcher.AddCommand("stop", "Stop the LED driver control loop");
    stopCmd.SetCallback(callback);
    return stopCmd;
}

inline Command& AddFillCmd(Dispatcher& dispatcher, const Callback& callback)
{
    Command& fillCmd = dispatcher.AddCommand("fill", "Fill all LEDs with a solid color");
    fillCmd.AddOption<int>("red", "Red component (0-255)",
                           std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd.AddOption<int>("green", "Green component (0-255)",
                           std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd.AddOption<int>("blue", "Blue component (0-255)",
                           std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd.SetCallback(callback);

    return fillCmd;
}

} // namespace openskydimo::commands
