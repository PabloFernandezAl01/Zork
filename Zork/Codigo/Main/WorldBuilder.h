#pragma once

#include "Item.h"
#include "Room.h"

#include <map>
#include <memory>
#include <string>

struct WorldData
{
	std::map<std::string, std::shared_ptr<Room>> rooms;
	std::map<std::string, std::shared_ptr<Item>> items;
	std::string initialRoomId;
};

/*
Builds the fixed content of West Zork. Gameplay rules remain in GameWorld.
*/
class WorldBuilder
{
public:

	static WorldData Build();

private:

	static void InitializeRooms(WorldData& world);
	static void ConnectRooms(WorldData& world);
	static void InitializeItems(WorldData& world);
	static void PlaceItems(WorldData& world);

	static void AddRoom(WorldData& world, const std::shared_ptr<Room>& room);
	static void AddItem(WorldData& world, const std::shared_ptr<Item>& item);
};
