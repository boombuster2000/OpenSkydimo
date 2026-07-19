#include "openskydimo/CommandDispatcher/Command.h"

#include <format>

Command::Command(const std::string& name, const std::string& description) : CommandNode(name, description)
{
}

void Command::SetCallback(const Callback& callback)
{
    m_callback = callback;
}

Response Command::Execute(const std::vector<std::string> args)
{
    if ((args.empty() && !m_options.empty()) || (!args.empty() && args[0] == "help"))
        return PrintHelp();

    if (args.size() > m_options.size())
        return MakeError(1, std::format("too many arguments for command '{}': expected {}, got {}", GetName(),
                                        m_options.size(), args.size()));
    for (size_t i = 0; i < m_options.size(); ++i)
    {
        if (i >= args.size())
            return MakeError(1, std::format("missing argument for option '{}'", m_options[i].GetName()));

        try
        {
            m_options[i].SetValue(args[i]);
        }
        catch (const std::invalid_argument& e)
        {
            return MakeError(1, std::format("invalid value for option '{}': {}", m_options[i].GetName(), e.what()));
        }
    }
    if (!m_callback)
        return MakeError(1, std::format("command '{}' has no callback set", GetName()));
    
    return m_callback(*this);
}

Response Command::PrintHelp() const
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
            help += std::format("  {:<{}}  {:<8}{}\n", opt.GetName(), maxLen, std::format("<{}>", opt.GetTypeName()),
                                opt.GetDescription());
        }
    }

    return {0, help};
}
