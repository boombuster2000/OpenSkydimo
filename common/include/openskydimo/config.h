#pragma once
#include <cstdlib>
#include <filesystem>
#include <string>

static const std::string s_socketPath = "/tmp/openskydimo.sock";

inline std::filesystem::path GetConfigPath()
{
    if (const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME"))
        return std::filesystem::path(xdgConfigHome) / "openskydimo" / "config.json";

    const char* home = std::getenv("HOME");
    if (!home)
        throw std::runtime_error("Neither XDG_CONFIG_HOME nor HOME is set");

    return std::filesystem::path(home) / ".config" / "openskydimo" / "config.json";
}
