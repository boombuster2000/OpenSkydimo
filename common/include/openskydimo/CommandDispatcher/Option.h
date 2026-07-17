#pragma once

#include <functional>
#include <variant>

#include "Describable.h"

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

    [[nodiscard]] OptionVariant GetValue() const;

    void SetValue(std::string value);

    [[nodiscard]] std::string GetTypeName() const;

private:
    OptionVariant m_value;
    std::function<bool(OptionVariant)> m_validator;
};
