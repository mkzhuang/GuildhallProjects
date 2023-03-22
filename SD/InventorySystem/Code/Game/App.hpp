#pragma once
#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

class Game;

class App
{
public:
	App();
	~App();
	void Startup();
	void RunFrame();
	void Shutdown();
	void Run();
	void Quit();

	bool IsQuitting() const {return g_isQuitting;};
	void HandleDeveloperCheatCode();
	void HandleQuitRequested();

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

private:
	Game* m_theGame = nullptr;
	Camera m_devCamera;
	double m_timeLastFrame = 0.0;
};

static bool Event_QuitApp(EventArgs& args);


