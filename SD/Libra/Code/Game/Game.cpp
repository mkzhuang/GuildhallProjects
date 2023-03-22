#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

RandomNumberGenerator RNG;

std::vector<Texture*> g_textures;
std::vector<SoundID> g_soundIds;
SpriteSheet* g_tileSpriteSheet;
SpriteAnimDefinition* g_exlosionAnimation;

SoundPlaybackID musicPlaybackID;

static float screenDimensionX;
static float screenDimensionY;
static float gameLoseDelay;

Game::Game(App* const& owner)
	: m_theApp(owner)
	, m_gameClock(Clock::GetSystemClock())
{	
	LoadAssets();
}


void Game::Startup()
{
	TileDefinition::InitializeTileDefs();
	MapDefinition::InitializeMapDefs();

	musicPlaybackID = PlaySound(g_soundIds[SOUND_TYPE_ATTRACT], true, g_gameSoundVolume);

	screenDimensionX = g_gameConfigBlackboard.GetValue("screenCameraDimensionX", 200.f);
	screenDimensionY = g_gameConfigBlackboard.GetValue("screenCameraDimensionY", 100.f);
	gameLoseDelay = g_gameConfigBlackboard.GetValue("gameLoseDelay", 0.f);
	m_joystickMenuDelay = g_gameConfigBlackboard.GetValue("joystickMenuDelay", 0.f);
}


void Game::Update()
{
	static float slowMoMultiplier = g_gameConfigBlackboard.GetValue("debugSlowMoMultiplier", 0.f);
	static float fastMoMultiplier = g_gameConfigBlackboard.GetValue("debugFastMoMultiplier", 0.f);

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

	if (m_isAttractMode)
	{
		HandleKeyPressed();
		HandleControllerInput();
		m_joystickMenuDelay -= deltaSeconds;
		g_theAudio->SetSoundPlaybackSpeed(musicPlaybackID, g_gameSoundPlaySpeed);
	} 
	else if (m_isWinning)
	{
		HandleKeyPressed();
		HandleControllerInput();
	}
	else if (m_isPerished && m_perishedTimer >= gameLoseDelay)
	{
		m_gameClock.Pause();
		HandleKeyPressed();
		HandleControllerInput();
	}
	else
	{
		if (m_isPerished) 
		{
 			m_perishedTimer += deltaSeconds;
		}
		m_currentWorld->Update(deltaSeconds); //world update
		HandleKeyPressed();
		HandleControllerInput();
		g_theAudio->SetSoundPlaybackSpeed(musicPlaybackID, g_gameSoundPlaySpeed);
	}
	UpdateCamera();
}


void Game::Render() const
{
	if (m_isAttractMode)
	{
		g_theRenderer->BeginCamera(m_attractCamera);
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
		DrawAttractScreen();
		g_theRenderer->EndCamera(m_attractCamera);
	}
	else if (m_isWinning)
	{
		g_theRenderer->BeginCamera(m_attractCamera);
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
		DrawVictoryScreen();
		g_theRenderer->EndCamera(m_attractCamera);
	}
	else 
	{
		m_currentWorld->Render(); //world renderer
		g_theRenderer->BeginCamera(m_attractCamera);
		if (m_gameClock.IsPaused()) DrawPausePanel();
		g_theRenderer->EndCamera(m_attractCamera);

		if (m_isPerished && m_perishedTimer >= gameLoseDelay)
		{
			g_theRenderer->BeginCamera(m_attractCamera);
			DrawDefeatScreen();
			g_theRenderer->EndCamera(m_attractCamera);
		}
	}
}


