#include "openskydimo/logger.h"

#include <format>
#include <iostream>
#include <ostream>
#include <utility>

namespace openskydimo
{

Logger::Logger(std::string systemName) : m_systemName(std::move(systemName))
{
}

void Logger::Log(const std::string& message) const
{
    std::cout << std::format("[{}] - {}", m_systemName, message) << std::endl;
}

} // namespace openskydimo