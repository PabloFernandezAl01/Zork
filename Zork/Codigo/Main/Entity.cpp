#include "Entity.h"

#include <ostream>

Entity::Entity(const std::string& id, const std::string& name, const std::string& description)
	: m_id(id)
	, m_name(name)
	, m_description(description)
{
}

const std::string& Entity::GetId() const
{
	return m_id;
}

const std::string& Entity::GetName() const
{
	return m_name;
}

const std::string& Entity::GetDescription() const
{
	return m_description;
}

void Entity::PrintInformation(std::ostream& output) const
{
	output << m_name << '\n';
	output << m_description << '\n';
}

void Entity::SetName(const std::string& name)
{
	m_name = name;
}

void Entity::SetDescription(const std::string& description)
{
	m_description = description;
}
