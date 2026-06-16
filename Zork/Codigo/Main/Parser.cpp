#include "Parser.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

Command Parser::Parse(const std::string& input) const
{
	const std::string normalizedInput = Normalize(input);

	Command command;
	if (normalizedInput.empty())
	{
		return command;
	}

	// Checks if the input has any direction indication
	Direction direction;
	if (TryParseDirection(normalizedInput, direction))
	{
		command.type = CommandType::Move;
		command.direction = direction;
		return command;
	}

	// Split the player input into words so tokens[0] can be treated as the verb
	// and the following tokens as command targets. Example: "coger mapa" -> ["coger", "mapa"].
	std::istringstream stream(normalizedInput);
	std::vector<std::string> tokens;
	std::string token;
	while (stream >> token)
	{
		tokens.push_back(token);
	}

	if (tokens.empty())
	{
		return command;
	}

	const std::string& verb = tokens[0];

	if (verb == "mirar" || verb == "m")
	{
		command.type = CommandType::Look;
		return command;
	}

	if (verb == "inventario" || verb == "i")
	{
		command.type = CommandType::Inventory;
		return command;
	}

	if (verb == "ayuda" || verb == "help")
	{
		command.type = CommandType::Help;
		return command;
	}

	if (verb == "quit" || verb == "terminar")
	{
		command.type = CommandType::Quit;
		return command;
	}

	if ((verb == "examinar" || verb == "x") && tokens.size() >= 2)
	{
		command.type = CommandType::Examine;
		command.firstTarget = tokens[1];
		return command;
	}

	if ((verb == "coger" || verb == "tomar") && tokens.size() >= 2)
	{
		command.type = CommandType::Take;
		command.firstTarget = tokens[1];
		return command;
	}

	if (verb == "soltar" && tokens.size() >= 2)
	{
		command.type = CommandType::Drop;
		command.firstTarget = tokens[1];
		return command;
	}

	if (verb == "meter" && tokens.size() >= 4 && tokens[2] == "en")
	{
		command.type = CommandType::Put;
		command.firstTarget = tokens[1];
		command.secondTarget = tokens[3];
		return command;
	}

	if (verb == "sacar" && tokens.size() >= 4 && tokens[2] == "de")
	{
		command.type = CommandType::Remove;
		command.firstTarget = tokens[1];
		command.secondTarget = tokens[3];
		return command;
	}

	return command;
}

std::string Parser::Normalize(const std::string& text)
{
	std::string normalizedText = Trim(text);

	std::transform(normalizedText.begin(), normalizedText.end(), normalizedText.begin(),
		[](unsigned char character)
		{
			return static_cast<char>(std::tolower(character));
		});

	return normalizedText;
}

std::string Parser::Trim(const std::string& text)
{
	const std::string whitespace = " \t\n\r";
	const std::string::size_type firstCharacter = text.find_first_not_of(whitespace);

	if (firstCharacter == std::string::npos)
	{
		return "";
	}

	const std::string::size_type lastCharacter = text.find_last_not_of(whitespace);
	return text.substr(firstCharacter, lastCharacter - firstCharacter + 1);
}

bool Parser::TryParseDirection(const std::string& text, Direction& direction)
{
	if (text == "n" || text == "norte")
	{
		direction = Direction::North;
		return true;
	}

	if (text == "s" || text == "sur")
	{
		direction = Direction::South;
		return true;
	}

	if (text == "e" || text == "este")
	{
		direction = Direction::East;
		return true;
	}

	if (text == "o" || text == "oeste")
	{
		direction = Direction::West;
		return true;
	}

	if (text == "abajo")
	{
		direction = Direction::Down;
		return true;
	}

	if (text == "entrar")
	{
		direction = Direction::Enter;
		return true;
	}

	if (text == "salir")
	{
		direction = Direction::Exit;
		return true;
	}

	return false;
}
