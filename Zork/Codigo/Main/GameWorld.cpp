#include "GameWorld.h"
#include "Parser.h"

#include <algorithm>
#include <iostream>

bool GameWorld::IsAValidItem(const std::string& target) const
{
	return std::find_if(m_items.begin(), m_items.end(),
		[&target](const std::pair<const std::string, std::shared_ptr<Item>>& itemEntry)
		{
			return itemEntry.second->MatchesTarget(target);
		}) != m_items.end();
}

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

	if (!IsAValidItem(target))
	{
		output << "No se lo que intentas examinar.\n";
		return;
	}

	// In WestZork there is item uniqueness so an item either is in the room, in the player inventory or it does not exist in the world. (Not sure if this also happens in Zork)
	const Item* item = room->FindItem(target);  // <------- Search first in the current room
	if (item == nullptr)
	{
		item = m_player.FindItem(target); // <------- And then in the player inventory
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

	// Try to find the container item
	// IMPORTANT: The container item must be in player's inventory
	Item* container = m_player.FindItem(containerTarget);
	if (container == nullptr)
	{
		output << "No tienes ese objeto.\n";
		return;
	}

	// Check first if it's actually a container item
	if (!container->IsContainer())
	{
		output << "No se puede guardar nada en " << container->GetName() << ".\n";
		return;
	}

	// Try to find the target item in the container item
	Item* item = container->FindItem(containerTarget);
	if (item == nullptr)
	{
		output << "Este objeto no esta en " << container->GetName() << ".\n";
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
		"Cripta bajo la iglesia",
		"Un frio seco sube desde la piedra. En la oscuridad, algo espera.");

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
	oldChurch->AddExit(Direction::Down, "cripta");

	crypt->AddExit(Direction::Exit, "iglesia_vieja");

	/*
	*                      SPECIFIC ROOMS CONFIGURATION
	*    // -------------------------------------------------------- \\
	*/

	backCell->SetDark(true);
	backCell->SetLocked(true);
	crypt->SetDark(true);
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
		"Farol apagado",
		"Un farol viejo, pero parece que todavia podria encenderse.");
	lantern->AddAlias("farol");

	lantern->SetLightSource(true);

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

	std::shared_ptr<Item> smallKey = std::make_shared<Item>(
		"llave",
		"Llave pequena",
		"Una llave pequena de hierro ennegrecido.");
	smallKey->AddAlias("llave");
	smallKey->AddAlias("llave pequena");

	std::shared_ptr<Item> safeBox = std::make_shared<Item>(
		"caja_fuerte",
		"Caja fuerte",
		"Una caja fuerte compacta detras de la barra.");
	safeBox->AddAlias("caja");

	safeBox->SetContainer(true);

	std::shared_ptr<Item> revolver = std::make_shared<Item>(
		"revolver",
		"Revolver descargado",
		"Un revolver frio al tacto. No hay balas en el tambor.");
	revolver->AddAlias("revolver");

	revolver->SetWeapon(true);

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
	AddItem(smallKey);
	AddItem(safeBox);
	AddItem(revolver);
	AddItem(ammunition);
	AddItem(sheriffDiary);
	AddItem(silverCross);
	AddItem(eliasHat);

	m_player.SetCurrentRoomId("entrada_pueblo");
}
