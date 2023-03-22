#include "Engine/GUI/GUI_Button.hpp"
#include "Engine/Core/EventSystem.hpp"

GUI_Button::GUI_Button()
{
}


GUI_Button::~GUI_Button()
{
	RemoveHandler();
}


void GUI_Button::Update()
{
	GUI_Text::Update();
}


void GUI_Button::Render(Renderer* renderer) const
{
	GUI_Text::Render(renderer);
}


void GUI_Button::OnClick()
{
	CallHandler(m_handlerName);
}


void GUI_Button::SetHandler(std::string const& handlerName)
{
	m_handlerName = handlerName;
}


void GUI_Button::RemoveHandler()
{
	UnregisterEventHandler(m_handlerName);
}


