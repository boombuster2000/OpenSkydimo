#pragma once
#include <filesystem>

#include <nlohmann/json.hpp>
#include <string>

class ConfigFileHandler
{
public:
    nlohmann::json config;

public:
    explicit ConfigFileHandler(const std::string& filepath,
                               const nlohmann::json& defaultConfig = nlohmann::json::object());

    void Load();
    void Save() const;

private:
    void EnsureDirectoryExists() const;
    void WriteJsonAtomically(const nlohmann::json& data) const;
    void InitConfig();

private:
    std::filesystem::path m_filepath;
    nlohmann::json m_defaultConfig;
};
