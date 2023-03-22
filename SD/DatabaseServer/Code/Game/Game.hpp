#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"

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
	void RenderAttract() const;
	void RenderGameplay() const;
	void HandleCommonInput();
	void SwitchToGameplayMode();
	void ResetGame();

private:
	void RenderPausePanel() const;

private:
	App* m_theOwner = nullptr;
	GameMode m_currentGameMode = GameMode::ATTRACT;
	GameMode m_nextGameMode = GameMode::ATTRACT;
	bool m_isSlowMode = false;
	bool m_isFastMode = false;
	Camera m_worldCamera;
	Camera m_UICamera;
	Clock m_gameClock;
};