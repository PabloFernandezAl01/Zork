#include "GameWorld.h"
#include "Parser.h"
#include "WorldAliases.h"
#include "WorldBuilder.h"
#include "WorldIds.h"

#include <algorithm>
#include <cassert>
#include <ostream>
#include <utility>
#include <vector>

GameWorld::GameWorld()
	: m_player(PlayerIds::Player, "Willian Munny", "Un antiguo alguacil retirado que vuelve al pueblo y se da cuenta de que algo raro esta ocurriendo.")
{
	WorldData world = WorldBuilder::Build();

	m_player.SetCurrentRoomId(world.initialRoomId);
	m_rooms = std::move(world.rooms);
	m_items = std::move(world.items);
}

GameResult GameWorld::ExecuteCommand(const Command& command, std::ostream& output)
{
	switch (command.type)
	{
	case CommandType::Move:
		return MovePlayer(command.direction, output);
	case CommandType::Look:
		return Look(output);
	case CommandType::Examine:
		return Examine(command.firstTarget, output);
	case CommandType::Inventory:
		return ShowInventory(output);
	case CommandType::Take:
		return TakeItem(command.firstTarget, output);
	case CommandType::Drop:
		return DropItem(command.firstTarget, output);
	case CommandType::Put:
		return PutItemIntoContainer(command.firstTarget, command.secondTarget, output);
	case CommandType::Remove:
		return TakeItemFromContainer(command.firstTarget, command.secondTarget, output);
	case CommandType::Open:
		return Open(command.firstTarget, command.secondTarget, output);
	case CommandType::TurnOn:
		return TurnOnItem(command.firstTarget, command.secondTarget, output);
	case CommandType::Load:
		return LoadItem(command.firstTarget, command.secondTarget, output);
	case CommandType::Break:
		return BreakObstacle(command.firstTarget, command.secondTarget, output);
	case CommandType::Shoot:
		return ShootTarget(command.firstTarget, command.secondTarget, output);
	case CommandType::Help:
		return ShowHelp(output);
	case CommandType::Quit:
		return GameResult::Quit;
	default:
		output << "No he entendido eso.\n";
		return GameResult::Running;
	}
}

