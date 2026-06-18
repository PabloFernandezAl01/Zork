#pragma once

#include "Room.h" // <--- To reuse Direction struct

#include <cstddef>
#include <string>
#include <vector>

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
	Open,
	TurnOn,
	Load,
	Break,
	Shoot,
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

	// Joins a range of tokens into one target sentence.
	// Examples: ["caja", "fuerte"] -> "caja fuerte".
	static std::string JoinTokens(const std::vector<std::string>& tokens, std::size_t firstToken, std::size_t lastToken);

	// Removes common Spanish articles/prepositions from the beginning of a target.
	// Examples: "la caja fuerte" -> "caja fuerte", "a el sheriff" -> "sheriff".
	static std::string RemoveLeadingArticle(const std::string& target);

	// Builds a clean target sentence from a token range.
	static std::string BuildTarget(const std::vector<std::string>& tokens, std::size_t firstToken, std::size_t lastToken);

	// Finds the last occurrence of a connector token, such as "en", "de" or "con".
	static bool TryFindLastToken(const std::vector<std::string>& tokens, const std::string& targetToken, std::size_t& tokenIndex);

	// Parses commands with one target after the verb.
	static bool TryParseSingleTargetCommand(const std::vector<std::string>& tokens, CommandType type, Command& command);

	// Parses commands with two targets separated by a connector.
	static bool TryParseTwoTargetCommand(const std::vector<std::string>& tokens, const std::string& connector, CommandType type, Command& command);
};
