#pragma once

#include <exception>
#include <functional>
#include <stdexcept>
#include <string>
#include <variant>

class Option
{
public:
    using OptionVariant = std::variant<std::string, int, float, bool>;

    template <typename T>
    Option(std::string name, std::string description, std::in_place_type_t<T>, std::function<bool(T)> validator)
        : m_name(name), m_description(description)
    {
        m_value = T{};

        m_validator = [validator](const OptionVariant& value) { return validator(std::get<T>(value)); };
    }

    ~Option() = default;

    OptionVariant GetValue() const
    {
        return m_value;
    };

    void SetValue(const std::string& value)
    {

        OptionVariant newValue;

        std::visit(
            [&newValue, &value](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, int>)
                    try
                    {
                        newValue = std::stoi(value);
                    }
                    catch (const std::exception&)
                    {
                        throw std::invalid_argument("Invalid value for option, expected int.");
                    }
                else if constexpr (std::is_same_v<T, float>)
                    try
                    {
                        newValue = std::stof(value);
                    }
                    catch (const std::exception&)
                    {
                        throw std::invalid_argument("Invalid value for option, expected float.");
                    }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    if (value == "true" || value == "1")
                        newValue = true;
                    else if (value == "false" || value == "0")
                        newValue = false;
                    else
                        throw std::invalid_argument("Invalid value for option, expected bool.");
                }
                else if constexpr (std::is_same_v<T, std::string>)
                    newValue = value;
            },
            m_value);

        if (!m_validator(newValue))
            throw std::invalid_argument("Invalid value for option");

        m_value = std::move(newValue);
    };

    std::string GetName() const
    {
        return m_name;
    }
    std::string GetDescription() const
    {
        return m_description;
    }

private:
    std::string m_name;
    std::string m_description;
    OptionVariant m_value;
    std::function<bool(OptionVariant)> m_validator;
};
