#include "openskydimo/command_dispatcher/Option.h"

#include <format>
#include <stdexcept>

using namespace openskydimo::command_dispatcher;

OptionVariant Option::GetValue() const
{
    return m_value;
}

void Option::SetValue(std::string value)
{
    OptionVariant newValue;
    std::visit(
        [&newValue, &value]<typename T0>(T0&&) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, int>)
            {
                try
                {
                    newValue = std::stoi(value);
                }
                catch (const std::exception&)
                {
                    throw std::invalid_argument(std::format("expected int, got '{}'", value));
                }
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                try
                {
                    newValue = std::stof(value);
                }
                catch (const std::exception&)
                {
                    throw std::invalid_argument(std::format("expected float, got '{}'", value));
                }
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                if (value == "true" || value == "1")
                    newValue = true;
                else if (value == "false" || value == "0")
                    newValue = false;
                else
                    throw std::invalid_argument(std::format("expected bool, got '{}'", value));
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                newValue = value;
            }
        },
        m_value);

    if (!m_validator(newValue))
        throw std::invalid_argument(std::format("value '{}' failed validation", value));

    m_value = std::move(newValue);
}

std::string Option::GetTypeName() const

{
    return std::visit(
        []<typename T>(const T&) -> std::string {
            if constexpr (std::is_same_v<T, std::string>)
                return "string";
            else if constexpr (std::is_same_v<T, int>)
                return "int";
            else if constexpr (std::is_same_v<T, float>)
                return "float";
            else if constexpr (std::is_same_v<T, bool>)
                return "bool";

            return "unknown";
        },
        m_value);
}
