#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Game/Explosion.hpp"
#include "Game/MapDefinition.hpp"
#include "Engine/Core/HeatMaps.hpp"

class World;
struct Tile;
class Player;

struct RaycastResultLibra : public RaycastResult2D
{
public:
	RaycastResultLibra(Vec2 startPos, Vec2 forwardNorm, float maxDist,
		bool didImpact = false, Vec2 impactPosition = Vec2::ZERO,
		float impactDistance = 0.f, Vec2 impactSurfaceNormal = Vec2::ZERO);
	~RaycastResultLibra() {}
};


class Map
{
public:
	Map(World* owner, MapDefinition const& mapDef);
	~Map() {}
	void Startup(Player* player);

	void Update(float deltaSeconds);
	void Render() const;
	void PopulateDistanceFieldForEntity(TileHeatMap& out_distanceField, IntVec2 const& referenceCoords, float maxCost, Entity* e);
	void AddEntityToMap(Entity& e);
	void AddEntityToList(Entity& e, EntityList& list);
	void RemoveEntityFromMap(Entity& e);
	void RemoveEntityFromList(Entity& e, EntityList& list);
	void AddExplosionToMap(Vec2 const& position, float orientation, ExplosionType explosionType);
	bool IsAlive(Entity* e) const;
	bool IsPlayerAlive() const;
	bool IsPointInSolid(Vec2 const& point);
	bool IsTileSolidForEntity(Entity* e, Tile const* tile);
	bool IsTileSolid(Tile const* tile);
	bool IsTileWater(Tile const* tile);
	IntVec2 RollSpawnLocation(bool treatWaterAsSolid);
	Entity* CreateEntityOfType(EntityType type, Vec2 const& position, float orientation);
	Tile* GetTileByPosition(Vec2 const& position);
	Tile* GetTileByCoordinates(IntVec2 const& coordinates);
	int GetTileIndexByCoordinates(IntVec2 const& coordinates);
	IntVec2 GetMapDimensions() const;
	Player* SpawnPlayer();
	Player* GetPlayer() const;
	void PopulateDistanceFieldForEntityPathToGoal(TileHeatMap& out_distanceField, float maxCost, Entity* e);
	std::vector<Vec2> GenerateEntityPathToGoal(TileHeatMap const& distanceField, IntVec2 const& goalCoords);
	bool HasLineOfSight(Vec2 const& startPos, Vec2 const& targetPos);

private:
	RaycastResultLibra FastRaycastVsTile(Vec2 const& start, Vec2 const& forward, float maxLength);
	void UpdatePlayerDuringFading(float deltaSeconds);
	void UpdateEntities(float deltaSeconds);
	void PushEntitiesOutOfEachOther(float deltaSeconds);
	void PushEntitiesOutOfWall(float deltaSeconds);
	void CheckProjectileHits(float deltaSeconds);
	void CheckProjectilesWithEntities(EntityList projectiles, EntityList actors);
	void CheckBulletVsActor(Bullet& bullet, Entity& actor);
	void UpdateTiles();
	void DeleteGarbageEntities();
	void PushTwoEntitiesOutOfEachOther(Entity& a, Entity& b);
	void PushEntityOutOfWalls(Entity& e, Tile& tile);
	void PushEntityOutOfTile(Entity& e, Tile& tile);
	void RenderFading() const;
	void RenderEntities() const;
	void RenderEntitiesOfTypes(EntityType entityTypeIndex) const;
	void RenderEntitiesHealth(EntityType entityTypeIndex) const;
	void RenderTiles() const;
	void UpdateCamera(float deltaSeconds);
	void AddScreenshake(float deltaScreenshake);
	void SetCameraOrtho();
	void CheckIfGoToNextLevel();
	IntVec2 RollSpawnLocationWithinWall();
	IntVec2 RollRandomDirection();
	void CreateMap();
	void GenerateMapImage();
	void GenerateSpawn();
	void GenerateGoal();
	void SpawnEnemies();

private:
	World* m_world = nullptr;
	Player* m_player = nullptr;
	std::vector<Tile> m_tiles;
	EntityList m_allEntities;
	EntityList m_entityListsByType[NUM_ENTITY_TYPES] = {};
	EntityList m_actorListsByFaction[NUM_ENTITY_FACTIONS] = {};
	EntityList m_projectileListsByFaction[NUM_ENTITY_FACTIONS] = {};
	EntityList m_explosions;
	IntVec2 m_dimensions;
	Camera m_worldCamera;
	float m_screenshakeAmount = 0.f;
	float m_fadeDelay = 1.f;
	float m_fadeTimer = 1.f;
	float m_discoveryDelay = .1f;
	float m_discoveryTimer = .1f;
	bool m_isFadingIn = true;
	bool m_isFadingOut = false;
	bool m_isPlayerDiscovered = false;
	Vec2 m_lastPlayerPosition;
	Vec2 m_exit;
	MapDefinition const& m_mapDef;
	TileHeatMap m_solidTileHeatMap;
};


