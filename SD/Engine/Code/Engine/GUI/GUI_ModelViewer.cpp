#include "Engine/GUI/GUI_ModelViewer.hpp"
#include "Engine/Mesh/Mesh.hpp"
#include "Engine/Mesh/MeshBuilder.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/GUI/GUI_Canvas.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

GUI_ModelViewer::GUI_ModelViewer()
{
}


GUI_ModelViewer::~GUI_ModelViewer()
{
	GUI_Element::~GUI_Element();

	//delete m_mesh;
	//m_mesh = nullptr;
}


void GUI_ModelViewer::Update()
{
	GUI_Element::Update();
	if (m_mesh && m_isHover)
	{
		float deltaTime = static_cast<float>(Clock::GetSystemClock().GetDeltaTime());

		if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
		{
			Vec2 mousePos = m_canvas->m_canvasBounds.GetPointAtUV(Window::GetWindowContext()->GetNormalizedCursorPos());
			Vec2 offset = mousePos - m_canvas->m_storedMousePosition;
			std::string info = Stringf("Offset: %.2f, %.2f", offset.x, offset.y);
			DebugAddMessage(info, 0.f, Rgba8::YELLOW, Rgba8::YELLOW);
			if (offset.GetLengthSquared() > 0.f)
			{
				Mat44 mouseDelta = Mat44::CreateRotationFromAngleAxis(Vec3(offset * deltaTime * 0.01f));
				Mat44 viewMatrix = m_camera.GetViewMatrix();
				Mat44 projectionMatrix = m_camera.GetProjectionMatrix();
				//mouseDelta.Append(viewMatrix);
				//mouseDelta.Append(projectionMatrix);
				m_mesh->Rotate(mouseDelta);
			}
		}
		
		MouseWheelState wheelState = g_theInput->GetMouseWheelState();
		switch (wheelState)
		{
			case MouseWheelState::WHEEL_UP:
			{
				float fov = m_camera.GetCameraFOV();
				fov -= 5.f;
				if (fov < 20.f)
				{
					fov = 20.f;
				}
				m_camera.SetFOV(fov);
				break;
			}
			case MouseWheelState::WHEEL_DOWN:
			{
				float fov = m_camera.GetCameraFOV();
				fov += 5.f;
				if (fov > 125.f)
				{
					fov = 125.f;
				}
				m_camera.SetFOV(fov);
				break;
			}
			case MouseWheelState::WHEEL_IDLE:
			default:
				break;
		}
	}
}


void GUI_ModelViewer::Render(Renderer* renderer) const
{
	GUI_Element::Render(renderer);
}


void GUI_ModelViewer::RenderModel(Renderer* renderer) const
{
	renderer->BeginCamera(m_camera);
	if (m_mesh) m_mesh->Render(renderer);
	renderer->EndCamera(m_camera);
}


void GUI_ModelViewer::SetupCamera()
{
	m_camera.SetRenderTransform(Vec3(0.f, -1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f));
	Vec2 viewportMins = m_parent->m_elementWindow.GetUVForPoint(m_elementWindow.m_mins);
	Vec2 viewportMaxs = m_parent->m_elementWindow.GetUVForPoint(m_elementWindow.m_maxs);
	m_camera.SetViewport(AABB2(viewportMins, viewportMaxs));
	Vec2 const& viewportDimensions = m_camera.GetViewport().GetDimensions();
	float viewportAspect = viewportDimensions.x / viewportDimensions.y;
	m_camera.SetPerspectiveView(Window::GetWindowContext()->GetAspect() * viewportAspect, 60.f, 0.1f, 100.f);
	m_camera.SetTransform(Vec3(-20.f, 0.f, 45.f), EulerAngles(0.f, 65.f, 0.f));
}


void GUI_ModelViewer::SetMesh(MeshBuilderConfig config, Renderer* renderer)
{
	MeshBuilder meshBuilder;
	meshBuilder.LoadFromConfig(config);
	if (m_mesh) 
	{
		delete m_mesh;
		m_mesh = nullptr;
	}
	m_mesh = new Mesh(&meshBuilder, renderer);
}


void GUI_ModelViewer::SetMesh(Mesh* mesh)
{
	if (m_mesh)
	{
		m_mesh->m_modelMatrix = Mat44();
	}
	m_mesh = mesh;
}



