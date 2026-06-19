#include "WestZork.h"

#include "AsciiArt.h"

#include <iostream>
#include <string>

WestZork::WestZork() {}

void WestZork::Run()
{
	std::cout << "-------------------------- WEST ZORK ------------------------------\n\n";
	for (const char* artChunk : Presentation::WestZorkAsciiArt)
	{
		std::cout << artChunk;
	}
	std::cout << '\n';
	std::cout << "Yellowville, Texas. 1870.\n\n";
	std::cout << "Regresas de visitar a un viejo amigo y encuentras el pueblo vacio.\n";
	std::cout << "Tu hermano Ned no ha ido a recibirte. Tampoco esta en casa.\n";
	std::cout << "El silencio tiene esa desagradable costumbre de parecer culpable.\n\n";
	std::cout << "Eres Willian Munny, antiguo alguacil, retirado por voluntad propia\n";
	std::cout << "y reincorporado por la obstinada tendencia de tu familia a meterse\n";
	std::cout << "en problemas. Tendras que averiguar que ha ocurrido con Ned.\n";
	std::cout << "Yellowville, con su conocida hospitalidad, no piensa ponertelo facil.\n\n";
	std::cout << "Escribe \"ayuda\" si necesitas que el juego te explique como jugar.\n";
	std::cout << "Promete no juzgarte demasiado.\n\n";

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
		std::cout << "\nHas ganado. Ned vive, Little Bill no y Yellowville tendra que buscarse\n";
		std::cout << "otro sheriff. Considerando su historial, seguro que sale estupendamente.\n";
		break;
	case GameResult::Defeat:
		std::cout << "\nHas perdido. La proxima vez comprueba el tambor antes de protagonizar\n";
		std::cout << "un duelo. Si, el consejo llega tarde. De nada.\n";
		break;
	case GameResult::Quit:
		std::cout << "Abandonas la busqueda. Yellowville vuelve a quedarse en silencio,\n";
		std::cout << "que es justo como le gusta guardar sus cadaveres y sus secretos.\n";
		break;
	case GameResult::FatalError:
		std::cout << "La partida ha terminado por un error interno. Ni siquiera Yellowville\n";
		std::cout << "puede fingir que esto formaba parte del plan.\n";
		break;
	case GameResult::Running:
		std::cout << "Fin de la partida. Una conclusion tecnicamente correcta y narrativamente deplorable.\n";
		break;
	}
}