void Game::HandleKeyPressed()
{
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

	if (m_isAttractMode)
	{
		if (g_theInput->WasKeyJustPressed('P'))
		{
			if (m_isStartOrExit)
			{
				m_isAttractMode = false;
				PlaySound(g_soundIds[SOUND_TYPE_SELECT], false, g_gameSoundVolume);
				g_theAudio->StopSound(musicPlaybackID);
				musicPlaybackID = PlaySound(g_soundIds[SOUND_TYPE_IN_GAME], true, g_gameSoundVolume);

				m_currentWorld = new World(this);
				m_currentWorld->Startup();

				return;
			}
			else
			{
				g_theApp->HandleQuitRequested();
			}
		}

		if (g_theInput->WasKeyJustPressed('D') || g_theInput->WasKeyJustPressed('E')) 
		{
			m_isStartOrExit = !m_isStartOrExit;
			PlaySound(g_soundIds[SOUND_TYPE_SELECT], false, g_gameSoundVolume);
		}

		if (g_theInput->WasKeyJustPressed(VK_ESCAPE))
		{
			g_theApp->HandleQuitRequested();
		}

		return;
	}

	if (m_isWinning)
	{
		if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('P') || g_theInput->WasKeyJustPressed(VK_ESCAPE))
		{
			ResetGame();
		}

		return;
	}

	if (m_isPerished)
	{
		if (m_perishedTimer < gameLoseDelay) return;

		if (g_theInput->WasKeyJustPressed('N'))
		{
			m_isPerished = false;
			m_gameClock.Unpause();
			m_perishedTimer = 0.f;
			m_currentWorld->SpawnPlayer();
		}

		if (g_theInput->WasKeyJustPressed(VK_ESCAPE))
		{
			ResetGame();
		}

		return;
	}

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock.StepFrame();
		PlaySound(g_soundIds[SOUND_TYPE_PAUSE], false, g_gameSoundVolume);
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		if (m_gameClock.IsPaused())
		{
			m_gameClock.Unpause();
			PlaySound(g_soundIds[SOUND_TYPE_UNPAUSE], false, g_gameSoundVolume);
		}
		else
		{
			m_gameClock.Pause();
			PlaySound(g_soundIds[SOUND_TYPE_PAUSE], false, g_gameSoundVolume);
		}

		return;
	}

	m_isSlowMode = g_theInput->IsKeyDown('T') ? true : false;
	m_isFastMode = g_theInput->IsKeyDown('Y') ? true : false;

	if (g_theInput->WasKeyJustPressed(VK_ESCAPE)) 
	{
		if (m_gameClock.IsPaused())
		{
			m_gameClock.Unpause();
			m_isAttractMode = true;
			m_isStartOrExit = true;
			PlaySound(g_soundIds[SOUND_TYPE_SELECT], false, g_gameSoundVolume);
			g_theAudio->StopSound(musicPlaybackID);
			ResetGame();
		}
		else
		{
			m_gameClock.Pause();
			PlaySound(g_soundIds[SOUND_TYPE_PAUSE], false, g_gameSoundVolume);
		}
	}
}


void Game::HandleControllerInput()
{
	XboxController const& controller = g_theInput->GetController(0);

	if (m_isAttractMode)
	{
		if (controller.WasButtonJustPressed(XboxButtonID::START) || controller.WasButtonJustPressed(XboxButtonID::BUTTON_A))
		{
			if (m_isStartOrExit)
			{
				m_isAttractMode = false;
				PlaySound(g_soundIds[SOUND_TYPE_SELECT], false, g_gameSoundVolume);
				g_theAudio->StopSound(musicPlaybackID);
				musicPlaybackID = PlaySound(g_soundIds[SOUND_TYPE_IN_GAME], true, g_gameSoundVolume);
				return;
			}
			else
			{
				g_theApp->HandleQuitRequested();
			}
		}

		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.3f && m_joystickMenuDelay < 0.f)
		{
			m_isStartOrExit = !m_isStartOrExit;
			m_joystickMenuDelay = g_gameConfigBlackboard.GetValue("joystickMenuDelay", 0.f);
			PlaySound(g_soundIds[SOUND_TYPE_SELECT], false, g_gameSoundVolume);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::BACK))
		{
			g_theApp->HandleQuitRequested();
		}

		return;
	}

	if (m_isWinning)
	{
		if (controller.WasButtonJustPressed(XboxButtonID::START) || controller.WasButtonJustPressed(XboxButtonID::BACK) || controller.WasButtonJustPressed(XboxButtonID::BUTTON_A))
		{
			ResetGame();
		}

		return;
	}

	if (m_isPerished)
	{
		if (m_perishedTimer < gameLoseDelay) return;

		if (controller.WasButtonJustPressed(XboxButtonID::START) || controller.WasButtonJustPressed(XboxButtonID::BUTTON_A))
		{
			m_isPerished = false;
			m_gameClock.Unpause();
			m_perishedTimer = 0.f;
			m_currentWorld->SpawnPlayer();
		}

		if (controller.WasButtonJustPressed(XboxButtonID::BACK))
		{
			ResetGame();
		}

		return;
	}

	if (controller.WasButtonJustPressed(XboxButtonID::START))
	{
		if (m_gameClock.IsPaused())
		{
			m_gameClock.Unpause();
			PlaySound(g_soundIds[SOUND_TYPE_UNPAUSE], false, g_gameSoundVolume);
		}
		else
		{
			m_gameClock.Pause();
			PlaySound(g_soundIds[SOUND_TYPE_PAUSE], false, g_gameSoundVolume);
		}

		return;
	}

	if (controller.WasButtonJustPressed(XboxButtonID::BACK)) 
	{
		if (m_gameClock.IsPaused())
		{
			m_gameClock.Unpause();
			m_isAttractMode = true;
			m_isStartOrExit = true;
			PlaySound(g_soundIds[SOUND_TYPE_SELECT], false, g_gameSoundVolume);
			g_theAudio->StopSound(musicPlaybackID);
			ResetGame();
		}
		else
		{
			m_gameClock.Pause();
			PlaySound(g_soundIds[SOUND_TYPE_PAUSE], false, g_gameSoundVolume);
		}
	}
}


