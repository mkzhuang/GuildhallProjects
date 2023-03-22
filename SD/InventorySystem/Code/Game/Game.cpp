#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Game/InventorySystem.hpp"
#include "Game/Chest.hpp"
#include "Game/WorldInventory.hpp"
#include "Game/Filter.hpp"
#include "Game/Item.hpp"
#include "Game/Inventory.hpp"
#include "Game/ItemKeyTable.hpp"
#include "Game/ItemFactory.hpp"
#include "Game/DatabaseClient.hpp"
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
#include "Engine/Mesh/MeshBuilder.hpp"
#include "Engine/Mesh/Mesh.hpp"
#include "Engine/GUI/GUI_Element.hpp"
#include "Engine/GUI/GUI_Canvas.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Game.hpp"

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
static float UICameraDimensionX = 0.f;
static float UICameraDimensionY = 0.f;
static double slowMoMultiplier = 0.f;
static double fastMoMultiplier = 0.f;
static float joystickMenuDelay = 0.f;
static float joystickMenuSensitivity = 0.f;

int fpsrecodingCount = 0;
float sumOfFPS = 0.f;
float fps = 60.f;

Game::Game(App* const& owner)
	: m_theOwner(owner)
	, m_gameClock(Clock::GetSystemClock())
{
	worldCameraMinX = g_gameConfigBlackboard.GetValue("worldCameraMinX", 0.f);
	worldCameraMinY = g_gameConfigBlackboard.GetValue("worldCameraMinY", 0.f);
	worldCameraMaxX = g_gameConfigBlackboard.GetValue("worldCameraMaxX", 0.f);
	worldCameraMaxY = g_gameConfigBlackboard.GetValue("worldCameraMaxY", 0.f);
	worldCameraDimensionX =  worldCameraMaxX - worldCameraMinX;
	worldCameraDimensionY =  worldCameraMaxY - worldCameraMinY;
	IntVec2 const& windowDimensions = Window::GetWindowContext()->GetClientDimensions();
	UICameraDimensionX = static_cast<float>(windowDimensions.x);
	UICameraDimensionY = static_cast<float>(windowDimensions.y);
	slowMoMultiplier = static_cast<double>(g_gameConfigBlackboard.GetValue("debugSlowMoMultiplier", 1.f));
	fastMoMultiplier = static_cast<double>(g_gameConfigBlackboard.GetValue("debugFastMoMultiplier", 1.f));
	joystickMenuDelay = g_gameConfigBlackboard.GetValue("joystickMenuDelay", 0.1f);
	joystickMenuSensitivity = g_gameConfigBlackboard.GetValue("joystickMenuSensitivity", 0.1f);

	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/UI/panel_brown.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/UI/scroll_border.jpg");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/UI/panel_blue.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/UI/arrow_up.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/UI/arrow_down.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/applepie/applepie.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/beefstew/beefstew.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/book01/book01.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/book02/book02.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/bread/bread.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/daedric_sword/daedric_sword.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/dawnguard_helmet/dawnguard_helmet.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/dragonscale_armor/dragonscale_armor.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/dwarven_greatsword/dwarven_greatsword.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/dwarven_staff/dwarven_staff.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/ebony_dagger/ebony_dagger.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/glass_hammer/glass_hammer.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/glass_mace/glass_mace.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/goatskin/goatskin.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/iron_battleaxe/iron_battleaxe.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/iron_bow/iron_bow.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/iron_shield/iron_shield.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/iron_waraxe/iron_waraxe.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/ladle/ladle.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/grand_soulgem/grand_soulgem.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Models/sweetroll/sweetroll.png");
	g_theRenderer->CreateOrGetShader("Data/Shaders/SpriteLit");
	g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/VeraMonoFont");

	attractMusic = g_theAudio->CreateOrGetSound("Data/Audio/C418 - Minecraft - Volume Alpha - 13 Wet Hands.mp3");
	gameMusic = g_theAudio->CreateOrGetSound("Data/Audio/C418 - Minecraft - Volume Alpha - 15 Chris.mp3");
	selectSound = g_theAudio->CreateOrGetSound("Data/Audio/Select.wav");
	pauseSound = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
	unpauseSound = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");

	Filter::InitializeFilter();
	ItemKeyTable::AddItemKey(ItemKeyType::String, "name");
	ItemKeyTable::AddItemKey(ItemKeyType::Float, "weight");
	ItemKeyTable::AddItemKey(ItemKeyType::Float, "value");
	ItemKeyTable::AddItemKey(ItemKeyType::String, "category");
	ItemKeyTable::AddItemKey(ItemKeyType::String, "model");
	ItemKeyTable::AddItemKey(ItemKeyType::String, "texture");
	ItemKeyTable::AddItemKey(ItemKeyType::Float, "scale");

	std::string user = g_gameConfigBlackboard.GetValue("user", "");
	std::string password = g_gameConfigBlackboard.GetValue("password", "");
	g_theClient->SendLoginRequest(user, password);
	g_theClient->WaitUntilConnectionIsProcessed();
	std::string inventoryID = g_theClient->GetLastReceivedData();

	//g_theClient->SendViewItemFactoryRequest();
	//g_theClient->WaitUntilConnectionIsProcessed();
	//std::string itemFactoryString = g_theClient->GetLastReceivedData();
	//ItemFactory::LoadFromJsonString(itemFactoryString);
	ItemFactory::LoadFromFile("Data/Inventory/Items.xml");

	InventorySystemConfig inventorySystemConfig;
	inventorySystemConfig.inventoryLayoutPath = "Data/Inventory/InventoryLayout.xml";
	inventorySystemConfig.buttonEventPath = "Data/Inventory/InventoryButtonEvents.xml";
	inventorySystemConfig.renderer = g_theRenderer;
	inventorySystemConfig.inventoryID = inventoryID;
	inventorySystemConfig.canvasBounds = AABB2(Vec2::ZERO, Vec2(UICameraDimensionX, UICameraDimensionY));
	m_inventory = new InventorySystem(inventorySystemConfig);

	g_theClient->GetInventoryContent(inventoryID);
	g_theClient->WaitUntilConnectionIsProcessed();
	std::string inventoryContent = g_theClient->GetLastReceivedData();
	m_inventory->LoadItems(inventoryContent);
	//m_inventory->Startup();

	InventorySystemConfig chestConfig;
	chestConfig.inventoryLayoutPath = "Data/Inventory/ChestLayout.xml";
	chestConfig.buttonEventPath = "Data/Inventory/ChestButtonEvents.xml";
	chestConfig.inventory = m_inventory->m_inventory;
	chestConfig.renderer = g_theRenderer;
	chestConfig.inventoryID = "1";
	chestConfig.canvasBounds = AABB2(Vec2::ZERO, Vec2(UICameraDimensionX, UICameraDimensionY));
	m_chest = new Chest(chestConfig);

	g_theClient->GetInventoryContent(m_chest->m_inventoryID);
	g_theClient->WaitUntilConnectionIsProcessed();
	std::string chestContent = g_theClient->GetLastReceivedData();
	m_chest->LoadItemsFromJsonString(chestContent);
	//m_chest->Startup();

	InventorySystemConfig worldInventoryConfig;
	worldInventoryConfig.inventory = m_inventory->m_inventory;
	worldInventoryConfig.renderer = g_theRenderer;
	worldInventoryConfig.inventoryID = "2";
	m_worldInventory = new WorldInventory(worldInventoryConfig);

	g_theClient->GetInventoryContent(m_worldInventory->m_inventoryID);
	g_theClient->WaitUntilConnectionIsProcessed();
	std::string worldInventoryContent = g_theClient->GetLastReceivedData();
	m_worldInventory->LoadItemsFromJsonString(worldInventoryContent);

	g_theClient->GetWorldLocation(m_worldInventory->m_inventoryID);
	g_theClient->WaitUntilConnectionIsProcessed();
	std::string worldInventoryPos = g_theClient->GetLastReceivedData();
	m_worldInventory->LoadItemPosFromJsonString(worldInventoryPos);

	m_fpsWatch.Start(&m_gameClock, 0.05);
	g_theGame = this;
}


