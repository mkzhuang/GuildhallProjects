#include "Engine/Core/Vertex_PCU.hpp"

Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& tint, Vec2 const& uxTexCoords)
	: m_position(position)
	, m_color(tint)
	, m_uvTexCoords(uxTexCoords)
{
}


