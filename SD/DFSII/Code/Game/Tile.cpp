#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Texture.hpp"

std::vector<TileDefinition> TileDefinition::s_tileDefs;

TileDefinition::TileDefinition()
{
}


TileDefinition::TileDefinition(std::string const& name, bool isObstacle, bool isWater, float cost, IntVec2 const& baseSpriteCoord, IntVec2 const& topSpriteCoord, Rgba8 const& imageColor)
{
	m_name = name;
	m_isObstacle = isObstacle;
	m_isWater = isWater;
	m_cost = cost;
	Texture const* texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/DFSII/Assets.png");
	SpriteSheet spriteSheet(*texture, IntVec2(71, 19));
	m_baseUVs = spriteSheet.GetSpriteUVs(baseSpriteCoord);
	m_topUVs = spriteSheet.GetSpriteUVs(topSpriteCoord);
	if (m_name == "bridge") // need change 
	{
		m_topUVs.m_mins.x += .00175f;
		m_topUVs.m_mins.y += .01f;
		m_topUVs.m_maxs.x -= .00175f;
		m_topUVs.m_maxs.y -= .01f;
	}
	m_imageColor = imageColor;
}


void TileDefinition::InitializeTileDefs()
{
	CreateTileDef("road",	false, false, 1.f, IntVec2(11, 16), IntVec2(70, 18), Rgba8(200, 100, 50,  255));
	CreateTileDef("grass",	false, false, 1.5f, IntVec2(11, 15), IntVec2(70, 18), Rgba8(0,   255, 0,   255));
	CreateTileDef("tree",	false, false, 3.5f, IntVec2(11, 15), IntVec2(1,  15), Rgba8(0,   150, 0,   255));
	CreateTileDef("water",	false, true, 8.f, IntVec2(9,  15), IntVec2(70, 18), Rgba8(0,   0,   255, 255));
	CreateTileDef("bridge",	false, false, 1.f, IntVec2(9,  15), IntVec2(4,  4 ), Rgba8(200, 150, 50,  255)); // need change
	CreateTileDef("house",	false, false, 99.f, IntVec2(11, 15), IntVec2(7,  3 ), Rgba8(255, 255, 0,   255));
}


void TileDefinition::CreateTileDef(std::string const& name, bool isObstacle, bool isWater, float cost, IntVec2 const& baseSpriteCoord, IntVec2 const& topSpriteCoord, Rgba8 const& imageColor)
{
	TileDefinition road(name, isObstacle, isWater, cost, baseSpriteCoord, topSpriteCoord, imageColor);
	TileDefinition::s_tileDefs.push_back(road);
}


TileDefinition const& TileDefinition::GetDefinitionByName(std::string const& name)
{
	for (int index = 0; index < (int)s_tileDefs.size(); index++)
	{
		TileDefinition const& tileDef = s_tileDefs[index];
		if (tileDef.m_name == name)
		{
			return tileDef;
		}
	}

	ERROR_AND_DIE("tile def does not exits");
}


TileDefinition const& TileDefinition::GetDefinitionByColor(Rgba8 const& color)
{
	for (int index = 0; index < (int)s_tileDefs.size(); index++)
	{
		TileDefinition const& tileDef = s_tileDefs[index];
		if (tileDef.m_imageColor == color)
		{
			return tileDef;
		}
	}

	ERROR_AND_DIE("tile def does not exits");
}


bool Tile::IsObstacle() const
{
	return m_tileDef->m_isObstacle;
}


bool Tile::IsWater() const
{
	return m_tileDef->m_isWater;
}


void Tile::SetTile(std::string const& tileName)
{
	m_tileDef = &TileDefinition::GetDefinitionByName(tileName);
}


void Tile::SetTileDefinition(TileDefinition const* tileDef)
{
	m_tileDef = tileDef;
}


void Tile::SetCoordinate(IntVec2 const& coordinate)
{
	m_tileCoordinates = coordinate;
}


