#include "Game/Tile.hpp"

std::vector<TileDefinition> TileDefinition::s_tileDefs;

TileDefinition::TileDefinition(XmlElement* xmlElement)
{
	m_name =					ParseXmlAttribute(*xmlElement, "name", m_name);
	m_isSolid =					ParseXmlAttribute(*xmlElement, "isSolid", m_isSolid);
	m_isWater =					ParseXmlAttribute(*xmlElement, "isWater", m_isWater);
	m_isDestructible =			ParseXmlAttribute(*xmlElement, "isDestructible", m_isDestructible);
	m_maxHealth =				ParseXmlAttribute(*xmlElement, "maxHealth", m_maxHealth);
	m_tileAfterDestructed =		ParseXmlAttribute(*xmlElement, "newTileName", m_tileAfterDestructed);
	IntVec2 spriteCoordinate =	ParseXmlAttribute(*xmlElement, "spriteCoords", IntVec2::ZERO);
	m_mapImageColor =			ParseXmlAttribute(*xmlElement, "mapColor", m_mapImageColor);
	m_tint =					ParseXmlAttribute(*xmlElement, "tint", m_tint);
	int spriteIndex = spriteCoordinate.x + spriteCoordinate.y * 8;
	m_uvBounds = g_tileSpriteSheet->GetSpriteUVs(spriteIndex);
}


void TileDefinition::InitializeTileDefs()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/TileDefinitions.xml");
	XmlElement* root = doc.RootElement();

	XmlElement* element = root->FirstChildElement();
	while (element)
	{
		if (std::string(element->Name()) == "TileDefinition")
		{
			TileDefinition newTileDef(element);
			s_tileDefs.push_back(newTileDef);
			element = element->NextSiblingElement();
		}
	}
}


TileDefinition const& TileDefinition::GetDefinitionByName(std::string const& name)
{
	for (int tileDefIndex = 0; tileDefIndex < int(s_tileDefs.size()); tileDefIndex++)
	{
		TileDefinition const& tileDef = s_tileDefs[tileDefIndex];
		if (tileDef.m_name == name)
		{
			return tileDef;
		}
	}

	ERROR_AND_DIE("No such tile definition");
}


bool Tile::IsTileSolid() const
{
	return m_tileDef->m_isSolid;
}


bool Tile::IsTileWater() const
{
	return m_tileDef->m_isWater;
}


void Tile::SetTileType(std::string tileType)
{
	m_tileDef = &TileDefinition::GetDefinitionByName(tileType);
	if (m_tileDef->m_isDestructible)
	{
		m_health = m_tileDef->m_maxHealth;
	}
}


