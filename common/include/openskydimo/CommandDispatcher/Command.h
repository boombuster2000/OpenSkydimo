#pragma once
#include "CommandNode.h"
#include "Option.h"
#include "openskydimo/types/Response.h"
#include <algorithm>
#include <format>
#include <functional>
#include <string>
#include <utility>
#include <vector>
class Command;
using Callback = std::function<Response(const Command&)>;
class Command : public CommandNode
{
public:
    Command(const std::string& name, const std::string& description) : CommandNode(name, description)
    {
    }
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
    void SetCallback(const Callback& callback)
    {
        m_callback = callback;
    }
    Response Execute(const std::vector<std::string> args) override
    {
        if (args.empty() || args[0] == "help")
            return PrintHelp();

        for (size_t i = 0; i < m_options.size(); ++i)
        {
            if (i >= args.size())
                return {0, std::format("missing argument for option '{}'", m_options[i].GetName())};
            try
            {
                m_options[i].SetValue(args[i]);
            }
            catch (const std::invalid_argument& e)
            {
                return {0, std::format("invalid value for option '{}': {}", m_options[i].GetName(), e.what())};
            }
        }
        return m_callback(*this);
    }

private:
    [[nodiscard]] Response PrintHelp() const
    {
        std::string usage = GetName();
        for (const auto& opt : m_options)
            usage += std::format(" <{}>", opt.GetName());

        std::string help = std::format("{} - {}\n\nUsage:\n  {}\n", GetName(), GetDescription(), usage);

        if (!m_options.empty())
        {
            size_t maxLen = 0;
            for (const auto& opt : m_options)
                maxLen = std::max(maxLen, opt.GetName().size());

            help += "\nArguments:\n";
            for (const auto& opt : m_options)
            {
                help += std::format("  {:<{}}  {:<8}{}\n", opt.GetName(), maxLen,
                                    std::format("<{}>", opt.GetTypeName()), opt.GetDescription());
            }
        }
        return {0, help};
    }

private:
    std::vector<Option> m_options;
    Callback m_callback;
};
