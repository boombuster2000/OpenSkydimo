#pragma once

#include <functional>
#include <utility>
#include <variant>

#include "Describable.h"

namespace openskydimo::command_dispatcher
{

using OptionVariant = std::variant<std::string, int, float, bool>;

class Option : public Describable
{
public:
    template <typename T>
    Option(std::string name, std::string description, std::in_place_type_t<T>, std::function<bool(T)> validator)
        : Describable(std::move(name), std::move(description))
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

} // namespace openskydimo::command_dispatcher
