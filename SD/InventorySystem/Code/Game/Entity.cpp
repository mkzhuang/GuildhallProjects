#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

Entity::Entity(Game* owner, Vec3 const& startPos)
	: m_game(owner)
	, m_position(startPos)
{
}


void Entity::DebugRender() const
{
}


Mat44 Entity::GetModelMatrix() const
{
	Mat44 model;
	model.SetTranslation3D(m_position);
	Mat44 rotation = m_orientationDegree.GetAsMatrix_XFwd_YLeft_ZUp();
	model.Append(rotation);
	return model;
}


Vec3 Entity::GetForwardNormal() const
{
	return Vec3::ZERO;
}


