#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/TileMaterialDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/TileSetDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/ActorDefinition.hpp"
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
std::vector<Texture*> g_textures;
SpriteSheet* g_tileSpriteSheet;
std::vector<SoundID> g_soundIDs;

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
	g_theGame = this;

	LoadConfig();
	LoadAssets();

	TileMaterialDefinition::InitializeDefinitions();
	TileDefinition::InitializeDefinitions();
	TileSetDefinition::InitializeDefinitions();
	ActorDefinition::InitializeDefinitions("Data/Definitions/ProjectileActorDefinitions.xml");
	WeaponDefinition::InitializeDefinitions("Data/Definitions/WeaponDefinitions.xml");
	ActorDefinition::InitializeDefinitions("Data/Definitions/ActorDefinitions.xml");
	MapDefinition::InitializeDefinitions();

	//DebugRenderSetParentClock(Clock::GetSystemClock());
	SubscribeEventCallbackFunction("debugSpawnScreenMessage", Event_SpawnScreenMessage);
	SubscribeEventCallbackFunction("keys", Command_Keys);
}


void Game::Startup()
{
	g_theAudio->SetNumListeners(1);
	musicPlaybackID = PlaySound(g_soundIDs[SOUND_ATTRACT], true, g_gameMusicVolume);
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
			case GameMode::LOBBY:
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
	case GameMode::LOBBY:		UpdateLobby(deltaSeconds); break;
	case GameMode::GAMEPLAY:	UpdateGameplay(deltaSeconds); break;
	}

	HandleCommonInput();
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
		case GameMode::LOBBY:
			break;
		case GameMode::GAMEPLAY:
			g_theAudio->SetNumListeners(m_playerCounter);
			MapDefinition const* mapDef = MapDefinition::GetByName(g_gameConfigBlackboard.GetValue("defaultMapName", "Default"));
			m_currentMap = new Map(this, mapDef);
			break;
		}
	}
}


void Game::UpdateAttract(float deltaSeconds)
{
	UNUSED(deltaSeconds)

		if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N'))
		{
			SwitchToLobbyMode();
			PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
 			Camera* playerCamera = new Camera();
			Camera* playerUICamera = new Camera();
			Player* player = new Player(this, Vec3::ZERO, playerCamera, playerUICamera);
			player->m_playerIndex = 0;
			AddPlayer(player);
			return;
		}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->HandleQuitRequested();
	}

	XboxController const& controller = g_theInput->GetController(0);

	if (controller.WasButtonJustPressed(XboxButtonID::START) || controller.WasButtonJustPressed(XboxButtonID::BUTTON_A))
	{
		SwitchToLobbyMode();
		PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
		Camera* playerCamera = new Camera();
		Camera* playerUICamera = new Camera();
		Player* player = new Player(this, Vec3::ZERO, playerCamera, playerUICamera);
		player->m_playerIndex = 0;
		player->m_controllerIndex = 0;
		AddPlayer(player);
		return;
	}

	if (controller.WasButtonJustPressed(XboxButtonID::BACK))
	{
		g_theApp->HandleQuitRequested();
	}

	m_UICamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(UICameraDimensionX, UICameraDimensionY));
}


