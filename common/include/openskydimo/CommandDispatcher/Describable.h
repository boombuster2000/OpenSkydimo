#pragma once

#include <string>
#include <utility>

class Describable
{
public:
    Describable(std::string name, std::string description)
        : m_name(std::move(name)), m_description(std::move(description))
    {
    }

    virtual ~Describable() = default;

    [[nodiscard]] std::string GetName() const
    {
        return m_name;
    }

    [[nodiscard]] std::string GetDescription() const
    {
        return m_description;
    }

private:
    std::string m_name;
    std::string m_description;
};
