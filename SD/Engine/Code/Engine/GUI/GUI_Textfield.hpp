#pragma once
#include "Engine/GUI/GUI_Text.hpp"
#include "Engine/Core/EngineCommon.hpp"

class GUI_Textfield : public GUI_Text
{
public:
	GUI_Textfield();
	~GUI_Textfield();

	void Update();
	void Render(Renderer* renderer) const;

	void SetTextBoxColor(Rgba8 const& color);

	void CallEventHandler();
	void SetHandler(std::string const& handlerName);
	void RemoveHandler();

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_CharInput(EventArgs& args);

public:
	Rgba8 m_textBoxColor = Rgba8::CLEAR;
	std::string m_handlerName = "";
};