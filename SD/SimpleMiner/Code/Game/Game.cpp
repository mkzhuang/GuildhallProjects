#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/BlockColorDefinition.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/TemplateDefinition.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine//Core/DevConsole.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Core/EventSystem.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef OPAQUE

RandomNumberGenerator RNG;
Game* g_theGame;

SoundID attractMusic;
SoundID gameMusic;
SoundID selectSound;
SoundID pauseSound;
SoundID unpauseSound;
SoundPlaybackID musicPlaybackID;

static float worldCameraMinX = 0.f;
static float worldCameraMinY = 0.f;
static float worldCameraMaxX = 0.f;
static float worldCameraMaxY = 0.f;
static float worldCameraDimensionX = 0.f;
static float worldCameraDimensionY = 0.f;
static float worldCameraCenterX = 0.f;
static float worldCameraCenterY = 0.f;
static float UICameraDimensionX = 0.f;
static float UICameraDimensionY = 0.f;
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
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BasicSprites_64x64.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BasicSprites_64x64_CrackIncluded.png");
	if (!g_gameConfigBlackboard.GetValue("disableShader", false))
	{
		g_theRenderer->CreateOrGetShader("Data/Shaders/World");
	}

	worldCameraMinX = g_gameConfigBlackboard.GetValue("worldCameraMinX", 0.f);
	worldCameraMinY = g_gameConfigBlackboard.GetValue("worldCameraMinY", 0.f);
	worldCameraMaxX = g_gameConfigBlackboard.GetValue("worldCameraMaxX", 0.f);
	worldCameraMaxY = g_gameConfigBlackboard.GetValue("worldCameraMaxY", 0.f);
	worldCameraDimensionX =  worldCameraMaxX - worldCameraMinX;
	worldCameraDimensionY =  worldCameraMaxY - worldCameraMinY;
	worldCameraCenterX = g_gameConfigBlackboard.GetValue("worldCameraCenterX", 10.f);
	worldCameraCenterY = g_gameConfigBlackboard.GetValue("worldCameraCenterY", 5.f);
	UICameraDimensionX = g_gameConfigBlackboard.GetValue("UICameraDimensionX", 20.f);
	UICameraDimensionY = g_gameConfigBlackboard.GetValue("UICameraDimensionY", 10.f);
	UICameraCenterX = g_gameConfigBlackboard.GetValue("UICameraCenterX", 10.f);
	UICameraCenterY = g_gameConfigBlackboard.GetValue("UICameraCenterY", 5.f);
	slowMoMultiplier = static_cast<double>(g_gameConfigBlackboard.GetValue("debugSlowMoMultiplier", 1.f));
	fastMoMultiplier = static_cast<double>(g_gameConfigBlackboard.GetValue("debugFastMoMultiplier", 1.f));
	joystickMenuDelay = g_gameConfigBlackboard.GetValue("joystickMenuDelay", 0.1f);
	joystickMenuSensitivity = g_gameConfigBlackboard.GetValue("joystickMenuSensitivity", 0.1f);
	
	attractMusic = g_theAudio->CreateOrGetSound("Data/Audio/C418 - Minecraft - Volume Alpha - 13 Wet Hands.mp3");
	gameMusic = g_theAudio->CreateOrGetSound("Data/Audio/C418 - Minecraft - Volume Alpha - 15 Chris.mp3");
	selectSound = g_theAudio->CreateOrGetSound("Data/Audio/Select.wav");
	pauseSound = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
	unpauseSound = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");

	BlockDefinition::InitializeBlockDefinitions();
	BlockColorDefinition::InitializeBlockColorDefinitions();
	EntityDefinition::InitializeDefinitions();
	TemplateDefinition::InitalizeDefinitions("Data/Definitions/TreeTemplateDefinitions.xml");
	TemplateDefinition::LoadFromSpriteFile("Data/3DSprites/oak.3dsprite");
	TemplateDefinition::LoadFromSpriteFile("Data/3DSprites/spruce.3dsprite");
	TemplateDefinition::LoadFromSpriteFile("Data/3DSprites/house.3dsprite");
	SubscribeEventCallbackFunction("debugSpawnScreenMessage", Event_SpawnScreenMessage);
	g_theGame = this;
}


