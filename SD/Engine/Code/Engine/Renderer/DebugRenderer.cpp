#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/DebugShape.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Renderer/BillBoard.hpp"
#include "Engine/Window/Window.hpp"

typedef std::vector<DebugShape*> ShapeList;
constexpr float DEBUG_MESSAGE_LINES = 40.5f;

class DebugRenderer
{

public:
	DebugRenderer() {}
	~DebugRenderer() {}

	void Startup(DebugRenderConfig const& config);
	void ShutDown();
	void BeginFrame();
	void RenderWorld(Camera const& camera);
	void RenderScreen(Camera const& camera);
	void EndFrame();
	void SetVisible(bool isVisible);
	void Clear();
	void SetParentClock(Clock* parent);
	void AddShapeToList(DebugShape& newShape);
	void AddTextToList(DebugShape& newText);
	void AddMessageToList(DebugShape& newMessage);
	void IncreaseClockTimeDilation();
	void DecreaseClockTimeDilation();

	BitmapFont& GetBitmapFont() const;
	Clock const& GetClock();

	static bool Command_DebugRenderClear(EventArgs& args);
	static bool Command_DebugRenderToggle(EventArgs& args);

private:
	Renderer* m_renderer = nullptr;
	bool m_isHidden = false;
	Clock m_debugClock;
	ShapeList m_debugShapes;
	ShapeList m_debugTexts;
	ShapeList m_debugMessages;
};

static DebugRenderer debugRenderer;

void DebugRenderer::Startup(DebugRenderConfig const& config)
{
	m_renderer = config.m_renderer;
	m_isHidden = config.m_startHidden;
	SubscribeEventCallbackFunction("debugRenderClear", Command_DebugRenderClear);
	SubscribeEventCallbackFunction("debugRenderToggle", Command_DebugRenderToggle);
}


void DebugRenderer::ShutDown()
{

}


void DebugRenderer::BeginFrame()
{

}


void DebugRenderer::RenderWorld(Camera const& camera)
{
	if (m_isHidden) return;

	m_renderer->BeginCamera(camera);

	//ToDo: put same type into list, so only need to set render once for all draw calls
	for (int shapeIndex = 0; shapeIndex < (int)m_debugShapes.size(); shapeIndex++)
	{
		DebugShape* shape = m_debugShapes[shapeIndex];
		if (shape)
		{
			m_renderer->SetBlendMode(shape->m_blendMode);
			m_renderer->SetRasterizerState(shape->m_cullMode, shape->m_fillMode, WindingOrder::COUNTERCLOCKWISE);
			m_renderer->SetDepthStencilState(shape->m_depthTest, shape->m_writeDepth);
			if (shape->m_type == DebugShapeType::BILLBOARD_TEXT)
			{
				Mat44 modelMatrix = GetModelMatrixCameraOpposingXYZ(shape->GetModelMatrix().GetTranslation3D(), camera);
				m_renderer->SetModelMatrix(modelMatrix);
			}
			else
			{
				m_renderer->SetModelMatrix(shape->GetModelMatrix());
			}
			m_renderer->SetModelColor(InterpolateBetweenColor(shape->m_startColor, shape->m_endColor, shape->m_timer.GetElapsedFraction()));
			m_renderer->BindTexture(shape->m_texture);
			m_renderer->DrawVertexArray((int)shape->m_verts.size(), shape->m_verts.data());
		}
	}

	m_renderer->EndCamera(camera);
}


