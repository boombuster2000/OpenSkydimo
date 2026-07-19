#include "openskydimo/CommandDispatcher/CommandGroup.h"

#include <algorithm>
#include <format>
#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

CommandGroup::CommandGroup(const std::string& name, const std::string& description) : CommandNode(name, description)
{
}

Command* CommandGroup::AddCommand(const std::string& name, const std::string& description)
{
    auto [it, inserted] = m_subcommands.emplace(name, std::make_unique<Command>(name, description));
    return static_cast<Command*>(it->second.get());
}

CommandGroup* CommandGroup::AddCommandGroup(const std::string& name, const std::string& description)
{
    auto [it, inserted] = m_subcommands.emplace(name, std::make_unique<CommandGroup>(name, description));
    return static_cast<CommandGroup*>(it->second.get());
}

Response CommandGroup::Execute(std::vector<std::string> args)
{
    if (args.empty() || args[0] == "help")
        return PrintHelp();

    if (const auto it = m_subcommands.find(args[0]); it != m_subcommands.end())
        return it->second->Execute(std::vector(args.begin() + 1, args.end()));

    return MakeError(1, std::format("Unknown command: {}", args[0]));
}

Response CommandGroup::PrintHelp() const
{
    std::string help =
        std::format("{} - {}\n\nUsage:\n  {} <command> [args]\n", GetName(), GetDescription(), GetName());

    if (!m_subcommands.empty())
    {
        std::vector<std::pair<std::string, const CommandNode*>> sorted;
        sorted.reserve(m_subcommands.size());
        for (const auto& [name, node] : m_subcommands)
            sorted.emplace_back(name, node.get());
        std::ranges::sort(sorted, {}, &std::pair<std::string, const CommandNode*>::first);

        size_t maxLen = 0;
        for (const auto& name : sorted | std::views::keys)
            maxLen = std::max(maxLen, name.size());

        help += "\nCommands:\n";
        for (const auto& [name, node] : sorted)
            help += std::format("  {:<{}}  {}\n", name, maxLen, node->GetDescription());
    }

    help += std::format("\nRun '{} <command> help' for more information on a command.\n", GetName());
    return {0, help};
}
