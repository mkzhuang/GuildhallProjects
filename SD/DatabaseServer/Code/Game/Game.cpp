#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

RandomNumberGenerator RNG;

SoundID attractMusic;
SoundID gameMusic;
SoundID selectSound;
SoundID pauseSound;
SoundID unpauseSound;
SoundPlaybackID musicPlaybackID;

static float worldCameraSizeX = 0.f;
static float worldCameraSizeY = 0.f;
static float worldCameraCenterX = 0.f;
static float worldCameraCenterY = 0.f;
static float UICameraSizeX = 0.f;
static float UICameraSizeY = 0.f;
static float UICameraCenterX = 0.f;
static float UICameraCenterY = 0.f;
static double slowMoMultiplier = 0.f;
static double fastMoMultiplier = 0.f;
static float joystickMenuDelay = 0.f;
static float joystickMenuSensitivity = 0.f;

Game::Game(App* const& owner)
	: m_theOwner(owner)
	, m_gameClock(Clock::GetSystemClock())
{
	worldCameraSizeX = g_gameConfigBlackboard.GetValue("worldCameraDimensionX", 20.f);
	worldCameraSizeY = g_gameConfigBlackboard.GetValue("worldCameraDimensionY", 10.f);
	worldCameraCenterX = g_gameConfigBlackboard.GetValue("worldCameraCenterX", 10.f);
	worldCameraCenterY = g_gameConfigBlackboard.GetValue("worldCameraCenterY", 5.f);
	UICameraSizeX = g_gameConfigBlackboard.GetValue("UICameraDimensionX", 20.f);
	UICameraSizeY = g_gameConfigBlackboard.GetValue("UICameraDimensionY", 10.f);
	UICameraCenterX = g_gameConfigBlackboard.GetValue("UICameraCenterX", 10.f);
	UICameraCenterY = g_gameConfigBlackboard.GetValue("UICameraCenterY", 5.f);
	slowMoMultiplier = static_cast<double>(g_gameConfigBlackboard.GetValue("debugSlowMoMultiplier", 1.f));
	fastMoMultiplier = static_cast<double>(g_gameConfigBlackboard.GetValue("debugFastMoMultiplier", 1.f));
	joystickMenuDelay = g_gameConfigBlackboard.GetValue("joystickMenuDelay", 0.1f);
	joystickMenuSensitivity = g_gameConfigBlackboard.GetValue("joystickMenuSensitivity", 0.1f);
}


void Game::Startup()
{
}


void Game::Update()
{
	m_gameClock.SetTimeDilation(1.0);
	g_gameSoundPlaySpeed = 1.f;

	if (m_gameClock.IsPaused())
	{
		g_gameSoundPlaySpeed = 0.f;
	}
	else
	{
		if (m_isSlowMode)
		{
			m_gameClock.SetTimeDilation(slowMoMultiplier);
			g_gameSoundPlaySpeed *= static_cast<float>(slowMoMultiplier);
		}
		if (m_isFastMode)
		{
			m_gameClock.SetTimeDilation(fastMoMultiplier);
			g_gameSoundPlaySpeed *= static_cast<float>(fastMoMultiplier);
		}
	}

	float deltaSeconds = static_cast<float>(m_gameClock.GetDeltaTime());

	UpdateGameMode();

	switch (m_currentGameMode)
	{
		case GameMode::ATTRACT:		UpdateAttract(deltaSeconds); break;
		case GameMode::GAMEPLAY:	UpdateGameplay(deltaSeconds); break;
	}
	HandleCommonInput();

	UpdateCamera(deltaSeconds);
}


void Game::UpdateGameMode()
{
	if (m_currentGameMode != m_nextGameMode)
	{
		m_currentGameMode = m_nextGameMode;
	}
}


void Game::UpdateAttract(float deltaSeconds)
{
	UNUSED(deltaSeconds)

	if (g_theInput->WasKeyJustPressed('N'))
	{
		SwitchToGameplayMode();
		return;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->HandleQuitRequested();
	}

	XboxController const& controller = g_theInput->GetController(0);

	if (controller.WasButtonJustPressed(XboxButtonID::START) || controller.WasButtonJustPressed(XboxButtonID::BUTTON_A))
	{
		SwitchToGameplayMode();
		return;
	}

	if (controller.WasButtonJustPressed(XboxButtonID::BACK))
	{
		g_theApp->HandleQuitRequested();
	}
}


