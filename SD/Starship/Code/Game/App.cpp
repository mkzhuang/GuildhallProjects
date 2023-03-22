#include "Game/App.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <thread>

Window* g_theWindow;
Renderer* g_theRenderer;
AudioSystem* g_theAudio;

static float screenCameraSizeX = 0.f;
static float screenCameraSizeY = 0.f;
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
	screenCameraSizeX = g_gameConfigBlackboard.GetValue("screenCameraDimensionX", 20.f);
	screenCameraSizeY = g_gameConfigBlackboard.GetValue("screenCameraDimensionY", 10.f);
	windowBounds.SetDimensions(Vec2(screenCameraSizeX, screenCameraSizeY));
	float screenhalfDimensionX = 0.5f * screenCameraSizeX;
	g_gameConfigBlackboard.SetValue("screenCameraCenterX", std::to_string(screenhalfDimensionX));
	float screenhalfDimensionY = 0.5f * screenCameraSizeY;
	g_gameConfigBlackboard.SetValue("screenCameraCenterY", std::to_string(screenhalfDimensionY));
	windowBounds.SetCenter(Vec2(screenhalfDimensionX, screenhalfDimensionY));
	float worldhalfDimensionX = 0.5f * g_gameConfigBlackboard.GetValue("worldCameraDimensionX", 0.f);
	g_gameConfigBlackboard.SetValue("worldCameraCenterX", std::to_string(worldhalfDimensionX));
	float worldhalfDimensionY = 0.5f * g_gameConfigBlackboard.GetValue("worldCameraDimensionY", 0.f);
	g_gameConfigBlackboard.SetValue("worldCameraCenterY", std::to_string(worldhalfDimensionY));

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

	g_theDevConsole->Startup();
	g_theEventSystem->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();

	SubscribeEventCallbackFunction("QuitApp", QuitApp);

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
	g_theAudio->Shutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->ShutDown();
	g_theDevConsole->ShutDown();
	g_theEventSystem->ShutDown();

	delete m_theGame;
	m_theGame = nullptr;

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

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		delete m_theGame;
		m_theGame = nullptr;
		m_theGame = new Game(this);
		m_theGame->Startup();
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
	Clock::GetSystemClock().SystemBeginFrame();
}


void App::Update()
{
	HandleDeveloperCheatCode();
	if (g_theDevConsole->IsOpen())
	{
		g_theInput->ConsumeAllKeys();
	}
	m_theGame->Update();
}


void App::Render() const
{
	m_theGame->Render();

	g_theDevConsole->Render(windowBounds);
}


void App::EndFrame()
{
	g_theAudio->EndFrame();
	g_theRenderer->EndFrame();
	g_theWindow->EndFrame();
	g_theInput->EndFrame();
	g_theEventSystem->EndFrame();
	g_theDevConsole->EndFrame();
	std::this_thread::yield();
}


static bool QuitApp(EventArgs& args)
{
	UNUSED(args)

	g_isQuitting = true;
	return false;
}


