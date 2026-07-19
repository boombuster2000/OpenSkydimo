#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "openskydimo/types/Response.h"

#include "CommandNode.h"
#include "Option.h"

class Command;
using Callback = std::function<Response(const Command&)>;
class Command : public CommandNode
{
public:
    Command(const std::string& name, const std::string& description);

    ~Command() override = default;

    template <typename T>
    Option& AddOption(std::string name, std::string description, std::function<bool(T)> validator)
    {
        m_options.emplace_back(std::move(name), std::move(description), std::in_place_type<T>, std::move(validator));
        return m_options.back();
    }

    template <typename T>
    T GetOption(const std::string& name) const
    {
        auto it = std::find_if(m_options.begin(), m_options.end(),
                               [&](const Option& option) { return option.GetName() == name; });
        if (it == m_options.end())
            throw std::logic_error("Option not found: " + name);
        return std::get<T>(it->GetValue());
    }

    void SetCallback(const Callback& callback);

    Response Execute(std::vector<std::string> args) override;

private:
    [[nodiscard]] Response PrintHelp() const;

private:
    std::vector<Option> m_options;
    Callback m_callback;
};
