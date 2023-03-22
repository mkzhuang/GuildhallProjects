#pragma once
#include "Game/Map.hpp"

class Game;
class Player;

class World
{
public:
	World(Game* owner);
	~World() {}
	void Startup();

	void Update(float deltaSeconds);
	void Render() const;
	void GoToNextLevel();
	void SpawnPlayer();

private:
	Game* m_game = nullptr;
	Map* m_currentMap = nullptr;
	std::vector<Map*> m_maps;
	int m_mapCounter = 0;
};


