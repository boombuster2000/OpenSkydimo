#pragma once
#include <filesystem>

#include <nlohmann/json.hpp>
#include <string>

class ConfigFileHandler
{
public:
    nlohmann::json config;

public:
    explicit ConfigFileHandler(const std::string& filepath);

    void Load();
    void Save() const;

private:
    void EnsureDirectoryExists() const;
    void WriteJsonAtomically(const nlohmann::json& data) const;
    void InitConfig() const;

private:
    std::filesystem::path m_filepath;
};
