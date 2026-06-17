#pragma once

#include "Room.h" // <--- To reuse Direction struct

#include <string>

// All type of actions required for WestZork
enum class CommandType
{
	Move,
	Look,
	Examine,
	Inventory,
	Take,
	Drop,
	Put,
	Remove,
	Help,
	Quit,
	Unknown
};

struct Command
{
	CommandType type = CommandType::Unknown;
	Direction direction = Direction::North;
	std::string firstTarget;
	std::string secondTarget;

	/*
	* Examples:
	* 
	* 1.- "Coger botella": 
	* 
	*	+ type would be CommandType::Take
	*   + direction would be Direction::North (default value)
	*   + firstTarget would be "botella"
	*   + secondTarget would be empty (default value)
	* 
	* 2.- "Este": 
	* 
	*	+ type would be CommandType::Move
	*   + direction would be Direction::East
	*   + firstTarget would be empty (default value)
	*   + secondTarget would be empty (default value)
	* 
	* 3.- "Meter botella en bolsa":
	*
	*	+ type would be CommandType::Put
	*   + direction would be Direction::North (default value)
	*   + firstTarget would be "botella"
	*   + secondTarget would be "bolsa"
	*/
};

class Parser
{
public:

	// Converts the raw player input into a Command.
	// Examples: "norte" -> Move North, "coger mapa" -> Take "mapa",
	// "meter llave en caja" -> Put "llave" into "caja".
	Command Parse(const std::string& input) const;

private:

	// Trims leading/trailing whitespace and converts the text to lowercase.
	// Examples: "  Coger Mapa  " -> "coger mapa", "NORTE" -> "norte".
	static std::string Normalize(const std::string& text);

	// Removes spaces, tabs and line breaks from the beginning and end of a string.
	// Examples: "  mirar" -> "mirar", "inventario   " -> "inventario".
	static std::string Trim(const std::string& text);

	// Checks whether the whole text is a movement command and writes its Direction.
	// Examples: "n" -> North, "oeste" -> West, "abajo" -> Down.
	static bool TryParseDirection(const std::string& text, Direction& direction);
};