void Game::Startup()
{
	musicPlaybackID = PlaySound(attractMusic, true, g_gameSoundVolume);

	m_worldCamera.SetRenderTransform(Vec3(0.f, -1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f));

	FireEvent("debugSpawnBasis");
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

	g_theAudio->SetSoundPlaybackSpeed(musicPlaybackID, g_gameSoundPlaySpeed);
	float deltaSeconds = static_cast<float>(m_gameClock.GetDeltaTime());

	UpdateGameMode();

	if (g_theWindow->HasFocus())
	{
		if (g_theDevConsole->IsOpen())
		{
			g_theInput->SetMouseMode(false, false, false);
		}
		else
		{
			switch (m_currentGameMode)
			{
			case GameMode::ATTRACT:
				g_theInput->SetMouseMode(false, false, false);
				break;
			case GameMode::GAMEPLAY:
				g_theInput->SetMouseMode(true, true, true);
				break;
			}
		}
	}

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
		switch (m_currentGameMode)
		{
		case GameMode::ATTRACT:
			break;
		case GameMode::GAMEPLAY:
			m_currentWorld = new World(this);
			m_currentWorld->Startup(Vec3(0.f, 0.f, 75.f), EulerAngles());
			break;
		}
	}
}


void Game::UpdateAttract(float deltaSeconds)
{
	UNUSED(deltaSeconds)

	if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N'))
	{
		SwitchToGameplayMode();
		PlaySound(selectSound, false, g_gameSoundVolume);
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
		PlaySound(selectSound, false, g_gameSoundVolume);
		return;
	}

	if (controller.WasButtonJustPressed(XboxButtonID::BACK))
	{
		g_theApp->HandleQuitRequested();
	}
}


void Game::UpdateGameplay(float deltaSeconds)
{
	m_currentWorld->Update(deltaSeconds);

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock.StepFrame();
		PlaySound(pauseSound, false, g_gameSoundVolume);
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		if (m_gameClock.IsPaused())
		{
			m_gameClock.Unpause();
			PlaySound(unpauseSound, false, g_gameSoundVolume);
		}
		else
		{
			m_gameClock.Pause();
			PlaySound(pauseSound, false, g_gameSoundVolume);
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		PlaySound(selectSound, false, g_gameSoundVolume);
		ResetGame();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		g_theJobSystem->ClearAllJobs();
		Vec3 pos = m_currentWorld->GetPlayerPos();
		EulerAngles orientation = m_currentWorld->GetPlayerOrientation();
		delete m_currentWorld;
		m_currentWorld = nullptr;
		m_currentWorld = new World(this);
		m_currentWorld->Startup(pos, orientation);
	}

	XboxController const& controller = g_theInput->GetController(0);

	if (controller.WasButtonJustPressed(XboxButtonID::BACK))
	{
		PlaySound(selectSound, false, g_gameSoundVolume);
		ResetGame();
	}
}


void Game::UpdateCamera(float deltaSeconds)
{
	UNUSED(deltaSeconds)

	m_UICamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(UICameraDimensionX, UICameraDimensionY));
	m_worldCamera.SetPerspectiveView(g_theWindow->GetAspect(), 60.f, 0.1f, 100'000.f);
}


void Game::Render() const
{

	switch (m_currentGameMode)
	{
		case GameMode::ATTRACT:		RenderAttract(); break;
		case GameMode::GAMEPLAY:	RenderGameplay(); break;
	}

	RenderUI();
}


void Game::RenderUI() const
{
	g_theRenderer->BeginCamera(m_UICamera);

	g_theRenderer->EndCamera(m_UICamera);
}


