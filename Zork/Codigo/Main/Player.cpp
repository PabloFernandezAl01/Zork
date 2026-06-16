#include "Player.h"

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

void Player::SetCurrentRoomId(const std::string& roomId)
{
	m_currentRoomId = roomId;
}

void Player::AddItemToInventory(const std::shared_ptr<Item>& item)
{
	m_inventory.push_back(item);
}