void Game::UpdateCamera()
{
	m_attractCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(screenDimensionX, screenDimensionY));
}


void Game::GoToNextLevel()
{
	m_currentWorld->GoToNextLevel();
}

void Game::SetGameWin()
{
	m_isWinning = true;
	g_theAudio->StopSound(musicPlaybackID);
	PlaySound(g_soundIds[SOUND_TYPE_VICTORY], false, g_gameSoundVolume);
}

void Game::SetGameLose()
{
	m_isPerished = true;
}

void Game::ResetGame()
{
	g_theAudio->StopSound(musicPlaybackID);
	delete m_currentWorld;
	m_currentWorld = nullptr;
	g_gameSoundPlaySpeed = 1.f;
	m_isAttractMode = true;
	m_isStartOrExit = true;
	m_isSlowMode = false;
	m_isFastMode = false;
	m_isWinning = false;
	m_isPerished = false;
	g_isDebugging = false;
	g_isNoClip = false;
	g_isDebugCamera = false;
	Startup();
}


void Game::LoadAssets()
{
	g_textures.resize(NUM_TEXTURE_TYPES);
	g_textures[TEXTURE_TYPE_TERRAIN_TILES] = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	g_tileSpriteSheet = new SpriteSheet(*g_textures[TEXTURE_TYPE_TERRAIN_TILES], IntVec2(8, 8));
	g_textures[TEXTURE_TYPE_EXPLOSION_ANIM] = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Explosion_5x5.png");
	SpriteSheet* explosionSpriteSheet = new SpriteSheet(*g_textures[TEXTURE_TYPE_EXPLOSION_ANIM], IntVec2(5, 5));
	float explosionLife = g_gameConfigBlackboard.GetValue("explosionLife", 0.f);
	g_exlosionAnimation = new SpriteAnimDefinition(*explosionSpriteSheet, 0, 24, explosionLife, SpriteAnimPlaybackType::ONCE);
	g_textures[TEXTURE_TYPE_ATTRACT_SCREEN] = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/AttractScreen.png");
	g_textures[TEXTURE_TYPE_GAME_OVER_SCREEN] = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/YouDiedScreen.png");
	g_textures[TEXTURE_TYPE_VICTORY_SCREEN] = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/VictoryScreen.jpg");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyAries.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBolt.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBullet.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank3.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank4.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankBase.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankTop.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTurretBase.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyCannon.png");

	g_soundIds.resize(NUM_SOUND_TYPES);
	g_soundIds[SOUND_TYPE_ATTRACT] = g_theAudio->CreateOrGetSound("Data/Audio/C418 - Minecraft - Volume Alpha - 13 Wet Hands.mp3");
	g_soundIds[SOUND_TYPE_IN_GAME] = g_theAudio->CreateOrGetSound("Data/Audio/C418 - Minecraft - Volume Alpha - 20 Dog.mp3");
	g_soundIds[SOUND_TYPE_VICTORY] = g_theAudio->CreateOrGetSound("Data/Audio/Victory.mp3");
	g_soundIds[SOUND_TYPE_GAME_OVER] = g_theAudio->CreateOrGetSound("Data/Audio/GameOver.mp3");
	g_soundIds[SOUND_TYPE_LEVEL_END] = g_theAudio->CreateOrGetSound("Data/Audio/NewLevel.wav");
	g_soundIds[SOUND_TYPE_SELECT] = g_theAudio->CreateOrGetSound("Data/Audio/Select.wav");
	g_soundIds[SOUND_TYPE_PAUSE] = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
	g_soundIds[SOUND_TYPE_UNPAUSE] = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");
	g_soundIds[SOUND_TYPE_PLAYER_SHOOT] = g_theAudio->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
	g_soundIds[SOUND_TYPE_ENEMY_SHOOT] = g_theAudio->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
	g_soundIds[SOUND_TYPE_PLAYER_HIT] = g_theAudio->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	g_soundIds[SOUND_TYPE_ENEMY_HIT] = g_theAudio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	g_soundIds[SOUND_TYPE_ENEMY_DIE] = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	g_soundIds[SOUND_TYPE_BULLET_BOUNCE] = g_theAudio->CreateOrGetSound("Data/Audio/BulletBounce.wav");
	g_soundIds[SOUND_TYPE_BULLET_RICOCHET] = g_theAudio->CreateOrGetSound("Data/Audio/BulletRicochet.wav");
	g_soundIds[SOUND_TYPE_PLAYER_DISCOVERY] = g_theAudio->CreateOrGetSound("Data/Audio/Detected.wav");
}


