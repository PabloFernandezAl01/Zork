#include "Room.h"

#include <algorithm>
#include <ostream>

std::string DirectionUtils::ToText(Direction direction)
{
	switch (direction)
	{
	case Direction::North:
		return "norte";
	case Direction::South:
		return "sur";
	case Direction::East:
		return "este";
	case Direction::West:
		return "oeste";
	case Direction::Enter:
		return "entrar";
	case Direction::Exit:
		return "salir";
	default:
		return "desconocida";
	}
}

Exit::Exit()
	: isLocked(false)
{
}

Exit::Exit(const std::string& targetRoomId, bool locked)
	: targetRoomId(targetRoomId)
	, isLocked(locked)
{
}

Room::Room(const std::string& id, const std::string& name, const std::string& description)
	: Entity(id, name, description)
	, m_isDark(false)
{
}

bool Room::IsDark() const
{
	return m_isDark;
}

void Room::SetDark(bool dark)
{
	m_isDark = dark;
}

Exit* Room::FindExit(Direction direction)
{
	const auto it = m_exits.find(direction);
	return it != m_exits.end() ? &it->second : nullptr;
}

const Exit* Room::FindExit(Direction direction) const
{
	const auto it = m_exits.find(direction);
	return it != m_exits.end() ? &it->second : nullptr;
}

const std::map<Direction, Exit>& Room::GetExits() const
{
	return m_exits;
}

const std::vector<std::shared_ptr<Item>>& Room::GetItems() const
{
	return m_items;
}

Item* Room::FindItem(const std::string& target)
{
	const auto it = std::find_if(m_items.begin(), m_items.end(),
		[&target](const std::shared_ptr<Item>& item)
		{
			return item->MatchesTarget(target);
		});

	return it != m_items.end() ? it->get() : nullptr;
}

const Item* Room::FindItem(const std::string& target) const
{
	const auto it = std::find_if(m_items.begin(), m_items.end(),
		[&target](const std::shared_ptr<Item>& item)
		{
			return item->MatchesTarget(target);
		});

	return it != m_items.end() ? it->get() : nullptr;
}

void Room::PrintInformation(std::ostream& output) const
{
	Entity::PrintInformation(output);

	if (m_items.empty())
	{
		output << "No ves ningun objeto aqui.\n";
	}
	else
	{
		output << "Ves:\n";
		for (const std::shared_ptr<Item>& item : m_items)
		{
			output << "- " << item->GetName() << '\n';
		}
	}
}

void Room::PrintExists(std::ostream& output) const
{
	if (m_exits.empty())
	{
		output << "No hay conexiones desde aqui.\n";
		return;
	}

	output << "Conexiones disponibles:\n";
	for (const auto& exit : m_exits)
	{
		output << "- " << DirectionUtils::ToText(exit.first) << '\n';
	}
}

void Room::AddExit(Direction direction, const std::string& targetRoomId, bool locked)
{
	m_exits[direction] = Exit(targetRoomId, locked);
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
