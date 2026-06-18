#include "GameWorld.h"
#include "Parser.h"

#include <algorithm>
#include <iostream>

GameWorld::GameWorld()
	: m_player("player", "James", "Un antiguo alguacil que vuelve al pueblo en busca de su hermano Elias.")
{
	InitializeWorld();
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
		PutItemIntoContainer(command.firstTarget, isRunning, command.secondTarget, output);
		break;
	case CommandType::Remove:
		TakeItemFromContainer(command.firstTarget, isRunning, command.secondTarget, output);
		break;
	case CommandType::Open:
		Open(command.firstTarget, command.secondTarget, isRunning, output);
		break;
	case CommandType::TurnOn:
		TurnOnItem(command.firstTarget, isRunning, output);
		break;
	case CommandType::Load:
		LoadItem(command.firstTarget, command.secondTarget, isRunning, output);
		break;
	case CommandType::Break:
		BreakObstacle(command.firstTarget, command.secondTarget, isRunning, output);
		break;
	case CommandType::Shoot:
		Shoot(command.firstTarget, command.secondTarget, isRunning, output);
		break;
	case CommandType::Help:
		ShowHelp(output);
		break;
	case CommandType::Quit:
		isRunning = false;
		break;
	default:
		output << "No he entendido eso.\n";
		break;
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

	if (CanPlayerSee(*room))
	{
		room->PrintInformation(output);
	}
	else
	{
		output << "Esta demasiado oscuro. Necesitas una fuente de luz encendida para poder ver.\n";
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
		output << "No puedes ir hacia " << DirectionUtils::ToText(direction) << ". El mapa te puede dar alguna pista...\n";
		return;
	}

	Room* nextRoom = FindRoomById(nextRoomId);
	if (nextRoom == nullptr) // <--------- Just a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "The pointer to nextRoom must never be nullptr.\n";
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

	// Inventory items can be examined by touch even in a dark room. Darkness
	// only prevents the player from discovering and inspecting room contents.
	const Item* item = m_player.FindItem(target);
	if (item == nullptr)
	{
		if (!CanPlayerSee(*room))
		{
			output << "Esta demasiado oscuro para examinar nada de la sala.\n";
			return;
		}

		if (!IsAValidItem(target))
		{
			output << "No se lo que intentas examinar.\n";
			return;
		}

		// In WestZork every item is unique, so after checking the inventory the
		// target can only be directly present in the current room or unavailable.
		item = room->FindItem(target);
		if (item == nullptr)
		{
			output << "No encuentras lo que intentas examinar.\n";
			return;
		}
	}

	// If the item was found, show it's information (name, descripction and items inside, if any)
	item->PrintInformation(output);
	if (item->GetId() == "mapa_rasgado")
	{
		room->PrintExists(output);
	}
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

	if (!CanPlayerSee(*room))
	{
		output << "Esta demasiado oscuro para encontrar objetos en la sala.\n";
		return;
	}

	if (!IsAValidItem(target))
	{
		output << "No se lo que intentas coger.\n";
		return;
	}

	const Item* item = room->FindItem(target);
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

	if (!IsAValidItem(target))
	{
		output << "No se lo que intentas soltar.\n";
		return;
	}

	const Item* item = m_player.FindItem(target);
	if (item == nullptr)
	{
		output << "No tienes ese objeto.\n";
		return;
	}

	room->AddItem(m_player.RemoveItemFromInventory(item->GetId()));
	output << "Sueltas " << item->GetName() << ".\n";
}

