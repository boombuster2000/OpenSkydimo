#pragma once

#include <string>
#include <vector>

#include "openskydimo/types/Response.h"

#include "Describable.h"

namespace openskydimo::command_dispatcher
{

class CommandNode : public Describable
{
    using Response = types::Response;

public:
    CommandNode(const std::string& name, const std::string& description) : Describable(name, description)
    {
    }

    ~CommandNode() override = default;

    virtual Response Execute(std::vector<std::string> args) = 0;
};

} // namespace openskydimo::command_dispatcher
