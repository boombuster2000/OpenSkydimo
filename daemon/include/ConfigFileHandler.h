#pragma once

#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

class ConfigFileHandler
{
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
            m_config = nlohmann::json::parse(fileStream);
        }
        catch (const nlohmann::json::parse_error& e)
        {
            throw std::runtime_error(std::format("Config file contains invalid JSON: {}", e.what()));
        }
    }

private:
    void InitConfig() const
    {
        std::filesystem::create_directories(m_filepath.parent_path());

        std::fstream fileStream(m_filepath, std::ios::out);

        if (!fileStream.is_open())
            throw std::runtime_error("Failed to create and open config file.");

        const nlohmann::json defaultConfig = {{"port", ""}, {"ledCount", 0}, {"lastEffect", nullptr}};
        fileStream << defaultConfig.dump(4);
        fileStream.close();
    }

private:
    std::filesystem::path m_filepath;
    nlohmann::json m_config;
};
