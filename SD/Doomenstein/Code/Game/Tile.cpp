#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/TileMaterialDefinition.hpp"

Tile::Tile(AABB3 bounds, const TileDefinition* definition)
	: m_bounds(bounds)
	, m_definition(definition)
{
}


bool Tile::IsAir() const
{
	if (m_definition->m_wallMaterialDefinition)
	{
		return false;
	}
	if (m_definition->m_ceilingMaterialDefinition)
	{
		return false;
	}
	if (m_definition->m_floorMaterialDefinition)
	{
		return false;
	}
	return true;
}


bool Tile::IsSolid() const
{
	return m_definition->m_isSolid;
}


bool Tile::HasFloor() const
{
	if (m_definition->m_floorMaterialDefinition)
	{
		return true;
	}
	else
	{
		return false;
	}
}


