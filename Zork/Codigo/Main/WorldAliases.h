#pragma once

#include <string>
#include <vector>

/*
Player-facing aliases for scenario elements that are not represented by Item.
They are kept separate from WorldIds.h because aliases may change without
changing the stable identifiers used by gameplay rules.
*/
namespace WorldAliases
{
	const std::vector<std::string> CellDoor =
	{
		"celda",
		"celda trasera",
		"puerta",
		"puerta celda",
		"puerta de celda",
		"puerta de la celda",
		"puerta de la celda trasera"
	};

	const std::vector<std::string> CryptLock =
	{
		"cerradura",
		"cerradura de la cripta",
		"cerradura del muro norte",
		"acceso",
		"acceso a la cripta",
		"acceso de la cripta",
		"muro norte",
		"acceso del muro norte"
	};

	const std::vector<std::string> ChurchChains =
	{
		"cadena",
		"cadenas",
		"puerta de la iglesia",
		"entrada de la iglesia",
		"acceso de la iglesia"
	};

	const std::vector<std::string> Sheriff =
	{
		"sheriff",
		"enemigo",
		"secuestrador"
	};
}