void Game::RenderAttract() const
{
	g_theRenderer->BeginCamera(m_UICamera);
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	
	float offset = SinDegrees(GetTotalGameTime() * 60.f) * 100.f;

	std::vector<Vertex_PCU> titleVertexArray;
	titleVertexArray.reserve(400);
	std::string gameTitle = g_gameConfigBlackboard.GetValue("gameTitle", "untitled game");
	AddVertsForTextTriangles2D(titleVertexArray, gameTitle, Vec2(32.f, 632.f), 80.f, Rgba8(50, 50, 50, static_cast<unsigned char>(120 - offset)));
	AddVertsForTextTriangles2D(titleVertexArray, gameTitle, Vec2(40.f, 640.f), 80.f, Rgba8(100, 100, 255, static_cast<unsigned char>(150 - offset)));
	AddVertsForTextTriangles2D(titleVertexArray, "Music: C418 - Wet Hands", Vec2(40.f, 40.f), 16.f, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(titleVertexArray.size()), titleVertexArray.data());

	Rgba8 ringColor(static_cast<unsigned char>(155 - offset), static_cast<unsigned char>(155 + offset), static_cast<unsigned char>(155 + offset), static_cast<unsigned char>(150 - offset));
	DebugDrawRing(Vec2(800.f, 400.f), 160.f + offset, 20.f - 0.1f * offset, ringColor);

	g_theRenderer->EndCamera(m_UICamera);
}


void Game::RenderGameplay() const
{
	g_theRenderer->BeginCamera(m_worldCamera);

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
	g_theRenderer->SetSamplerState(SamplerMode::POINTWRAP);

	m_currentWorld->Render();

	g_theRenderer->EndCamera(m_worldCamera);

	g_theRenderer->BeginCamera(m_UICamera);

	m_currentWorld->RenderUI(m_UICamera);

	g_theRenderer->EndCamera(m_UICamera);

	DebugRenderWorld(m_worldCamera);
	DebugRenderScreen(m_UICamera);

	if (m_gameClock.IsPaused())
	{
		RenderPausePanel();
	}
}


void Game::HandleCommonInput()
{
	m_isSlowMode = g_theInput->IsKeyDown('T') ? true : false;
	//m_isFastMode = g_theInput->IsKeyDown('Y') ? true : false;

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
		g_theAudio->SetSoundPlaybackVolume(musicPlaybackID, g_gameSoundVolume);
	}
}


void Game::SwitchToGameplayMode()
{
	m_nextGameMode = GameMode::GAMEPLAY;
	g_theAudio->StopSound(musicPlaybackID);
	musicPlaybackID = PlaySound(gameMusic, true, g_gameSoundVolume);
}


void Game::ResetGame()
{
 	g_theJobSystem->ClearAllJobs();
	delete m_currentWorld;
	m_currentWorld = nullptr;
	m_currentGameMode = GameMode::ATTRACT;
	m_nextGameMode = GameMode::ATTRACT;
	g_theAudio->StopSound(musicPlaybackID);

	Startup();
}


float Game::GetTotalGameTime() const
{
	return static_cast<float>(m_gameClock.GetTotalTime());
}


Camera* Game::GetCamera()
{
	return &m_worldCamera;
}

void Game::RenderPausePanel() const
{
	g_theRenderer->BeginCamera(m_UICamera);
	std::vector<Vertex_PCU> pauseVertexArray;
	AABB2 pauseLineA(Vec2(1565.f, 80.f), Vec2(1552.5f, 35.f));
	AABB2 pauseLineB(Vec2(1545.f, 80.f), Vec2(1532.5f, 35.f));
	AddVertsForAABB2D(pauseVertexArray, pauseLineA, Rgba8::RED);
	AddVertsForAABB2D(pauseVertexArray, pauseLineB, Rgba8::RED);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(pauseVertexArray.size()), pauseVertexArray.data());
	g_theRenderer->EndCamera(m_UICamera);
}


bool Game::Event_SpawnScreenMessage(EventArgs& args)
{
	std::string text = args.GetValue("text", "error");
	float duration = args.GetValue("duration", 0.f);
	Rgba8 color = args.GetValue("color", Rgba8::WHITE);
	DebugAddMessage(text, duration, color, color);
	return false;
}