void Game::UpdateGameplay(float deltaSeconds)
{
	UNUSED(deltaSeconds)

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock.StepFrame();
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		if (m_gameClock.IsPaused())
		{
			m_gameClock.Unpause();
		}
		else
		{
			m_gameClock.Pause();
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		ResetGame();
	}

	XboxController const& controller = g_theInput->GetController(0);

	if (controller.WasButtonJustPressed(XboxButtonID::BACK))
	{
		ResetGame();
	}
}


void Game::UpdateCamera(float deltaSeconds)
{
	UNUSED(deltaSeconds)

	m_UICamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(UICameraSizeX, UICameraSizeY));
	m_worldCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(worldCameraSizeX, worldCameraSizeY));
}


void Game::Render() const
{
	switch (m_currentGameMode)
	{
		case GameMode::ATTRACT:		RenderAttract(); break;
		case GameMode::GAMEPLAY:	RenderGameplay(); break;
	}
}


void Game::RenderAttract() const
{
	g_theRenderer->BeginCamera(m_UICamera);
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	//float offset = SinDegrees(static_cast<float>(m_gameClock.GetTotalTime()) * 60.f) * 100.f;

	//std::vector<Vertex_PCU> titleVertexArray;
	//titleVertexArray.reserve(400);
	//AddVertsForTextTriangles2D(titleVertexArray, "Protogame2D", Vec2(32.f, 632.f), 80.f, Rgba8(50, 50, 50, static_cast<unsigned char>(120 - offset)));
	//AddVertsForTextTriangles2D(titleVertexArray, "Protogame2D", Vec2(40.f, 640.f), 80.f, Rgba8(100, 100, 255, static_cast<unsigned char>(150 - offset)));
	//AddVertsForTextTriangles2D(titleVertexArray, "Music: C418 - Wet Hands", Vec2(40.f, 40.f), 16.f, Rgba8(255, 255, 255, 255));
	//g_theRenderer->BindTexture(nullptr);
	//g_theRenderer->DrawVertexArray(int(titleVertexArray.size()), titleVertexArray.data());

	//Rgba8 ringColor(static_cast<unsigned char>(155 - offset), static_cast<unsigned char>(155 + offset), static_cast<unsigned char>(155 + offset), static_cast<unsigned char>(150 - offset));
	//DebugDrawRing(Vec2(800.f, 400.f), 160.f + offset, 20.f - 0.1f * offset, ringColor);

	g_theRenderer->EndCamera(m_UICamera);
}


void Game::RenderGameplay() const
{
	g_theRenderer->BeginCamera(m_worldCamera);
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));

	float offset = SinDegrees(static_cast<float>(m_gameClock.GetTotalTime()) * 60.f) * 12.5f;

	Rgba8 ringColor(static_cast<unsigned char>(155 + offset), static_cast<unsigned char>(155 - offset), static_cast<unsigned char>(155 - offset), static_cast<unsigned char>(150 + offset));
	DebugDrawRing(Vec2(100.f, 50.f), 20.f - offset, 2.5f + 0.1f * offset, ringColor);

	if (m_gameClock.IsPaused())
	{
		RenderPausePanel();
	}

	g_theRenderer->EndCamera(m_worldCamera);
}


void Game::HandleCommonInput()
{
	m_isSlowMode = g_theInput->IsKeyDown('T') ? true : false;
	m_isFastMode = g_theInput->IsKeyDown('Y') ? true : false;

	if (g_theInput->WasKeyJustPressed('M'))
	{
		if (g_gameSoundVolume == 0.f)
		{
			g_gameSoundVolume = 1.f;
		}
		else
		{
			g_gameSoundVolume = 0.f;
		}
	}
}


void Game::SwitchToGameplayMode()
{
	m_nextGameMode = GameMode::GAMEPLAY;
}


void Game::ResetGame()
{
	m_currentGameMode = GameMode::ATTRACT;
	m_nextGameMode = GameMode::ATTRACT;
}


void Game::RenderPausePanel() const
{
	std::vector<Vertex_PCU> pauseVertexArray;
	AABB2 screenBox(Vec2(0.f, 0.f), Vec2(worldCameraSizeX, worldCameraSizeY));
	AddVertsForAABB2D(pauseVertexArray, screenBox, Rgba8(150, 150, 150, 150));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(pauseVertexArray.size()), pauseVertexArray.data());
}


