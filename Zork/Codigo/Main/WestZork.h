#pragma once

#include "GameWorld.h"
#include "Parser.h"

class WestZork
{

public:

	WestZork();

	void Run();

private:

	GameWorld m_world;
	Parser m_parser;
};