void DebugRenderer::RenderScreen(Camera const& camera)
{
	if (m_isHidden) return;

	m_renderer->BeginCamera(camera);

	m_renderer->SetBlendMode(BlendMode::ALPHA);
	m_renderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	m_renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
	m_renderer->SetSamplerState(SamplerMode::POINTCLAMP);
	m_renderer->BindTexture(&GetBitmapFont().GetTexture());

	for (int textIndex = 0; textIndex < (int)m_debugTexts.size(); textIndex++)
	{
		DebugShape* text = m_debugTexts[textIndex];
		if (text)
		{
			m_renderer->DrawVertexArray((int)text->m_verts.size(), text->m_verts.data());
		}
	}

	//Vec2 screenDimensions(1600.f, 800.f);
	Vec2 screenDimensions = camera.GetOrthoTopRight();
	float cellHeight = screenDimensions.y / DEBUG_MESSAGE_LINES;
	std::vector<Vertex_PCU> messageVerts;
	float lineCounter = 0.f;
	BitmapFont& font = GetBitmapFont();
	for (int messageIndex = 0; messageIndex < (int)m_debugMessages.size(); messageIndex++)
	{
		DebugShape* message = m_debugMessages[messageIndex];
		if (message)
		{
			AABB2 lineBox(Vec2(0.f, screenDimensions.y - (cellHeight * (lineCounter + 1.f))), Vec2(screenDimensions.x, screenDimensions.y - (cellHeight * lineCounter)));
			font.AddVertsForTextInBox2D(messageVerts, lineBox, cellHeight, message->m_messgae, message->m_startColor, 0.6f, Vec2(0.f, 0.5f));
			lineCounter += 1.f;
		}
	}
	m_renderer->DrawVertexArray((int)messageVerts.size(), messageVerts.data());

	m_renderer->EndCamera(camera);
}


void DebugRenderer::EndFrame()
{
	for (int shapeIndex = 0; shapeIndex < (int)m_debugShapes.size(); shapeIndex++)
	{
		DebugShape*& shape = m_debugShapes[shapeIndex];
		if (shape && shape->isDead())
		{
			delete shape;
			shape = nullptr;
		}
	}

	for (int textIndex = 0; textIndex < (int)m_debugTexts.size(); textIndex++)
	{
		DebugShape*& text = m_debugTexts[textIndex];
		if (text && text->isDead())
		{
			delete text;
			text = nullptr;
		}
	}

	for (int messageIndex = 0; messageIndex < (int)m_debugMessages.size(); messageIndex++)
	{
		DebugShape*& message = m_debugMessages[messageIndex];
		if (message && message->isDead())
		{
			delete message;
			message = nullptr;
		}
	}
}


void DebugRenderer::SetVisible(bool isVisible)
{
	m_isHidden = !isVisible;
}


void DebugRenderer::Clear()
{
	for (int shapeIndex = 0; shapeIndex < (int)m_debugShapes.size(); shapeIndex++)
	{
		DebugShape*& shape = m_debugShapes[shapeIndex];
		if (shape)
		{
			delete shape;
			shape = nullptr;
		}
	}
	m_debugShapes.clear();
	m_debugShapes.resize(0);

	for (int textIndex = 0; textIndex < (int)m_debugTexts.size(); textIndex++)
	{
		DebugShape*& text = m_debugTexts[textIndex];
		if (text)
		{
			delete text;
			text = nullptr;
		}
	}
	m_debugTexts.clear();
	m_debugTexts.resize(0);

	for (int messageIndex = 0; messageIndex < (int)m_debugMessages.size(); messageIndex++)
	{
		DebugShape*& message = m_debugMessages[messageIndex];
		if (message)
		{
			delete message;
			message = nullptr;
		}
	}
	m_debugMessages.clear();
	m_debugMessages.resize(0);
}


void DebugRenderer::SetParentClock(Clock* parent)
{
	m_debugClock.SetParent(*parent);
}


void DebugRenderer::AddShapeToList(DebugShape& newShape)
{
	for (int shapeIndex = 0; shapeIndex < (int)m_debugShapes.size(); shapeIndex++)
	{
		if (!m_debugShapes[shapeIndex])
		{
			m_debugShapes[shapeIndex] = &newShape;
			return;
		}
	}
	m_debugShapes.push_back(&newShape);
}


void DebugRenderer::AddTextToList(DebugShape& newText)
{
	for (int textIndex = 0; textIndex < (int)m_debugTexts.size(); textIndex++)
	{
		if (!m_debugTexts[textIndex])
		{
			m_debugTexts[textIndex] = &newText;
			return;
		}
	}
	m_debugTexts.push_back(&newText);
}


