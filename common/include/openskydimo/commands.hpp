#pragma once
#include "CLI/App.hpp"
#include "types.h"

namespace openskydimo::commands
{

inline void AddFillCmd(CLI::App* app, const std::function<void()>& callback, ColorRGB& color)
{
    const auto fill_cmd = app->add_subcommand("fill", "Sets all the LEDs to a single color");

    fill_cmd->add_option("r", color.r, "Red (0-255)")->required()->check(CLI::Range(0, 255));
    fill_cmd->add_option("g", color.g, "Green (0-255)")->required()->check(CLI::Range(0, 255));
    fill_cmd->add_option("b", color.b, "Blue (0-255)")->required()->check(CLI::Range(0, 255));
    fill_cmd->callback(callback);
}

} // namespace openskydimo::commands
