#include "GameWorld.h"
#include "Parser.h"
#include "WorldIds.h"

#include <algorithm>
#include <iostream>

bool GameWorld::TargetsCellDoor(const std::string& target)
{
	return target == WorldTargets::Cell || target == WorldTargets::CellDoor;
}

bool GameWorld::TargetsCryptLock(const std::string& target)
{
	return target == WorldTargets::Lock || target == WorldTargets::CryptLock;
}

bool GameWorld::TargetsChurchChains(const std::string& target)
{
	return target == WorldTargets::Chains || target == WorldTargets::Chain;
}

GameWorld::GameWorld()
	: m_player(PlayerIds::Player, "James", "Un antiguo alguacil que vuelve al pueblo en busca de su hermano Elias.")
{
	InitializeWorld();
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
		return TurnOnItem(command.firstTarget, output);
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

void GameWorld::AddRoom(const std::shared_ptr<Room>& room)
{
	m_rooms[room->GetId()] = room;
}

void GameWorld::AddItem(const std::shared_ptr<Item>& item)
{
	m_items[item->GetId()] = item;
}

GameResult GameWorld::Look(std::ostream& output) const
{
	const Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen. 
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		return GameResult::FatalError;
	}

	if (CanPlayerSee(*room))
	{
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
	if (currentRoom == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen. 
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		return GameResult::FatalError;
	}

	// Checks if there is an exit in that direction
	const Exit* exit = currentRoom->FindExit(direction);
	if (exit == nullptr)
	{
		output << "No puedes ir hacia " << DirectionUtils::ToText(direction) << ". El mapa te puede dar alguna pista...\n";
		return GameResult::Running;
	}

	Room* nextRoom = FindRoomById(exit->targetRoomId);
	if (nextRoom == nullptr) // <--------- Just a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "The pointer to nextRoom must never be nullptr.\n";
		return GameResult::FatalError;
	}

	if (exit->isLocked)
	{
		output << "El paso hacia " << nextRoom->GetName() << " esta bloqueado.\n";
		return GameResult::Running;
	}

	// Actually changes player room
	m_player.SetCurrentRoomId(nextRoom->GetId());
	return Look(output);
}