void DebugRenderer::AddMessageToList(DebugShape& newMessage)
{
	for (int messageIndex = 0; messageIndex < (int)m_debugMessages.size(); messageIndex++)
	{
		if (!m_debugMessages[messageIndex])
		{
			m_debugMessages[messageIndex] = &newMessage;
			return;
		}
	}
	m_debugMessages.push_back(&newMessage);
}

void DebugRenderer::IncreaseClockTimeDilation()
{
	float dilation = Clamp(static_cast<float>(m_debugClock.GetTimeDilation() + 0.1), 0.1f, 10.f);
	m_debugClock.SetTimeDilation(dilation);
}


void DebugRenderer::DecreaseClockTimeDilation()
{
	float dilation = Clamp(static_cast<float>(m_debugClock.GetTimeDilation() - 0.1), 0.1f, 10.f);
	m_debugClock.SetTimeDilation(dilation);
}


BitmapFont& DebugRenderer::GetBitmapFont() const
{
	return *(m_renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont"));
}


Clock const& DebugRenderer::GetClock()
{
	return m_debugClock;
}


bool DebugRenderer::Command_DebugRenderClear(EventArgs& args)
{
	UNUSED(args)

	DebugRenderClear();
	return false;
}


bool DebugRenderer::Command_DebugRenderToggle(EventArgs& args)
{
	UNUSED(args)

	debugRenderer.SetVisible(debugRenderer.m_isHidden);
	return false;
}


void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	debugRenderer.Startup(config);
}


void DebugRenderSystemShutdown()
{
	debugRenderer.ShutDown();
}


void DebugRenderSetVisible()
{
	debugRenderer.SetVisible(true);
}


void DebugRenderSetHidden()
{
	debugRenderer.SetVisible(false);
}


void DebugRenderClear()
{
	debugRenderer.Clear();
}


void DebugRenderSetParentClock(Clock& parent)
{
	debugRenderer.SetParentClock(&parent);
}


void DebugRenderIncreaseClockTimeDilation()
{
	debugRenderer.IncreaseClockTimeDilation();
}


void DebugRenderDecreaseClockTimeDilation()
{
	debugRenderer.DecreaseClockTimeDilation();
}


Clock const& DebugRenderGetClock()
{
	return debugRenderer.GetClock();
}


void DebugRenderBeginFrame()
{
	debugRenderer.BeginFrame();
}


void DebugRenderWorld(const Camera& camera)
{
	debugRenderer.RenderWorld(camera);
}


void DebugRenderScreen(const Camera& camera)
{
	debugRenderer.RenderScreen(camera);
}


void DebugRenderEndFrame()
{
	debugRenderer.EndFrame();
}


void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	Mat44 modelMatrix = Mat44::CreateTranslation3D(pos);
	DebugShape* point = new DebugShape(DebugShapeType::POINT, modelMatrix, debugRenderer.GetClock(), duration, startColor, endColor, mode);
	AddVertsForSphere3D(point->m_verts, Vec3::ZERO, radius, 16.f, 8.f);
	debugRenderer.AddShapeToList(*point);
}


void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	Rgba8 xrayStartColor(startColor.r, startColor.g, startColor.b, 50);
	Rgba8 xrayEndColor(endColor.r, endColor.g, endColor.b, 50);
	DebugShape* xrayLine = new DebugShape(DebugShapeType::LINE, Mat44(), debugRenderer.GetClock(), duration, xrayStartColor, xrayEndColor, mode);
	AddVertsForCylinder3D(xrayLine->m_verts, start, end, radius, 16.f);
	debugRenderer.AddShapeToList(*xrayLine);

	DebugShape* line = new DebugShape(DebugShapeType::LINE, Mat44(), debugRenderer.GetClock(), duration, startColor, endColor, DebugRenderMode::USEDEPTH);
	AddVertsForCylinder3D(line->m_verts, start, end, radius, 16.f);
	debugRenderer.AddShapeToList(*line);
}


void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugShape* cylinder = new DebugShape(DebugShapeType::WIRE_CYLINDER, Mat44(), debugRenderer.GetClock(), duration, startColor, endColor, mode, FillMode::WIREFRAME);
	AddVertsForCylinder3D(cylinder->m_verts, base, top, radius, 32.f);
	debugRenderer.AddShapeToList(*cylinder);
}


