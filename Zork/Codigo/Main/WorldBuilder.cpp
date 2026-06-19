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
		"El viejo arco de madera anuncia YELLOWVILLE con tres letras torcidas y un optimismo conmovedor. "
		"El camino esta cubierto de polvo, no hay un alma a la vista y hasta los buitres parecen guardar las distancias."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::AbandonedStable,
		"Establo abandonado",
		"El establo se inclina hacia un lado como si estuviera cansado de fingir que sigue en pie. "
		"Huele a heno podrido, madera humeda y decisiones tomadas por caballos."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::MainStreet,
		"Calle Principal",
		"La Calle Principal cruza Yellowville de lado a lado, aunque lo de \"principal\" resulta bastante generoso. "
		"Las ventanas estan cerradas, las fachadas vacias y alguien ha dejado unas cadenas nuevas en la puerta de la iglesia."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::Saloon,
		"Saloon",
		"Las puertas del saloon crujen con el viento, porque nadie se ha molestado en recibirte. "
		"Botellas rotas, mesas volcadas y una barra abandonada cuentan una noche pesima sin necesidad de testigos."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::SheriffOffice,
		"Oficina del Sheriff",
		"Una oficina pequena y demasiado ordenada para un pueblo que se esta cayendo a pedazos. "
		"Hay papeles sobre el escritorio y una puerta con barrotes al fondo. Little Bill siempre aprecio el orden, "
		"sobre todo cuando podia imponerlo a tiros."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::BackCell,
		"Celda trasera",
		"Una celda estrecha donde la humedad ha ganado hace tiempo. Las paredes conservan aranazos, "
		"pero ninguna explicacion amable sobre quien los hizo."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::OldChurch,
		"Iglesia vieja",
		"El campanario esta torcido, los bancos cubiertos de polvo y la fe parece haberse marchado antes que los vecinos. "
		"En el muro norte hay un acceso sellado con una cerradura en forma de cruz. Sutil no es."));

	AddRoom(world, std::make_shared<Room>(
		RoomIds::Crypt,
		"Cripta al norte de la iglesia",
		"El pasadizo desemboca en una cripta fria. Ned Munny esta atado contra el muro y Little Bill, "
		"que al parecer confunde la justicia con no dejar testigos, te apunta con su revolver."));

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
		"Una botella vacia con la etiqueta casi borrada. No resolvera el misterio, pero resume bastante bien el pueblo.");
	whiskyBottle->AddAlias("botella");
	whiskyBottle->AddAlias("whisky");

	std::shared_ptr<Item> tornMap = std::make_shared<Item>(
		ItemIds::TornMap,
		"Mapa rasgado",
		"Un mapa incompleto de Yellowville. Le falta una esquina y le sobra polvo, pero aun distingue la iglesia, "
		"el saloon y la oficina del sheriff. Para ser papel mojado, sabe bastante.");
	tornMap->AddAlias("mapa");
	tornMap->SetRequiresLightToExamine(true);

	std::shared_ptr<Item> lantern = std::make_shared<Item>(
		ItemIds::Lantern,
		"Farol",
		"Un farol viejo con combustible suficiente. Una extravagancia tecnologica para quien prefiera ver el peligro venir.");
	lantern->AddAlias("farol");
	lantern->AddAlias("lampara");
	lantern->AddAlias("linterna");
	lantern->SetLightState(LightState::Off);

	std::shared_ptr<Item> matches = std::make_shared<Item>(
		ItemIds::Matches,
		"Cerillas",
		"Una pequena caja de cerillas humedecida por fuera. Por dentro, milagrosamente, algunas siguen secas.");
	matches->AddAlias("cerilla");

	std::shared_ptr<Item> boltCutter = std::make_shared<Item>(
		ItemIds::BoltCutter,
		"Cizalla oxidada",
		"Una cizalla pesada con las hojas comidas por el oxido. Fea, incomoda y mucho mas convincente que una llave.");
	boltCutter->AddAlias("cizalla");

	std::shared_ptr<Item> potatoSack = std::make_shared<Item>(
		ItemIds::PotatoSack,
		"Saco de patatas podridas",
		"Un saco de arpillera lleno de patatas podridas. El olor podria desalojar el pueblo si el pueblo no estuviera ya desalojado.");
	potatoSack->AddAlias("saco");
	potatoSack->AddAlias("saco de patatas");
	potatoSack->AddAlias("patatas");
	potatoSack->SetContainerState(ContainerState::Closed);

	std::shared_ptr<Item> smallKey = std::make_shared<Item>(
		ItemIds::SmallKey,
		"Llave pequena",
		"Una llave pequena de hierro ennegrecido. En Yellowville, las llaves pequenas suelen proteger secretos grandes. "
		"O cajones mediocres. Emocionante, en cualquier caso.");
	smallKey->AddAlias("llave");
	smallKey->AddAlias("llave pequena");

	std::shared_ptr<Item> bartenderNote = std::make_shared<Item>(
		ItemIds::BartenderNote,
		"Nota del tabernero",
		"La nota esta escrita deprisa: \"Little Bill se lleva a cualquiera que haga demasiadas preguntas. "
		"Anoche oi a Ned Munny gritar desde la celda trasera de su oficina. Si alguien encuentra esto, que no confie en la placa.\"");
	bartenderNote->AddAlias("nota");
	bartenderNote->AddAlias("nota del tabernero");
	bartenderNote->SetRequiresLightToExamine(true);

	std::shared_ptr<Item> safeBox = std::make_shared<Item>(
		ItemIds::SafeBox,
		"Caja fuerte",
		"Una caja fuerte compacta tras la barra. La cerradura es pequena y casi parece pedir una llave pequena. "
		"El suspense tiene dias poco inspirados.");
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
		"Un revolver bien cuidado, frio al tacto y sin una sola bala en el tambor. La parte de disparar se vende por separado.");
	revolver->AddAlias("revolver");
	revolver->SetLoadState(LoadState::Unloaded);

	std::shared_ptr<Item> ammunition = std::make_shared<Item>(
		ItemIds::Ammunition,
		"Municion",
		"Unas pocas balas envueltas en papel aceitado. Justo lo que le falta a un revolver descargado. Casualidades del Oeste.");
	ammunition->AddAlias("balas");

	std::shared_ptr<Item> sheriffDiary = std::make_shared<Item>(
		ItemIds::SheriffDiary,
		"Diario del sheriff",
		"Una entrada reciente dice: \"He llevado a Ned Munny a la cripta bajo la iglesia. "
		"La cruz de plata que abre el muro norte queda oculta en la celda. Al amanecer, Yellowville tendra un problema menos.\" "
		"Little Bill escribe sus crimenes con una caligrafia impecable. Todo un profesional.");
	sheriffDiary->AddAlias("diario");
	sheriffDiary->SetRequiresLightToExamine(true);

	std::shared_ptr<Item> silverCross = std::make_shared<Item>(
		ItemIds::SilverCross,
		"Cruz de plata",
		"Una cruz de plata pequena, demasiado limpia para esta celda. Sus bordes tienen la forma precisa de una llave, "
		"porque hasta la Iglesia disfruta complicando las puertas.");
	silverCross->AddAlias("cruz");

	std::shared_ptr<Item> nedMunnyHat = std::make_shared<Item>(
		ItemIds::NedMunnyHat,
		"Sombrero de Ned Munny",
		"El sombrero de Ned. Reconocerias esa cinta azul en cualquier parte y esa mancha de whisky en dos condados. "
		"Tu hermano estuvo aqui, y no parece que se marchara por voluntad propia.");
	nedMunnyHat->AddAlias("sombrero");

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
	AddItem(world, nedMunnyHat);
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
	placeItem(RoomIds::OldChurch, ItemIds::NedMunnyHat);
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
