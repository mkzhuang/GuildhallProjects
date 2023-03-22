#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"

enum EnemyType
{
	ENEMY_TYPE_SCORPIO,
	ENEMY_TYPE_LEO,
	ENEMY_TYPE_ARIES,
	ENEMY_TYPE_CAPRICORN,

	NUM_ENEMY_TYPES
};

struct MapDefinition
{
public:
	MapDefinition(XmlElement* xmlElement);

	static void InitializeMapDefs();
	static std::vector<MapDefinition> s_mapDefs;
	static MapDefinition const& GetDefinitionByName(std::string const& name);

public:
	std::string m_name = "UNUSED MAP DEF";
	std::string m_mapImageName;
	IntVec2 m_mapImageOffset = IntVec2::ZERO;
	IntVec2 m_dimensions = IntVec2::ZERO;
	std::string m_fillTileType;
	std::string m_wallTileType;
	std::string m_worm1TileType;
	int m_worm1Num = 0;
	int m_worm1Length = 0;
	std::string m_worm2TileType;
	int m_worm2Num = 0;
	int m_worm2Length = 0;
	std::string m_worm3TileType;
	int m_worm3Num = 0;
	int m_worm3Length = 0;
	std::string m_startBunkerTileType;
	std::string m_endBunkerTileType;
	std::string m_startFloorTileType;
	std::string m_endFloorTileType;
	int m_enemyCounts[NUM_ENEMY_TYPES] = {};
};