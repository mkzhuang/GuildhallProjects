#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <string>

class Renderer;
class GUI_Canvas;

class GameInfo
{
public:
	GameInfo(Renderer* renderer, AABB2 const& infoBound);
	~GameInfo();

	void Update();
	void Render() const;

	bool AddLog(EventArgs& args);
	bool UpdatePlayerName(EventArgs& args);
	bool UpdateStatus(EventArgs& args);
	bool UpdateItems(EventArgs& args);

public:
	Renderer* m_renderer = nullptr;
	GUI_Canvas* m_canvas = nullptr;
};