void Game::DrawPausePanel() const
{
	std::vector<Vertex_PCU> pauseVertexArray;
	AABB2 screenBox(Vec2(0.f, 0.f), Vec2(screenDimensionX, screenDimensionY));
	AddVertsForAABB2D(pauseVertexArray, screenBox, Rgba8(150, 150, 150, 150));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(pauseVertexArray.size()), pauseVertexArray.data());
}


void Game::DrawAttractScreen() const
{
	float offset = SinDegrees(static_cast<float>(m_gameClock.GetTotalTime()) * 60.f) * 100.f;

	std::vector<Vertex_PCU> backgroundImageVertexArray;
	AABB2 backgroundImageBounds(Vec2(0.f, 0.f), Vec2(screenDimensionX, screenDimensionY));
	AddVertsForAABB2D(backgroundImageVertexArray, backgroundImageBounds, Rgba8::WHITE);
	g_theRenderer->BindTexture(g_textures[TEXTURE_TYPE_ATTRACT_SCREEN]);
	g_theRenderer->DrawVertexArray(int(backgroundImageVertexArray.size()), backgroundImageVertexArray.data());

	std::vector<Vertex_PCU> titleVertexArray;
	titleVertexArray.reserve(400);
	AddVertsForTextTriangles2D(titleVertexArray, "Libra", Vec2(112.f, 512.f), 160.f, Rgba8(50, 50, 50, 120 - static_cast<unsigned char>(offset)));
	AddVertsForTextTriangles2D(titleVertexArray, "Libra", Vec2(120.f, 504.f), 160.f, Rgba8(100, 100, 255, 150 - static_cast<unsigned char>(offset)));
	AddVertsForTextTriangles2D(titleVertexArray, "START", Vec2(1320.f, 160.f), 40.f, Rgba8(255, 255, 255, 255));
	AddVertsForTextTriangles2D(titleVertexArray, "EXIT", Vec2(1320.f, 80.f), 40.f, Rgba8(255, 255, 255, 255));
	AddVertsForTextTriangles2D(titleVertexArray, "Music: C418 - Wet Hands", Vec2(40.f, 40.f), 16.f, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(titleVertexArray.size()), titleVertexArray.data());

	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");
	AABB2 textureBounds(Vec2(480.f, 80.f), Vec2(800.f, 400.f));
	std::vector<Vertex_PCU> textureVertexArray;
	AddVertsForAABB2D(textureVertexArray, textureBounds, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(testTexture);
	g_theRenderer->DrawVertexArray(int(textureVertexArray.size()), textureVertexArray.data());

	Vec2 trianglePos = m_isStartOrExit ? Vec2(1296.f, 188.f) : Vec2(1296.f, 108.f);
	DrawSelectTriangle(trianglePos, Rgba8(255, 255, 255, 255));

	Texture* testTexture_8x2 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestSpriteSheet_8x2.png");
	SpriteSheet* testSpriteSheet = new SpriteSheet(*testTexture_8x2, IntVec2(8, 2));
	const SpriteDefinition& testSpriteDef = testSpriteSheet->GetSpriteDef(5);

	Vec2 uvAtMins, uvAtMaxs;
	testSpriteDef.GetUVs(uvAtMins, uvAtMaxs);
	AABB2 bounds(Vec2(880.f, 400.f), Vec2(960.f, 480.f));
	std::vector<Vertex_PCU> testVerts;
	AddVertsForAABB2D(testVerts, bounds, Rgba8(255, 255, 255, 255), uvAtMins, uvAtMaxs);
	g_theRenderer->BindTexture(&testSpriteDef.GetTexture());
	g_theRenderer->DrawVertexArray(int(testVerts.size()), testVerts.data());

	BitmapFont* testFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	std::vector<Vertex_PCU> textVerts;
	testFont->AddVertsForText2D(textVerts, Vec2(80.f, 240.f), 80.f, "Hello, world", Rgba8(0, 0, 200, 255), 0.3f);
	testFont->AddVertsForText2D(textVerts, Vec2(800.f, 480.f), 40.f, "It's nice to have options!", Rgba8(200, 0, 0, 255), 0.6f);
	TransformVertexArrayXY3D(int(textVerts.size()), textVerts.data(), 1.f, -10.f, Vec2::ZERO);
	g_theRenderer->BindTexture(&testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), textVerts.data());
}


