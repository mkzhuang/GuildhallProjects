#include "Engine/GUI/UI3DViewer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine//Mesh/Mesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Mesh/Mesh.hpp"

constexpr float ROTATE_DEGREES = 30.f;

UI3DViewer::UI3DViewer()
{

}


UI3DViewer::UI3DViewer(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, MeshBuilderConfig config, bool isDraggable)
	: UIElement(name, bound, anchor, renderer, isDraggable)
{
	m_hasOwnCamera = true;
	m_camera.SetRenderTransform(Vec3(0.f, -1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f));
	Vec2 clientDimensions(Window::GetWindowContext()->GetClientDimensions());
	AABB2 clientBound(Vec2::ZERO, clientDimensions);
	Vec2 const& viewportMins = clientBound.GetUVForPoint(m_screenWindow.m_mins);
	Vec2 const& viewportMaxs = clientBound.GetUVForPoint(m_screenWindow.m_maxs);
	m_camera.SetViewport(AABB2(viewportMins, viewportMaxs));
	Vec2 const& viewportDimensions = m_camera.GetViewport().GetDimensions();
	float viewportAspect = viewportDimensions.x / viewportDimensions.y;
	m_camera.SetPerspectiveView(Window::GetWindowContext()->GetAspect() * viewportAspect, 60.f, 0.1f, 100.f);
	m_camera.SetTransform(Vec3(-20.f, 0.f, 45.f), EulerAngles(0.f, 65.f, 0.f));
	SetMesh(config);
}


UI3DViewer::~UI3DViewer()
{
	delete m_mesh;
}


void UI3DViewer::Update()
{
	if (m_mesh)
	{
		float delta = static_cast<float>(Clock::GetSystemClock().GetDeltaTime());
		m_mesh->RotateAboutZ(delta * ROTATE_DEGREES);
	}

	for (UIElement* child : m_children)
	{
		child->Update();
	}
}


void UI3DViewer::Render() const
{
	for (UIElement* child : m_children)
	{
		if (child) child->Render();
	}
}


void UI3DViewer::OnClick()
{
	if (m_parent) m_parent->OnClick();
}


void UI3DViewer::RenderModel() const
{
	m_renderer->BeginCamera(m_camera);
	if (m_mesh)
	{
		m_mesh->Render(m_renderer);
	}
	m_renderer->EndCamera(m_camera);
}


void UI3DViewer::SetMesh(MeshBuilderConfig config)
{
	MeshBuilder meshBuilder;
	meshBuilder.LoadFromConfig(config);
	delete m_mesh;
	m_mesh = new Mesh(&meshBuilder, m_renderer);
}


