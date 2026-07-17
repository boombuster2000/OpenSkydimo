#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "openskydimo/types/Response.h"

#include "Command.h"
#include "CommandNode.h"

class CommandGroup : public CommandNode
{
public:
    CommandGroup(const std::string& name, const std::string& description) : CommandNode(name, description)
    {
    }

    ~CommandGroup() override = default;

    Command* AddCommand(const std::string& name, const std::string& description)
    {
        auto [it, inserted] = m_subcommands.emplace(name, std::make_unique<Command>(name, description));
        return dynamic_cast<Command*>(it->second.get());
    }

    CommandGroup* AddCommandGroup(const std::string& name, const std::string& description)
    {
        auto [it, inserted] = m_subcommands.emplace(name, std::make_unique<CommandGroup>(name, description));
        return dynamic_cast<CommandGroup*>(it->second.get());
    }

    Response Execute(std::vector<std::string> args) override
    {
        if (!args.empty())
        {
            if (const auto it = m_subcommands.find(args[0]); it != m_subcommands.end())
                return it->second->Execute(std::vector(args.begin() + 1, args.end()));
        }

        return {1, "Unknown Command"};
    }

private:
    std::unordered_map<std::string, std::unique_ptr<CommandNode>> m_subcommands;
};
