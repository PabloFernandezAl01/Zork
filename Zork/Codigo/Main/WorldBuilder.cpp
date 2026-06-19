#include "WorldBuilder.h"
#include "WorldIds.h"

#include <cassert>

WorldData WorldBuilder::Build()
{
	WorldData world;

	InitializeRooms(world);
	ConnectRooms(world);
	InitializeItems(world);
	PlaceItems(world);

	world.initialRoomId = RoomIds::TownEntrance;
	return world;
}

void WorldBuilder::InitializeRooms(WorldData& world)
{
	AddRoom(world, std::make_shared<Room>(
		RoomIds::TownEntrance,
		"Entrada al pueblo",
		"El viejo arco de madera marca la entrada a Yellowville. El polvo cubre el camino y el silencio pesa demasiado."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::AbandonedStable,
		"Establo abandonado",
		"Un establo medio hundido. Huele a heno podrido, madera humeda y algo que lleva anos sin moverse."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::MainStreet,
		"Calle Principal",
		"La calle cruza el pueblo de lado a lado. Las fachadas vacias parecen observar cada paso."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::Saloon,
		"Saloon",
		"Las puertas del saloon crujen con el viento. Botellas rotas y mesas volcadas cuentan una mala noche."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::SheriffOffice,
		"Oficina del Sheriff",
		"Una oficina pequena, con barrotes oxidados al fondo y papeles amarillentos sobre el escritorio."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::BackCell,
		"Celda trasera",
		"Una celda oscura y estrecha. La humedad se pega a la piel y cuesta distinguir el suelo."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::OldChurch,
		"Iglesia vieja",
		"El campanario esta torcido y la puerta principal conserva marcas profundas de cadenas."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::Crypt,
		"Cripta al norte de la iglesia",
		"Un pasadizo de piedra conduce a una cripta olvidada. El sheriff mantiene a Elias cautivo y te apunta con su revolver."));

	world.rooms.at(RoomIds::BackCell)->SetDark(true);
	world.rooms.at(RoomIds::Crypt)->SetDark(true);
}

void WorldBuilder::ConnectRooms(WorldData& world)
{
	Room& townEntrance = *world.rooms.at(RoomIds::TownEntrance);
	Room& abandonedStable = *world.rooms.at(RoomIds::AbandonedStable);
	Room& mainStreet = *world.rooms.at(RoomIds::MainStreet);
	Room& saloon = *world.rooms.at(RoomIds::Saloon);
	Room& sheriffOffice = *world.rooms.at(RoomIds::SheriffOffice);
	Room& backCell = *world.rooms.at(RoomIds::BackCell);
	Room& oldChurch = *world.rooms.at(RoomIds::OldChurch);
	Room& crypt = *world.rooms.at(RoomIds::Crypt);

	townEntrance.AddExit(Direction::East, abandonedStable);
	townEntrance.AddExit(Direction::North, mainStreet);

	abandonedStable.AddExit(Direction::West, townEntrance);

	mainStreet.AddExit(Direction::South, townEntrance);
	mainStreet.AddExit(Direction::West, saloon);
	mainStreet.AddExit(Direction::East, sheriffOffice);
	mainStreet.AddExit(Direction::North, oldChurch, true);

	saloon.AddExit(Direction::East, mainStreet);

	sheriffOffice.AddExit(Direction::West, mainStreet);
	sheriffOffice.AddExit(Direction::East, backCell, true);

	backCell.AddExit(Direction::West, sheriffOffice);

	oldChurch.AddExit(Direction::South, mainStreet);
	oldChurch.AddExit(Direction::North, crypt, true);

	crypt.AddExit(Direction::South, oldChurch);
}

void WorldBuilder::InitializeItems(WorldData& world)
{
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
		"La nota dice: \"El sheriff se lleva a cualquiera que hace preguntas. Anoche oi a un hombre llamado Elias gritar desde la celda trasera de su oficina.\"");
	bartenderNote->AddAlias("nota");
	bartenderNote->AddAlias("nota del tabernero");
	bartenderNote->SetRequiresLightToExamine(true);

	std::shared_ptr<Item> safeBox = std::make_shared<Item>(
		ItemIds::SafeBox,
		"Caja fuerte",
		"Una caja fuerte compacta detras de la barra. La cerradura parece pequena.");
	safeBox->AddAlias("caja");
	safeBox->AddAlias("cerradura");
	safeBox->AddAlias("cerradura de la caja fuerte");
	safeBox->SetContainerState(ContainerState::Open);
	const bool noteAdded = safeBox->AddItem(bartenderNote);
	assert(noteAdded);
	(void)noteAdded;
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
		"Una entrada reciente dice: \"He trasladado a Elias a la cripta bajo la iglesia. Oculte en la celda la cruz de plata que abre la cerradura del muro norte.\"");
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

	AddItem(world, whiskyBottle);
	AddItem(world, tornMap);
	AddItem(world, lantern);
	AddItem(world, matches);
	AddItem(world, boltCutter);
	AddItem(world, potatoSack);
	AddItem(world, smallKey);
	AddItem(world, bartenderNote);
	AddItem(world, safeBox);
	AddItem(world, revolver);
	AddItem(world, ammunition);
	AddItem(world, sheriffDiary);
	AddItem(world, silverCross);
	AddItem(world, eliasHat);
}

void WorldBuilder::PlaceItems(WorldData& world)
{
	const auto placeItem = [&world](const std::string& roomId, const std::string& itemId)
	{
		const bool added = world.rooms.at(roomId)->AddItem(world.items.at(itemId));
		assert(added);
		(void)added;
	};

	placeItem(RoomIds::TownEntrance, ItemIds::WhiskyBottle);
	placeItem(RoomIds::TownEntrance, ItemIds::TornMap);
	placeItem(RoomIds::AbandonedStable, ItemIds::Lantern);
	placeItem(RoomIds::AbandonedStable, ItemIds::Matches);
	placeItem(RoomIds::MainStreet, ItemIds::BoltCutter);
	placeItem(RoomIds::MainStreet, ItemIds::PotatoSack);
	placeItem(RoomIds::Saloon, ItemIds::SmallKey);
	placeItem(RoomIds::Saloon, ItemIds::SafeBox);
	placeItem(RoomIds::SheriffOffice, ItemIds::Revolver);
	placeItem(RoomIds::SheriffOffice, ItemIds::Ammunition);
	placeItem(RoomIds::SheriffOffice, ItemIds::SheriffDiary);
	placeItem(RoomIds::BackCell, ItemIds::SilverCross);
	placeItem(RoomIds::OldChurch, ItemIds::EliasHat);
}

void WorldBuilder::AddRoom(WorldData& world, const std::shared_ptr<Room>& room)
{
	assert(room != nullptr);
	if (room == nullptr)
	{
		return;
	}

	world.rooms[room->GetId()] = room;
}

void WorldBuilder::AddItem(WorldData& world, const std::shared_ptr<Item>& item)
{
	assert(item != nullptr);
	if (item == nullptr)
	{
		return;
	}

	world.items[item->GetId()] = item;
}
