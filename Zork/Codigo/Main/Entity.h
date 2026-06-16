#pragma once

#include <string>

class Entity
{
public:

	Entity(const std::string& id, const std::string& name, const std::string& description);
	virtual ~Entity() = default;

	const std::string& GetId() const;
	const std::string& GetName() const;
	const std::string& GetDescription() const;

	void SetName(const std::string& name);
	void SetDescription(const std::string& description);

private:

	std::string m_id;
	std::string m_name;
	std::string m_description;
};
