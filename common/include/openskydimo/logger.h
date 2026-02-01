#pragma once
#include <string>

namespace openskydimo
{
class Logger
{

public:
    explicit Logger(std::string systemName);

    void Log(const std::string& message) const;

private:
    std::string m_systemName;
};
} // namespace openskydimo