#pragma once
#include "Describable.h"
#include <format>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

using OptionVariant = std::variant<std::string, int, float, bool>;

class Option : public Describable
{
public:
    template <typename T>
    Option(const std::string name, const std::string description, std::in_place_type_t<T>,
           std::function<bool(T)> validator)
        : Describable(name, description)
    {
        m_value = T{};
        m_validator = [validator](const OptionVariant& value) { return validator(std::get<T>(value)); };
    }

    [[nodiscard]] OptionVariant GetValue() const
    {
        return m_value;
    }

    void SetValue(std::string value)
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

    [[nodiscard]] std::string GetTypeName() const
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

private:
    OptionVariant m_value;
    std::function<bool(OptionVariant)> m_validator;
};