void Game::Startup()
{
	musicPlaybackID = PlaySound(attractMusic, true, g_gameSoundVolume);
	m_player = new Player(this, Vec3(1.f, 1.f, 0.f));
	m_bounds = AABB3(Vec3::ZERO, Vec3(10.f, 5.f, 3.f));
	SetRoomVerts();

	m_worldCamera.SetRenderTransform(Vec3(0.f, -1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f));
	m_worldCamera.SetTransform(Vec3(3.f, -4.f, 5.f), EulerAngles(75.f, 30.f, 0.f));

	DebugAddWorldBasis(Mat44(), -1.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);

	Mat44 xAxis;
	xAxis.Append(Mat44::CreateTranslation3D(Vec3(0.1f, 0.f, 0.1f)));
	xAxis.Append(Mat44::CreateXRotationDegrees(90.f));
	DebugAddWorldText("x - forward", xAxis, 0.1f, Vec2::ZERO, -1.f, Rgba8::RED, Rgba8::RED, DebugRenderMode::USEDEPTH);

	Mat44 yAxis;
	yAxis.Append(Mat44::CreateTranslation3D(Vec3(-0.1f, 0.1f, 0.f)));
	yAxis.Append(Mat44::CreateZRotationDegrees(90.f));
	DebugAddWorldText("y - left", yAxis, 0.1f, Vec2::ZERO, -1.f, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USEDEPTH);

	Mat44 zAxis;
	zAxis.Append(Mat44::CreateTranslation3D(Vec3(0.f, 0.1f, 0.1f)));
	zAxis.Append(Mat44::CreateYRotationDegrees(-90.f));
	DebugAddWorldText("z - up", zAxis, 0.1f, Vec2::ZERO, -1.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USEDEPTH);
}