void Game::DrawSelectTriangle(Vec2 pos, Rgba8 color) const
{
	std::vector<Vertex_PCU> selectTriangleVertexArray;
	selectTriangleVertexArray.reserve(3);
	selectTriangleVertexArray.emplace_back(Vec3(0.f, 0.f, 0.f), color, Vec2(0.f, 0.f));
	selectTriangleVertexArray.emplace_back(Vec3(-16.f, 8.f, 0.f), color, Vec2(0.f, 0.f));
	selectTriangleVertexArray.emplace_back(Vec3(-16.f, -8.f, 0.f), color, Vec2(0.f, 0.f));

	TransformVertexArrayXY3D(int(selectTriangleVertexArray.size()), &selectTriangleVertexArray[0], 1.5f, 0.f, pos);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(selectTriangleVertexArray.size()), selectTriangleVertexArray.data());
}


void Game::DrawVictoryScreen() const
{
	std::vector<Vertex_PCU> victoryImageVertexArray;
	AABB2 victoryImageBounds(Vec2(0.f, 0.f), Vec2(screenDimensionX, screenDimensionY));
	AddVertsForAABB2D(victoryImageVertexArray, victoryImageBounds, Rgba8::WHITE);
	g_theRenderer->BindTexture(g_textures[TEXTURE_TYPE_VICTORY_SCREEN]);
	g_theRenderer->DrawVertexArray(int(victoryImageVertexArray.size()), victoryImageVertexArray.data());
}


void Game::DrawDefeatScreen() const
{
	std::vector<Vertex_PCU> defeatImageVertexArray;
	AABB2 defeatImageBounds(Vec2(0.f, 0.f), Vec2(screenDimensionX, screenDimensionY));
	AddVertsForAABB2D(defeatImageVertexArray, defeatImageBounds, Rgba8::WHITE);
	g_theRenderer->BindTexture(g_textures[TEXTURE_TYPE_GAME_OVER_SCREEN]);
	g_theRenderer->DrawVertexArray(int(defeatImageVertexArray.size()), defeatImageVertexArray.data());
}


