#pragma once
#include "Game/World.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"

class Game
{
public:
	Game(App* const& owner);
	~Game() {}
	void Startup();

	void Update();
	void Render() const;
	void HandleKeyPressed();
	void HandleControllerInput();
	void UpdateCamera();
	void GoToNextLevel();
	void SetGameWin();
	void SetGameLose();
	void ResetGame();
	void LoadAssets();
	void DrawPausePanel() const;
	void DrawAttractScreen() const;
	void DrawSelectTriangle(Vec2 pos, Rgba8 color) const;
	void DrawVictoryScreen() const;
	void DrawDefeatScreen() const;

private:
	App* m_theApp = nullptr;
	World* m_currentWorld = nullptr;
	bool m_isAttractMode = true;
	bool m_isStartOrExit = true;
	bool m_isWinning = false;
	bool m_isPerished = false;
	bool m_isSlowMode = false;
	bool m_isFastMode = false;
	float m_joystickMenuDelay = 0.f;
	float m_perishedTimer = 0.f;
	Camera m_attractCamera;
	Clock m_gameClock;
};