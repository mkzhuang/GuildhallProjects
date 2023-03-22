#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

bool g_isQuitting = false;

void DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float lineWidth, Rgba8 color)
{
	Vec2 directional = (endPos - startPos).GetNormalized() * lineWidth * 0.5f;
	Vec2 perpendicular = directional.GetRotated90Degrees();

	Vec2 BL = startPos - directional + perpendicular;
	Vec2 BR = startPos - directional - perpendicular;
	Vec2 TL = endPos + directional + perpendicular;
	Vec2 TR = endPos + directional - perpendicular;

	Vertex_PCU verts[NUM_LINE_VERTS];
	verts[0] = Vertex_PCU(Vec3(BL.x, BL.y, 0.f), color, Vec2(0.f, 0.f));
	verts[1] = Vertex_PCU(Vec3(BR.x, BR.y, 0.f), color, Vec2(0.f, 0.f));
	verts[2] = Vertex_PCU(Vec3(TL.x, TL.y, 0.f), color, Vec2(0.f, 0.f));
	verts[3] = Vertex_PCU(Vec3(BR.x, BR.y, 0.f), color, Vec2(0.f, 0.f));
	verts[4] = Vertex_PCU(Vec3(TL.x, TL.y, 0.f), color, Vec2(0.f, 0.f));
	verts[5] = Vertex_PCU(Vec3(TR.x, TR.y, 0.f), color, Vec2(0.f, 0.f));

	g_theRenderer->DrawVertexArray(NUM_LINE_VERTS, verts);
}


void DebugDrawRing(Vec2 const& center, float radius, float lineWidth, Rgba8 color)
{
	Vertex_PCU verts[NUM_RING_VERTS];
	float pieDegrees = 360.f / NUM_RING_PARTITIONS;
	float halfWdith = 0.5f * lineWidth;
	for (int ringPartition = 0; ringPartition < NUM_RING_PARTITIONS; ringPartition++)
	{
		float lesserDegrees = ringPartition * pieDegrees;
		float moreDegrees = (ringPartition + 1) * pieDegrees;
		Vec2 closerLess = center + Vec2((radius - halfWdith) * CosDegrees(lesserDegrees),
			(radius - halfWdith) * SinDegrees(lesserDegrees));
		Vec2 farLess = center + Vec2((radius + halfWdith) * CosDegrees(lesserDegrees),
			(radius + halfWdith) * SinDegrees(lesserDegrees));
		Vec2 closerMore = center + Vec2((radius - halfWdith) * CosDegrees(moreDegrees),
			(radius - halfWdith) * SinDegrees(moreDegrees));
		Vec2 farMore = center + Vec2((radius + halfWdith) * CosDegrees(moreDegrees),
			(radius + halfWdith) * SinDegrees(moreDegrees));

		verts[ringPartition * 6] = Vertex_PCU(Vec3(closerLess.x, closerLess.y, 0.f), color, Vec2(0.f, 0.f));
		verts[ringPartition * 6 + 1] = Vertex_PCU(Vec3(farLess.x, farLess.y, 0.f), color, Vec2(0.f, 0.f));
		verts[ringPartition * 6 + 2] = Vertex_PCU(Vec3(closerMore.x, closerMore.y, 0.f), color, Vec2(0.f, 0.f));
		verts[ringPartition * 6 + 3] = Vertex_PCU(Vec3(farLess.x, farLess.y, 0.f), color, Vec2(0.f, 0.f));
		verts[ringPartition * 6 + 4] = Vertex_PCU(Vec3(closerMore.x, closerMore.y, 0.f), color, Vec2(0.f, 0.f));
		verts[ringPartition * 6 + 5] = Vertex_PCU(Vec3(farMore.x, farMore.y, 0.f), color, Vec2(0.f, 0.f));
	}

	g_theRenderer->DrawVertexArray(NUM_RING_VERTS, verts);
}