void Game::Shuntdown()
{
	delete m_player;
	m_player = nullptr;
	m_inventory->Shutdown();
	delete m_inventory;
	m_inventory = nullptr;
	m_chest->Shutdown();
	delete m_chest;
	m_chest = nullptr;
	delete m_worldInventory;
	m_worldInventory = nullptr;
}


void Game::Update()
{
	if (m_fpsWatch.CheckDurationElapsedAndDecrement())
	{
		fpsrecodingCount++;
		sumOfFPS += 1.f / static_cast<float>(m_gameClock.GetDeltaTime());
	}
	if (fpsrecodingCount == 10)
	{
		fps = sumOfFPS / 10.f;
		fpsrecodingCount = 0;
		sumOfFPS = 0.f;
	}

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
		if (g_theDevConsole->IsOpen() || m_inventory->IsOpen() || m_chest->IsOpen())
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
	if (!m_chest->IsOpen() && !m_inventory->IsOpen())
	{
		m_player->Update(deltaSeconds);
		m_player->KeepInBound(m_bounds);
	}
	m_inventory->Update();
	m_chest->Update();
	m_worldInventory->Update(deltaSeconds);

	std::string gameClockInfo = Stringf("Game           | Time: %.2f, FPS: %.2f, Dilation: %.2f", m_gameClock.GetTotalTime(), 1.0 / m_gameClock.GetDeltaTime(), m_gameClock.GetTimeDilation());
	DebugAddScreenText(gameClockInfo, Vec2(840.f, 840.f), 0.f, Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE);

	Clock const& devConsoleClock = g_theDevConsole->GetClock();
	std::string devConsoleClockInfo = Stringf("Dev Console    | Time: %.2f, FPS: %.2f, Dilation: %.2f", devConsoleClock.GetTotalTime(), 1.0 / devConsoleClock.GetDeltaTime(), devConsoleClock.GetTimeDilation());
	DebugAddScreenText(devConsoleClockInfo, Vec2(840.f, 820.f), 0.f, Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE);

	Clock const& debugClock = DebugRenderGetClock();
	std::string debugClockInfo = Stringf("Debug Renderer | Time: %.2f, FPS: %.2f, Dilation: %.2f", debugClock.GetTotalTime(), 1.0 / debugClock.GetDeltaTime(), debugClock.GetTimeDilation());
	DebugAddScreenText(debugClockInfo, Vec2(840.f, 800.f), 0.f, Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE);

	Vec3 displacement = m_player->m_position - m_chest->m_position;
	float distance = displacement.GetLength();
	m_chest->m_isPlayerNear = distance < 1.5f ? true : false;

	std::string keyInfo = m_chest->m_isPlayerNear ? "Press I for inventory, K for chest" : "Press I for inventory";
	DebugAddMessage(keyInfo, 0.f, Rgba8::BLUE, Rgba8::BLUE);

	if (!m_inventory->m_canvas->IsTextfieldFocused())
	{
		if (g_theInput->WasKeyJustPressed('I') && !m_chest->IsOpen())
		{
			m_inventory->ToggleOpen();
		}

		if (g_theInput->WasKeyJustPressed('K') && !m_inventory->IsOpen() && m_chest->m_isPlayerNear)
		{
			m_chest->ToggleOpen();
		}

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
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		if (m_inventory->IsOpen())
		{
			m_inventory->ToggleOpen();
		}
		else
		{
			PlaySound(selectSound, false, g_gameSoundVolume);
			ResetGame();
		}
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
	m_worldCamera.SetPerspectiveView(g_theWindow->GetAspect(), 60.f, 0.1f, 100.f);
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
	AddVertsForTextTriangles2D(titleVertexArray, gameTitle, Vec2(32.f, 732.f), 80.f, Rgba8(50, 50, 50, static_cast<unsigned char>(120 - offset)));
	AddVertsForTextTriangles2D(titleVertexArray, gameTitle, Vec2(40.f, 740.f), 80.f, Rgba8(100, 100, 255, static_cast<unsigned char>(150 - offset)));
	AddVertsForTextTriangles2D(titleVertexArray, "Music: C418 - Wet Hands", Vec2(40.f, 40.f), 16.f, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(titleVertexArray.size()), titleVertexArray.data());

	Rgba8 ringColor(static_cast<unsigned char>(155 - offset), static_cast<unsigned char>(155 + offset), static_cast<unsigned char>(155 + offset), static_cast<unsigned char>(150 - offset));
	DebugDrawRing(Vec2(800.f, 400.f), 160.f + offset, 20.f - 0.1f * offset, ringColor);

	g_theRenderer->EndCamera(m_UICamera);
}


void Game::RenderGameplay() const
{
	g_theRenderer->ClearScreen(Rgba8(100, 125, 200, 255));

	g_theRenderer->BeginCamera(m_worldCamera);

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
	g_theRenderer->SetSamplerState(SamplerMode::POINTCLAMP);

	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	g_theRenderer->BindTexture(texture);
	g_theRenderer->DrawVertexArray(m_roomVerts);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetSamplerState(SamplerMode::POINTCLAMP);

	m_player->Render();
	m_chest->RenderModel();
	m_worldInventory->Render();

	g_theRenderer->EndCamera(m_worldCamera);



	DebugRenderWorld(m_worldCamera);

	g_theRenderer->BeginCamera(m_UICamera);

	m_inventory->Render();
	m_chest->Render();

	g_theRenderer->EndCamera(m_UICamera);

	g_theRenderer->ClearDepth();
	m_inventory->RenderModel();

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


void Game::SwitchToGameplayMode()
{
	m_nextGameMode = GameMode::GAMEPLAY;
	g_theAudio->StopSound(musicPlaybackID);
	musicPlaybackID = PlaySound(gameMusic, true, g_gameSoundVolume);
}


void Game::ResetGame()
{
	m_currentGameMode = GameMode::ATTRACT;
	m_nextGameMode = GameMode::ATTRACT;
	g_theAudio->StopSound(musicPlaybackID);
	delete m_player;
	m_player = nullptr;
	if (m_inventory->IsOpen()) m_inventory->ToggleOpen();
	if (m_chest->IsOpen()) m_chest->ToggleOpen();

	Startup();
}


float Game::GetTotalGameTime() const
{
	return static_cast<float>(m_gameClock.GetTotalTime());
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


void Game::SetRoomVerts()
{
	Vec3 corners[8] = {};
	m_bounds.GetCorners(corners);
	Vec3 nearBL = corners[0];
	Vec3 nearBR = corners[1];
	Vec3 nearTR = corners[2];
	Vec3 nearTL = corners[3];
	Vec3 farBL = corners[4];
	Vec3 farBR = corners[5];
	Vec3 farTR = corners[6];
	Vec3 farTL = corners[7];

	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	IntVec2 grid(8, 8);
	SpriteSheet spriteSheet(*texture, grid);

	AABB2 wallUV = spriteSheet.GetSpriteUVs(IntVec2(1, 6));
	AABB2 floorUV = spriteSheet.GetSpriteUVs(IntVec2(0, 5));

	// back wall
	{
		Vec3 offset = farTR - farBL;
		int xSteps = static_cast<int>(fabsf(offset.x));
		int zSteps = static_cast<int>(fabsf(offset.z));
		for (int z = 0; z < zSteps; z++)
		{
			for (int x = 0; x < xSteps; x++)
			{
				Vec3 mins = farBL + Vec3(static_cast<float>(x) * 1.f, 0.f, static_cast<float>(z) * 1.f);
				Vec3 maxs = mins + Vec3(1.f, 0.f, 1.f);
				AddVertsForQuad3D(m_roomVerts, mins, Vec3(maxs.x, mins.y, mins.z), maxs, Vec3(mins.x, mins.y, maxs.z), Rgba8::WHITE, wallUV);
			}
		}
	}

	// right wall
	{
		Vec3 offset = nearTR - farBR;
		int ySteps = static_cast<int>(fabsf(offset.y));
		int zSteps = static_cast<int>(fabsf(offset.z));
		for (int z = 0; z < zSteps; z++)
		{
			for (int y = 0; y < ySteps; y++)
			{
				Vec3 mins = farBR + Vec3(0.f, static_cast<float>(y) * -1.f, static_cast<float>(z) * 1.f);
				Vec3 maxs = mins + Vec3(0.f, -1.f, 1.f);
				AddVertsForQuad3D(m_roomVerts, mins, Vec3(mins.x, maxs.y, mins.z), maxs, Vec3(mins.x, mins.y, maxs.z), Rgba8::WHITE, wallUV);
			}
		}
	}

	// bottom floor
	{
		Vec3 offset = farBR - nearBL;
		int xSteps = static_cast<int>(fabsf(offset.x));
		int ySteps = static_cast<int>(fabsf(offset.y));
		for (int y = 0; y < ySteps; y++)
		{
			for (int x = 0; x < xSteps; x++)
			{
				Vec3 mins = nearBL + Vec3(static_cast<float>(x) * 1.f, static_cast<float>(y) * 1.f, 0.f);
				Vec3 maxs = mins + Vec3(1.f, 1.f, 0.f);
				AddVertsForQuad3D(m_roomVerts, mins, Vec3(maxs.x, mins.y, mins.z), maxs, Vec3(mins.x, maxs.y, mins.z), Rgba8::WHITE, floorUV);
			}
		}
	}
}


