#include "Engine/GUI/GUI_Text.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

GUI_Text::GUI_Text()
{
}

GUI_Text::~GUI_Text()
{
}


void GUI_Text::Update()
{

	if (m_isBufferDirty)
	{
		GUI_Element::ReconstructElement();
		CreateTextVerts();
		m_isBufferDirty = false;
	}

	GUI_Element::Update();
}


void GUI_Text::Render(Renderer* renderer) const
{
	GUI_Element::Render(renderer);

	if (m_textVerts.size() != 0)
	{
		//renderer->SetSamplerState(SamplerMode::BILINEARCLAMP);
		renderer->BindShader(m_shader);
		renderer->BindTexture(&m_font->GetTexture());
		if (m_hasDropShadow)
		{
			renderer->DrawVertexArray(m_shadowVerts);
		}
		renderer->DrawVertexArray(m_textVerts);
		//renderer->SetSamplerState(SamplerMode::POINTCLAMP);
		renderer->BindShader(nullptr);
	}
}


void GUI_Text::SetText(std::string const& text)
{
	m_text = text;
	SetBufferDirty();
}

void GUI_Text::SetFont(BitmapFont* font)
{
	m_font = font;
	SetBufferDirty();
}


void GUI_Text::SetShader(Shader* shader)
{
	m_shader = shader;
	SetBufferDirty();
}


void GUI_Text::SetHasDropShadow(bool hasDropShadow)
{
	m_hasDropShadow = hasDropShadow;
	SetBufferDirty();
}


void GUI_Text::SetTextColor(Rgba8 const& color)
{
	m_textColor = color;
	SetBufferDirty();
}


void GUI_Text::SetShadowColor(Rgba8 const& color)
{
	m_textShadowColor = color;
	SetBufferDirty();
}


void GUI_Text::SetTextHeight(float textHeight)
{
	m_textHeight = textHeight;
	SetBufferDirty();
}

void GUI_Text::SetTextAspect(float textAspect)
{
	m_textAspect = textAspect;
	SetBufferDirty();
}

void GUI_Text::SetTextAlignment(Vec2 const& textAlignment)
{
	m_textAlignment = textAlignment;
	SetBufferDirty();
}


void GUI_Text::SetIsParentRelative(bool isParentRelative, AABB2 const& boundsUVs)
{
	m_isParentWindowRelative = isParentRelative;
	m_relativeBoundUVs = boundsUVs;
	SetBufferDirty();
}


std::string const& GUI_Text::GetText() const
{
	return m_text;
}


void GUI_Text::CreateTextVerts()
{
	m_shadowVerts.clear();
	m_textVerts.clear();
	
	if (m_text.empty()) return;

	if (m_isParentWindowRelative)
	{
		float textWidth = m_font->GetTextWidth(m_textHeight, m_text);
		AABB2 box = m_parent->m_elementWindow.GetBoxWithIn(m_relativeBoundUVs);
		float boxHeight = box.GetDimensions().y;
		float boxWidth = box.GetDimensions().x;
		textWidth = textWidth <= boxWidth ? textWidth : boxWidth;
		float textHeight = m_textHeight <= boxHeight ? m_textHeight : boxHeight;
		AABB2 textBox(Vec2::ZERO, Vec2(textWidth, textHeight));
		box.AlignBoxWithin(textBox, m_textAlignment);
		m_font->AddVertsForTextInBox2D(m_textVerts, textBox, m_textHeight, m_text, m_textColor, m_textAspect, m_textAlignment);

		if (m_hasDropShadow)
		{
			float offsetAmount = 4.f * (textBox.GetDimensions().y / 32.f);
			textBox.Translate(Vec2(offsetAmount, -offsetAmount));
			m_font->AddVertsForTextInBox2D(m_shadowVerts, textBox, m_textHeight, m_text, m_textShadowColor, m_textAspect, m_textAlignment);
		}
	}
	else
	{
		float textWidth = m_font->GetTextWidth(m_textHeight, m_text);
		float boxHeight = m_elementWindow.GetDimensions().y;
		float boxWidth = m_elementWindow.GetDimensions().x;
		textWidth = textWidth <= boxWidth ? textWidth : boxWidth;
		float textHeight = m_textHeight <= boxHeight ? m_textHeight : boxHeight;
		AABB2 textBox(Vec2::ZERO, Vec2(textWidth, textHeight));
		m_elementWindow.AlignBoxWithin(textBox, m_textAlignment);
		m_font->AddVertsForTextInBox2D(m_textVerts, textBox, m_textHeight, m_text, m_textColor, m_textAspect, m_textAlignment);

		if (m_hasDropShadow)
		{
			float offsetAmount = 4.f * (textBox.GetDimensions().y / 32.f);
			textBox.Translate(Vec2(offsetAmount, -offsetAmount));
			m_font->AddVertsForTextInBox2D(m_shadowVerts, textBox, m_textHeight, m_text, m_textShadowColor, m_textAspect, m_textAlignment);
		}
	}
}