void GameWorld::PutItemIntoContainer(const std::string& itemTarget, bool& isRunning, const std::string& containerTarget, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr) // <--------- Just  a check that warns the programmer. This should not ever happen.
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	if (!IsAValidItem(itemTarget))
	{
		output << "No se lo que intentas meter.\n";
		return;
	}

	if (!IsAValidItem(containerTarget))
	{
		output << "No se donde intentas meterlo.\n";
		return;
	}

	// Try to find the item to move it into the item container
	// IMPORTANT: The target item must be in player's inventory
	const Item* item = m_player.FindItem(itemTarget);
	if (item == nullptr)
	{
		output << "No tienes ese objeto.\n";
		return;
	}

	// Try to find the item container (could be in player inventory or in current room)
	Item* containerItem = m_player.FindItem(containerTarget);
	if (containerItem == nullptr)
	{
		containerItem = room->FindItem(containerTarget);
	}

	// Didn't find it
	if (containerItem == nullptr)
	{
		output << "No tienes el objeto donde quieres meter " << item->GetName() << ".\n";
		return;
	}

	// If found, check if it's really a container item
	if (!containerItem->IsContainer())
	{
		output << containerItem->GetName() << " no puede contener objetos.\n";
		return;
	}

	if (containerItem->IsLocked())
	{
		output << containerItem->GetName() << " esta bloqueado.\n";
		return;
	}

	if (!containerItem->IsOpen())
	{
		output << containerItem->GetName() << " esta cerrado.\n";
		return;
	}

	// If found, check if the item it's not the same as the container item ("Meter botella en botella")
	if (item->GetId() == containerItem->GetId())
	{
		output << "No puedes meter un objeto dentro de si mismo.\n";
		return;
	}

	if (item->IsContainer())
	{
		output << "No puedes meter " << item->GetName() << "en " << containerItem->GetName() << ".\n";
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

	if (!IsAValidItem(containerTarget))
	{
		output << "No se de donde intentas sacar eso.\n";
		return;
	}

	if (!IsAValidItem(itemTarget))
	{
		output << "No se lo que intentas sacar.\n";
		return;
	}

	// Try to find the container item (could be in player inventory or in current room)
	Item* container = m_player.FindItem(containerTarget);
	if (container == nullptr)
	{
		container = room->FindItem(containerTarget);
	}

	if (container == nullptr)
	{
		output << "No encuentras ese objeto aqui.\n";
		return;
	}

	// Check first if it's actually a container item
	if (!container->IsContainer())
	{
		output << "No se puede guardar nada en " << container->GetName() << ".\n";
		return;
	}

	if (container->IsLocked())
	{
		output << container->GetName() << " esta bloqueado.\n";
		return;
	}

	if (!container->IsOpen())
	{
		output << container->GetName() << " esta cerrado.\n";
		return;
	}

	// Try to find the target item in the container item
	Item* item = container->FindItem(itemTarget);
	if (item == nullptr)
	{
		output << "Este objeto no esta en " << container->GetName() << ".\n";
		return;
	}

	m_player.AddItemToInventory(container->RemoveItem(item->GetId()));
	output << "Sacas " << item->GetName() << " de " << container->GetName() << ".\n";
}

void GameWorld::Open(const std::string& target, const std::string& toolTarget, bool& isRunning, std::ostream& output)
{
	const bool targetsRoomLock =
		target == "celda" ||
		target == "puerta de la celda" ||
		target == "cerradura" ||
		target == "cerradura de la cripta";

	if (targetsRoomLock)
	{
		// This handles commands to unlock items or rooms
		Unlock(target, toolTarget, isRunning, output);
	}
	else
	{
		OpenItem(target, toolTarget, isRunning, output);
	}
}

