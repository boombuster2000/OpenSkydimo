#pragma once
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "openskydimo/types/Response.h"

#include "Option.h"
class Command;
using Callback = std::function<Response(Command&)>;

class Command
{
public:
    Command(std::string name, std::string description) : m_name(std::move(name)), m_description(std::move(description))
    {
    }

    ~Command() = default;
    Command(Command&&) noexcept = default;
    Command& operator=(Command&&) noexcept = default;
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    std::string GetName() const
    {
        return m_name;
    }

    std::string GetDescription() const
    {
        return m_description;
    }

    template <typename T>
    Option& AddOption(std::string name, std::string description, std::function<bool(T)> validator)
    {
        if (m_isCommandGroup)
            throw std::logic_error("Cannot add a normal option: this Command already has subcommands.");

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

    Command& AddCommand(const std::string& name, const std::string& description)
    {
        if (!m_options.empty())
            throw std::logic_error("Cannot add a subcommand: this Command already has normal options.");

        m_isCommandGroup = true;

        auto [it, inserted] = m_subcommands.emplace(name, Command(name, description));
        return it->second;
    }

    void SetCallback(const Callback& callback)
    {
        if (m_isCommandGroup)
            throw std::logic_error("Cannot set a callback: this Command is a command group.");

        m_callback = callback;
    }

    Response Execute(const std::vector<std::string>& args)
    {
        if (!args.empty() && args[0] == "help")
        {
            Response response;
            response.code = 0;
            if (m_isCommandGroup)
            {
                for (const auto& command : m_subcommands | std::views::values)
                {
                    response.message += command.GetName() + "\t" + command.GetDescription() + "\n";
                }

                return response;
            }

            for (const auto& option : m_options)
            {
                response.message += option.GetName() + "\t" + option.GetDescription() + "\n";
            }

            return response;
        }

        if (!args.empty())
        {
            if (const auto it = m_subcommands.find(args[0]); it != m_subcommands.end())
                return it->second.Execute(std::vector(args.begin() + 1, args.end()));
        }

        if (m_callback)
        {
            for (size_t i = 0; i < m_options.size(); ++i)
            {
                if (i >= args.size())
                {
                    return MakeError(1, std::format("missing argument for option '{}'", m_options[i].GetName()));
                }

                try
                {
                    m_options[i].SetValue(args[i]);
                }
                catch (const std::invalid_argument& e)
                {
                    return MakeError(
                        1, std::format("invalid value for option '{}': {}", m_options[i].GetName(), e.what()));
                }
            }

            return m_callback(*this);
        }

        return {.code = 1, .message = "error."};
    }

private:
    std::string m_name;
    std::string m_description;
    Callback m_callback;
    std::vector<Option> m_options;
    std::unordered_map<std::string, Command> m_subcommands;
    bool m_isCommandGroup = false;
};
