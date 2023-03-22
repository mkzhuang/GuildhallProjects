#include "Engine/GUI/UISystem.hpp"
#include "Engine/GUI/UICanvas.hpp"
#include "Engine/GUI/UIImage.hpp"
#include "Engine/GUI/UIText.hpp"
#include "Engine/GUI/UIButton.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/GUI/UILayout.hpp"

UISystem::UISystem(UISystemConfig const& config)
	: m_renderer(config.renderer)
	, m_input(config.input)
	, m_font(config.font)
{
	m_canvas = new UICanvas();
}


UISystem::~UISystem()
{
}


void UISystem::Startup()
{
}


void UISystem::Shutdown()
{
	delete m_canvas;
}


void UISystem::Update()
{
	if (IsOpen())
	{
		m_canvas->Update();

		if (m_input->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
		{
			m_canvas->UpdateOnFocus();
		}
	}
}


void UISystem::Render() const
{
	if (IsOpen())
	{
		m_canvas->Render(m_renderer);
	}
}


void UISystem::RenderModel() const
{
	if (IsOpen())
	{
		m_canvas->RenderModel(m_renderer);
	}
}


bool UISystem::IsOpen() const
{
	return m_isOpen;
}


void UISystem::LoadLayout(std::string layoutPath)
{
	UILayout layout;
	layout.InitializeLayout(m_renderer, m_font);
	UIElement* rootElement = layout.CreateLayout(layoutPath, m_canvas->m_screenWindow);
	m_canvas->SetRootElement(rootElement);
	m_canvas->SetViewerElement();
}


void UISystem::ToggleOpen()
{
	m_isOpen = !m_isOpen;
	if (!IsOpen())
	{
		m_canvas->ClearFocusAndHover();
	}
}


