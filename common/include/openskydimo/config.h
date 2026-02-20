#pragma once
#include <filesystem>
#include <string>

static const std::string s_socketPath = "/tmp/openskydimo.sock";

inline std::filesystem::path GetConfigFilePath()
{
    namespace fs = std::filesystem;

    const char* homeDir = std::getenv("HOME");
    if (!homeDir)
        throw std::runtime_error("HOME environment variable not set");

    const fs::path configDir = fs::path(homeDir) / ".config" / "openskydimo";

    if (!fs::exists(configDir))
        fs::create_directories(configDir);

    return configDir / "config.json";
}
