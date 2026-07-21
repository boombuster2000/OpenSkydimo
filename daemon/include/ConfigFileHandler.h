#pragma once
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

class ConfigFileHandler
{
public:
    nlohmann::json config;

public:
    explicit ConfigFileHandler(const std::string& filepath) : m_filepath(filepath)
    {
    }

    void Load()
    {
        if (!std::filesystem::exists(m_filepath))
            InitConfig();

        std::fstream fileStream(m_filepath, std::ios::in);
        if (!fileStream.is_open())
            throw std::runtime_error("Failed to open config file for reading.");

        try
        {
            config = nlohmann::json::parse(fileStream);
        }
        catch (const nlohmann::json::parse_error& e)
        {
            throw std::runtime_error(std::format("Config file contains invalid JSON: {}", e.what()));
        }
    }

    void Save() const
    {
        EnsureDirectoryExists();

        std::fstream fileStream(m_filepath, std::ios::out);
        if (!fileStream.is_open())
            throw std::runtime_error("Failed to open config file for writing.");

        try
        {
            fileStream << config.dump(4);
        }
        catch (const nlohmann::json::type_error& e)
        {
            throw std::runtime_error(std::format("Failed to serialize config: {}", e.what()));
        }
    }

private:
    void EnsureDirectoryExists() const
    {
        std::filesystem::create_directories(m_filepath.parent_path());
    }

    void InitConfig() const
    {
        EnsureDirectoryExists();

        std::fstream fileStream(m_filepath, std::ios::out);
        if (!fileStream.is_open())
            throw std::runtime_error("Failed to create and open config file.");

        const nlohmann::json defaultConfig = {{"port", ""}, {"ledCount", 0}, {"lastEffect", nullptr}};
        fileStream << defaultConfig.dump(4);
    }

private:
    std::filesystem::path m_filepath;
};
