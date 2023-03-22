#include "Game/MapDefinition.hpp"

std::vector<MapDefinition> MapDefinition::s_mapDefs;

MapDefinition::MapDefinition(XmlElement* xmlElement)
{
	m_name =								ParseXmlAttribute(*xmlElement, "name", m_name);
	m_mapImageName =						ParseXmlAttribute(*xmlElement, "mapImageName", m_mapImageName);
	m_mapImageOffset =						ParseXmlAttribute(*xmlElement, "mapImageOffset", m_mapImageOffset);
	m_dimensions =							ParseXmlAttribute(*xmlElement, "dimensions", m_dimensions);
	m_fillTileType =						ParseXmlAttribute(*xmlElement, "fillTileType", "Grass");
	m_wallTileType =						ParseXmlAttribute(*xmlElement, "edgeTileType", "Stone");
	m_worm1TileType =						ParseXmlAttribute(*xmlElement, "worm1TileType", "Grass");
	m_worm1Num =							ParseXmlAttribute(*xmlElement, "worm1Count", m_worm1Num);
	m_worm1Length =							ParseXmlAttribute(*xmlElement, "worm1MaxLength", m_worm1Length);
	m_worm2TileType =						ParseXmlAttribute(*xmlElement, "worm2TileType", "Grass");
	m_worm2Num =							ParseXmlAttribute(*xmlElement, "worm2Count", m_worm2Num);
	m_worm2Length =							ParseXmlAttribute(*xmlElement, "worm2MaxLength", m_worm2Length);
	m_worm3TileType =						ParseXmlAttribute(*xmlElement, "worm3TileType", "Grass");
	m_worm3Num =							ParseXmlAttribute(*xmlElement, "worm3Count", m_worm3Num);
	m_worm3Length =							ParseXmlAttribute(*xmlElement, "worm3MaxLength", m_worm3Length);
	m_startBunkerTileType =					ParseXmlAttribute(*xmlElement, "startBunkerTileType", "Stone");
	m_endBunkerTileType =					ParseXmlAttribute(*xmlElement, "endBunkerTileType", "Stone");
	m_startFloorTileType =					ParseXmlAttribute(*xmlElement, "startFloorTileType", "Grass");
	m_endFloorTileType =					ParseXmlAttribute(*xmlElement, "endFloorTileType", "Grass");
	m_enemyCounts[ENEMY_TYPE_SCORPIO] =		ParseXmlAttribute(*xmlElement, "scorpioCount", 0);
	m_enemyCounts[ENEMY_TYPE_LEO] =			ParseXmlAttribute(*xmlElement, "leoCount", 0);
	m_enemyCounts[ENEMY_TYPE_ARIES] =		ParseXmlAttribute(*xmlElement, "ariesCount", 0);
	m_enemyCounts[ENEMY_TYPE_CAPRICORN] =	ParseXmlAttribute(*xmlElement, "capricornCount", 0);
}


void MapDefinition::InitializeMapDefs()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/MapDefinitions.xml");
	XmlElement* root = doc.RootElement();

	XmlElement* element = root->FirstChildElement();
	while (element)
	{
		if (std::string(element->Name()) == "MapDefinition")
		{
			MapDefinition newMapDef(element);
			s_mapDefs.push_back(newMapDef);
			element = element->NextSiblingElement();
		}
	}
}


MapDefinition const& MapDefinition::GetDefinitionByName(std::string const& name)
{
	for (int mapDefIndex = 0; mapDefIndex < int(s_mapDefs.size()); mapDefIndex++)
	{
		MapDefinition const& mapDef = s_mapDefs[mapDefIndex];
		if (mapDef.m_name == name)
		{
			return mapDef;
		}
	}

	ERROR_AND_DIE("No such map definition");
}


