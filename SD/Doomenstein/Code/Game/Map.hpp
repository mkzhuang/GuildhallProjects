#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Game/Actor.hpp"
#include "Game/SpawnInfo.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"

#include <vector>

//------------------------------------------------------------------------------------------------
class Game;
class MapDefinition;
class Shader;
class Texture;
struct AABB2;
struct Vertex_PCU;
class IndexBuffer;
class VertexBuffer;
class Player;

struct RaycastResultDoomenstein : public RaycastResult3D
{
public:
	RaycastResultDoomenstein(bool didImpact, Vec3 impactPosition, float impactDistance, Vec3 impactSurtaceNormal,
							 Vec3 startPosition, Vec3 forwardNormal, float maxDistance);
	~RaycastResultDoomenstein() {}

public:
	Actor* m_impactActor = nullptr;
	bool m_hitFloor = false;
	float m_impactFraction = 0.0f;
};

struct RaycastFilter
{
	Actor* m_ignoreActor = nullptr;
};

//------------------------------------------------------------------------------------------------
class Map
{
public:
	Map( Game* game, const MapDefinition* definition );
	~Map();

	void Update(float deltaSeconds);
	void Render(Camera const& camera);

	void CreateTiles();
	void CreateGeometry();
	void CreateBuffers();
	RaycastResultDoomenstein RaycastAll(Vec3 const& start, Vec3 const& direction, float distance, RaycastFilter filter = RaycastFilter()) const;

	void SpawnWave();
	void UpdateActors(float deltaSeconds);
	void CollideActors();
	void CollideActors(Actor* actorA, Actor* actorB);
	void CollideActorsWithMap();
	void CollideActorWithMap(Actor* actor);
	void PushActorOutOfWall(Actor* actor, Tile const* tile);
	Clock& GetGameClock() const;

	void RespawnPlayer(Player* controller);
	Actor* SpawnActor(SpawnInfo const& spawnInfo);
	void DestroyActor(ActorUID const uid);
	Actor* FindActorByUID(ActorUID const uid) const;
	Actor* GetClosestVisibleEnemy(Actor* actor);
	Player* GetPlayerController(int index) const;
	std::vector<Player*> GetAllPlayerControllers() const;
	void IncreaseKills();
	Game* GetGame();

	void PossessNextActor();

private:
	int GetDemonCount() const;
	void AddVertsForTile(IntVec2 const& tileCoord);
	void AddVertsForFloorTile(IntVec2 const& tileCoord);
	void AddVertsForSolidTile(IntVec2 const& tileCoord);
	Tile const* GetTileByPosition(Vec2 const& position) const;
	Tile const* GetTileByCoordinate(IntVec2 const& coordinate) const;
	void AddWorldBasis() const;
	RaycastResultDoomenstein RaycastWorldXY(Vec3 const& start, Vec3 const& direction, float distance) const;
	RaycastResultDoomenstein RaycastWorldZ(Vec3 const& start, Vec3 const& direction, float distance) const;
	RaycastResultDoomenstein RaycastWorldActors(Vec3 const& start, Vec3 const& direction, float distance, RaycastFilter filter = RaycastFilter()) const;

protected:

	// Info
	Game* m_game = nullptr;
	int m_actorSalt = 0x0000fffe;

	// Map
	MapDefinition const* m_definition = nullptr;
	std::vector<SpawnInfo> m_marineSpawnInfos;
	std::vector<SpawnInfo> m_demonSpawnInfos;
	std::vector<Tile> m_tiles;
	IntVec2 m_dimensions;

	// Rendering
	std::vector<Actor*> m_actors;
	std::vector<Vertex_PNCU> m_vertices;
	std::vector<unsigned int> m_indices;
	const Texture* m_texture = nullptr;
	Shader* m_shader = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	bool m_displayLightSettings = true;
	EulerAngles m_sunDirection = EulerAngles(0.f, 135.f, 0.f);
	float m_sunIntensity = 0.4f;
	Vec3 m_sunColor;
	float m_ambientIntensity = 0.8f;

public:
	Stopwatch m_waveTimer;
	int m_waveCounter = 0;
	int m_currentWaveDemonCounts = 0;
	int m_demonMultiplierPerWave = 4;
};


