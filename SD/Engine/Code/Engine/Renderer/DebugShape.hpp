#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

#include <vector>

enum class DebugShapeType
{
	INVALID,
	POINT,
	LINE,
	WIRE_CYLINDER,
	WIRE_SPHERE,
	ARROW,
	BOX,
	BASIS,
	TEXT,
	BILLBOARD_TEXT
};

struct DebugShape
{
public:
	DebugShape() {}
	DebugShape(DebugShapeType type, Mat44 const& modelMatrix, Clock const& parentClock, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode renderMode = DebugRenderMode::USEDEPTH, FillMode fillMode = FillMode::SOLID);
	~DebugShape() {}

	Mat44 GetModelMatrix() const;
	bool isDead() const;

public:
	DebugShapeType m_type = DebugShapeType::INVALID;
	Mat44 m_modelMatrix;
	Stopwatch m_timer;
	Rgba8 m_startColor;
	Rgba8 m_endColor;
	FillMode m_fillMode = FillMode::SOLID;
	DepthTest m_depthTest = DepthTest::LESSEQUAL;
	BlendMode m_blendMode = BlendMode::OPAQUE;
	CullMode m_cullMode = CullMode::BACK;
	bool m_writeDepth = true;
	Texture const* m_texture = nullptr;
	DebugRenderMode m_renderMode = DebugRenderMode::USEDEPTH;
	std::vector<Vertex_PCU> m_verts;
	std::string m_messgae;
};


