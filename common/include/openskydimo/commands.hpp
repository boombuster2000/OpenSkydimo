#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "CommandDispatcher/Command.h"
#include "CommandDispatcher/CommandGroup.h"

namespace openskydimo::commands
{

inline CommandGroup* AddSetCmd(CommandGroup* root)
{
    return root->AddCommandGroup("set", "Configure LED driver settings");
}

inline Command* AddSetPortCmd(CommandGroup* setCmdGroup, const Callback& callback)
{
    Command* setPortCmd = setCmdGroup->AddCommand("port", "Configure the serial port for LED communication");
    setPortCmd->AddOption<std::string>("port", "Serial port path (e.g. /dev/ttyUSB0)",
                                       std::function([](const std::string& value) { return !value.empty(); }));
    setPortCmd->SetCallback(callback);

    return setPortCmd;
}

inline Command* AddSetCountCmd(CommandGroup* setCmdGroup, const Callback& callback)
{
    Command* setCountCmd = setCmdGroup->AddCommand("count", "Configure the total number of LEDs in the strip");
    setCountCmd->AddOption<int>("count", "Number of LEDs (1-255)",
                                std::function([](const int value) { return value >= 1 && value <= 255; }));
    setCountCmd->SetCallback(callback);
    return setCountCmd;
}

inline Command* AddStartCmd(CommandGroup* root, const Callback& callback)
{
    Command* startCmd = root->AddCommand("start", "Start the LED driver control loop");
    startCmd->SetCallback(callback);
    return startCmd;
}

inline Command* AddStopCmd(CommandGroup* root, const Callback& callback)
{
    Command* stopCmd = root->AddCommand("stop", "Stop the LED driver control loop");
    stopCmd->SetCallback(callback);
    return stopCmd;
}

inline Command* AddFillCmd(CommandGroup* root, const Callback& callback)
{
    Command* fillCmd = root->AddCommand("fill", "Fill all LEDs with a solid color");
    fillCmd->AddOption<int>("red", "Red component (0-255)",
                            std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd->AddOption<int>("green", "Green component (0-255)",
                            std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd->AddOption<int>("blue", "Blue component (0-255)",
                            std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd->SetCallback(callback);

    return fillCmd;
}

} // namespace openskydimo::commands
