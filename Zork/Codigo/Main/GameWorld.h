#pragma once

#include "Item.h"
#include "Player.h"
#include "Room.h"

#include <map>
#include <memory>
#include <iosfwd>
#include <string>

struct Command;

enum class GameResult
{
	Running,
	Victory,
	Defeat,
	Quit,
	FatalError
};

/*
GameWorld owns the main entities and coordinates gameplay rules.
*/

class GameWorld
{
public:

	GameWorld();

	// Entry point for the class, called from WestZork::Run()
	GameResult ExecuteCommand(const Command& command, std::ostream& output);

	// Shows the current room information (name, desc, items, exists)
	GameResult Look(std::ostream& output) const;

private:
	
	/*
	*  Rooms & items queries and handling
	*/
	bool IsAValidItem(const std::string& target) const;

	Room* FindRoomById(const std::string& roomId);
	const Room* FindRoomById(const std::string& roomId) const;

	Item* FindItemById(const std::string& itemId);
	const Item* FindItemById(const std::string& itemId) const;
	Item* FindAccessibleItem(const std::string& target);
	const Item* FindAccessibleItem(const std::string& target) const;
	static bool TargetsCellDoor(const std::string& target);
	static bool TargetsCryptLock(const std::string& target);
	static bool TargetsChurchChains(const std::string& target);

	Room* GetCurrentRoom();
	const Room* GetCurrentRoom() const;
	bool CanPlayerSee(const Room& room) const;

	/*
	*  Commands handling
	*/
	GameResult MovePlayer(Direction direction, std::ostream& output);
	GameResult Examine(const std::string& target, std::ostream& output) const;
	GameResult ReadItem(const Item& item, const Room& room, std::ostream& output) const;
	GameResult ShowInventory(std::ostream& output) const;
	GameResult TakeItem(const std::string& target, std::ostream& output);
	GameResult DropItem(const std::string& target, std::ostream& output);
	GameResult PutItemIntoContainer(const std::string& itemTarget, const std::string& containerTarget, std::ostream& output);
	GameResult TakeItemFromContainer(const std::string& itemTarget, const std::string& containerTarget, std::ostream& output);
	GameResult Open(const std::string& target, const std::string& toolTarget, std::ostream& output);
	GameResult OpenItem(const std::string& target, const std::string& toolTarget, std::ostream& output);
	GameResult Unlock(const std::string& target, const std::string& toolTarget, std::ostream& output);
	GameResult TurnOnItem(const std::string& target, std::ostream& output);
	GameResult LoadItem(const std::string& target, const std::string& ammunitionTarget, std::ostream& output);
	GameResult BreakObstacle(const std::string& target, const std::string& toolTarget, std::ostream& output);
	GameResult ShootTarget(const std::string& target, const std::string& weaponTarget, std::ostream& output);
	GameResult ShowHelp(std::ostream& output) const;

	Player m_player;

	std::map<std::string, std::shared_ptr<Room>> m_rooms;
	std::map<std::string, std::shared_ptr<Item>> m_items;
};
