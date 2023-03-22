#include "Engine/GUI/UITextfield.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EventSystem.hpp"

UITextfield* textField;

UITextfield::UITextfield()
{
}


UITextfield::UITextfield(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, std::string const& text, BitmapFont* font, float textSize, Vec2 const& alignment, bool isDraggable)
	: UIElement(name, bound, anchor, renderer, isDraggable)
	, m_text(text)
	, m_font(font)
	, m_textSize(textSize)
	, m_alignment(alignment)
{
	SubscribeEventCallbackFunction("KeyPressed", Event_KeyPressed);
	SubscribeEventCallbackFunction("CharInput", Event_CharInput);
	textField = this;

	m_focusColor = Rgba8::GRAY;
}


UITextfield::~UITextfield()
{
}


void UITextfield::Update()
{
	m_inputThisFrame = false;
	if (m_isBufferDirty)
	{
		m_verts.clear();
		m_font->AddVertsForTextInBox2D(m_verts, m_screenWindow, m_textSize, m_text, m_color, 0.6f, m_alignment);
		m_renderer->CopyCPUToGPU(m_verts.data(), sizeof(Vertex_PCU) * m_verts.size(), m_vertexBuffer);
		m_isBufferDirty = false;
	}

	for (UIElement* child : m_children)
	{
		child->Update();
	}
}


void UITextfield::Render() const
{
	std::vector<Vertex_PCU> verts;
	if (m_isFocus)
	{
		AddVertsForAABB2D(verts, GetScreenWindow(), GetFocusColor());
	}
	if (m_isHover && !m_isFocus)
	{
		AddVertsForAABB2D(verts, GetScreenWindow(), GetHoverColor());
	}
	m_renderer->BindTexture(nullptr);
	m_renderer->DrawVertexArray(verts);

	if (m_text != "")
	{
		m_renderer->BindTexture(&m_font->GetTexture());
		m_renderer->DrawVertexBuffer(m_vertexBuffer, (int)m_verts.size());
	}

	for (UIElement* child : m_children)
	{
		if (child) child->Render();
	}
}


void UITextfield::OnClick()
{
	if (m_parent) m_parent->OnClick();
}


void UITextfield::SetText(std::string const& text)
{
	m_text = text;
	m_isBufferDirty = true;
}


void UITextfield::SetTextSize(float textSize)
{
	m_textSize = textSize;
	m_isBufferDirty = true;
}


void UITextfield::SetAlignment(Vec2 const& alignment)
{
	m_alignment = alignment;
	m_isBufferDirty = true;
}


std::string const& UITextfield::GetText()
{
	return m_text;
}


bool UITextfield::HasInputThisFrame()
{
	return m_inputThisFrame;
}


bool UITextfield::Event_KeyPressed(EventArgs& args)
{
	if (!textField->m_isFocus) return false;

	unsigned char key = static_cast<unsigned char>(args.GetValue("key", "")[0]);
	switch (key)
	{
		case 8: //backspace
			if (textField->m_text.size() > 0)
			{
				textField->m_text.erase(textField->m_text.size() - 1, 1);
				textField->m_isBufferDirty = true;
				textField->m_inputThisFrame = true;
			}
			break;
	}

	return false;
}


bool UITextfield::Event_CharInput(EventArgs& args)
{
	if (!textField->m_isFocus) return false;

	int charCode = args.GetValue("key", 0);
	if ((charCode >= 32) && (charCode <= 126) && (charCode != '`') && (charCode != '~'))
	{
		textField->m_text.insert(textField->m_text.size(), 1, static_cast<unsigned char>(charCode));
		textField->m_isBufferDirty = true;
		textField->m_inputThisFrame = true;
	}

	return false;
}


