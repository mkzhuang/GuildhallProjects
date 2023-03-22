#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"

class InputSystem;

struct WindowConfig
{
	InputSystem* m_inputSystem = nullptr;
	std::string m_windowTitle = "Untitled App";
	float m_clientAspect = 2.0f;
	bool m_isFullscreen = false;
};


class Window
{
public:
	Window(WindowConfig const& config);
	~Window();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	WindowConfig const& GetConfig() const;
	void* GetOSWindowHandle() const {return m_osWindowHandle;}
	static Window* GetWindowContext();
	Vec2 GetNormalizedCursorPos() const;
	IntVec2 GetClientDimensions() const;
	float GetAspect() const;
	bool HasFocus() const;

protected:
	void CreateOSWindow();
	void RunMessagePump();

protected:
	WindowConfig m_config;
	void* m_osWindowHandle = nullptr;
	static Window* s_mainWindow;
	IntVec2 m_windowDimensions;
};


