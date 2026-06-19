#pragma once

#include "Direction.h"
#include "Entity.h"
#include "Item.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct Exit
{
	Exit();
	Exit(const std::string& targetRoomId, const std::string& targetRoomName, bool locked);

	std::string targetRoomId;
	std::string targetRoomName;
	bool isLocked;
};

class Room : public Entity
{
public:

	Room(const std::string& id, const std::string& name, const std::string& description);

	/*
	*  Gameplay attributes GET/SET
	*/
	bool IsDark() const;
	void SetDark(bool dark);

	Exit* FindExit(Direction direction);
	const Exit* FindExit(Direction direction) const;

	Item* FindItem(const std::string& target);
	const Item* FindItem(const std::string& target) const;

	/*
	*  Prints to the output the info of the item:
	* - Name
	* - Description
	* - Names of the items in the room
	* - In Zork this would also prints the exists but in WestZork thats handled by another command ("Examinar mapa")
	*/
	void PrintInformation(std::ostream& output) const override;

	// Prints the names and directions of the rooms connected to the current room.
	void PrintExists(std::ostream& output) const;

	void AddExit(Direction direction, const Room& targetRoom, bool locked = false);

	/*
	* Item addition and removing
	*/
	bool AddItem(const std::shared_ptr<Item>& item);
	std::shared_ptr<Item> RemoveItem(const std::string& itemId);

private:

	bool m_isDark;

	std::map<Direction, Exit> m_exits;
	std::vector<std::shared_ptr<Item>> m_items;
};
