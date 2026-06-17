#pragma once

#include "Entity.h"
#include "Item.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

enum class Direction
{
	North,
	South,
	East,
	West,
	Down,
	Enter,
	Exit
};

std::string DirectionToText(Direction direction);

class Room : public Entity
{
public:

	Room(const std::string& id, const std::string& name, const std::string& description);

	bool IsDark() const;
	void SetDark(bool dark);

	bool IsVisited() const;
	void SetVisited(bool visited);

	bool IsLocked() const;
	void SetLocked(bool locked);

	const std::map<Direction, std::string>& GetExits() const;
	bool TryGetExit(Direction direction, std::string& outRoomId) const;
	const std::vector<std::shared_ptr<Item>>& GetItems() const;

	void PrintInformation(std::ostream& output) const override;

	void AddExit(Direction direction, const std::string& targetRoomId);
	void AddItem(const std::shared_ptr<Item>& item);

	std::shared_ptr<Item> RemoveItem(const std::string& itemId);

private:

	bool m_isDark;
	bool m_isVisited;
	bool m_isLocked;

	std::map<Direction, std::string> m_exits;
	std::vector<std::shared_ptr<Item>> m_items;
};