bool GameWorld::IsAValidItem(const std::string& target) const
{
	return std::find_if(m_items.begin(), m_items.end(),
		[&target](const std::pair<const std::string, std::shared_ptr<Item>>& itemEntry)
		{
			return itemEntry.second->MatchesTarget(target);
		}) != m_items.end();
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

Item* GameWorld::FindAccessibleItem(const std::string& target)
{
	Item* item = m_player.FindItem(target);
	if (item != nullptr)
	{
		return item;
	}

	Room* room = GetCurrentRoom();
	if (room == nullptr || !CanPlayerSee(*room))
	{
		return nullptr;
	}

	return room->FindItem(target);
}

const Item* GameWorld::FindAccessibleItem(const std::string& target) const
{
	const Item* item = m_player.FindItem(target);
	if (item != nullptr)
	{
		return item;
	}

	const Room* room = GetCurrentRoom();
	if (room == nullptr || !CanPlayerSee(*room))
	{
		return nullptr;
	}

	return room->FindItem(target);
}

bool GameWorld::MatchesAnyAlias(const std::string& target, const std::vector<std::string>& aliases)
{
	return std::find(aliases.begin(), aliases.end(), target) != aliases.end();
}

GameWorld::ScenarioTarget GameWorld::ResolveScenarioTarget(const Room& room, const std::string& target)
{
	if (room.GetId() == RoomIds::SheriffOffice &&
		MatchesAnyAlias(target, WorldAliases::CellDoor))
	{
		return ScenarioTarget::CellDoor;
	}

	if (room.GetId() == RoomIds::OldChurch &&
		MatchesAnyAlias(target, WorldAliases::CryptLock))
	{
		return ScenarioTarget::CryptLock;
	}

	if (room.GetId() == RoomIds::MainStreet &&
		MatchesAnyAlias(target, WorldAliases::ChurchChains))
	{
		return ScenarioTarget::ChurchChains;
	}

	if (room.GetId() == RoomIds::Crypt &&
		MatchesAnyAlias(target, WorldAliases::Sheriff))
	{
		return ScenarioTarget::Sheriff;
	}

	return ScenarioTarget::None;
}

GameResult GameWorld::Look(std::ostream& output) const
{
	const Room* room = GetCurrentRoom();
	assert(room != nullptr);

	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	if (CanPlayerSee(*room))
	{
		output << "Estas en: ";
		room->PrintInformation(output);
	}
	else
	{
		output << "Esta demasiado oscuro. Necesitas una fuente de luz encendida para poder ver.\n";
	}

	return GameResult::Running;
}

GameResult GameWorld::MovePlayer(Direction direction, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	assert(currentRoom != nullptr);

	// A missing current room means the world state is corrupted.
	if (currentRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	// Reject movement when the room has no exit in the requested direction.
	const Exit* exit = currentRoom->FindExit(direction);
	if (exit == nullptr)
	{
		output << "No hay ninguna salida hacia " << DirectionUtils::ToText(direction) << ". El mapa te puede dar alguna pista...\n";
		return GameResult::Running;
	}

	Room* nextRoom = FindRoomById(exit->targetRoomId);
	assert(nextRoom != nullptr);

	// Every configured exit must point to an existing room.
	if (nextRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	// Locked exits remain unusable until their associated puzzle is solved.
	if (exit->isLocked)
	{
		output << "El paso hacia " << nextRoom->GetName() << " esta bloqueado. Quiza necesites hacer algo para pasar...\n";
		return GameResult::Running;
	}

	// Complete the movement and describe the destination.
	m_player.SetCurrentRoomId(nextRoom->GetId());
	return Look(output);
}

GameResult GameWorld::Examine(const std::string& target, std::ostream& output) const
{
	const Room* room = GetCurrentRoom(); 
	assert(room != nullptr);

	// Examining requires the player to belong to a valid room.
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// Inventory items can generally be examined by touch even in a dark room.
	// Objects whose information must be read still require light.
	const Item* item = FindAccessibleItem(target);
	if (item == nullptr)
	{
		// Darkness hides room items, so avoid revealing whether the target is present.
		if (!CanPlayerSee(*room))
		{
			output << "Esta demasiado oscuro para ver que hay aqui.\n";
			return GameResult::Running;
		}

		// Distinguish an unknown noun from a known item that is currently elsewhere.
		if (!IsAValidItem(target))
		{
			output << "No se lo que intentas examinar.\n";
			return GameResult::Running;
		}

		output << "No encuentras lo que intentas examinar.\n";
		return GameResult::Running;
	}

	return ReadItem(*item, *room, output);
}

GameResult GameWorld::ReadItem(const Item& item, const Room& room, std::ostream& output) const
{
	// Written or visual details cannot be examined without a light source.
	if (!CanPlayerSee(room) && item.RequiresLightToExamine())
	{
		output << "Esta demasiado oscuro para leer " << item.GetName() << ".\n";
		return GameResult::Running;
	}

	// Present the item's readable information and any item-specific context.
	item.PrintInformation(output);

	// Examining the map also exposes the exits of the current room.
	if (item.GetId() == ItemIds::TornMap)
	{
		room.PrintExits(output);
	}

	return GameResult::Running;
}

GameResult GameWorld::ShowInventory(std::ostream& output) const
{
	m_player.PrintInformation(output);
	return GameResult::Running;
}

GameResult GameWorld::TakeItem(const std::string& target, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	assert(room != nullptr);

	// Taking an item requires a valid source room.
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// Room items cannot be located while the player is in darkness.
	if (!CanPlayerSee(*room))
	{
		output << "Esta demasiado oscuro para encontrar objetos en la sala.\n";
		return GameResult::Running;
	}

	// Reject nouns that do not identify any item in the game.
	if (!IsAValidItem(target))
	{
		output << "No se lo que intentas coger.\n";
		return GameResult::Running;
	}

	// A valid item may still be in another room or inside a container.
	const Item* item = room->FindItem(target);
	if (item == nullptr)
	{
		output << "No hay ningun \"" << target << "\" en " << room->GetName() << ".\n";
		return GameResult::Running;
	}

	const std::shared_ptr<Item> removedItem = room->RemoveItem(item->GetId());
	assert(removedItem != nullptr);

	// Removal must succeed after finding the item in this room.
	if (removedItem == nullptr)
	{
		return GameResult::FatalError;
	}

	const bool addedToInventory = m_player.AddItemToInventory(removedItem);
	assert(addedToInventory);

	// Restore the item if the inventory transfer unexpectedly fails.
	if (!addedToInventory)
	{
		const bool restored = room->AddItem(removedItem);
		assert(restored);
		(void)restored;
		return GameResult::FatalError;
	}

	output << "Cogido.\n";
	return GameResult::Running;
}

GameResult GameWorld::DropItem(const std::string& target, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	assert(room != nullptr);

	// Dropping an item requires a valid destination room.
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// Reject nouns that do not identify any item in the game.
	if (!IsAValidItem(target))
	{
		output << "No se lo que intentas soltar.\n";
		return GameResult::Running;
	}

	// Only items carried directly by the player can be dropped.
	const Item* item = m_player.FindItem(target);
	if (item == nullptr)
	{
		output << "No tienes ese objeto.\n";
		return GameResult::Running;
	}

	const std::shared_ptr<Item> removedItem = m_player.RemoveItemFromInventory(item->GetId());
	assert(removedItem != nullptr);

	// Removal must succeed after finding the item in the inventory.
	if (removedItem == nullptr)
	{
		return GameResult::FatalError;
	}

	const bool addedToRoom = room->AddItem(removedItem);
	assert(addedToRoom);

	// Restore the inventory if the room transfer unexpectedly fails.
	if (!addedToRoom)
	{
		const bool restored = m_player.AddItemToInventory(removedItem);
		assert(restored);
		(void)restored;
		return GameResult::FatalError;
	}

	output << "Soltado.\n";
	return GameResult::Running;
}

GameResult GameWorld::PutItemIntoContainer(const std::string& itemTarget, const std::string& containerTarget, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	assert(room != nullptr);

	// The command needs a valid room to resolve accessible containers.
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// Validate both nouns separately to provide a precise command error.
	if (!IsAValidItem(itemTarget))
	{
		output << "No se lo que intentas meter.\n";
		return GameResult::Running;
	}

	if (!IsAValidItem(containerTarget))
	{
		output << "No se donde intentas meterlo.\n";
		return GameResult::Running;
	}

	// Try to find the item to move it into the item container
	// IMPORTANT: The target item must be in player's inventory
	const Item* item = m_player.FindItem(itemTarget);
	if (item == nullptr)
	{
		output << "No tienes ese objeto.\n";
		return GameResult::Running;
	}

	// Try to find the item container (could be in player inventory or in current room)
	Item* containerItem = FindAccessibleItem(containerTarget);
	// In darkness, an unfound container may be hidden in the room.
	if (containerItem == nullptr && !CanPlayerSee(*room))
	{
		output << "Esta demasiado oscuro para encontrar eso en la sala.\n";
		return GameResult::Running;
	}

	// Didn't find it
	if (containerItem == nullptr)
	{
		output << "No tienes el objeto donde quieres meter " << item->GetName() << ".\n";
		return GameResult::Running;
	}

	// The destination must support storing other items.
	if (!containerItem->IsContainer())
	{
		output << containerItem->GetName() << " no puede contener objetos.\n";
		return GameResult::Running;
	}

	// Locked containers cannot be manipulated even if their state says open.
	if (containerItem->IsLocked())
	{
		output << containerItem->GetName() << " esta bloqueado.\n";
		return GameResult::Running;
	}

	// Items can only be inserted through an open container.
	if (!containerItem->IsOpen())
	{
		output << containerItem->GetName() << " esta cerrado.\n";
		return GameResult::Running;
	}

	// Prevent circular ownership by inserting a container into itself.
	if (item->GetId() == containerItem->GetId())
	{
		output << "No puedes meter un objeto dentro de si mismo.\n";
		return GameResult::Running;
	}

	// Nested containers are intentionally unsupported by this simplified model.
	if (item->IsContainer())
	{
		output << "No puedes meter " << item->GetName() << " en " << containerItem->GetName() << ".\n";
		return GameResult::Running;
	}

	// An active light source must stay exposed to keep illuminating the world.
	if (item->IsLightSource() && item->IsTurnedOn())
	{
		output << "No puedes guardar " << item->GetName() << " mientras esta encendido.\n";
		return GameResult::Running;
	}

	const std::shared_ptr<Item> removedItem = m_player.RemoveItemFromInventory(item->GetId());
	assert(removedItem != nullptr);

	// Removal must succeed after finding the item in the inventory.
	if (removedItem == nullptr)
	{
		return GameResult::FatalError;
	}

	const bool addedToContainer = containerItem->AddItem(removedItem);
	assert(addedToContainer);

	// Restore the inventory if the container transfer unexpectedly fails.
	if (!addedToContainer)
	{
		const bool restored = m_player.AddItemToInventory(removedItem);
		assert(restored);
		(void)restored;
		return GameResult::FatalError;
	}

	output << "Metido.\n";
	return GameResult::Running;
}

GameResult GameWorld::TakeItemFromContainer(const std::string& itemTarget, const std::string& containerTarget, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	assert(room != nullptr);

	// The command needs a valid room to resolve accessible containers.
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// Validate the source and contained item nouns independently.
	if (!IsAValidItem(containerTarget))
	{
		output << "No se de donde intentas sacar eso.\n";
		return GameResult::Running;
	}

	if (!IsAValidItem(itemTarget))
	{
		output << "No se lo que intentas sacar.\n";
		return GameResult::Running;
	}

	// Try to find the container item (could be in player inventory or in current room)
	Item* container = FindAccessibleItem(containerTarget);

	// In darkness, an unfound container may be hidden in the room.
	if (container == nullptr && !CanPlayerSee(*room))
	{
		output << "Esta demasiado oscuro para encontrar eso en la sala.\n";
		return GameResult::Running;
	}

	if (container == nullptr)
	{
		output << "No encuentras ese objeto aqui.\n";
		return GameResult::Running;
	}

	// The source must support storing other items.
	if (!container->IsContainer())
	{
		output << "No se puede guardar nada en " << container->GetName() << ".\n";
		return GameResult::Running;
	}

	// Locked containers cannot expose their contents.
	if (container->IsLocked())
	{
		output << container->GetName() << " esta bloqueado.\n";
		return GameResult::Running;
	}

	// Closed containers must be opened before removing their contents.
	if (!container->IsOpen())
	{
		output << container->GetName() << " esta cerrado.\n";
		return GameResult::Running;
	}

	// Try to find the target item in the container item
	Item* item = container->FindItem(itemTarget);
	if (item == nullptr)
	{
		output << "Este objeto no esta en " << container->GetName() << ".\n";
		return GameResult::Running;
	}

	const std::shared_ptr<Item> removedItem = container->RemoveItem(item->GetId());
	assert(removedItem != nullptr);

	// Removal must succeed after finding the item in the container.
	if (removedItem == nullptr)
	{
		return GameResult::FatalError;
	}

	const bool addedToInventory = m_player.AddItemToInventory(removedItem);
	assert(addedToInventory);

	// Restore the container if the inventory transfer unexpectedly fails.
	if (!addedToInventory)
	{
		const bool restored = container->AddItem(removedItem);
		assert(restored);
		(void)restored;
		return GameResult::FatalError;
	}

	output << "Sacado.\n";
	return GameResult::Running;
}

GameResult GameWorld::Open(const std::string& target, const std::string& toolTarget, std::ostream& output)
{
	const Room* room = GetCurrentRoom();
	assert(room != nullptr);

	// Opening a target depends on the scenario associated with the current room.
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	const ScenarioTarget scenarioTarget = ResolveScenarioTarget(*room, target);
	// World obstacles use exit-unlocking rules; regular items use container rules.
	if (scenarioTarget == ScenarioTarget::CellDoor || scenarioTarget == ScenarioTarget::CryptLock)
	{
		return Unlock(scenarioTarget, toolTarget, output);
	}

	return OpenItem(target, toolTarget, output);
}

GameResult GameWorld::OpenItem(const std::string& target, const std::string& toolTarget, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	assert(room != nullptr);

	// The command needs a valid room to resolve accessible items.
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// Check if the item is in the room or in the player's inventory
	Item* item = FindAccessibleItem(target);
	// In darkness, an unfound item may simply be hidden in the room.
	if (item == nullptr && !CanPlayerSee(*room))
	{
		output << "Esta demasiado oscuro para encontrar lo que intentas abrir.\n";
		return GameResult::Running;
	}

	if (item == nullptr)
	{
		output << "No encuentras lo que intentas abrir.\n";
		return GameResult::Running;
	}

	// Only containers have an open/closed state.
	if (!item->IsContainer())
	{
		output << item->GetName() << " no se puede abrir.\n";
		return GameResult::Running;
	}

	// Opening an already open container has no further effect.
	if (item->IsOpen())
	{
		output << item->GetName() << " ya esta abierto.\n";
		return GameResult::Running;
	}

	// Locked containers require their specific unlocking rule.
	if (item->IsLocked())
	{
		// The safe box is the only lockable item container in this scenario.
		if (item->GetId() != ItemIds::SafeBox)
		{
			output << "No sabes como abrir " << item->GetName() << ".\n";
			return GameResult::Running;
		}

		const Item* key = toolTarget.empty() ? m_player.FindItemById(ItemIds::SmallKey) : m_player.FindItem(toolTarget);

		// Accept the implicit key or verify the explicitly named tool.
		if (key == nullptr || key->GetId() != ItemIds::SmallKey)
		{
			output << "Necesitas la llave adecuada para abrir " << item->GetName() << ".\n";
			return GameResult::Running;
		}
	}
	else if (!toolTarget.empty())
	{
		// An unlocked container does not consume or require a supplied tool.
		if (!IsAValidItem(toolTarget))
		{
			output << "No encuentras esa herramienta.\n";
			return GameResult::Running;
		}

		const Item* tool = m_player.FindItem(toolTarget);
		if (tool == nullptr)
		{
			output << "No tienes esa herramienta.\n";
			return GameResult::Running;
		}

		output << "No necesitas " << tool->GetName() << " para abrir " << item->GetName() << ".\n";
		return GameResult::Running;
	}

	item->SetContainerState(ContainerState::Open);
	output << "Abierto.\n";
	return GameResult::Running;
}

GameResult GameWorld::Unlock(ScenarioTarget target, const std::string& toolTarget, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	assert(currentRoom != nullptr);

	// Scenario locks must always be resolved from a valid current room.
	if (currentRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	assert(target == ScenarioTarget::CellDoor || target == ScenarioTarget::CryptLock);

	// ------------- The user is trying to unlock "Celda trasera" -----------------

	// The cell door is opened with the small key from the sheriff's office.
	if (target == ScenarioTarget::CellDoor)
	{
		assert(currentRoom->GetId() == RoomIds::SheriffOffice);

		Exit* cellExit = currentRoom->FindExit(Direction::East);
		assert(cellExit != nullptr);

		// The scenario assumes the cell is connected through the east exit.
		if (cellExit == nullptr)
		{
			return GameResult::FatalError;
		}

		// Repeating the command after solving the lock is harmless.
		if (!cellExit->isLocked)
		{
			output << "La puerta de la celda ya esta abierta.\n";
			return GameResult::Running;
		}

		// If we get here it means we are in "Oficina del Sheriff" and "Celda trasera" it's locked
		const Item* smallKey = toolTarget.empty() ? m_player.FindItemById(ItemIds::SmallKey) : m_player.FindItem(toolTarget);

		// Accept the implicit key or verify the explicitly named tool.
		if (smallKey == nullptr || smallKey->GetId() != ItemIds::SmallKey)
		{
			output << "Necesitas la llave adecuada para abrir la puerta de la celda.\n";
			return GameResult::Running;
		}

		cellExit->isLocked = false;
		output << "Puerta abierta.\n";
		return GameResult::Running;
	}

	// ------------- The user is trying to unlock "Cripta al norte de la iglesia" -----------------

	assert(currentRoom->GetId() == RoomIds::OldChurch);

	Exit* cryptExit = currentRoom->FindExit(Direction::North);
	assert(cryptExit != nullptr);

	// The scenario assumes the crypt is connected through the north exit.
	if (cryptExit == nullptr)
	{
		return GameResult::FatalError;
	}

	// Repeating the command after solving the lock is harmless.
	if (!cryptExit->isLocked)
	{
		output << "La cerradura ya esta abierta.\n";
		return GameResult::Running;
	}

	// If we get here it means we are in "Iglesia vieja" and "Cripta al norte de la iglesia" it's locked
	const Item* silverCross = toolTarget.empty() ? m_player.FindItemById(ItemIds::SilverCross) : m_player.FindItem(toolTarget);

	// The cross-shaped lock can only be opened with the silver cross.
	if (silverCross == nullptr || silverCross->GetId() != ItemIds::SilverCross)
	{
		output << "La cerradura tiene forma de cruz. Necesitas algo que encaje en ella.\n";
		return GameResult::Running;
	}

	cryptExit->isLocked = false;
	output << "El acceso a la cripta queda abierto.\n";
	return GameResult::Running;
}

GameResult GameWorld::TurnOnItem(const std::string& target, const std::string& toolTarget, std::ostream& output)
{
	const Room* room = GetCurrentRoom();
	assert(room != nullptr);

	// Lighting an item requires a valid room for the visibility update.
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// To turn on a light source item, it must be in player's inventory
	Item* item = m_player.FindItem(target);
	if (item == nullptr)
	{
		output << "Debes tener el objeto para poder encenderlo.\n";
		return GameResult::Running;
	}

	// Only items configured as light sources can be turned on.
	if (!item->IsLightSource())
	{
		output << item->GetName() << " no se puede encender.\n";
		return GameResult::Running;
	}

	// Avoid applying the action twice to an active light source.
	if (item->IsTurnedOn())
	{
		output << item->GetName() << " ya esta encendido.\n";
		return GameResult::Running;
	}

	// Allow both "encender farol" and the explicit "encender farol con cerillas".
	const Item* matches = toolTarget.empty() ? m_player.FindItemById(ItemIds::Matches) : m_player.FindItem(toolTarget);
	// Matches are required whether inferred or explicitly supplied.
	if (matches == nullptr || matches->GetId() != ItemIds::Matches)
	{
		output << "Necesitas algo con lo que encender " << item->GetName() << ".\n";
		return GameResult::Running;
	}

	item->SetLightState(LightState::On);
	output << "Enciendes " << item->GetName() << ".\n";

	// Immediately reveal the room when the newly lit item makes it visible.
	if (room->IsDark())
	{
		return Look(output);
	}

	return GameResult::Running;
}

GameResult GameWorld::LoadItem(const std::string& target, const std::string& ammunitionTarget, std::ostream& output)
{
	const Room* room = GetCurrentRoom();
	assert(room != nullptr);

	// Loading is only valid while the player belongs to a valid room.
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// To charge the item "pistola", it must be in player's inventory
	Item* item = m_player.FindItem(target);
	if (item == nullptr)
	{
		output << "Debes tener el objeto para poder cargarlo.\n";
		return GameResult::Running;
	}

	// Only weapons support a loaded/unloaded state.
	if (!item->IsWeapon())
	{
		output << item->GetName() << " no se puede cargar.\n";
		return GameResult::Running;
	}

	// Do not consume more ammunition when the weapon is already loaded.
	if (item->IsLoaded())
	{
		output << item->GetName() << " ya esta cargado.\n";
		return GameResult::Running;
	}

	// The item "Municion" must also be in player's inventory
	// This is just to allow the command to not mention "municion" so the input could be "Cargar revolver" and also "Cargar revolver con municion"
	const Item* ammunition = ammunitionTarget.empty() ? m_player.FindItemById(ItemIds::Ammunition) : m_player.FindItem(ammunitionTarget);

	// Accept implicit ammunition or verify the explicitly named inventory item.
	if (ammunition == nullptr || ammunition->GetId() != ItemIds::Ammunition)
	{
		output << "Necesitas la municion adecuada para cargar " << item->GetName() << ".\n";
		return GameResult::Running;
	}

	const std::shared_ptr<Item> removedAmmunition = m_player.RemoveItemFromInventory(ammunition->GetId());
	assert(removedAmmunition != nullptr);

	// Consuming ammunition must succeed after finding it in the inventory.
	if (removedAmmunition == nullptr)
	{
		return GameResult::FatalError;
	}

	item->SetLoadState(LoadState::Loaded);
	output << "Revolver cargado.\n";
	return GameResult::Running;
}

GameResult GameWorld::BreakObstacle(const std::string& target, const std::string& toolTarget, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	assert(currentRoom != nullptr);

	// Breaking a scenario obstacle requires a valid current room.
	if (currentRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	// This command only solves the chained church entrance.
	if (ResolveScenarioTarget(*currentRoom, target) != ScenarioTarget::ChurchChains)
	{
		output << "No sabes como romper eso.\n";
		return GameResult::Running;
	}

	Exit* churchExit = currentRoom->FindExit(Direction::North);
	assert(churchExit != nullptr);

	// The scenario assumes the church is connected through the north exit.
	if (churchExit == nullptr)
	{
		return GameResult::FatalError;
	}

	// Repeating the command after cutting the chains is harmless.
	if (!churchExit->isLocked)
	{
		output << "Las cadenas ya estan rotas.\n";
		return GameResult::Running;
	}

	const Item* boltCutter = toolTarget.empty() ? m_player.FindItemById(ItemIds::BoltCutter) : m_player.FindItem(toolTarget);

	// The bolt cutter is the only tool capable of removing the chains.
	if (boltCutter == nullptr || boltCutter->GetId() != ItemIds::BoltCutter)
	{
		output << "Las cadenas son demasiado gruesas para romperlas con las manos.\n";
		return GameResult::Running;
	}

	churchExit->isLocked = false;
	output << "Cortadas. La entrada a la iglesia queda libre.\n";
	return GameResult::Running;
}

GameResult GameWorld::ShootTarget(const std::string& target, const std::string& weaponTarget, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	assert(currentRoom != nullptr);

	// Shooting a scenario target requires a valid current room.
	if (currentRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	// The sheriff in the crypt is the only supported shooting target.
	if (ResolveScenarioTarget(*currentRoom, target) != ScenarioTarget::Sheriff)
	{
		output << "No sabes a que intentas disparar.\n";
		return GameResult::Running;
	}

	// The player cannot aim at the sheriff without seeing him.
	if (!CanPlayerSee(*currentRoom))
	{
		output << "No ves nada.\n";
		return GameResult::Running;
	}

	Item* weapon = weaponTarget.empty() ? m_player.FindItemById(ItemIds::Revolver) : m_player.FindItem(weaponTarget);

	// Accept the implicit revolver or verify the explicitly named weapon.
	if (weapon == nullptr || weapon->GetId() != ItemIds::Revolver)
	{
		output << "Necesitas el Revolver para disparar al sheriff.\n";
		return GameResult::Running;
	}

	// Pulling the trigger unloaded gives the sheriff time to defeat the player.
	if (!weapon->IsLoaded())
	{
		output << "Aprietas el gatillo, pero el Revolver esta descargado. El Sheriff te dispara y luego remata la tarea con Ned Munny\n";
		return GameResult::Defeat;
	}

	weapon->SetLoadState(LoadState::Unloaded);
	output << "Disparas al sheriff antes de que pueda reaccionar. Ned Munny queda libre.\n";
	return GameResult::Victory;
}

GameResult GameWorld::ShowHelp(std::ostream& output) const
{
	output << "Comandos disponibles:\n";
	output << "- Movimiento: norte/n, sur/s, este/e, oeste/o\n";
	output << "- Observacion: mirar/m, examinar/x [objeto]\n";
	output << "- Inventario: inventario/i, coger [objeto], soltar/tirar [objeto]\n";
	output << "- Contenedores: meter [objeto] en [contenedor], sacar [objeto] de [contenedor]\n";
	output << "- Acciones: abrir [objeto], encender [objeto], cargar [objeto]\n";
	output << "- Puzles: romper [objeto] con [herramienta], disparar [objetivo]\n";
	output << "- Sistema: ayuda, terminar\n";
	return GameResult::Running;
}

Room* GameWorld::GetCurrentRoom()
{
	return FindRoomById(m_player.GetCurrentRoomId());
}

const Room* GameWorld::GetCurrentRoom() const
{
	return FindRoomById(m_player.GetCurrentRoomId());
}

bool GameWorld::CanPlayerSee(const Room& room) const
{
	if (!room.IsDark())
	{
		return true;
	}

	return m_player.HasTurnedOnLightSource() || room.HasTurnedOnLightSource();
}
