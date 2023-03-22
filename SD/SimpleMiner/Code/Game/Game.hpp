#pragma once
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"

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
	Camera* GetCamera();

private:
	void RenderPausePanel() const;
	static bool Event_SpawnScreenMessage(EventArgs& args);

private:
	App* m_theOwner = nullptr;
	GameMode m_currentGameMode = GameMode::ATTRACT;
	GameMode m_nextGameMode = GameMode::ATTRACT;
	bool m_isSlowMode = false;
	bool m_isFastMode = false;
	Camera m_worldCamera;
	Camera m_UICamera;
	Clock m_gameClock;
	World* m_currentWorld = nullptr;
};


