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

	if ((verb == "mirar" || verb == "m") && tokens.size() == 1)
	{
		command.type = CommandType::Look;
		return command;
	}

	if ((verb == "inventario" || verb == "i") && tokens.size() == 1)
	{
		command.type = CommandType::Inventory;
		return command;
	}

	if ((verb == "ayuda" || verb == "help") && tokens.size() == 1)
	{
		command.type = CommandType::Help;
		return command;
	}

	if ((verb == "quit" || verb == "terminar") && tokens.size() == 1)
	{
		command.type = CommandType::Quit;
		return command;
	}

	if ((verb == "examinar" || verb == "x") && tokens.size() >= 2)
	{
		if (TryParseSingleTargetCommand(tokens, CommandType::Examine, command))
		{
			return command;
		}
	}

	if ((verb == "coger" || verb == "tomar") && tokens.size() >= 2)
	{
		if (TryParseSingleTargetCommand(tokens, CommandType::Take, command))
		{
			return command;
		}
	}

	if (verb == "soltar" && tokens.size() >= 2)
	{
		if (TryParseSingleTargetCommand(tokens, CommandType::Drop, command))
		{
			return command;
		}
	}

	if (verb == "meter")
	{
		if (TryParseTwoTargetCommand(tokens, "en", CommandType::Put, command))
		{
			return command;
		}
	}

	if (verb == "sacar")
	{
		if (TryParseTwoTargetCommand(tokens, "de", CommandType::Remove, command))
		{
			return command;
		}
	}

	if (verb == "abrir")
	{
		if (TryParseTwoTargetCommand(tokens, "con", CommandType::Open, command) ||
			TryParseSingleTargetCommand(tokens, CommandType::Open, command))
		{
			return command;
		}
	}

	if (verb == "encender")
	{
		if (TryParseSingleTargetCommand(tokens, CommandType::TurnOn, command))
		{
			return command;
		}
	}

	if (verb == "cargar")
	{
		if (TryParseTwoTargetCommand(tokens, "con", CommandType::Load, command) ||
			TryParseSingleTargetCommand(tokens, CommandType::Load, command))
		{
			return command;
		}
	}

	if (verb == "romper")
	{
		if (TryParseTwoTargetCommand(tokens, "con", CommandType::Break, command) ||
			TryParseSingleTargetCommand(tokens, CommandType::Break, command))
		{
			return command;
		}
	}

	if (verb == "disparar")
	{
		if (TryParseTwoTargetCommand(tokens, "con", CommandType::Shoot, command) ||
			TryParseSingleTargetCommand(tokens, CommandType::Shoot, command))
		{
			return command;
		}
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

	return false;
}

std::string Parser::JoinTokens(const std::vector<std::string>& tokens, std::size_t firstToken, std::size_t lastToken)
{
	// The range is half-open: [firstToken, lastToken). This matches STL conventions
	// and makes it easy to pass connectorIndex as the end of the first target.
	std::string joinedText;

	for (std::size_t i = firstToken; i < lastToken; ++i)
	{
		if (!joinedText.empty())
		{
			joinedText += ' ';
		}

		joinedText += tokens[i];
	}

	return joinedText;
}

std::string Parser::RemoveLeadingArticle(const std::string& target)
{
	// Articles and simple prepositions are player-facing noise, not part of the
	// object name. Removing them lets "disparar al sheriff" match "sheriff".
	static const std::vector<std::string> articles =
	{
		"el ",
		"la ",
		"los ",
		"las ",
		"un ",
		"una ",
		"unos ",
		"unas ",
		"a ",
		"al ",
		"del "
	};

	std::string cleanTarget = target;
	bool removedArticle = true;
	while (removedArticle)
	{
		// Repeat until no prefix is removed so combined forms such as
		// "a el sheriff" become just "sheriff".
		removedArticle = false;
		for (const std::string& article : articles)
		{
			if (cleanTarget.compare(0, article.size(), article) == 0)
			{
				cleanTarget = cleanTarget.substr(article.size());
				removedArticle = true;
				break;
			}
		}
	}

	return cleanTarget;
}

std::string Parser::BuildTarget(const std::vector<std::string>& tokens, std::size_t firstToken, std::size_t lastToken)
{
	// Every command target goes through the same cleanup path so item matching
	// stays predictable for one-target and two-target commands.
	return RemoveLeadingArticle(Trim(JoinTokens(tokens, firstToken, lastToken)));
}

bool Parser::TryFindLastToken(const std::vector<std::string>& tokens, const std::string& targetToken, std::size_t& tokenIndex)
{
	// Use the last connector so object names can contain connector words.
	// Example: "sacar cruz de plata de caja fuerte" splits at the final "de".
	for (std::size_t i = tokens.size(); i > 0; --i)
	{
		const std::size_t currentIndex = i - 1;
		if (tokens[currentIndex] == targetToken)
		{
			tokenIndex = currentIndex;
			return true;
		}
	}

	return false;
}

bool Parser::TryParseSingleTargetCommand(const std::vector<std::string>& tokens, CommandType type, Command& command)
{
	if (tokens.size() < 2)
	{
		return false;
	}

	const std::string target = BuildTarget(tokens, 1, tokens.size());
	if (target.empty())
	{
		return false;
	}

	command.type = type;
	command.firstTarget = target;
	return true;
}

bool Parser::TryParseTwoTargetCommand(const std::vector<std::string>& tokens, const std::string& connector, CommandType type, Command& command)
{
	std::size_t connectorIndex = 0;
	if (!TryFindLastToken(tokens, connector, connectorIndex))
	{
		return false;
	}

	if (connectorIndex <= 1 || connectorIndex + 1 >= tokens.size())
	{
		// Invalid forms like "meter en caja" or "romper cadenas con" do not
		// produce partial commands; GameWorld should only receive clear intent.
		return false;
	}

	const std::string firstTarget = BuildTarget(tokens, 1, connectorIndex);
	const std::string secondTarget = BuildTarget(tokens, connectorIndex + 1, tokens.size());
	if (firstTarget.empty() || secondTarget.empty())
	{
		return false;
	}

	command.type = type;
	command.firstTarget = firstTarget;
	command.secondTarget = secondTarget;
	return true;
}
