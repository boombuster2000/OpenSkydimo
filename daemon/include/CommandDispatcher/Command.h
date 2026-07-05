// Command.h
#pragma once
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Option.h"

class Command
{
public:
    using Callback = std::function<std::string()>;

    Command(std::string name, std::string description) : m_name(std::move(name)), m_description(std::move(description))
    {
    }

    ~Command() = default;
    Command(Command&&) noexcept = default;
    Command& operator=(Command&&) noexcept = default;
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    template <typename T>
    Option& AddOption(std::string name, std::string description, std::function<bool(T)> validator)
    {
        if (isCommandGroup)
            throw std::logic_error("Cannot add a normal option: this Command already has subcommands.");

        m_options.emplace_back(std::move(name), std::move(description), std::in_place_type<T>, std::move(validator));
        return m_options.back();
    }

    template <typename T>
    T GetOption(std::string name) const
    {
        auto it = std::find_if(m_options.begin(), m_options.end(),
                               [&](const Option& option) { return option.GetName() == name; });

        if (it == m_options.end())
            throw std::logic_error("Option not found: " + name);

        return std::get<T>(it->GetValue());
    }

    Command& AddCommand(std::string name, std::string description)
    {
        if (!m_options.empty())
            throw std::logic_error("Cannot add a subcommand: this Command already has normal options.");

        isCommandGroup = true;

        auto [it, inserted] = m_subcommands.emplace(name, Command(name, description));
        return it->second;
    }

    void SetCallback(Callback callback)
    {
        if (isCommandGroup)
            throw std::logic_error("Cannot set a callback: this Command is a command group.");

        m_callback = [this, callback = std::move(callback)]() { return callback(); };
    }

    std::string Execute(const std::vector<std::string>& args)
    {
        if (!args.empty())
        {
            auto it = m_subcommands.find(args[0]);
            if (it != m_subcommands.end())
                return it->second.Execute(std::vector<std::string>(args.begin() + 1, args.end()));
        }

        if (m_callback)
        {
            for (size_t i = 0; i < m_options.size(); ++i)
                m_options[i].SetValue(args[i]);

            return m_callback();
        }

        return "idk";
    }

private:
    std::string m_name;
    std::string m_description;
    Callback m_callback;
    std::vector<Option> m_options;
    std::unordered_map<std::string, Command> m_subcommands;
    bool isCommandGroup = false;
};
