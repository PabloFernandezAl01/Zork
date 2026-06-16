#pragma once

#include "Item.h"
#include "Player.h"
#include "Room.h"

#include <map>
#include <memory>
#include <string>

/*
GameWorld keeps ownership of rooms and items. These methods return non-owning
pointers because the requested entity may not exist.
*/

class GameWorld
{
public:

	GameWorld();

	Player& GetPlayer();
	const Player& GetPlayer() const;

	Room* FindRoomById(const std::string& roomId);
	const Room* FindRoomById(const std::string& roomId) const;

	Item* FindItemById(const std::string& itemId);
	const Item* FindItemById(const std::string& itemId) const;

	void AddRoom(const std::shared_ptr<Room>& room);
	void AddItem(const std::shared_ptr<Item>& item);

private:

	Player m_player;

	std::map<std::string, std::shared_ptr<Room>> m_rooms;
	std::map<std::string, std::shared_ptr<Item>> m_items;
};
