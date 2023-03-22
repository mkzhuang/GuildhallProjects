#include "Engine/GUI/GUI_Textfield.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"

GUI_Textfield* currentTextfield;

GUI_Textfield::GUI_Textfield()
{
	SubscribeEventCallbackFunction("KeyPressed", Event_KeyPressed);
	SubscribeEventCallbackFunction("CharInput", Event_CharInput);
	currentTextfield = this;
}


GUI_Textfield::~GUI_Textfield()
{
	RemoveHandler();
}


void GUI_Textfield::Update()
{
	GUI_Text::Update();
}


void GUI_Textfield::Render(Renderer* renderer) const
{
	if (m_textBoxColor != Rgba8::CLEAR)
	{
		std::vector<Vertex_PCU> boxVerts;
		AddVertsForAABB2D(boxVerts, m_elementWindow, m_textBoxColor);
		renderer->BindTexture(nullptr);
		renderer->DrawVertexArray(boxVerts);
	}

	GUI_Text::Render(renderer);
}


void GUI_Textfield::SetTextBoxColor(Rgba8 const& color)
{
	m_textBoxColor = color;
	SetBufferDirty();
}


void GUI_Textfield::CallEventHandler()
{
	CallHandler(m_handlerName);
}


void GUI_Textfield::SetHandler(std::string const& handlerName)
{
	m_handlerName = handlerName;
}


void GUI_Textfield::RemoveHandler()
{
	UnregisterEventHandler(m_handlerName);
}


bool GUI_Textfield::Event_KeyPressed(EventArgs& args)
{
	if (!currentTextfield->m_isFocus) return false;

	unsigned char key = args.GetValue("key", (unsigned char)0);
	switch (key)
	{
		case 8: //backspace
			if (currentTextfield->m_text.size() > 0)
			{
				currentTextfield->m_text.erase(currentTextfield->m_text.size() - 1, 1);
				currentTextfield->SetBufferDirty();
				currentTextfield->CallEventHandler();
			}
			break;
	}

	return false;
}


bool GUI_Textfield::Event_CharInput(EventArgs& args)
{
	if (!currentTextfield->m_isFocus) return false;

	int charCode = args.GetValue("key", 0);
	if ((charCode >= 32) && (charCode <= 126) && (charCode != '`') && (charCode != '~'))
	{
		currentTextfield->m_text.insert(currentTextfield->m_text.size(), 1, static_cast<unsigned char>(charCode));
		currentTextfield->SetBufferDirty();
		currentTextfield->CallEventHandler();
	}

	return false;
}


