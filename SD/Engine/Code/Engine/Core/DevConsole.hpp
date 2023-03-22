#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"

#include <mutex>

class RemoteConsole;

enum class DevConsoleMode
{
	HIDDEN,
	OPEN_FULL
};

struct DevConsoleConfig
{
	float m_consoleLines = 0.f;
	Renderer* m_renderer = nullptr;
	int m_maxCommandHistory = 128;
	double m_caretBlinkTimer = 0.5;
	bool m_hasRemoteConsole = false;
};

struct DevConsoleLine
{
	Rgba8 m_tint;
	std::string m_text = "";
};

class DevConsole
{
public:
	DevConsole(DevConsoleConfig const& config);
	~DevConsole();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	void ExecuteXmlCommandScriptFile(std::string const& XmlFilePath);
	void ExecuteXmlCommandScriptNode(XmlElement const& commandScriotXmlElement);
	void Execute(std::string const& consoleCommandText);
	void AddLine(Rgba8 const& color, std::string const& text);
	void ClearLines();
	void Render(AABB2 const& bounds, Renderer* rendererOverride=nullptr) const;

	DevConsoleMode GetMode() const;
	void SetMode(DevConsoleMode mode);
	void ToggleMode(DevConsoleMode mode);
	bool IsOpen() const;

	void ClearInput();
	void AddToCommandHistory(std::string input);
	void SetCaretPosition(int position);
	void RemoveCharacterOnLine(int direction);
	void ResetCaret();
	std::string const GetPreviousCommand();
	std::string const GetNextCommand();
	Clock const& GetClock() const;

	static Rgba8 const INFO_ERROR;
	static Rgba8 const INFO_WARNING;
	static Rgba8 const INFO_MAJOR;
	static Rgba8 const INFO_MINOR;
	static Rgba8 const INFO_INPUT;
	static Rgba8 const COLOR_CARET;

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_CharInput(EventArgs& args);
	static bool Event_Paste(EventArgs& args);
	static bool Command_Clear(EventArgs& args);
	static bool Command_Help(EventArgs& args);
	static bool Command_ExecuteCommandFromFile(EventArgs& args);

protected:
	void Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect=1.f) const;

protected:
	DevConsoleConfig m_config;
	DevConsoleMode m_mode = DevConsoleMode::HIDDEN;
	std::vector<DevConsoleLine> m_lines;
	int m_frameNumber = 0;
	Clock m_clock;
	std::string m_inputLine = "";
	int m_caretPosition = 0;
	bool m_caretVisible = true;
	Stopwatch m_caretStopwatch;
	std::vector<std::string> m_commandHistory;
	int m_historyIndex = -1;
	std::mutex m_devconsoleMutex;
};


