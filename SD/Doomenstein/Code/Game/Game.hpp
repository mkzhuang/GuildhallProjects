#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Game/Map.hpp"

class Player;

enum class GameMode
{
	INVALID_MODE = -1,

	ATTRACT,
	LOBBY,
	GAMEPLAY,

	NUM_MODES
};

class Game
{
public:
	Game(App* const& owner);
	~Game() {}
	void Startup();

	void Update();
	void UpdateGameMode();
	void UpdateAttract(float deltaSeconds);
	void UpdateLobby(float deltaSeconds);
	void UpdateGameplay(float deltaSeconds);
	void Render() const;
	void RenderUI() const;
	void RenderAttract() const;
	void RenderLobby() const;
	void RenderGameplay() const;
	void HandleCommonInput();
	void SwitchToLobbyMode();
	void SwitchToGameplayMode();
	void ResetGame();
	float GetTotalGameTime() const;
	Clock& GetGameClock();
	void AddPlayer(Player* player);
	Player* GetPlayer(int index) const;
	int GetPlayerCount() const;
	std::vector<Player*> GetAllPlayer() const;
	int GetAlivePlayerCount() const;

private:
	void RenderPausePanel() const;
	void LoadConfig();
	void LoadAssets();
	static bool Event_SpawnScreenMessage(EventArgs& args);
	static bool Command_Keys(EventArgs& args);

private:
	App* m_theOwner = nullptr;
	Map* m_currentMap = nullptr;
	GameMode m_currentGameMode = GameMode::ATTRACT;
	GameMode m_nextGameMode = GameMode::ATTRACT;
	bool m_isSlowMode = false;
	bool m_isFastMode = false;
	bool m_isRaycastDebugOn = true;
	Camera m_worldCamera;
	Camera m_UICamera;
	Clock m_gameClock;
	std::vector<Player*> m_players;
	std::vector<Camera*> m_playerCamera;
	std::vector<Camera*> m_playerUICamera;
	int m_playerCounter = 0;
	Stopwatch m_endGameWatch;
};


