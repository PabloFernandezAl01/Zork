#pragma once

#include "Entity.h"
#include "Item.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

// All possible movement directions in WestZork
enum class Direction
{
	North,
	South,
	East,
	West,
	Enter,
	Exit
};

struct DirectionUtils
{
	// Enum to string converter
	static std::string ToText(Direction direction);
};

struct Exit
{
	Exit();
	Exit(const std::string& targetRoomId, bool locked);

	std::string targetRoomId;
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

	const std::map<Direction, Exit>& GetExits() const;
	const std::vector<std::shared_ptr<Item>>& GetItems() const;
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

	// Prints the directions that connect the current room with adjacent rooms.
	void PrintExists(std::ostream& output) const;

	void AddExit(Direction direction, const std::string& targetRoomId, bool locked = false);

	/*
	* Item addition and removing
	*/
	void AddItem(const std::shared_ptr<Item>& item);
	std::shared_ptr<Item> RemoveItem(const std::string& itemId);

private:

	bool m_isDark;

	std::map<Direction, Exit> m_exits;
	std::vector<std::shared_ptr<Item>> m_items;
};
