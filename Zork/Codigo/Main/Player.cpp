#include "Player.h"

#include <algorithm>
#include <ostream>

Player::Player(const std::string& id, const std::string& name, const std::string& description)
	: Entity(id, name, description)
{
}

const std::string& Player::GetCurrentRoomId() const
{
	return m_currentRoomId;
}

const std::vector<std::shared_ptr<Item>>& Player::GetInventory() const
{
	return m_inventory;
}

void Player::PrintInformation(std::ostream& output) const
{
	Entity::PrintInformation(output);

	if (m_inventory.empty())
	{
		output << "No llevas nada.\n";
		return;
	}

	output << "Llevas:\n";
	for (const std::shared_ptr<Item>& item : m_inventory)
	{
		output << "- " << item->GetName() << '\n';
	}
}

void Player::SetCurrentRoomId(const std::string& roomId)
{
	m_currentRoomId = roomId;
}

void Player::AddItemToInventory(const std::shared_ptr<Item>& item)
{
	m_inventory.push_back(item);
}

std::shared_ptr<Item> Player::RemoveItemFromInventory(const std::string& itemId)
{
	const auto it = std::find_if(m_inventory.begin(), m_inventory.end(),
		[&itemId](const std::shared_ptr<Item>& item)
		{
			return item->GetId() == itemId;
		});

	if (it == m_inventory.end())
	{
		return nullptr;
	}

	// Same as in Room, it just gets removed from the vector
	const auto item = *it;
	m_inventory.erase(it);

	// The item itself is returned so it can be moved to the room where the player is or inside an item
	return item;
}
