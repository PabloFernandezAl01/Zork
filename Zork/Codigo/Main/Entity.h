#pragma once

#include <iosfwd>
#include <string>

/*
 * Entity acts as the common abstract concept for every gameplay object in the
 * world. I marked PrintInformation as an abstract method to remark the concept
 * of abstract class and to avoid instantation of the Entity class.
 *
 * Room, Item and Player all share an id, a name and a description, so Entity
 * centralizes that data and exposes a polymorphic PrintInformation() function
 * that derived classes can specialize when they need to present themselves
 * differently.
 */
class Entity
{
public:

	Entity(const std::string& id, const std::string& name, const std::string& description);
	virtual ~Entity() = default;

	const std::string& GetId() const;
	const std::string& GetName() const;
	const std::string& GetDescription() const;

	virtual void PrintInformation(std::ostream& output) const = 0;

	void SetName(const std::string& name);
	void SetDescription(const std::string& description);

private:

	std::string m_id;
	std::string m_name;
	std::string m_description;
};
