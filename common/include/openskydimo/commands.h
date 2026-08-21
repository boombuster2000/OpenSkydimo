#pragma once

#include <functional>
#include <string>

#include "command_dispatcher/Command.h"
#include "command_dispatcher/CommandGroup.h"

namespace openskydimo
{

inline command_dispatcher::CommandGroup* AddSetCmd(command_dispatcher::CommandGroup* root)
{
    return root->AddCommandGroup("set", "Configure LED driver settings");
}

inline command_dispatcher::Command* AddSetPortCmd(command_dispatcher::CommandGroup* setCmdGroup,
                                                  const command_dispatcher::Callback& callback)
{
    command_dispatcher::Command* setPortCmd =
        setCmdGroup->AddCommand("port", "Configure the serial port for LED communication");
    setPortCmd->AddOption<std::string>("port", "Serial port path (e.g. /dev/ttyUSB0)",
                                       std::function([](const std::string& value) { return !value.empty(); }));
    setPortCmd->SetCallback(callback);

    return setPortCmd;
}

inline command_dispatcher::Command* AddSetCountCmd(command_dispatcher::CommandGroup* setCmdGroup,
                                                   const command_dispatcher::Callback& callback)
{
    command_dispatcher::Command* setCountCmd =
        setCmdGroup->AddCommand("count", "Configure the total number of LEDs in the strip");
    setCountCmd->AddOption<int>("count", "Number of LEDs (1-255)",
                                std::function([](const int value) { return value >= 1 && value <= 255; }));
    setCountCmd->SetCallback(callback);
    return setCountCmd;
}

inline command_dispatcher::Command* AddStartCmd(command_dispatcher::CommandGroup* root,
                                                const command_dispatcher::Callback& callback)
{
    command_dispatcher::Command* startCmd = root->AddCommand("start", "Start the LED driver control loop");
    startCmd->SetCallback(callback);
    return startCmd;
}

inline command_dispatcher::Command* AddStopCmd(command_dispatcher::CommandGroup* root,
                                               const command_dispatcher::Callback& callback)
{
    command_dispatcher::Command* stopCmd = root->AddCommand("stop", "Stop the LED driver control loop");
    stopCmd->SetCallback(callback);
    return stopCmd;
}

inline command_dispatcher::Command* AddFillCmd(command_dispatcher::CommandGroup* root,
                                               const command_dispatcher::Callback& callback)
{
    command_dispatcher::Command* fillCmd = root->AddCommand("fill", "Fill all LEDs with a solid color");
    fillCmd->AddOption<int>("red", "Red component (0-255)",
                            std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd->AddOption<int>("green", "Green component (0-255)",
                            std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd->AddOption<int>("blue", "Blue component (0-255)",
                            std::function([](const int value) { return value >= 0 && value <= 255; }));
    fillCmd->SetCallback(callback);

    return fillCmd;
}

} // namespace openskydimo
