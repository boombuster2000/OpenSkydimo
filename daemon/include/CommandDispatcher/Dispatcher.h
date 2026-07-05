#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "CommandDispatcher/Command.h"

class Dispatcher
{
public:
    Dispatcher() = default;
    ~Dispatcher() = default;

    Command& AddCommand(const std::string& name, const std::string& description)
    {
        auto [it, inserted] = m_commands.emplace(name, Command(name, description));
        return it->second;
    }

    std::string Dispatch(const std::vector<std::string>& args)
    {
        if (args.empty())
            return "Empty";

        const auto it = m_commands.find(args[0]);

        if (it == m_commands.end())
            return "Command not found";

        return it->second.Execute(std::vector<std::string>(args.begin() + 1, args.end()));
    }

private:
    std::unordered_map<std::string, Command> m_commands;
};
