#pragma once

#include <string>

class Describable
{
public:
    Describable(std::string name, std::string description);

    virtual ~Describable() = default;

    [[nodiscard]] std::string GetName() const;

    [[nodiscard]] std::string GetDescription() const;

private:
    std::string m_name;
    std::string m_description;
};
