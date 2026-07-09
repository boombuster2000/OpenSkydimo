#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Command.h"

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

    Response Dispatch(const std::vector<std::string>& args)
    {
        if (args.empty())
            return {1, "Empty args."};

        if (args[0] == "help")
        {
            Response response;

            response.code = 0;
            for (const auto& command : m_commands | std::views::values)
            {
                response.message += command.GetName() + "\t" + command.GetDescription() + "\n";
            }

            return response;
        }

        const auto it = m_commands.find(args[0]);

        if (it == m_commands.end())
            return {1, "Command not found"};

        return it->second.Execute(std::vector(args.begin() + 1, args.end()));
    }

private:
    std::unordered_map<std::string, Command> m_commands;
};
