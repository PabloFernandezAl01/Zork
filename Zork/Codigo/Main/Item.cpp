#include "Item.h"

Item::Item(const std::string& id, const std::string& name, const std::string& description)
	: Entity(id, name, description)
	, m_isContainer(false)
	, m_isLightSource(false)
	, m_isWeapon(false)
{
}

bool Item::IsContainer() const
{
	return m_isContainer;
}

bool Item::IsLightSource() const
{
	return m_isLightSource;
}

bool Item::IsWeapon() const
{
	return m_isWeapon;
}

void Item::SetContainer(bool container)
{
	m_isContainer = container;
}

void Item::SetLightSource(bool lightSource)
{
	m_isLightSource = lightSource;
}

void Item::SetWeapon(bool weapon)
{
	m_isWeapon = weapon;
}

const std::vector<std::shared_ptr<Item>>& Item::GetContainedItems() const
{
	return m_containedItems;
}

void Item::AddItem(const std::shared_ptr<Item>& item)
{
	if (!m_isContainer)
		return;

	m_containedItems.push_back(item);
}
