#pragma once

#include "GameWorld.h"
#include "Parser.h"

class WestZork
{

public:

	WestZork();
	WestZork(const WestZork&) = delete;
	WestZork& operator=(const WestZork&) = delete;
	void Run();

private:

	GameWorld m_world;
	Parser m_parser;
};