void Game::UpdateLobby(float deltaSeconds)
{
	UNUSED(deltaSeconds)

	if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N'))
	{
		if (m_playerCounter == 2)
		{
			SwitchToGameplayMode();
			PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
			return;
		}

		Player* firstPlayer = GetPlayer(0);
		if (firstPlayer)
		{
			if (firstPlayer->m_controllerIndex == -1)
			{
				SwitchToGameplayMode();
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				return;
			}
			else
			{
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				Camera* playerCamera = new Camera();
				Camera* playerUICamera = new Camera();
				Player* player = new Player(this, Vec3::ZERO, playerCamera, playerUICamera);
				player->m_playerIndex = 1;
				AddPlayer(player);
				return;
			}
		}

		Player* secondPlayer = GetPlayer(1);
		if (secondPlayer)
		{
			if (secondPlayer->m_controllerIndex == -1)
			{
				SwitchToGameplayMode();
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				return;
			}
			else
			{
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				Camera* playerCamera = new Camera();
				Camera* playerUICamera = new Camera();
				Player* player = new Player(this, Vec3::ZERO, playerCamera, playerUICamera);
				player->m_playerIndex = 0;
				AddPlayer(player);
				return;
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
		{
			Player*& player = m_players[playerIndex];
			if (player && player->m_controllerIndex == -1)
			{
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				delete player;
				player = nullptr;
				m_playerCounter--;
			}
		}
		if (m_playerCounter == 0) ResetGame();
	}

	XboxController const& controller = g_theInput->GetController(0);

	if (controller.WasButtonJustPressed(XboxButtonID::START) || controller.WasButtonJustPressed(XboxButtonID::BUTTON_A))
	{
		if (m_playerCounter == 2)
		{
			SwitchToGameplayMode();
			PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
			return;
		}

		Player* firstPlayer = GetPlayer(0);
		if (firstPlayer)
		{
			if (firstPlayer->m_controllerIndex == -1)
			{
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				Camera* playerCamera = new Camera();
				Camera* playerUICamera = new Camera();
				Player* player = new Player(this, Vec3::ZERO, playerCamera, playerUICamera);
				player->m_playerIndex = 1;
				player->m_controllerIndex = 0;
				AddPlayer(player);
				return;
			}
			else
			{
				SwitchToGameplayMode();
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				return;
			}
		}

		Player* secondPlayer = GetPlayer(1);
		if (secondPlayer)
		{
			if (secondPlayer->m_controllerIndex == -1)
			{
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				Camera* playerCamera = new Camera();
				Camera* playerUICamera = new Camera();
				Player* player = new Player(this, Vec3::ZERO, playerCamera, playerUICamera);
				player->m_playerIndex = 0;
				player->m_controllerIndex = 0;
				AddPlayer(player);
				return;
			}
			else
			{
				SwitchToGameplayMode();
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				return;
			}
		}
	}

	if (controller.WasButtonJustPressed(XboxButtonID::BACK))
	{
		for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
		{
			Player*& player = m_players[playerIndex];
			if (player && player->m_controllerIndex == 0)
			{
				PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
				delete player;
				player = nullptr;
				m_playerCounter--;
			}
		}
		if (m_playerCounter == 0) ResetGame();
	}

	m_UICamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(UICameraDimensionX, UICameraDimensionY));
}


void Game::UpdateGameplay(float deltaSeconds)
{
	if (GetAlivePlayerCount() == 0)
	{
		if (m_endGameWatch.GetElapsedTime() == 0.f)
		{
			m_endGameWatch.Start(&m_gameClock, 5.f);
			return;
		}
		
		if (m_endGameWatch.HasDurationElapsed())
		{
			ResetGame();
			return;
		}
	}

	for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
	{
		Player* player = m_players[playerIndex];
		if (player)
		{
			player->Update(deltaSeconds);
		}
	}

	m_currentMap->Update(deltaSeconds);

	std::string gameClockInfo = Stringf("Game           | Time: %.2f, FPS: %.2f, Dilation: %.2f", m_gameClock.GetTotalTime(), 1.0 / m_gameClock.GetDeltaTime(), m_gameClock.GetTimeDilation());
	DebugAddScreenText(gameClockInfo, Vec2(900.f, 775.f), 0.f, Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE);

	Clock const& devConsoleClock = g_theDevConsole->GetClock();
	std::string devConsoleClockInfo = Stringf("Dev Console    | Time: %.2f, FPS: %.2f, Dilation: %.2f", devConsoleClock.GetTotalTime(), 1.0 / devConsoleClock.GetDeltaTime(), devConsoleClock.GetTimeDilation());
	DebugAddScreenText(devConsoleClockInfo, Vec2(900.f, 755.f), 0.f, Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE);

	Clock const& debugClock = DebugRenderGetClock();
	std::string debugClockInfo = Stringf("Debug Renderer | Time: %.2f, FPS: %.2f, Dilation: %.2f", debugClock.GetTotalTime(), 1.0 / debugClock.GetDeltaTime(), debugClock.GetTimeDilation());
	DebugAddScreenText(debugClockInfo, Vec2(900.f, 735.f), 0.f, Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE);

	//if (g_theInput->WasKeyJustPressed('N'))
	//{
	//	m_currentMap->PossessNextActor();
	//}

	if (g_theInput->WasKeyJustPressed('9'))
	{
		DebugRenderDecreaseClockTimeDilation();
	}

	if (g_theInput->WasKeyJustPressed('0'))
	{
		DebugRenderIncreaseClockTimeDilation();
	}

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock.StepFrame();
		PlaySound(g_soundIDs[SOUND_PAUSE], false, g_gameSoundVolume);
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		if (m_gameClock.IsPaused())
		{
			m_gameClock.Unpause();
			PlaySound(g_soundIDs[SOUND_UNPAUSE], false, g_gameSoundVolume);
		}
		else
		{
			m_gameClock.Pause();
			PlaySound(g_soundIDs[SOUND_PAUSE], false, g_gameSoundVolume);
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
		ResetGame();
	}

	XboxController const& controller = g_theInput->GetController(0);

	if (controller.WasButtonJustPressed(XboxButtonID::BACK))
	{
		PlaySound(g_soundIDs[SOUND_SELECT], false, g_gameSoundVolume);
		ResetGame();
	}

	m_UICamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(UICameraDimensionX, UICameraDimensionY));
}


