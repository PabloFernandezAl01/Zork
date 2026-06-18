#include "WestZork.h"

#include <iostream>
#include <string>

WestZork::WestZork() {}

void WestZork::Run()
{
	std::cout << "West Zork\n";
	std::cout << "Escribe \"ayuda\" para ver los comandos disponibles.\n\n";
	GameResult result = m_world.Look(std::cout);

	while (result == GameResult::Running)
	{
		std::cout << "\n> ";

		std::string input;
		if (!std::getline(std::cin, input))
		{
			result = GameResult::Quit;
			break;
		}

		// Create a command from the raw user input
		const Command command = m_parser.Parse(input);

		// Take the required action from that command, if any
		result = m_world.ExecuteCommand(command, std::cout);
	}

	switch (result)
	{
	case GameResult::Victory:
		std::cout << "Has ganado.\n";
		break;
	case GameResult::Defeat:
		std::cout << "Has perdido.\n";
		break;
	case GameResult::Quit:
		std::cout << "Fin de la partida.\n";
		break;
	case GameResult::FatalError:
		std::cout << "La partida ha terminado debido a un error interno.\n";
		break;
	case GameResult::Running:
		std::cout << "Fin de la partida.\n";
		break;
	}
}
