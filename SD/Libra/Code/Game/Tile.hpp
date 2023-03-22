#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Game/GameCommon.hpp"

enum TileType
{
	TILE_TYPE_NULL = -1,

	TILE_TYPE_SHORT_GRASS,
	TILE_TYPE_TALL_GRASS,
	TILE_TYPE_STONE,
	TILE_TYPE_DIRT,
	TILE_TYPE_FLOOR,
	TILE_TYPE_BRICK_WALL,
	TILE_TYPE_METAL_WALL,
	TILE_TYPE_LEVEL_ENTRANCE,
	TILE_TYPE_LEVEL_EXIT,

	NUM_TILE_TYPES
};

struct TileDefinition
{
public:
	TileDefinition(XmlElement* xmlElement);

	static void InitializeTileDefs();
	static std::vector<TileDefinition> s_tileDefs;
	static TileDefinition const& GetDefinitionByName(std::string const& name);

public:
	std::string m_name = "UNUSED TILE DEF";
	bool m_isSolid = false;
	bool m_isWater = false;
	bool m_isDestructible = false;
	int m_maxHealth = 0;
	std::string m_tileAfterDestructed = "NONE";
	AABB2 m_uvBounds = AABB2::ZERO_TO_ONE;
	Rgba8 m_mapImageColor = Rgba8(0, 0, 0, 0);
	Rgba8 m_tint = Rgba8::WHITE;
};


struct Tile
{
public:
	Tile() {}
	~Tile() {}

	bool IsTileSolid() const;
	bool IsTileWater() const;
	void SetTileType(std::string tileType);

public:
	int m_health = 0;
	IntVec2 m_tileCoords = IntVec2::ZERO;
	TileDefinition const* m_tileDef = nullptr;
};


