#pragma once
#include "Engine/GUI/GUI_Text.hpp"

class GUI_Button : public GUI_Text
{
public:
	GUI_Button();
	~GUI_Button();

	void Update();
	void Render(Renderer* renderer) const;

	void OnClick();

	void SetHandler(std::string const& handlerName);
	void RemoveHandler();

public:
	std::string m_handlerName = "";
};