void Game::Render() const
{
	switch (m_currentGameMode)
	{
	case GameMode::ATTRACT:		RenderAttract(); break;
	case GameMode::LOBBY:		RenderLobby(); break;
	case GameMode::GAMEPLAY:	RenderGameplay(); break;
	}
	//RenderUI();
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
	std::string gameTitle = g_gameConfigBlackboard.GetValue("gameTitle", "untitled game");
	AddVertsForTextTriangles2D(titleVertexArray, gameTitle, Vec2(32.f, 632.f), 80.f, Rgba8(50, 50, 50, static_cast<unsigned char>(120 - offset)));
	AddVertsForTextTriangles2D(titleVertexArray, gameTitle, Vec2(40.f, 640.f), 80.f, Rgba8(100, 100, 255, static_cast<unsigned char>(150 - offset)));

	AddVertsForTextTriangles2D(titleVertexArray, "Press Space to play with keyboard and mouse", Vec2(450.f, 70.f), 24.f, Rgba8::WHITE);
	AddVertsForTextTriangles2D(titleVertexArray, "Press Start to play with controller", Vec2(520.f, 40.f), 24.f, Rgba8::WHITE);
	AddVertsForTextTriangles2D(titleVertexArray, "Press Escape or Back to exit", Vec2(575.f, 10.f), 24.f, Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(titleVertexArray);

	Rgba8 ringColor(static_cast<unsigned char>(155 - offset), static_cast<unsigned char>(155 + offset), static_cast<unsigned char>(155 + offset), static_cast<unsigned char>(150 - offset));
	std::vector<Vertex_PCU> ringVerts;
	AddVertsForRing2D(ringVerts, Vec2(800.f, 400.f), 160.f + offset, 20.f - 0.1f * offset, 128.f, ringColor);
	g_theRenderer->BindTexture(nullptr);  //something wrong with this bind in debug, with texture, not working
	g_theRenderer->DrawVertexArray(ringVerts);

	g_theRenderer->EndCamera(m_UICamera);
}


void Game::RenderLobby() const
{

	g_theRenderer->BeginCamera(m_UICamera);
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	std::vector<Vertex_PCU> textVerts;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 uiBounds(m_UICamera.GetOrthoBottomLeft(), m_UICamera.GetOrthoTopRight());

	if (m_playerCounter == 1)
	{
		AABB2 infoBounds(Vec2::ZERO, Vec2(1200.f, 400.f));
		uiBounds.AlignBoxWithin(infoBounds, Vec2(0.5f, 0.5f));
		AABB2 playerBounds(Vec2::ZERO, Vec2(800.f, 200.f));
		AABB2 inputBounds(Vec2::ZERO, Vec2(800.f, 200.f));
		infoBounds.AlignBoxWithin(playerBounds, Vec2(0.5f, 1.f));
		infoBounds.AlignBoxWithin(inputBounds, Vec2(0.5f, 0.f));
		Player* curPlayer = nullptr;
		for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
		{
			Player* player = m_players[playerIndex];
			if (player)
			{
				curPlayer = player;
			}
		}
		if (!curPlayer) return;
		bool isKeyboard = curPlayer->m_controllerIndex == -1;
		std::string playerLine = isKeyboard ? std::string("keyboard and mouse") : std::string("controller");
		playerLine = "Player 1\n" + playerLine;
		font->AddVertsForTextInBox2D(textVerts, playerBounds, 40.f, playerLine, Rgba8::WHITE, 1.f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
		std::string startButton = isKeyboard ? std::string("Space") : std::string("Start");
		std::string exitButton = isKeyboard ? std::string("Escape") : std::string("Back");
		std::string newButton = isKeyboard ? std::string("Start") : std::string("Space");
		std::string inputLine = Stringf("Press %s to start game\nPress %s to exit game\nPress %s to join", startButton.c_str(), exitButton.c_str(), newButton.c_str());
		font->AddVertsForTextInBox2D(textVerts, inputBounds, 25.f, inputLine, Rgba8::WHITE, 1.f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
	}

	if (m_playerCounter == 2)
	{
		for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
		{
			Player* player = m_players[playerIndex];
			if (player)
			{
				int index = player->m_playerIndex;
				AABB2 InfoBounds(Vec2::ZERO, Vec2(1200.f, 400.f));
				uiBounds.AlignBoxWithin(InfoBounds, Vec2(0.5f, 1.f - static_cast<float>(index)));
				AABB2 PlayerBounds(Vec2::ZERO, Vec2(800.f, 200.f));
				AABB2 InputBounds(Vec2::ZERO, Vec2(800.f, 200.f));
				InfoBounds.AlignBoxWithin(PlayerBounds, Vec2(0.5f, 1.f));
				InfoBounds.AlignBoxWithin(InputBounds, Vec2(0.5f, 0.f));
				bool IsKeyboard = player->m_controllerIndex == -1;
				std::string PlayerLine = IsKeyboard ? std::string("keyboard and mouse") : std::string("controller");
				PlayerLine = Stringf("Player %d\n", index + 1) + PlayerLine;
				font->AddVertsForTextInBox2D(textVerts, PlayerBounds, 40.f, PlayerLine, Rgba8::WHITE, 1.f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT);
				std::string StartButton = IsKeyboard ? std::string("Space") : std::string("Start");
				std::string ExitButton = IsKeyboard ? std::string("Escape") : std::string("Back");
				std::string InputLine = Stringf("Press %s to start game\nPress %s to exit game", StartButton.c_str(), ExitButton.c_str());
				font->AddVertsForTextInBox2D(textVerts, InputBounds, 25.f, InputLine, Rgba8::WHITE, 1.f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT);
			}
		}
	}

	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);

	g_theRenderer->EndCamera(m_UICamera);
}


void Game::RenderGameplay() const
{
	
	if (GetAlivePlayerCount() == 0)
	{
		g_theRenderer->BeginCamera(m_UICamera);
		g_theRenderer->ClearScreen(Rgba8::BLACK);
		std::vector<Vertex_PCU> textVerts;
		BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
		AABB2 uiBounds(m_UICamera.GetOrthoBottomLeft(), m_UICamera.GetOrthoTopRight());
		AABB2 infoBounds(Vec2::ZERO, Vec2(1200.f, 100.f));
		uiBounds.AlignBoxWithin(infoBounds, Vec2(0.5f, 0.5f));
		int waves = m_currentMap->m_waveCounter - 1;
		std::string endGameLine = Stringf("Game Over\n\nYou survived %i waves", waves);
		font->AddVertsForTextInBox2D(textVerts, infoBounds, 100.f, endGameLine, Rgba8::WHITE, 1.f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT);
		g_theRenderer->BindTexture(&font->GetTexture());
		g_theRenderer->DrawVertexArray(textVerts);
		g_theRenderer->EndCamera(m_UICamera);
		return;
	}

	for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
	{
		Player* player = m_players[playerIndex];
		if (!player) continue;
		Camera const& worldCamera = *(player->m_camera);
		g_theRenderer->BeginCamera(worldCamera);
		g_theRenderer->ClearScreen(Rgba8::BLACK);

		if (player->GetActor())
		{
			m_currentMap->Render(worldCamera);
		}

		g_theRenderer->EndCamera(worldCamera);

		player->Render();
	}

	DebugRenderScreen(m_UICamera);

	if (m_gameClock.IsPaused())
	{
		RenderPausePanel();
	}
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
		g_theAudio->SetSoundPlaybackVolume(musicPlaybackID, g_gameSoundVolume);
	}
}


void Game::SwitchToLobbyMode()
{
	m_nextGameMode = GameMode::LOBBY;
}


void Game::SwitchToGameplayMode()
{
	m_nextGameMode = GameMode::GAMEPLAY;
	g_theAudio->StopSound(musicPlaybackID);
	musicPlaybackID = PlaySound(g_soundIDs[SOUND_GAME], true, g_gameMusicVolume);
	if (m_playerCounter == 1) return;

	for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
	{
		Player* player = m_players[playerIndex];
		if (player)
		{
			float index = static_cast<float>(player->m_playerIndex);
			player->m_camera->SetViewport(AABB2(Vec2(0.f, index * 0.5f), Vec2(1.f, (index + 1.f) * 0.5f)));
			player->m_UICamera->SetViewport(AABB2(Vec2(0.f, index * 0.5f), Vec2(1.f, (index + 1.f) * 0.5f)));
		}
	}
}


void Game::ResetGame()
{
	m_currentGameMode = GameMode::ATTRACT;
	m_nextGameMode = GameMode::ATTRACT;
	g_theAudio->StopSound(musicPlaybackID);
	m_players.clear();
	m_playerCounter = 0;
	delete m_currentMap;
	m_currentMap = nullptr;
	DebugRenderClear(); // clear debug render geometry memory usage

	Startup();
}


float Game::GetTotalGameTime() const
{
	return static_cast<float>(m_gameClock.GetTotalTime());
}


Clock& Game::GetGameClock()
{
	return m_gameClock;
}


void Game::AddPlayer(Player* player)
{
	player->m_UICamera->SetOrthoView(Vec2(0.f, 0.f), Vec2(UICameraDimensionX, UICameraDimensionY));

	for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
	{
		Player*& curPlayer = m_players[playerIndex];
		if (!curPlayer)
		{
			curPlayer = player;
			m_playerCounter++;
			return;
		}
	}

	m_players.push_back(player);
	m_playerCounter++;
}


Player* Game::GetPlayer(int index) const
{
	for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
	{
		Player* curPlayer = m_players[playerIndex];
		if (!curPlayer) continue;
		if (curPlayer->m_playerIndex == index) return curPlayer;
	}
	return nullptr;
}


int Game::GetPlayerCount() const
{
	return m_playerCounter;
}


std::vector<Player*> Game::GetAllPlayer() const
{
	return m_players;
}


int Game::GetAlivePlayerCount() const
{
	int count = 0;
	for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
	{
		Player* player = m_players[playerIndex];
		if (player && player->GetActor())
		{
			count++;
		}
	}

	return count;
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


void Game::LoadConfig()
{
	worldCameraMinX = g_gameConfigBlackboard.GetValue("worldCameraMinX", 0.f);
	worldCameraMinY = g_gameConfigBlackboard.GetValue("worldCameraMinY", 0.f);
	worldCameraMaxX = g_gameConfigBlackboard.GetValue("worldCameraMaxX", 0.f);
	worldCameraMaxY = g_gameConfigBlackboard.GetValue("worldCameraMaxY", 0.f);
	worldCameraDimensionX = worldCameraMaxX - worldCameraMinX;
	worldCameraDimensionY = worldCameraMaxY - worldCameraMinY;
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
	g_gameMusicVolume = g_gameConfigBlackboard.GetValue("soundVolume", 0.1f);
}


void Game::LoadAssets()
{
	g_textures.resize(NUM_TEXTURES);
	g_textures[TEXTURE_TILES]		= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	g_textures[TEXTURE_TEST]		= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
	g_tileSpriteSheet = new SpriteSheet(*g_textures[TEXTURE_TILES], IntVec2(8, 8));

	g_soundIDs.resize(NUM_SOUNDS);
	g_soundIDs[SOUND_ATTRACT]		= g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("menuMusic", ""));
	g_soundIDs[SOUND_GAME]			= g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("gameMusic", ""));
	g_soundIDs[SOUND_PAUSE]			= g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
	g_soundIDs[SOUND_UNPAUSE]		= g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");
	g_soundIDs[SOUND_SELECT]		= g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("clickSound", ""));
	g_soundIDs[SOUND_PISTOL]		= g_theAudio->CreateOrGetSound("Data/Audio/PistolFire.wav");
	g_soundIDs[SOUND_PLASMA]		= g_theAudio->CreateOrGetSound("Data/Audio/PlasmaFire.wav");
	g_soundIDs[SOUND_SHRINKGUN]		= g_theAudio->CreateOrGetSound("Data/Audio/ShrinkGunFire.wav");
	g_soundIDs[SOUND_PLAYER_HURT]	= g_theAudio->CreateOrGetSound("Data/Audio/PlayerHurt.wav");
	g_soundIDs[SOUND_PLAYER_DEATH]	= g_theAudio->CreateOrGetSound("Data/Audio/PlayerDeath1.wav");
	g_soundIDs[SOUND_DEMON_ATTACK]	= g_theAudio->CreateOrGetSound("Data/Audio/DemonAttack.wav");
	g_soundIDs[SOUND_DEMON_HURT]	= g_theAudio->CreateOrGetSound("Data/Audio/DemonHurt.wav");
	g_soundIDs[SOUND_DEMON_DEATH]	= g_theAudio->CreateOrGetSound("Data/Audio/DemonDeath.wav");
	g_soundIDs[SOUND_MAGMA_ATTACK]	= g_theAudio->CreateOrGetSound("Data/Audio/MagmaAttack.wav");
	g_soundIDs[SOUND_MAGMA_HURT]	= g_theAudio->CreateOrGetSound("Data/Audio/MagmaHurt.wav");
	g_soundIDs[SOUND_MAGMA_DEATH]	= g_theAudio->CreateOrGetSound("Data/Audio/MagmaDeath.wav");
	g_soundIDs[SOUND_FIREBALL_HIT]	= g_theAudio->CreateOrGetSound("Data/Audio/FireballHit.wav");

	g_theRenderer->CreateOrGetShader("Data/Shaders/Sprite");
	g_theRenderer->CreateOrGetShader("Data/Shaders/SpriteLit");
}


