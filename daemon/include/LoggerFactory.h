#pragma once

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

class LoggerFactory
{
public:
    static void Init(const std::string& path)
    {
        spdlog::flush_every(std::chrono::seconds(1));
        m_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, 1024 * 1024 * 5, 3);
    }

    static std::shared_ptr<spdlog::logger> Create(const std::string& name)
    {
        if (!m_sink)
            throw std::runtime_error("LoggerFactory::Init() must be called before Create()");

        auto logger = std::make_shared<spdlog::logger>(name, m_sink);
        spdlog::register_logger(logger);
        return logger;
    }

private:
    static inline std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> m_sink;
};
