#include "Engine/Renderer/DebugShape.hpp"

DebugShape::DebugShape(DebugShapeType type, Mat44 const& modelMatrix, Clock const& parentClock, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode renderMode, FillMode fillMode)
	: m_type(type)
	, m_modelMatrix(modelMatrix)
	, m_startColor(startColor)
	, m_endColor(endColor)
	, m_renderMode(renderMode)
	, m_fillMode(fillMode)
{
	if (duration == -1.f)
	{
		duration = FLT_MAX;
	}
	m_timer.Start(&parentClock, duration);
	switch (renderMode)
	{
		case DebugRenderMode::ALWAYS:
			m_depthTest = DepthTest::ALWAYS;
			m_writeDepth = false;
			break;
		case DebugRenderMode::USEDEPTH:
			m_depthTest = DepthTest::LESSEQUAL;
			m_writeDepth = true;
			break;
		case DebugRenderMode::XRAY:
			m_depthTest = DepthTest::ALWAYS;
			m_writeDepth = false;
			m_blendMode = BlendMode::ALPHA;
			break;
		default:
			break;
	}

	if (m_type == DebugShapeType::TEXT || m_type == DebugShapeType::BILLBOARD_TEXT)
	{
		m_blendMode = BlendMode::ALPHA;
		m_cullMode = CullMode::NONE;
	}
}


Mat44 DebugShape::GetModelMatrix() const
{
	return m_modelMatrix;
}


bool DebugShape::isDead() const
{
	return m_timer.HasDurationElapsed();
}


