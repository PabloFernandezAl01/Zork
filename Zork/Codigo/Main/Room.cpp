#include "Room.h"

#include <algorithm>

Room::Room(const std::string& id, const std::string& name, const std::string& description)
	: Entity(id, name, description)
	, m_isDark(false)
	, m_isVisited(false)
	, m_isLocked(false)
{
}

bool Room::IsDark() const
{
	return m_isDark;
}

bool Room::IsVisited() const
{
	return m_isVisited;
}

void Room::SetDark(bool dark)
{
	m_isDark = dark;
}

void Room::SetVisited(bool visited)
{
	m_isVisited = visited;
}

bool Room::IsLocked() const
{
	return m_isLocked;
}

void Room::SetLocked(bool locked)
{
	m_isLocked = locked;
}

const std::map<Direction, std::string>& Room::GetExits() const
{
	return m_exits;
}

const std::vector<std::shared_ptr<Item>>& Room::GetItems() const
{
	return m_items;
}

void Room::AddExit(Direction direction, const std::string& targetRoomId)
{
	m_exits[direction] = targetRoomId;
}

void Room::AddItem(const std::shared_ptr<Item>& item)
{
	m_items.push_back(item);
}

std::shared_ptr<Item> Room::RemoveItem(const std::string& itemId)
{
	const auto it = std::find_if(m_items.begin(), m_items.end(),
		[&itemId](const std::shared_ptr<Item>& item)
		{
			return item->GetId() == itemId;
		});

	if (it == m_items.end())
	{
		return nullptr;
	}

	// It just gets removed from the vector
	const auto item = *it;
	m_items.erase(it);

	// The item itself is returned so it can be moved to the player inventory or wherever
	return item;
}