void GameWorld::OpenItem(const std::string& target, const std::string& toolTarget, bool& isRunning, std::ostream& output)
{
	Room* room = GetCurrentRoom();
	if (room == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	// Check if the item is in the room or in the player's inventory
	Item* item = m_player.FindItem(target);
	if (item == nullptr)
	{
		item = room->FindItem(target);
	}

	if (item == nullptr)
	{
		output << "No encuentras lo que intentas abrir.\n";
		return;
	}

	if (!item->IsContainer())
	{
		output << item->GetName() << " no se puede abrir.\n";
		return;
	}

	if (item->IsOpen())
	{
		output << item->GetName() << " ya esta abierto.\n";
		return;
	}

	if (item->IsLocked())
	{
		if (item->GetId() != "caja_fuerte")
		{
			output << "No sabes como abrir " << item->GetName() << ".\n";
			return;
		}

		const std::string requiredKeyId = "llave";
		const Item* key = toolTarget.empty() ? m_player.FindItem(requiredKeyId) : m_player.FindItem(toolTarget);

		if (key == nullptr || key->GetId() != requiredKeyId)
		{
			output << "Necesitas la llave adecuada para abrir " << item->GetName() << ".\n";
			return;
		}
	}

	item->SetContainerState(ContainerState::Open);
	output << "Abres " << item->GetName() << ".\n";
}

void GameWorld::Unlock(const std::string& target, const std::string& toolTarget, bool& isRunning, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	if (currentRoom == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	const bool targetsBackCell = target == "celda" || target == "puerta de la celda";
	const bool targetsCrypt = target == "cerradura" || target == "cerradura de la cripta";

	if (!targetsBackCell && !targetsCrypt)
	{
		output << "No encuentras ninguna cerradura que puedas abrir.\n";
		return;
	}

	// ------------- The user is trying to unlock "Celda trasera" -----------------

	if (targetsBackCell)
	{
		if (currentRoom->GetId() != "oficina_sheriff")
		{
			output << "No encuentras la puerta de la celda aqui.\n";
			return;
		}

		Room* backCell = FindRoomById("celda_trasera");
		if (backCell == nullptr)
		{
			std::cerr << "The \"Celda trasera\" room must always exist in the world.\n";
			isRunning = false;
			return;
		}

		if (!backCell->IsLocked())
		{
			output << "La puerta de la celda ya esta abierta.\n";
			return;
		}

		// If we get here it means we are in "Oficina del Sheriff" and "Celda trasera" it's locked
		const std::string requiredToolId = "llave";
		const Item* smallKey = toolTarget.empty() ? m_player.FindItem(requiredToolId): m_player.FindItem(toolTarget);

		if (smallKey == nullptr || smallKey->GetId() != requiredToolId)
		{
			output << "Necesitas la llave adecuada para abrir la puerta de la celda.\n";
			return;
		}

		backCell->SetLocked(false);
		output << "Abres la puerta de la celda con la Llave pequena.\n";
		return;
	}

	// ------------- The user is trying to unlock "Cripta al norte de la iglesia" -----------------

	if (currentRoom->GetId() != "iglesia_vieja")
	{
		output << "No encuentras la cerradura de la cripta aqui.\n";
		return;
	}

	Room* crypt = FindRoomById("cripta");
	if (crypt == nullptr)
	{
		std::cerr << "The \"Cripta al norte de la iglesia\" room must always exist in the world.\n";
		isRunning = false;
		return;
	}

	if (!crypt->IsLocked())
	{
		output << "La cerradura de la cripta ya esta abierta.\n";
		return;
	}

	// If we get here it means we are in "Iglesia vieja" and "Cripta al norte de la iglesia" it's locked
	const std::string requiredToolId = "cruz_plata";
	const Item* silverCross = toolTarget.empty() ? m_player.FindItem(requiredToolId) : m_player.FindItem(toolTarget);

	if (silverCross == nullptr || silverCross->GetId() != requiredToolId)
	{
		output << "La cerradura tiene forma de cruz. Necesitas algo que encaje en ella.\n";
		return;
	}

	crypt->SetLocked(false);
	output << "Colocas la Cruz de plata en la cerradura. El acceso a la cripta queda abierto.\n";
}

void GameWorld::TurnOnItem(const std::string& target, bool& isRunning, std::ostream& output)
{
	if (GetCurrentRoom() == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	// To turn on a light source item, it must be in player's inventory
	Item* item = m_player.FindItem(target);
	if (item == nullptr)
	{
		output << "Debes tener el objeto para poder encenderlo.\n";
		return;
	}

	if (!item->IsLightSource())
	{
		output << item->GetName() << " no se puede encender.\n";
		return;
	}

	if (item->IsTurnedOn())
	{
		output << item->GetName() << " ya esta encendido.\n";
		return;
	}

	// The item used to turn on the light source item must also be in player's inventory
	if (m_player.FindItem("cerillas") == nullptr)
	{
		output << "Necesitas algo con lo que encender " << item->GetName() << ".\n";
		return;
	}

	item->SetLightState(LightState::On);
	output << "Enciendes " << item->GetName() << ".\n";

	// Immediately reveal the room when the newly lit item makes it visible.
	const Room* room = GetCurrentRoom();
	if (room != nullptr && room->IsDark())
	{
		Look(output, isRunning);
	}
}

void GameWorld::LoadItem(const std::string& target, const std::string& ammunitionTarget, bool& isRunning, std::ostream& output)
{
	if (GetCurrentRoom() == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	// To charge the item "pistola", it must be in player's inventory
	Item* item = m_player.FindItem(target);
	if (item == nullptr)
	{
		output << "Debes tener el objeto para poder cargarlo.\n";
		return;
	}

	if (!item->IsWeapon())
	{
		output << item->GetName() << " no se puede cargar.\n";
		return;
	}

	if (item->IsLoaded())
	{
		output << item->GetName() << " ya esta cargado.\n";
		return;
	}

	// The item "Municion" must also be in player's inventory
	const std::string ammunitionId = "municion";

	// This is just to allow the command to not mention "municion" so the input could be "Cargar revolver" and also "Cargar revolver con municion"
	const Item* ammunition = ammunitionTarget.empty() ? m_player.FindItem(ammunitionId) : m_player.FindItem(ammunitionTarget);

	if (ammunition == nullptr || ammunition->GetId() != ammunitionId)
	{
		output << "Necesitas la municion adecuada para cargar " << item->GetName() << ".\n";
		return;
	}

	m_player.RemoveItemFromInventory(ammunition->GetId());
	item->SetLoadState(LoadState::Loaded);
	output << "Cargas " << item->GetName() << ".\n";
}

void GameWorld::BreakObstacle(const std::string& target, const std::string& toolTarget, bool& isRunning, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	if (currentRoom == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	if (target != "cadenas" && target != "cadena")
	{
		output << "No sabes como romper eso.\n";
		return;
	}

	if (currentRoom->GetId() != "calle_principal")
	{
		output << "No encuentras esas cadenas aqui.\n";
		return;
	}

	Room* oldChurch = FindRoomById("iglesia_vieja");
	if (oldChurch == nullptr)
	{
		std::cerr << "The old church must always exist in the world.\n";
		isRunning = false;
		return;
	}

	if (!oldChurch->IsLocked())
	{
		output << "Las cadenas ya estan rotas.\n";
		return;
	}

	const std::string requiredToolId = "cizalla";
	const Item* boltCutter = toolTarget.empty() ? m_player.FindItem(requiredToolId) : m_player.FindItem(toolTarget);

	if (boltCutter == nullptr || boltCutter->GetId() != requiredToolId)
	{
		output << "Las cadenas son demasiado gruesas para romperlas con las manos.\n";
		return;
	}

	oldChurch->SetLocked(false);
	output << "Cortas las cadenas con la Cizalla oxidada. La entrada a la iglesia queda libre.\n";
}

void GameWorld::Shoot(const std::string& target, const std::string& weaponTarget, bool& isRunning, std::ostream& output)
{
	Room* currentRoom = GetCurrentRoom();
	if (currentRoom == nullptr)
	{
		std::cerr << "GetCurrentRoom() must always return a valid Room.\n";
		isRunning = false;
		return;
	}

	if (target != "sheriff")
	{
		output << "No sabes a que intentas disparar.\n";
		return;
	}

	if (!CanPlayerSee(*currentRoom))
	{
		output << "No ves nada.\n";
		return;
	}

	if (currentRoom->GetId() != "cripta")
	{
		output << "El sheriff no esta aqui.\n";
		return;
	}

	const std::string requiredWeaponId = "revolver";
	Item* weapon = weaponTarget.empty() ? m_player.FindItem(requiredWeaponId) : m_player.FindItem(weaponTarget);

	if (weapon == nullptr || weapon->GetId() != requiredWeaponId)
	{
		output << "Necesitas el Revolver para disparar al sheriff.\n";
		return;
	}

	if (!weapon->IsLoaded())
	{
		output << "Aprietas el gatillo, pero el Revolver esta descargado. El Sheriff te dispara y luego remata la tarea con Elias\n";
		output << "Has perdido.\n";
		isRunning = false;
		return;
	}

	weapon->SetLoadState(LoadState::Unloaded);
	output << "Disparas al sheriff antes de que pueda reaccionar. Elias queda libre.\n";
	output << "Has ganado.\n";
	isRunning = false;
}

void GameWorld::ShowHelp(std::ostream& output) const
{
	output << "Comandos disponibles:\n";
	output << "- Movimiento: norte/n, sur/s, este/e, oeste/o, entrar, salir\n";
	output << "- Observacion: mirar/m, examinar/x [objeto]\n";
	output << "- Inventario: inventario/i, coger [objeto], soltar [objeto]\n";
	output << "- Contenedores: meter [objeto] en [contenedor], sacar [objeto] de [contenedor]\n";
	output << "- Acciones: abrir [objeto], encender [objeto], cargar [objeto]\n";
	output << "- Puzles: romper [objeto] con [herramienta], disparar [objetivo]\n";
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

bool GameWorld::CanPlayerSee(const Room& room) const
{
	if (!room.IsDark())
	{
		return true;
	}

	// In WestZork a light source only illuminates the room while the player is
	// carrying it. A lit lantern left on the ground does not provide visibility.
	const auto& inventory = m_player.GetInventory();
	return std::find_if(inventory.begin(), inventory.end(),
		[](const std::shared_ptr<Item>& item)
		{
			return item->IsLightSource() && item->IsTurnedOn();
		}) != inventory.end();
}

void GameWorld::InitializeWorld()
{

	/*
	*                            ROOMS CREATION
	*    // -------------------------------------------------------- \\
	*/

	std::shared_ptr<Room> townEntrance = std::make_shared<Room>(
		"entrada_pueblo",
		"Entrada al pueblo",
		"El viejo arco de madera marca la entrada a West Zork. El polvo cubre el camino y el silencio pesa demasiado.");

	std::shared_ptr<Room> abandonedStable = std::make_shared<Room>(
		"establo_abandonado",
		"Establo abandonado",
		"Un establo medio hundido. Huele a heno podrido, madera humeda y algo que lleva anos sin moverse.");

	std::shared_ptr<Room> mainStreet = std::make_shared<Room>(
		"calle_principal",
		"Calle Principal",
		"La calle cruza el pueblo de lado a lado. Las fachadas vacias parecen observar cada paso.");

	std::shared_ptr<Room> saloon = std::make_shared<Room>(
		"saloon",
		"Saloon",
		"Las puertas del saloon crujen con el viento. Botellas rotas y mesas volcadas cuentan una mala noche.");

	std::shared_ptr<Room> sheriffOffice = std::make_shared<Room>(
		"oficina_sheriff",
		"Oficina del Sheriff",
		"Una oficina pequena, con barrotes oxidados al fondo y papeles amarillentos sobre el escritorio.");

	std::shared_ptr<Room> backCell = std::make_shared<Room>(
		"celda_trasera",
		"Celda trasera",
		"Una celda oscura y estrecha. La humedad se pega a la piel y cuesta distinguir el suelo.");

	std::shared_ptr<Room> oldChurch = std::make_shared<Room>(
		"iglesia_vieja",
		"Iglesia vieja",
		"El campanario esta torcido y la puerta principal conserva marcas profundas de cadenas.");

	std::shared_ptr<Room> crypt = std::make_shared<Room>(
		"cripta",
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

	townEntrance->AddExit(Direction::East, "establo_abandonado");
	townEntrance->AddExit(Direction::North, "calle_principal");

	abandonedStable->AddExit(Direction::West, "entrada_pueblo");

	mainStreet->AddExit(Direction::South, "entrada_pueblo");
	mainStreet->AddExit(Direction::West, "saloon");
	mainStreet->AddExit(Direction::East, "oficina_sheriff");
	mainStreet->AddExit(Direction::North, "iglesia_vieja");

	saloon->AddExit(Direction::East, "calle_principal");

	sheriffOffice->AddExit(Direction::West, "calle_principal");
	sheriffOffice->AddExit(Direction::East, "celda_trasera");

	backCell->AddExit(Direction::West, "oficina_sheriff");

	oldChurch->AddExit(Direction::South, "calle_principal");
	oldChurch->AddExit(Direction::North, "cripta");

	crypt->AddExit(Direction::South, "iglesia_vieja");

	/*
	*                      SPECIFIC ROOMS CONFIGURATION
	*    // -------------------------------------------------------- \\
	*/

	backCell->SetDark(true);
	backCell->SetLocked(true);
	crypt->SetDark(true);
	crypt->SetLocked(true);
	oldChurch->SetLocked(true);

	/*
	*                    ITEMS CREATION & CONFIGURATION
	*    // -------------------------------------------------------- \\
	*/

	std::shared_ptr<Item> whiskyBottle = std::make_shared<Item>(
		"botella_whisky",
		"Botella de whisky vacia",
		"Una botella vacia con una etiqueta casi borrada.");
	whiskyBottle->AddAlias("botella");
	whiskyBottle->AddAlias("whisky");

	std::shared_ptr<Item> tornMap = std::make_shared<Item>(
		"mapa_rasgado",
		"Mapa rasgado",
		"Un mapa incompleto del pueblo. Aun se distinguen la iglesia, el saloon y la oficina del sheriff.");
	tornMap->AddAlias("mapa");

	std::shared_ptr<Item> lantern = std::make_shared<Item>(
		"farol",
		"Farol",
		"Un farol viejo que aun conserva algo de combustible.");
	lantern->AddAlias("farol");

	lantern->SetLightState(LightState::Off);

	std::shared_ptr<Item> matches = std::make_shared<Item>(
		"cerillas",
		"Cerillas",
		"Una pequena caja de cerillas humedecida por fuera.");
	matches->AddAlias("cerilla");

	std::shared_ptr<Item> boltCutter = std::make_shared<Item>(
		"cizalla",
		"Cizalla oxidada",
		"Una cizalla pesada con las hojas comidas por el oxido.");
	boltCutter->AddAlias("cizalla");

	std::shared_ptr<Item> potatoSack = std::make_shared<Item>(
		"saco_patatas",
		"Saco de patatas podridas",
		"Un saco de arpillera lleno de patatas podridas. El olor resulta insoportable.");
	potatoSack->AddAlias("saco");
	potatoSack->AddAlias("saco de patatas");
	potatoSack->AddAlias("patatas");

	potatoSack->SetContainerState(ContainerState::Closed);

	std::shared_ptr<Item> smallKey = std::make_shared<Item>(
		"llave",
		"Llave pequena",
		"Una llave pequena de hierro ennegrecido.");
	smallKey->AddAlias("llave");
	smallKey->AddAlias("llave pequena");

	std::shared_ptr<Item> bartenderNote = std::make_shared<Item>(
		"nota_tabernero",
		"Nota del tabernero",
		"Una nota escrita con pulso tembloroso por el antiguo tabernero.");
	bartenderNote->AddAlias("nota");
	bartenderNote->AddAlias("nota del tabernero");

	std::shared_ptr<Item> safeBox = std::make_shared<Item>(
		"caja_fuerte",
		"Caja fuerte",
		"Una caja fuerte compacta detras de la barra. La cerradura parece pequena.");
	safeBox->AddAlias("caja");

	safeBox->SetContainerState(ContainerState::Open);
	safeBox->AddItem(bartenderNote);
	safeBox->SetContainerState(ContainerState::Locked);

	std::shared_ptr<Item> revolver = std::make_shared<Item>(
		"revolver",
		"Revolver",
		"Un revolver frio al tacto, con el tambor listo para recibir municion.");
	revolver->AddAlias("revolver");

	revolver->SetLoadState(LoadState::Unloaded);

	std::shared_ptr<Item> ammunition = std::make_shared<Item>(
		"municion",
		"Municion",
		"Unas pocas balas envueltas en papel aceitado.");
	ammunition->AddAlias("balas");

	std::shared_ptr<Item> sheriffDiary = std::make_shared<Item>(
		"diario_sheriff",
		"Diario del sheriff",
		"Un diario manchado con entradas cada vez mas desesperadas.");
	sheriffDiary->AddAlias("diario");

	std::shared_ptr<Item> silverCross = std::make_shared<Item>(
		"cruz_plata",
		"Cruz de plata",
		"Una cruz de plata pequena, demasiado limpia para este lugar.");
	silverCross->AddAlias("cruz");

	std::shared_ptr<Item> eliasHat = std::make_shared<Item>(
		"sombrero_elias",
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

	m_player.SetCurrentRoomId("entrada_pueblo");
}
