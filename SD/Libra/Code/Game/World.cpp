#include "Game/World.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"

World::World(Game* owner)
	:m_game(owner)
{
}


void World::Startup()
{
	m_maps.push_back(new Map(this, MapDefinition::GetDefinitionByName("Approach")));
	m_maps.push_back(new Map(this, MapDefinition::GetDefinitionByName("Courtyard")));
	m_maps.push_back(new Map(this, MapDefinition::GetDefinitionByName("Tunnel")));
	m_maps.push_back(new Map(this, MapDefinition::GetDefinitionByName("Sanctum")));
	m_maps.push_back(new Map(this, MapDefinition::GetDefinitionByName("Everything")));
	m_currentMap = m_maps[m_mapCounter];
	float playerOrientation = g_gameConfigBlackboard.GetValue("playerStartOrientation", 0.f);
	Player* player = new Player(m_currentMap, Vec2(1.5f, 1.5f), playerOrientation);
	m_currentMap->Startup(player);
}


void World::Update(float deltaSeconds)
{
	m_currentMap->Update(deltaSeconds);
	if (!m_currentMap->IsPlayerAlive())
	{
		m_game->SetGameLose();
		return;
	}
}


void World::Render() const
{
	m_currentMap->Render();
}


void World::GoToNextLevel()
{
	m_mapCounter++;
	if (m_mapCounter >= m_maps.size())
	{
		m_game->SetGameWin();
		return;
	}

	PlaySound(g_soundIds[SOUND_TYPE_LEVEL_END], false, g_gameSoundVolume);
	Player* player = m_currentMap->GetPlayer();
	m_currentMap = m_maps[m_mapCounter];
	m_currentMap->Startup(player);
}


void World::SpawnPlayer()
{
	m_currentMap->SpawnPlayer();
}


