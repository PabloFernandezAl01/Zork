#pragma once

#include "Item.h"
#include "Player.h"
#include "Room.h"

#include <map>
#include <memory>
#include <iosfwd>
#include <string>

struct Command;

/*
GameWorld owns the main entities and coordinates gameplay rules.
*/

class GameWorld
{
public:

	GameWorld();

	// Entry point for the class, called from WestZork::Run()
	void ExecuteCommand(const Command& command, bool& isRunning, std::ostream& output);

	// Shows the current room information (name, desc, items, exists)
	void Look(std::ostream& output, bool& isRunning) const;

private:
	
	/*
	*  Rooms & items queries and handling
	*/
	bool IsAValidItem(const std::string& target) const;

	Room* FindRoomById(const std::string& roomId);
	const Room* FindRoomById(const std::string& roomId) const;

	Item* FindItemById(const std::string& itemId);
	const Item* FindItemById(const std::string& itemId) const;

	Room* GetCurrentRoom();
	const Room* GetCurrentRoom() const;

	void AddRoom(const std::shared_ptr<Room>& room);
	void AddItem(const std::shared_ptr<Item>& item);

	/*
	*  Commands handling
	*/
	void MovePlayer(Direction direction, bool& isRunning, std::ostream& output);
	void Examine(const std::string& target, bool& isRunning, std::ostream& output) const;
	void ShowInventory(std::ostream& output) const;
	void TakeItem(const std::string& target, bool& isRunning, std::ostream& output);
	void DropItem(const std::string& target, bool& isRunning, std::ostream& output);
	void PutItemIntoContainer(const std::string& itemTarget, bool& isRunning, const std::string& containerTarget, std::ostream& output);
	void TakeItemFromContainer(const std::string& itemTarget, bool& isRunning, const std::string& containerTarget, std::ostream& output);
	void Open(const std::string& target, const std::string& toolTarget, bool& isRunning, std::ostream& output);
	void OpenItem(const std::string& target, const std::string& toolTarget, bool& isRunning, std::ostream& output);
	void Unlock(const std::string& target, const std::string& toolTarget, bool& isRunning, std::ostream& output);
	void TurnOnItem(const std::string& target, bool& isRunning, std::ostream& output);
	void LoadItem(const std::string& target, const std::string& ammunitionTarget, bool& isRunning, std::ostream& output);
	void BreakObstacle(const std::string& target, const std::string& toolTarget, bool& isRunning, std::ostream& output);
	void ShowHelp(std::ostream& output) const;

	/*
	*  WORLD SETUP
	* - Rooms creation
	* - Rooms conexions creation
	* - Items creation
	* - Items distribution in rooms
	*/
	void InitializeWorld(); 

	Player m_player;

	std::map<std::string, std::shared_ptr<Room>> m_rooms;
	std::map<std::string, std::shared_ptr<Item>> m_items;
};
