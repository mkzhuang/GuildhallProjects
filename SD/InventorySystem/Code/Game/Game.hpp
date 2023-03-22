#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"

class Prop;
class Player;
class InventorySystem;
class Chest;
class WorldInventory;

enum class GameMode
{
	INVALID_MODE = -1,

	ATTRACT,
	GAMEPLAY,

	NUM_MODES
};

class Game
{
public:
	Game(App* const& owner);
	~Game() {}
	void Startup();
	void Shuntdown();

	void Update();
	void UpdateGameMode();
	void UpdateAttract(float deltaSeconds);
	void UpdateGameplay(float deltaSeconds);
	void UpdateCamera(float deltaSeconds);
	void Render() const;
	void RenderUI() const;
	void RenderAttract() const;
	void RenderGameplay() const;
	void HandleCommonInput();
	void SwitchToGameplayMode();
	void ResetGame();
	float GetTotalGameTime() const;

private:
	void RenderPausePanel() const;
	void SetRoomVerts();

private:
	App* m_theOwner = nullptr;
	GameMode m_currentGameMode = GameMode::ATTRACT;
	GameMode m_nextGameMode = GameMode::ATTRACT;
	bool m_isSlowMode = false;
	bool m_isFastMode = false;
	Camera m_worldCamera;
	Camera m_UICamera;
	Clock m_gameClock;
	Stopwatch m_fpsWatch;
	Player* m_player = nullptr;
	AABB3 m_bounds;
	std::vector<Vertex_PCU> m_roomVerts;
	InventorySystem* m_inventory = nullptr;
	Chest* m_chest = nullptr;
	WorldInventory* m_worldInventory = nullptr;
};



