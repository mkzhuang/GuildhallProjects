#pragma once
#include "Engine/Core/EngineCommon.hpp"

class Renderer;
class InputSystem;
class UICanvas;
class BitmapFont;

struct UISystemConfig
{
	Renderer* renderer = nullptr;
	InputSystem* input = nullptr;
	BitmapFont* font = nullptr;
};

class UISystem
{
public:
	UISystem(UISystemConfig const& config);
	~UISystem();

	void Startup();
	void Shutdown();

	void Update();
	void Render() const;
	void RenderModel() const;

	bool IsOpen() const;
	void LoadLayout(std::string layoutPath);

	void ToggleOpen();


public:
	UISystemConfig m_config;
	Renderer* m_renderer = nullptr;
	InputSystem* m_input = nullptr;
	BitmapFont* m_font = nullptr;
	UICanvas* m_canvas = nullptr;
	bool m_isOpen = false;
};