#include "GameWorld.h"

GameWorld::GameWorld()
	: m_player("player", "James", "Un antiguo alguacil que vuelve al pueblo en busca de su hermano Elias.")
{
}

Player& GameWorld::GetPlayer()
{
	return m_player;
}

const Player& GameWorld::GetPlayer() const
{
	return m_player;
}

Room* GameWorld::FindRoomById(const std::string& roomId)
{
	const auto it = m_rooms.find(roomId);
	return it != m_rooms.end() ? it->second.get() : nullptr;
}

const Room* GameWorld::FindRoomById(const std::string& roomId) const
{
	const auto it = m_rooms.find(roomId);
	return it != m_rooms.end() ? it->second.get() : nullptr;
}

Item* GameWorld::FindItemById(const std::string& itemId)
{
	const auto it = m_items.find(itemId);
	return it != m_items.end() ? it->second.get() : nullptr;
}

const Item* GameWorld::FindItemById(const std::string& itemId) const
{
	const auto it = m_items.find(itemId);
	return it != m_items.end() ? it->second.get() : nullptr;
}

void GameWorld::AddRoom(const std::shared_ptr<Room>& room)
{
	m_rooms[room->GetId()] = room;
}

void GameWorld::AddItem(const std::shared_ptr<Item>& item)
{
	m_items[item->GetId()] = item;
}