void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	Mat44 modelMatrix = Mat44::CreateTranslation3D(center);
	DebugShape* sphere = new DebugShape(DebugShapeType::WIRE_SPHERE, modelMatrix, debugRenderer.GetClock(), duration, startColor, endColor, mode, FillMode::WIREFRAME);
	AddVertsForSphere3D(sphere->m_verts, Vec3::ZERO, radius, 32.f, 16.f);
	debugRenderer.AddShapeToList(*sphere);
}


void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& baseColor, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	UNUSED(baseColor)

	DebugShape* arrow = new DebugShape(DebugShapeType::ARROW, Mat44(), debugRenderer.GetClock(), duration, startColor, endColor, mode);
	AddVertsForArrow3D(arrow->m_verts, start, end, radius);
	debugRenderer.AddShapeToList(*arrow);
}


void DebugAddWorldBox(const AABB3& bounds, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugShape* box = new DebugShape(DebugShapeType::BOX, Mat44(), debugRenderer.GetClock(), duration, startColor, endColor, mode);
	AddVertsForAABB3D(box->m_verts, bounds);
	debugRenderer.AddShapeToList(*box);
}


void DebugAddWorldBasis(const Mat44& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugShape* basisArrows = new DebugShape(DebugShapeType::BASIS, basis, debugRenderer.GetClock(), duration, startColor, endColor, mode);
	AddVertsForBasis3D(basisArrows->m_verts, 0.05f);
	debugRenderer.AddShapeToList(*basisArrows);
}


void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	UNUSED(alignment)

	DebugShape* worldText = new DebugShape(DebugShapeType::TEXT, transform, debugRenderer.GetClock(), duration, startColor, endColor, mode);
	BitmapFont& font = debugRenderer.GetBitmapFont();
	worldText->m_texture = &font.GetTexture();
	font.AddVertsForTextInBox2D(worldText->m_verts, AABB2(Vec2(0.f, 0.f), Vec2(static_cast<float>(text.size() * textHeight), textHeight)), textHeight, text, Rgba8::WHITE, 0.6f, alignment);
	debugRenderer.AddShapeToList(*worldText);
}


void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	Mat44 pos = Mat44::CreateTranslation3D(origin);
	DebugShape* billboardText = new DebugShape(DebugShapeType::BILLBOARD_TEXT, pos, debugRenderer.GetClock(), duration, startColor, endColor, mode);
	BitmapFont& font = debugRenderer.GetBitmapFont();
	billboardText->m_texture = &font.GetTexture();
	Vec2 billboardDimensions(static_cast<float>(text.size() * textHeight), textHeight);
	font.AddVertsForTextInBox2D(billboardText->m_verts, AABB2(-0.5f * billboardDimensions, 0.5f * billboardDimensions), textHeight, text, Rgba8::WHITE, .6f, alignment);
	debugRenderer.AddShapeToList(*billboardText);
}


void DebugAddScreenText(const std::string& text, const Vec2& position, float duration, const Vec2& alignment, float size, const Rgba8& startColor, const Rgba8& endColor)
{
	DebugShape* screenText = new DebugShape(DebugShapeType::TEXT, Mat44(), debugRenderer.GetClock(), duration, startColor, endColor);
	BitmapFont& font = debugRenderer.GetBitmapFont();
	screenText->m_texture = &font.GetTexture();
	Vec2 screenTextDimensions(static_cast<float>(text.size() * size), size);
	font.AddVertsForTextInBox2D(screenText->m_verts, AABB2(position, position + screenTextDimensions), size, text, Rgba8::WHITE, .6f, alignment);
	debugRenderer.AddTextToList(*screenText);
}


void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	DebugShape* screenMessage = new DebugShape(DebugShapeType::TEXT, Mat44(), debugRenderer.GetClock(), duration, startColor, endColor);
	screenMessage->m_messgae = text;
	debugRenderer.AddMessageToList(*screenMessage);
}


