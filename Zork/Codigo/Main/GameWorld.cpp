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
	if (currentRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	// Checks if there is an exit in that direction
	const Exit* exit = currentRoom->FindExit(direction);
	if (exit == nullptr)
	{
		output << "No hay ninguna salida hacia " << DirectionUtils::ToText(direction) << ". El mapa te puede dar alguna pista...\n";
		return GameResult::Running;
	}

	Room* nextRoom = FindRoomById(exit->targetRoomId);
	assert(nextRoom != nullptr);
	if (nextRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	if (exit->isLocked)
	{
		output << "El paso hacia " << nextRoom->GetName() << " esta bloqueado. Quiza necesites hacer algo para pasar...\n";
		return GameResult::Running;
	}

	// Actually changes player room
	m_player.SetCurrentRoomId(nextRoom->GetId());
	return Look(output);
}

GameResult GameWorld::Examine(const std::string& target, std::ostream& output) const
{
	const Room* room = GetCurrentRoom(); 
	assert(room != nullptr);
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// Inventory items can generally be examined by touch even in a dark room.
	// Objects whose information must be read still require light.
	const Item* item = FindAccessibleItem(target);
	if (item == nullptr)
	{
		if (!CanPlayerSee(*room))
		{
			output << "Esta demasiado oscuro para ver que hay aqui.\n";
			return GameResult::Running;
		}

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
	if (!CanPlayerSee(room) && item.RequiresLightToExamine())
	{
		output << "Esta demasiado oscuro para leer " << item.GetName() << ".\n";
		return GameResult::Running;
	}

	// Present the item's readable information and any item-specific context.
	item.PrintInformation(output);

	if (item.GetId() == ItemIds::TornMap)
	{
		room.PrintExists(output);
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
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	if (!CanPlayerSee(*room))
	{
		output << "Esta demasiado oscuro para encontrar objetos en la sala.\n";
		return GameResult::Running;
	}

	if (!IsAValidItem(target))
	{
		output << "No se lo que intentas coger.\n";
		return GameResult::Running;
	}

	const Item* item = room->FindItem(target);
	if (item == nullptr)
	{
		output << "No hay ningun \"" << target << "\" en " << room->GetName() << ".\n";
		return GameResult::Running;
	}

	const std::shared_ptr<Item> removedItem = room->RemoveItem(item->GetId());
	assert(removedItem != nullptr);
	if (removedItem == nullptr)
	{
		return GameResult::FatalError;
	}

	const bool addedToInventory = m_player.AddItemToInventory(removedItem);
	assert(addedToInventory);
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
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	if (!IsAValidItem(target))
	{
		output << "No se lo que intentas soltar.\n";
		return GameResult::Running;
	}

	const Item* item = m_player.FindItem(target);
	if (item == nullptr)
	{
		output << "No tienes ese objeto.\n";
		return GameResult::Running;
	}

	const std::shared_ptr<Item> removedItem = m_player.RemoveItemFromInventory(item->GetId());
	assert(removedItem != nullptr);
	if (removedItem == nullptr)
	{
		return GameResult::FatalError;
	}

	const bool addedToRoom = room->AddItem(removedItem);
	assert(addedToRoom);
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
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

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

	// If found, check if it's really a container item
	if (!containerItem->IsContainer())
	{
		output << containerItem->GetName() << " no puede contener objetos.\n";
		return GameResult::Running;
	}

	if (containerItem->IsLocked())
	{
		output << containerItem->GetName() << " esta bloqueado.\n";
		return GameResult::Running;
	}

	if (!containerItem->IsOpen())
	{
		output << containerItem->GetName() << " esta cerrado.\n";
		return GameResult::Running;
	}

	// If found, check if the item it's not the same as the container item ("Meter botella en botella")
	if (item->GetId() == containerItem->GetId())
	{
		output << "No puedes meter un objeto dentro de si mismo.\n";
		return GameResult::Running;
	}

	if (item->IsContainer())
	{
		output << "No puedes meter " << item->GetName() << " en " << containerItem->GetName() << ".\n";
		return GameResult::Running;
	}

	const std::shared_ptr<Item> removedItem = m_player.RemoveItemFromInventory(item->GetId());
	assert(removedItem != nullptr);
	if (removedItem == nullptr)
	{
		return GameResult::FatalError;
	}

	const bool addedToContainer = containerItem->AddItem(removedItem);
	assert(addedToContainer);
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
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

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

	// Check first if it's actually a container item
	if (!container->IsContainer())
	{
		output << "No se puede guardar nada en " << container->GetName() << ".\n";
		return GameResult::Running;
	}

	if (container->IsLocked())
	{
		output << container->GetName() << " esta bloqueado.\n";
		return GameResult::Running;
	}

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
	if (removedItem == nullptr)
	{
		return GameResult::FatalError;
	}

	const bool addedToInventory = m_player.AddItemToInventory(removedItem);
	assert(addedToInventory);
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
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	const ScenarioTarget scenarioTarget = ResolveScenarioTarget(*room, target);
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
	if (room == nullptr)
	{
		return GameResult::FatalError;
	}

	// Check if the item is in the room or in the player's inventory
	Item* item = FindAccessibleItem(target);
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

	if (!item->IsContainer())
	{
		output << item->GetName() << " no se puede abrir.\n";
		return GameResult::Running;
	}

	if (item->IsOpen())
	{
		output << item->GetName() << " ya esta abierto.\n";
		return GameResult::Running;
	}

	if (item->IsLocked())
	{
		if (item->GetId() != ItemIds::SafeBox)
		{
			output << "No sabes como abrir " << item->GetName() << ".\n";
			return GameResult::Running;
		}

		const Item* key = toolTarget.empty() ? m_player.FindItemById(ItemIds::SmallKey) : m_player.FindItem(toolTarget);

		if (key == nullptr || key->GetId() != ItemIds::SmallKey)
		{
			output << "Necesitas la llave adecuada para abrir " << item->GetName() << ".\n";
			return GameResult::Running;
		}
	}
	else if (!toolTarget.empty())
	{
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
	if (currentRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	assert(target == ScenarioTarget::CellDoor || target == ScenarioTarget::CryptLock);

	// ------------- The user is trying to unlock "Celda trasera" -----------------

	if (target == ScenarioTarget::CellDoor)
	{
		assert(currentRoom->GetId() == RoomIds::SheriffOffice);

		Exit* cellExit = currentRoom->FindExit(Direction::East);
		assert(cellExit != nullptr);
		if (cellExit == nullptr)
		{
			return GameResult::FatalError;
		}

		if (!cellExit->isLocked)
		{
			output << "La puerta de la celda ya esta abierta.\n";
			return GameResult::Running;
		}

		// If we get here it means we are in "Oficina del Sheriff" and "Celda trasera" it's locked
		const Item* smallKey = toolTarget.empty() ? m_player.FindItemById(ItemIds::SmallKey) : m_player.FindItem(toolTarget);

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
	if (cryptExit == nullptr)
	{
		return GameResult::FatalError;
	}

	if (!cryptExit->isLocked)
	{
		output << "La cerradura ya esta abierta.\n";
		return GameResult::Running;
	}

	// If we get here it means we are in "Iglesia vieja" and "Cripta al norte de la iglesia" it's locked
	const Item* silverCross = toolTarget.empty() ? m_player.FindItemById(ItemIds::SilverCross) : m_player.FindItem(toolTarget);

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

	if (!item->IsLightSource())
	{
		output << item->GetName() << " no se puede encender.\n";
		return GameResult::Running;
	}

	if (item->IsTurnedOn())
	{
		output << item->GetName() << " ya esta encendido.\n";
		return GameResult::Running;
	}

	// Allow both "encender farol" and the explicit "encender farol con cerillas".
	const Item* matches = toolTarget.empty() ? m_player.FindItemById(ItemIds::Matches) : m_player.FindItem(toolTarget);
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

	if (!item->IsWeapon())
	{
		output << item->GetName() << " no se puede cargar.\n";
		return GameResult::Running;
	}

	if (item->IsLoaded())
	{
		output << item->GetName() << " ya esta cargado.\n";
		return GameResult::Running;
	}

	// The item "Municion" must also be in player's inventory
	// This is just to allow the command to not mention "municion" so the input could be "Cargar revolver" and also "Cargar revolver con municion"
	const Item* ammunition = ammunitionTarget.empty() ? m_player.FindItemById(ItemIds::Ammunition) : m_player.FindItem(ammunitionTarget);

	if (ammunition == nullptr || ammunition->GetId() != ItemIds::Ammunition)
	{
		output << "Necesitas la municion adecuada para cargar " << item->GetName() << ".\n";
		return GameResult::Running;
	}

	const std::shared_ptr<Item> removedAmmunition = m_player.RemoveItemFromInventory(ammunition->GetId());
	assert(removedAmmunition != nullptr);
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
	if (currentRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	if (ResolveScenarioTarget(*currentRoom, target) != ScenarioTarget::ChurchChains)
	{
		output << "No sabes como romper eso.\n";
		return GameResult::Running;
	}

	Exit* churchExit = currentRoom->FindExit(Direction::North);
	assert(churchExit != nullptr);
	if (churchExit == nullptr)
	{
		return GameResult::FatalError;
	}

	if (!churchExit->isLocked)
	{
		output << "Las cadenas ya estan rotas.\n";
		return GameResult::Running;
	}

	const Item* boltCutter = toolTarget.empty() ? m_player.FindItemById(ItemIds::BoltCutter) : m_player.FindItem(toolTarget);

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
	if (currentRoom == nullptr)
	{
		return GameResult::FatalError;
	}

	if (ResolveScenarioTarget(*currentRoom, target) != ScenarioTarget::Sheriff)
	{
		output << "No sabes a que intentas disparar.\n";
		return GameResult::Running;
	}

	if (!CanPlayerSee(*currentRoom))
	{
		output << "No ves nada.\n";
		return GameResult::Running;
	}

	Item* weapon = weaponTarget.empty() ? m_player.FindItemById(ItemIds::Revolver) : m_player.FindItem(weaponTarget);

	if (weapon == nullptr || weapon->GetId() != ItemIds::Revolver)
	{
		output << "Necesitas el Revolver para disparar al sheriff.\n";
		return GameResult::Running;
	}

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
	output << "  Ejemplos: abrir puerta de la celda con llave\n";
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

	// In WestZork a light source only illuminates the room while the player is
	// carrying it. A lit lantern left on the ground does not provide visibility.
	return m_player.HasTurnedOnLightSource();
}
