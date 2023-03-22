#include "Game/App.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/ChunkGenerationJob.hpp"

#include <thread>

Window* g_theWindow;
Renderer* g_theRenderer;
AudioSystem* g_theAudio;
JobSystem* g_theJobSystem;

static float UICameraSizeX = 0.f;
static float UICameraSizeY = 0.f;
AABB2 windowBounds;

App::App()
{

}


App::~App()
{
	GUARANTEE_OR_DIE(g_theDevConsole == nullptr, "Developer Console is not deleted!");
	GUARANTEE_OR_DIE(g_theEventSystem == nullptr, "Event System is not deleted!");
	GUARANTEE_OR_DIE(g_theInput == nullptr, "Input System is not deleted!");
	GUARANTEE_OR_DIE(g_theWindow == nullptr, "Window is not deleted!");
	GUARANTEE_OR_DIE(g_theRenderer == nullptr, "Renderer is not deleted!");
	GUARANTEE_OR_DIE(g_theAudio == nullptr, "Audio System is not deleted!");
}


void App::Startup()
{
	XmlDocument doc;
	doc.LoadFile("Data/GameConfig.xml");
	XmlElement* element = doc.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*element);
	UICameraSizeX = g_gameConfigBlackboard.GetValue("UICameraDimensionX", 20.f);
	UICameraSizeY = g_gameConfigBlackboard.GetValue("UICameraDimensionY", 10.f);
	windowBounds.SetDimensions(Vec2(UICameraSizeX, UICameraSizeY));
	float UIhalfDimensionX = 0.5f * UICameraSizeX;
	g_gameConfigBlackboard.SetValue("UICameraCenterX", std::to_string(UIhalfDimensionX));
	float UIhalfDimensionY = 0.5f * UICameraSizeY;
	g_gameConfigBlackboard.SetValue("UICameraCenterY", std::to_string(UIhalfDimensionY));
	windowBounds.SetCenter(Vec2(UIhalfDimensionX, UIhalfDimensionY));
	float worldCameraDimensionX =  g_gameConfigBlackboard.GetValue("worldCameraMaxX", 0.f) - g_gameConfigBlackboard.GetValue("worldCameraMinX", 0.f);
	float worldCenterX = g_gameConfigBlackboard.GetValue("worldCameraMinX", 0.f) + 0.5f * worldCameraDimensionX;
	g_gameConfigBlackboard.SetValue("worldCameraCenterX", std::to_string(worldCenterX));
	float worldCameraDimensionY = g_gameConfigBlackboard.GetValue("worldCameraMaxY", 0.f) - g_gameConfigBlackboard.GetValue("worldCameraMinY", 0.f);
	float worldCenterY = g_gameConfigBlackboard.GetValue("worldCameraMinX", 0.f) + 0.5f * worldCameraDimensionY;
	g_gameConfigBlackboard.SetValue("worldCameraCenterY", std::to_string(worldCenterY));

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_windowTitle = g_gameConfigBlackboard.GetValue("gameTitle", "untitled game");
	windowConfig.m_clientAspect = g_gameConfigBlackboard.GetValue("windowAspect", 1.f);
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window(windowConfig);

	RendererConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(renderConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_consoleLines = g_gameConfigBlackboard.GetValue("devConsoleLines", 10.f);
	devConsoleConfig.m_renderer = g_theRenderer;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	AudioSystemConfig audioSystemConfig;
	g_theAudio = new AudioSystem(audioSystemConfig);

	JobSystemConfig jobSystemConfig;
	jobSystemConfig.m_numberWorkerThreads = std::thread::hardware_concurrency();
	g_theJobSystem = new JobSystem(jobSystemConfig);

	g_theDevConsole->Startup();
	g_theEventSystem->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();
	g_theJobSystem->Startup();

	for (int threadIndex = 0; threadIndex < jobSystemConfig.m_numberWorkerThreads; threadIndex++)
	{
		g_theJobSystem->SetJobTypeForWorker(threadIndex, CHUNK_GEN_JOB_TYPE);
	}

	SubscribeEventCallbackFunction("QuitApp", Event_QuitApp);

	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Type help for a list of commands.");

	m_theGame = new Game(this);
	m_theGame->Startup();
}


void App::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}


void App::Shutdown()
{
	g_theJobSystem->ShutDown();
	g_theAudio->Shutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->ShutDown();
	g_theDevConsole->ShutDown();
	g_theEventSystem->ShutDown();

	delete m_theGame;
	m_theGame = nullptr;

	delete g_theJobSystem;
	g_theJobSystem = nullptr;
	delete g_theAudio;
	g_theAudio = nullptr;
	delete g_theRenderer;
	g_theRenderer = nullptr;
	delete g_theWindow;
	g_theWindow = nullptr;
	delete g_theInput;
	g_theInput = nullptr;
	delete g_theDevConsole;
	g_theDevConsole = nullptr;
	delete g_theEventSystem;
	g_theEventSystem = nullptr;
}


void App::Run()
{
	while (!g_isQuitting)
	{
		RunFrame();
	}
}


void App::Quit()
{

}


void App::HandleDeveloperCheatCode()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleMode(DevConsoleMode::OPEN_FULL);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		g_isDebugging = !g_isDebugging;
	}
}


void App::HandleQuitRequested()
{	
	g_isQuitting = true;
}


void App::BeginFrame()
{
	g_theDevConsole->BeginFrame();
	g_theEventSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
	g_theJobSystem->BeginFrame();
	Clock::SystemBeginFrame();
}


void App::Update()
{
	HandleDeveloperCheatCode();
	if (g_theDevConsole->IsOpen())
	{
		g_theInput->ConsumeAllKeys();
	}
	m_theGame->Update();
	m_devCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(UICameraSizeX, UICameraSizeY));
}


void App::Render() const
{
	m_theGame->Render();

	g_theRenderer->BeginCamera(m_devCamera);
	g_theDevConsole->Render(windowBounds);
	g_theRenderer->EndCamera(m_devCamera);
}


void App::EndFrame()
{
	g_theJobSystem->EndFrame();
	g_theAudio->EndFrame();
	g_theRenderer->EndFrame();
	g_theWindow->EndFrame();
	g_theInput->EndFrame();
	g_theEventSystem->EndFrame();
	g_theDevConsole->EndFrame();
	std::this_thread::yield();
}


static bool Event_QuitApp(EventArgs& args)
{
	UNUSED(args)

	g_isQuitting = true;
	return false;
}


