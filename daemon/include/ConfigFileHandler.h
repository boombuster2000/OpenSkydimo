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
        WriteJsonAtomically(config);
    }

private:
    void EnsureDirectoryExists() const
    {
        std::filesystem::create_directories(m_filepath.parent_path());
    }

    void WriteJsonAtomically(const nlohmann::json& data) const
    {
        const std::filesystem::path tempPath = m_filepath.string() + ".tmp";

        {
            std::ofstream fileStream(tempPath, std::ios::out | std::ios::trunc);
            if (!fileStream.is_open())
                throw std::runtime_error("Failed to open temp config file for writing.");

            try
            {
                fileStream << data.dump(4);
            }
            catch (const nlohmann::json::type_error& e)
            {
                throw std::runtime_error(std::format("Failed to serialize config: {}", e.what()));
            }

            fileStream.flush();
            if (fileStream.fail())
                throw std::runtime_error("Failed to write temp config file (disk full?).");
        }

        std::error_code ec;
        std::filesystem::rename(tempPath, m_filepath, ec);
        if (ec)
            throw std::runtime_error(std::format("Failed to replace config file: {}", ec.message()));
    }

    void InitConfig() const
    {
        EnsureDirectoryExists();
        const nlohmann::json defaultConfig = {{"port", ""}, {"led-count", 0}, {"last-effect", nullptr}};
        WriteJsonAtomically(defaultConfig);
    }

private:
    std::filesystem::path m_filepath;
};
