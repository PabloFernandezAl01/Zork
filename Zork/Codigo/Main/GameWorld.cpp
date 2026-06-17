#include "GameWorld.h"
#include "Parser.h"

#include <algorithm>
#include <iostream>
#include <vector>

Item* GameWorld::FindItemByTarget(const std::vector<std::shared_ptr<Item>>& items, const std::string& target)
{
	const auto it = std::find_if(items.begin(), items.end(),
		[&target](const std::shared_ptr<Item>& item)
		{
			return item->MatchesTarget(target);
		});

	return it != items.end() ? it->get() : nullptr;
}

GameWorld::GameWorld()
	: m_player("player", "James", "Un antiguo alguacil que vuelve al pueblo en busca de su hermano Elias.")
{
}

void GameWorld::ExecuteCommand(const Command& command, bool& isRunning, std::ostream& output)
{
	switch (command.type)
	{
	case CommandType::Move:
		MovePlayer(command.direction, isRunning, output);
		break;
	case CommandType::Look:
		Look(output, isRunning);
		break;
	case CommandType::Examine:
		Examine(command.firstTarget, isRunning, output);
		break;
	case CommandType::Inventory:
		ShowInventory(output);
		break;
	case CommandType::Take:
		TakeItem(command.firstTarget, isRunning, output);
		break;
	case CommandType::Drop:
		DropItem(command.firstTarget, isRunning, output);
		break;
	case CommandType::Put:
		PutItemInContainer(command.firstTarget, isRunning, command.secondTarget, output);
		break;
	case CommandType::Remove:
		TakeItemFromContainer(command.firstTarget, isRunning, command.secondTarget, output);
		break;
	case CommandType::Help:
		ShowHelp(output);
		break;
	case CommandType::Quit:
		isRunning = false;
		break;
	default:
		output << "No entiendo ese comando.\n";
		break;
	}
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

void GameWorld::Look(std::ostream& output, bool& isRunning) const
{
	const Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen. 
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	if (!room->IsDark())
	{
		room->PrintInformation(output);
	}
	else
	{
		output << "No se ve nada.\n";
	}
}

void GameWorld::MovePlayer(Direction direction, bool& isRunning, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	if (currentRoom == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen. 
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	// Checks if there is a Room in that direction
	std::string nextRoomId;
	if (!currentRoom->TryGetExit(direction, nextRoomId))
	{
		output << "No puedes ir hacia " << DirectionToText(direction) << ". El mapa te puede dar alguna pista...\n";
		return;
	}

	Room* nextRoom = FindRoomById(nextRoomId);
	if (nextRoom == nullptr) // <--------- Just a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "A pointer to a Room must never be nullptr.\n";
		isRunning = false;
		return;
	}

	// Room-specific gameplay logic
	if (nextRoom->IsLocked())
	{
		output << "El paso hacia " << nextRoom->GetName() << " esta bloqueado.\n";
		return;
	}

	// Actually changes player room
	m_player.SetCurrentRoomId(nextRoom->GetId());
	Look(output, isRunning);
}

void GameWorld::Examine(const std::string& target, bool& isRunning, std::ostream& output) const
{
	const Room* room = GetCurrentRoom(); 
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	// In WestZork there is item uniqueness so an item either is in the room, in the player inventory or it does not exist in the world. (Not sure if this also happens in Zork)
	const Item* item = FindItemByTarget(room->GetItems(), target);  // <------- Search first in the current room
	if (item == nullptr)
	{
		item = FindItemByTarget(m_player.GetInventory(), target); // <------- And then in the player inventory
	}

	if (item == nullptr)
	{
		output << "No encuentras lo que intentas examinar.\n";
		return;
	}

	// If the item was found, show it's information (name, descripction and items inside, if any)
	item->PrintInformation(output);
}

void GameWorld::ShowInventory(std::ostream& output) const
{
	m_player.PrintInformation(output);
}

void GameWorld::TakeItem(const std::string& target, bool& isRunning, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	const Item* item = FindItemByTarget(room->GetItems(), target);
	if (item == nullptr)
	{
		output << "No hay ningun \"" << target << "\" en " << room->GetName() << ".\n";
		return;
	}

	m_player.AddItemToInventory(room->RemoveItem(item->GetId()));
	output << "Has cogido " << item->GetName() << ".\n";
}

void GameWorld::DropItem(const std::string& target, bool& isRunning, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	const Item* item = FindItemByTarget(m_player.GetInventory(), target);
	if (item == nullptr)
	{
		output << "No tienes ese objeto.\n";
		return;
	}

	room->AddItem(m_player.RemoveItemFromInventory(item->GetId()));
	output << "Sueltas " << item->GetName() << ".\n";
}

void GameWorld::PutItemInContainer(const std::string& itemTarget, bool& isRunning, const std::string& containerTarget, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	// Try to find the item to move into the item container
	const Item* item = FindItemByTarget(m_player.GetInventory(), itemTarget);
	if (item == nullptr)
	{
		output << "No tienes ese objeto.\n";
		return;
	}

	// Try to find the item container (could be in player inventory or in current room)
	Item* containerItem = FindItemByTarget(m_player.GetInventory(), containerTarget);
	
	if (containerItem == nullptr && room != nullptr)
	{
		containerItem = FindItemByTarget(room->GetItems(), containerTarget);
	}

	// Didn't find it
	if (containerItem == nullptr)
	{
		output << "No he encontrado " << containerTarget << ".\n";
		return;
	}

	// If found, check if it's really a container item
	if (!containerItem->IsContainer())
	{
		output << containerItem->GetName() << " no puede contener objetos.\n";
		return;
	}

	// If found, check if the item it's not the same as the container item ("Meter botella en botella")
	if (item->GetId() == containerItem->GetId())
	{
		output << "No puedes meter un objeto dentro de si mismo.\n";
		return;
	}

	containerItem->AddItem(m_player.RemoveItemFromInventory(item->GetId()));
	output << "Metes " << item->GetName() << " en " << containerItem->GetName() << ".\n";
}

void GameWorld::TakeItemFromContainer(const std::string& itemTarget, bool& isRunning, const std::string& containerTarget, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	// Try to find the container item (could be in player inventory or in current room)
	Item* container = FindItemByTarget(m_player.GetInventory(), containerTarget);
	if (container == nullptr && room != nullptr)
	{
		container = FindItemByTarget(room->GetItems(), containerTarget);
	}

	// Didn't find it
	if (container == nullptr)
	{
		output << "No he encontrado " << containerTarget << ".\n";
		return;
	}

	// Check first if it's actually an container item
	if (!container->IsContainer())
	{
		output << container->GetName() << " no contiene objetos.\n";
		return;
	}

	// Find the item we want to take from the container
	const Item* item = FindItemByTarget(container->GetContainedItems(), itemTarget);
	if (item == nullptr)
	{
		output << "No encuentras ese objeto dentro de " << container->GetName() << ".\n";
		return;
	}

	m_player.AddItemToInventory(container->RemoveItem(item->GetId()));
	output << "Sacas " << item->GetName() << " de " << container->GetName() << ".\n";
}

void GameWorld::ShowHelp(std::ostream& output) const
{
	output << "Comandos disponibles:\n";
	output << "- Movimiento: norte/n, sur/s, este/e, oeste/o, abajo, entrar, salir\n";
	output << "- Observacion: mirar/m, examinar/x [objeto]\n";
	output << "- Inventario: inventario/i, coger [objeto], soltar [objeto]\n";
	output << "- Contenedores: meter [objeto] en [contenedor], sacar [objeto] de [contenedor]\n";
	output << "- Sistema: ayuda, terminar\n";
}

Room* GameWorld::GetCurrentRoom()
{
	return FindRoomById(m_player.GetCurrentRoomId());
}

const Room* GameWorld::GetCurrentRoom() const
{
	return FindRoomById(m_player.GetCurrentRoomId());
}