GameResult GameWorld::Examine(const std::string& target, std::ostream& output) const
{
	const Room* room = GetCurrentRoom(); 
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		return GameResult::FatalError;
	}

	// Inventory items can generally be examined by touch even in a dark room.
	// Objects whose information must be read still require light.
	const Item* item = FindAccessibleItem(target);
	if (item == nullptr)
	{
		if (!CanPlayerSee(*room))
		{
			output << "Esta demasiado oscuro para examinar nada de la sala.\n";
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
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
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

	m_player.AddItemToInventory(room->RemoveItem(item->GetId()));
	output << "Has cogido " << item->GetName() << ".\n";
	return GameResult::Running;
}

GameResult GameWorld::DropItem(const std::string& target, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
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

	room->AddItem(m_player.RemoveItemFromInventory(item->GetId()));
	output << "Sueltas " << item->GetName() << ".\n";
	return GameResult::Running;
}

GameResult GameWorld::PutItemIntoContainer(const std::string& itemTarget, const std::string& containerTarget, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
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
		output << "Esta demasiado oscuro para encontrar ese contenedor en la sala.\n";
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

	containerItem->AddItem(m_player.RemoveItemFromInventory(item->GetId()));
	output << "Metes " << item->GetName() << " en " << containerItem->GetName() << ".\n";
	return GameResult::Running;
}

GameResult GameWorld::TakeItemFromContainer(const std::string& itemTarget, const std::string& containerTarget, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
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
		output << "Esta demasiado oscuro para encontrar ese contenedor en la sala.\n";
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

	m_player.AddItemToInventory(container->RemoveItem(item->GetId()));
	output << "Sacas " << item->GetName() << " de " << container->GetName() << ".\n";
	return GameResult::Running;
}

GameResult GameWorld::Open(const std::string& target, const std::string& toolTarget, std::ostream& output)
{
	if (TargetsCellDoor(target) || TargetsCryptLock(target))
	{
		// This handles commands to unlock items or rooms
		return Unlock(target, toolTarget, output);
	}

	return OpenItem(target, toolTarget, output);
}

GameResult GameWorld::OpenItem(const std::string& target, const std::string& toolTarget, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
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

	item->SetContainerState(ContainerState::Open);
	output << "Abres " << item->GetName() << ".\n";
	return GameResult::Running;
}

GameResult GameWorld::Unlock(const std::string& target, const std::string& toolTarget, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	if (currentRoom == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		return GameResult::FatalError;
	}

	const bool targetsBackCell = TargetsCellDoor(target);
	const bool targetsCrypt = TargetsCryptLock(target);

	if (!targetsBackCell && !targetsCrypt)
	{
		output << "No encuentras ninguna cerradura que puedas abrir.\n";
		return GameResult::Running;
	}

	// ------------- The user is trying to unlock "Celda trasera" -----------------

	if (targetsBackCell)
	{
		if (currentRoom->GetId() != RoomIds::SheriffOffice)
		{
			output << "No encuentras la puerta de la celda aqui.\n";
			return GameResult::Running;
		}

		Exit* cellExit = currentRoom->FindExit(Direction::East);
		if (cellExit == nullptr)
		{
			std::cerr << "The exit to \"Celda trasera\" must always exist.\n";
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
		output << "Abres la puerta de la celda con la Llave pequena.\n";
		return GameResult::Running;
	}

	// ------------- The user is trying to unlock "Cripta al norte de la iglesia" -----------------

	if (currentRoom->GetId() != RoomIds::OldChurch)
	{
		output << "No encuentras la cerradura de la cripta aqui.\n";
		return GameResult::Running;
	}

	Exit* cryptExit = currentRoom->FindExit(Direction::North);
	if (cryptExit == nullptr)
	{
		std::cerr << "The exit to \"Cripta al norte de la iglesia\" must always exist.\n";
		return GameResult::FatalError;
	}

	if (!cryptExit->isLocked)
	{
		output << "La cerradura de la cripta ya esta abierta.\n";
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
	output << "Colocas la Cruz de plata en la cerradura. El acceso a la cripta queda abierto.\n";
	return GameResult::Running;
}

GameResult GameWorld::TurnOnItem(const std::string& target, std::ostream& output)
{
	if (GetCurrentRoom() == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
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

	// The item used to turn on the light source item must also be in player's inventory
	if (!m_player.HasItemById(ItemIds::Matches))
	{
		output << "Necesitas algo con lo que encender " << item->GetName() << ".\n";
		return GameResult::Running;
	}

	item->SetLightState(LightState::On);
	output << "Enciendes " << item->GetName() << ".\n";

	// Immediately reveal the room when the newly lit item makes it visible.
	const Room* room = GetCurrentRoom();
	if (room != nullptr && room->IsDark())
	{
		return Look(output);
	}

	return GameResult::Running;
}

GameResult GameWorld::LoadItem(const std::string& target, const std::string& ammunitionTarget, std::ostream& output)
{
	if (GetCurrentRoom() == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
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

	m_player.RemoveItemFromInventory(ammunition->GetId());
	item->SetLoadState(LoadState::Loaded);
	output << "Cargas " << item->GetName() << ".\n";
	return GameResult::Running;
}

GameResult GameWorld::BreakObstacle(const std::string& target, const std::string& toolTarget, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	if (currentRoom == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		return GameResult::FatalError;
	}

	if (!TargetsChurchChains(target))
	{
		output << "No sabes como romper eso.\n";
		return GameResult::Running;
	}

	if (currentRoom->GetId() != RoomIds::MainStreet)
	{
		output << "No encuentras esas cadenas aqui.\n";
		return GameResult::Running;
	}

	Exit* churchExit = currentRoom->FindExit(Direction::North);
	if (churchExit == nullptr)
	{
		std::cerr << "The exit to the old church must always exist.\n";
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
	output << "Cortas las cadenas con la Cizalla oxidada. La entrada a la iglesia queda libre.\n";
	return GameResult::Running;
}

GameResult GameWorld::ShootTarget(const std::string& target, const std::string& weaponTarget, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	if (currentRoom == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		return GameResult::FatalError;
	}

	if (target != WorldTargets::Sheriff)
	{
		output << "No sabes a que intentas disparar.\n";
		return GameResult::Running;
	}

	if (!CanPlayerSee(*currentRoom))
	{
		output << "No ves nada.\n";
		return GameResult::Running;
	}

	if (currentRoom->GetId() != RoomIds::Crypt)
	{
		output << "El sheriff no esta aqui.\n";
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
		output << "Aprietas el gatillo, pero el Revolver esta descargado. El Sheriff te dispara y luego remata la tarea con Elias\n";
		return GameResult::Defeat;
	}

	weapon->SetLoadState(LoadState::Unloaded);
	output << "Disparas al sheriff antes de que pueda reaccionar. Elias queda libre.\n";
	return GameResult::Victory;
}

GameResult GameWorld::ShowHelp(std::ostream& output) const
{
	output << "Comandos disponibles:\n";
	output << "- Movimiento: norte/n, sur/s, este/e, oeste/o, entrar, salir\n";
	output << "- Observacion: mirar/m, examinar/x [objeto]\n";
	output << "- Inventario: inventario/i, coger [objeto], soltar [objeto]\n";
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

	// In WestZork a light source only illuminates the room while the player is
	// carrying it. A lit lantern left on the ground does not provide visibility.
	return m_player.HasTurnedOnLightSource();
}

void GameWorld::InitializeWorld()
{

	/*
	*                            ROOMS CREATION
	*    // -------------------------------------------------------- \\
	*/

	std::shared_ptr<Room> townEntrance = std::make_shared<Room>(
		RoomIds::TownEntrance,
		"Entrada al pueblo",
		"El viejo arco de madera marca la entrada a Yellowville. El polvo cubre el camino y el silencio pesa demasiado.");

	std::shared_ptr<Room> abandonedStable = std::make_shared<Room>(
		RoomIds::AbandonedStable,
		"Establo abandonado",
		"Un establo medio hundido. Huele a heno podrido, madera humeda y algo que lleva anos sin moverse.");

	std::shared_ptr<Room> mainStreet = std::make_shared<Room>(
		RoomIds::MainStreet,
		"Calle Principal",
		"La calle cruza el pueblo de lado a lado. Las fachadas vacias parecen observar cada paso.");

	std::shared_ptr<Room> saloon = std::make_shared<Room>(
		RoomIds::Saloon,
		"Saloon",
		"Las puertas del saloon crujen con el viento. Botellas rotas y mesas volcadas cuentan una mala noche.");

	std::shared_ptr<Room> sheriffOffice = std::make_shared<Room>(
		RoomIds::SheriffOffice,
		"Oficina del Sheriff",
		"Una oficina pequena, con barrotes oxidados al fondo y papeles amarillentos sobre el escritorio.");

	std::shared_ptr<Room> backCell = std::make_shared<Room>(
		RoomIds::BackCell,
		"Celda trasera",
		"Una celda oscura y estrecha. La humedad se pega a la piel y cuesta distinguir el suelo.");

	std::shared_ptr<Room> oldChurch = std::make_shared<Room>(
		RoomIds::OldChurch,
		"Iglesia vieja",
		"El campanario esta torcido y la puerta principal conserva marcas profundas de cadenas.");

	std::shared_ptr<Room> crypt = std::make_shared<Room>(
		RoomIds::Crypt,
		"Cripta al norte de la iglesia",
		"Un pasadizo de piedra conduce a una cripta olvidada. En la oscuridad, algo espera.");

	AddRoom(townEntrance);
	AddRoom(abandonedStable);
	AddRoom(mainStreet);
	AddRoom(saloon);
	AddRoom(sheriffOffice);
	AddRoom(backCell);
	AddRoom(oldChurch);
	AddRoom(crypt);

	/*
	*                            ROOMS CONEXIONS CREATION
	*    // --------------------------------------------------------------------- \\
	*/

	townEntrance->AddExit(Direction::East, RoomIds::AbandonedStable);
	townEntrance->AddExit(Direction::North, RoomIds::MainStreet);

	abandonedStable->AddExit(Direction::West, RoomIds::TownEntrance);

	mainStreet->AddExit(Direction::South, RoomIds::TownEntrance);
	mainStreet->AddExit(Direction::West, RoomIds::Saloon);
	mainStreet->AddExit(Direction::East, RoomIds::SheriffOffice);
	mainStreet->AddExit(Direction::North, RoomIds::OldChurch, true);

	saloon->AddExit(Direction::East, RoomIds::MainStreet);

	sheriffOffice->AddExit(Direction::West, RoomIds::MainStreet);
	sheriffOffice->AddExit(Direction::East, RoomIds::BackCell, true);

	backCell->AddExit(Direction::West, RoomIds::SheriffOffice);

	oldChurch->AddExit(Direction::South, RoomIds::MainStreet);
	oldChurch->AddExit(Direction::North, RoomIds::Crypt, true);

	crypt->AddExit(Direction::South, RoomIds::OldChurch);

	/*
	*                      SPECIFIC ROOMS CONFIGURATION
	*    // -------------------------------------------------------- \\
	*/

	backCell->SetDark(true);
	crypt->SetDark(true);

	/*
	*                    ITEMS CREATION & CONFIGURATION
	*    // -------------------------------------------------------- \\
	*/

	std::shared_ptr<Item> whiskyBottle = std::make_shared<Item>(
		ItemIds::WhiskyBottle,
		"Botella de whisky vacia",
		"Una botella vacia con una etiqueta casi borrada.");
	whiskyBottle->AddAlias("botella");
	whiskyBottle->AddAlias("whisky");

	std::shared_ptr<Item> tornMap = std::make_shared<Item>(
		ItemIds::TornMap,
		"Mapa rasgado",
		"Un mapa incompleto del pueblo. Aun se distinguen la iglesia, el saloon y la oficina del sheriff.");
	tornMap->AddAlias("mapa");
	tornMap->SetRequiresLightToExamine(true);

	std::shared_ptr<Item> lantern = std::make_shared<Item>(
		ItemIds::Lantern,
		"Farol",
		"Un farol viejo que aun conserva algo de combustible.");
	lantern->AddAlias("farol");
	lantern->AddAlias("lampara");
	lantern->AddAlias("linterna");

	lantern->SetLightState(LightState::Off);

	std::shared_ptr<Item> matches = std::make_shared<Item>(
		ItemIds::Matches,
		"Cerillas",
		"Una pequena caja de cerillas humedecida por fuera.");
	matches->AddAlias("cerilla");

	std::shared_ptr<Item> boltCutter = std::make_shared<Item>(
		ItemIds::BoltCutter,
		"Cizalla oxidada",
		"Una cizalla pesada con las hojas comidas por el oxido.");
	boltCutter->AddAlias("cizalla");

	std::shared_ptr<Item> potatoSack = std::make_shared<Item>(
		ItemIds::PotatoSack,
		"Saco de patatas podridas",
		"Un saco de arpillera lleno de patatas podridas. El olor resulta insoportable.");
	potatoSack->AddAlias("saco");
	potatoSack->AddAlias("saco de patatas");
	potatoSack->AddAlias("patatas");

	potatoSack->SetContainerState(ContainerState::Closed);

	std::shared_ptr<Item> smallKey = std::make_shared<Item>(
		ItemIds::SmallKey,
		"Llave pequena",
		"Una llave pequena de hierro ennegrecido.");
	smallKey->AddAlias("llave");
	smallKey->AddAlias("llave pequena");

	std::shared_ptr<Item> bartenderNote = std::make_shared<Item>(
		ItemIds::BartenderNote,
		"Nota del tabernero",
		"Una nota escrita con pulso tembloroso por el antiguo tabernero.");
	bartenderNote->AddAlias("nota");
	bartenderNote->AddAlias("nota del tabernero");
	bartenderNote->SetRequiresLightToExamine(true);

	std::shared_ptr<Item> safeBox = std::make_shared<Item>(
		ItemIds::SafeBox,
		"Caja fuerte",
		"Una caja fuerte compacta detras de la barra. La cerradura parece pequena.");
	safeBox->AddAlias("caja");

	safeBox->SetContainerState(ContainerState::Open);
	safeBox->AddItem(bartenderNote);
	safeBox->SetContainerState(ContainerState::Locked);

	std::shared_ptr<Item> revolver = std::make_shared<Item>(
		ItemIds::Revolver,
		"Revolver",
		"Un revolver frio al tacto, con el tambor listo para recibir municion.");
	revolver->AddAlias("revolver");

	revolver->SetLoadState(LoadState::Unloaded);

	std::shared_ptr<Item> ammunition = std::make_shared<Item>(
		ItemIds::Ammunition,
		"Municion",
		"Unas pocas balas envueltas en papel aceitado.");
	ammunition->AddAlias("balas");

	std::shared_ptr<Item> sheriffDiary = std::make_shared<Item>(
		ItemIds::SheriffDiary,
		"Diario del sheriff",
		"Un diario manchado con entradas cada vez mas desesperadas.");
	sheriffDiary->AddAlias("diario");
	sheriffDiary->SetRequiresLightToExamine(true);

	std::shared_ptr<Item> silverCross = std::make_shared<Item>(
		ItemIds::SilverCross,
		"Cruz de plata",
		"Una cruz de plata pequena, demasiado limpia para este lugar.");
	silverCross->AddAlias("cruz");

	std::shared_ptr<Item> eliasHat = std::make_shared<Item>(
		ItemIds::EliasHat,
		"Sombrero de Elias",
		"El sombrero de tu hermano. Reconocerias esa cinta azul en cualquier parte.");
	eliasHat->AddAlias("sombrero");

	townEntrance->AddItem(whiskyBottle);
	townEntrance->AddItem(tornMap);
	abandonedStable->AddItem(lantern);
	abandonedStable->AddItem(matches);
	mainStreet->AddItem(boltCutter);
	mainStreet->AddItem(potatoSack);
	saloon->AddItem(smallKey);
	saloon->AddItem(safeBox);
	sheriffOffice->AddItem(revolver);
	sheriffOffice->AddItem(ammunition);
	sheriffOffice->AddItem(sheriffDiary);
	backCell->AddItem(silverCross);
	oldChurch->AddItem(eliasHat);

	AddItem(whiskyBottle);
	AddItem(tornMap);
	AddItem(lantern);
	AddItem(matches);
	AddItem(boltCutter);
	AddItem(potatoSack);
	AddItem(smallKey);
	AddItem(bartenderNote);
	AddItem(safeBox);
	AddItem(revolver);
	AddItem(ammunition);
	AddItem(sheriffDiary);
	AddItem(silverCross);
	AddItem(eliasHat);

	m_player.SetCurrentRoomId(RoomIds::TownEntrance);
}