bool Game::Event_SpawnScreenMessage(EventArgs& args)
{
	std::string text = args.GetValue("text", "error");
	float duration = args.GetValue("duration", 0.f);
	DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
	return false;
}


bool Game::Command_Keys(EventArgs& args)
{
	UNUSED(args)

		if (!g_theDevConsole->IsOpen()) return false;

	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "## Keys ##");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "~                           - Toggle dev console");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "ESC                         - Exit to attract screen");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F8                          - Reset game");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "T                           - Slow mode");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Y                           - Fast mode");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "O                           - Step frame");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "P                           - Toggle pause");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "W/S/A/D/Left Joystick       - Move forward/backward/left/right");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Z/C                         - Move upward/downward");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Q/E                         - Roll to left/right");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Mouse/Right JoyStick        - Aim camera");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Left Mouse/Right Shoulder   - fire weapon");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Shift/Left Shoulder         - Sprint");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F                           - Toggle free fly cam");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "N                           - Possess next actor");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "1                           - Equip weapon at slot 1: Pistol");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "2                           - Equip weapon at slot 2: Plasma Rifle");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "3                           - Equip weapon at slot 2: Shrink Gun");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "9                           - Decrease debug clock speed");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "0                           - Increase debug clock speed");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "<-/ X Button                - Equip previous weapon");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "->/ Y Button                - Equip next weapon");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F1                          - Toggle light info");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F2                          - Decrease ambient intensity");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F3                          - Increase ambient intensity");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F4                          - Decrease sun intensity");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F5                          - Increase sun intensity");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F6                          - Decrease sun pitch");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F7                          - Increase sun pitch");
	return false;
}


