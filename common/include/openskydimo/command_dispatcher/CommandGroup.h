#pragma once

#include <memory>
#include <unordered_map>

#include "openskydimo/types/Response.h"

#include "Command.h"
#include "CommandNode.h"

namespace openskydimo::command_dispatcher
{

class CommandGroup : public CommandNode
{

    using Response = types::Response;

public:
    CommandGroup(const std::string& name, const std::string& description);

    ~CommandGroup() override = default;

    Command* AddCommand(const std::string& name, const std::string& description);

    CommandGroup* AddCommandGroup(const std::string& name, const std::string& description);

    Response Execute(std::vector<std::string> args) override;

private:
    [[nodiscard]] Response PrintHelp() const;

private:
    std::unordered_map<std::string, std::unique_ptr<CommandNode>> m_subcommands;
};

} // namespace openskydimo::command_dispatcher
