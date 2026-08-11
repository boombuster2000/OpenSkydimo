#pragma once
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace openskydimo
{

inline std::filesystem::path GetRuntimeDir()
{
    if (const char* xdgRuntimeDir = std::getenv("XDG_RUNTIME_DIR"); xdgRuntimeDir && *xdgRuntimeDir)
        return xdgRuntimeDir;

    return "/tmp";
}

inline std::string GetSocketPath()
{
    return (GetRuntimeDir() / "openskydimo.sock").string();
}

inline std::filesystem::path GetConfigPath()
{
    if (const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME"); xdgConfigHome && *xdgConfigHome)
        return std::filesystem::path(xdgConfigHome) / "openskydimo" / "config.json";

    const char* home = std::getenv("HOME");
    if (!home || !*home)
        throw std::runtime_error("Neither XDG_CONFIG_HOME nor HOME is set");

    return std::filesystem::path(home) / ".config" / "openskydimo" / "config.json";
}

} // namespace openskydimo
