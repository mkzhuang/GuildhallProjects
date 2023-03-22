#include "Game/App.hpp"
#include "Game/DatabaseClient.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Net/NetSystem.hpp"

#include <thread>

Window* g_theWindow;
Renderer* g_theRenderer;
AudioSystem* g_theAudio;
NetSystem* g_theNet = nullptr;
DatabaseClient* g_theClient;

static float consoleCameraDimensionX = 0.f;
static float consoleCameraDimensionY = 0.f;
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
	GUARANTEE_OR_DIE(g_theNet == nullptr, "Net System is not deleted!");
	GUARANTEE_OR_DIE(g_theClient == nullptr, "DatabaseClient is not deleted!");
}


void App::Startup()
{
	XmlDocument doc;
	doc.LoadFile("Data/GameConfig.xml");
	XmlElement* element = doc.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*element);

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

	NetSystemConfig netSystemConfig;
	g_theNet = new NetSystem(netSystemConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_consoleLines = g_gameConfigBlackboard.GetValue("devConsoleLines", 10.f);
	devConsoleConfig.m_renderer = g_theRenderer;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	DatabaseClientConfig databaseClientConfig;
	databaseClientConfig.console = g_theDevConsole;
	databaseClientConfig.serverAddr = g_gameConfigBlackboard.GetValue("serverAddr", "loopback");
	g_theClient = new DatabaseClient(databaseClientConfig);

	AudioSystemConfig audioSystemConfig;
	g_theAudio = new AudioSystem(audioSystemConfig);

	g_theNet->Startup();
	g_theDevConsole->Startup();
	g_theClient->Startup();
	g_theEventSystem->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();

	SubscribeEventCallbackFunction("QuitApp", Event_QuitApp);

	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Type help for a list of commands.");

	m_theGame = new Game(this);
	m_theGame->Startup();

	IntVec2 const& windowDimensions = Window::GetWindowContext()->GetClientDimensions();
	consoleCameraDimensionX = static_cast<float>(windowDimensions.x);
	consoleCameraDimensionY = static_cast<float>(windowDimensions.y);
	windowBounds.SetDimensions(Vec2(consoleCameraDimensionX, consoleCameraDimensionY));
	windowBounds.SetCenter(Vec2(consoleCameraDimensionX * 0.5f, consoleCameraDimensionY * 0.5f));
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
	m_theGame->Shuntdown();

	g_theAudio->Shutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->ShutDown();
	g_theClient->Shutdown();
	g_theDevConsole->ShutDown();
	g_theEventSystem->ShutDown();
	g_theNet->Shutdown();

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
	delete g_theClient;
	g_theClient = nullptr;
	delete g_theDevConsole;
	g_theDevConsole = nullptr;
	delete g_theEventSystem;
	g_theEventSystem = nullptr;
	delete g_theNet;
	g_theNet = nullptr;
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

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_theGame->ResetGame();
	}
}


void App::HandleQuitRequested()
{	
	g_isQuitting = true;
}


void App::BeginFrame()
{
	g_theNet->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theClient->BeginFrame();
	g_theEventSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
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
	m_devCamera.SetOrthoView(Vec2(0.f, 0.f), windowBounds.GetDimensions());
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
	g_theAudio->EndFrame();
	g_theRenderer->EndFrame();
	g_theWindow->EndFrame();
	g_theInput->EndFrame();
	g_theEventSystem->EndFrame();
	g_theClient->EndFrame();
	g_theDevConsole->EndFrame();
	g_theNet->EndFrame();
	std::this_thread::yield();
}


static bool Event_QuitApp(EventArgs& args)
{
	UNUSED(args)

	g_isQuitting = true;
	return false;
}


