#pragma once

#include <string>

// Cardinal movement directions shared by the parser and the world model.
enum class Direction
{
	North,
	South,
	East,
	West
};

struct DirectionUtils
{
	static std::string ToText(Direction direction)
	{
		switch (direction)
		{
		case Direction::North:
			return "norte";
		case Direction::South:
			return "sur";
		case Direction::East:
			return "este";
		case Direction::West:
			return "oeste";
		default:
			return "desconocida";
		}
	}
};
