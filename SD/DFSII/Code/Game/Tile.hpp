#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <string>
#include <vector>

struct TileDefinition
{
public:
	TileDefinition();
	TileDefinition(std::string const& name, bool isObstacle, bool isWater, float cost, IntVec2 const& baseSpriteCoord, IntVec2 const& topSpriteCoord, Rgba8 const& imageColor);
	~TileDefinition() {}

	static void InitializeTileDefs();
	static void CreateTileDef(std::string const& name, bool isObstacle, bool isWater, float cost, IntVec2 const& baseSpriteCoord, IntVec2 const& topSpriteCoord, Rgba8 const& imageColor);
	static TileDefinition const& GetDefinitionByName(std::string const& name);
	static TileDefinition const& GetDefinitionByColor(Rgba8 const& color);

	static std::vector<TileDefinition> s_tileDefs;

public:
	std::string m_name = "invalid";
	bool m_isObstacle = false;
	bool m_isWater = false;
	AABB2 m_baseUVs = AABB2::ZERO_TO_ONE;
	AABB2 m_topUVs = AABB2::ZERO_TO_ONE;
	Rgba8 m_imageColor = Rgba8::WHITE;
	float m_cost = 0;
};

struct Tile
{
public:
	Tile() {}
	~Tile() {}

	bool IsObstacle() const;
	bool IsWater() const;
	void SetTile(std::string const& tileName);
	void SetTileDefinition(TileDefinition const* tileDef);
	void SetCoordinate(IntVec2 const& coordinate);

public:
	IntVec2 m_tileCoordinates = IntVec2::ZERO;
	TileDefinition const* m_tileDef = nullptr;
};


