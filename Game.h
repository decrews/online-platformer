#pragma once
#include <vector>
#include "Level.h"
#include "Player.h"

class Game
{
public:
	Game();
	~Game();


	std::vector<Level*> levels;
	Level* currentLevel;
	Player* player;
};

