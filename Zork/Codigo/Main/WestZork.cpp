#include "WestZork.h"

#include <iostream>
#include <string>

WestZork::WestZork() {}

void WestZork::Run()
{
	bool isRunning = true;

	std::cout << "West Zork\n";
	std::cout << "Escribe \"ayuda\" para ver los comandos disponibles.\n\n";
	m_world.Look(std::cout, isRunning);

	while (isRunning)
	{
		std::cout << "\n> ";

		std::string input;
		if (!std::getline(std::cin, input))
		{
			break;
		}

		// Create a command from the raw user input
		const Command command = m_parser.Parse(input);

		// Take the required action from that command, if any
		m_world.ExecuteCommand(command, isRunning, std::cout);
	}

	std::cout << "Fin de la partida.\n";
}
