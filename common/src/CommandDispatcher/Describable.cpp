#include "openskydimo/CommandDispatcher/Describable.h"

Describable::Describable(std::string name, std::string description)
    : m_name(std::move(name)), m_description(std::move(description))
{
}

std::string Describable::GetName() const
{
    return m_name;
}

std::string Describable::GetDescription() const
{
    return m_description;
}
