#include "Game/GameCommon.hpp"

bool g_isQuitting = false;
bool g_isDebugging = false;
float g_gameSoundVolume = 0.2f;
float g_gameSoundPlaySpeed = 1.f;

SoundPlaybackID PlaySound(SoundID sound, bool loop, float volume)
{
	if (sound == MISSING_SOUND_ID)
	{
		return NULL;
	}

	return g_theAudio->StartSound(sound, loop, volume, 0.f, g_gameSoundPlaySpeed);
}


void DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float lineWidth, Rgba8 color)
{
	std::vector<Vertex_PCU> verts;
	LineSegment2 lineSegment(startPos, endPos);
	AddVertsForLineSegment2D(verts, lineSegment, lineWidth, color);

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
}


void DebugDrawRing(Vec2 const& center, float radius, float lineWidth, Rgba8 color)
{
	static int partitions = g_gameConfigBlackboard.GetValue("debugRingPartitions", 0);

	std::vector<Vertex_PCU> verts;
	float pieDegrees = 360.f / static_cast<float>(partitions);
	float halfWdith = 0.5f * lineWidth;
	for (int ringPartition = 0; ringPartition < partitions; ringPartition++)
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

		verts.emplace_back(Vec3(closerLess.x, closerLess.y, 0.f), color, Vec2(0.f, 0.f));
		verts.emplace_back(Vec3(farLess.x, farLess.y, 0.f), color, Vec2(0.f, 0.f));
		verts.emplace_back(Vec3(closerMore.x, closerMore.y, 0.f), color, Vec2(0.f, 0.f));
		verts.emplace_back(Vec3(farLess.x, farLess.y, 0.f), color, Vec2(0.f, 0.f));
		verts.emplace_back(Vec3(closerMore.x, closerMore.y, 0.f), color, Vec2(0.f, 0.f));
		verts.emplace_back(Vec3(farMore.x, farMore.y, 0.f), color, Vec2(0.f, 0.f));
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
}


