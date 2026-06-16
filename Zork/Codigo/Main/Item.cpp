#include "Item.h"

#include <algorithm>

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

std::shared_ptr<Item> Item::RemoveItem(const std::string& itemId)
{
	if (!m_isContainer)
	{
		return nullptr;
	}

	const auto it = std::find_if(m_containedItems.begin(), m_containedItems.end(),
		[&itemId](const std::shared_ptr<Item>& item)
		{
			return item->GetId() == itemId;
		});

	if (it == m_containedItems.end())
	{
		return nullptr;
	}

	// Same as in Room and Player, it just gets removed from the vector
	const auto item = *it;
	m_containedItems.erase(it);

	// The item itself is returned so it can be moved to the player inventory or the room where the player is
	return item;
}
