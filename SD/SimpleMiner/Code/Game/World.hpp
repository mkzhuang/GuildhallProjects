#pragma once
#include "Game/BlockIterator.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"

#include <map>
#include <deque>
#include <vector>

constexpr float MAX_RAYCAST_LENGTH = 8.0f;
constexpr int MAX_LIGHT_STRENGTH = 15;
constexpr float REAL_TIME_RATIO = 200.f;
constexpr float DAYS_PER_SECOND = 1.f / (60.f * 60.f * 24.f);
constexpr float TIME_ACCELERATE_SCALE = 50.f;
constexpr int NUM_BUILD_CHUNK_PER_FRAME = 2;

const int GAME_CONSTANT_BUFFER_SLOT = 4;

struct GameConstant
{
	Vec4 CameraWorldPosition = Vec4(0.f, 0.f, 0.f, 1.f);
	Vec4 SkyColor = Vec4(0.f, 0.f, 0.f, 1.f);
	Vec4 OutdoorLightColor = Vec4(1.f, 1.f, 1.f, 1.f);
	Vec4 IndoorLightColor = Vec4(1.f, 0.9f, 0.8f, 1.f);
	float  FogStartDistance = 0.f;
	float  FogEndDistance = 0.f;
	float  FogMaxAlpha = 1.f;
	float  CurrentTime = 0.f;
};

bool operator<(IntVec2 const& a, IntVec2 const& b);

class Game;
class Entity;
class Player;
class Chunk;
struct Block;

struct GameRaycastResult3D : public RaycastResult3D
{
public:
	GameRaycastResult3D() {}
	GameRaycastResult3D(Vec3 const& rayStart, Vec3 const forwardNormal, float maxLength, bool didImpact, Vec3 const& impactPosition, float impactDistance, Vec3 const& impactSurfaceNormal);
	~GameRaycastResult3D() {}

public:
	BlockIterator m_impactBlock;
};

class World
{
public:
	World(Game* game);
	~World();
	void Startup(Vec3 const& pos, EulerAngles const& orientation);

	void Update(float deltaSeconds);
	void Render() const;
	void RenderUI(Camera camera) const;
	std::string GetChunksInfo() const;
	int GetWorldSeed() const;
	GameRaycastResult3D RaycastVsBlocks(Vec3 const& rayStart, Vec3 const& forwardNormal, float maxLength) const;
	void MarkLightingDirty(BlockIterator const& blockItr);
	void SetGameConstant();
	void BindGameConstantBuffer();
	Vec3 const& GetPlayerPos() const;
	EulerAngles const& GetPlayerOrientation() const;

private:
	void UpdateWorld(float deltaSeconds);
	bool ActivateNearestChunk();
	bool DeactivateFurthestChunk();
	void ActivateChunk(IntVec2 const& chunkCoords);
	void DeactivateChunk(IntVec2 const& chunkCoords);
	void RetrieveCompletedJobs();
	void AddActiveChunkToWorld(Chunk* chunk);
	IntVec2 GetChunkCoordinatesForPosition(Vec3 const& pos) const;
	Chunk* GetChunkForCoordinate(IntVec2 const& coordinate) const;
	BlockIterator GetBlockIteratorForPosition(Vec3 const& pos) const;
	void ProcessDirtyLighting();
	void ProcessNextDirtyLightBlock(BlockIterator const& blockItr);
	void GetMaxLight(int& maxIndoorNeighborLight, int& maxOutdoorNeighborLight, Block const* neighborBlock);
	void UndirtyAllBlocksInChunk(Chunk* chunk);
	void UpdateDebugInfo(float deltaSeconds);
	void PushEntityOutOfBlocks(Entity* entity);

public:
	int m_worldSeed = 0;
	Game* m_game = nullptr;
	Player* m_player = nullptr;
	std::map<IntVec2, Chunk*> m_activeChunks;
	std::map<IntVec2, Chunk*> m_generationChunks;
	std::vector<IntVec2> m_offsets;
	std::vector<IntVec2> m_offsetsReversed;
	Texture const* m_texture = nullptr;
	Shader* m_shader = nullptr;
	float m_chunkActivationRange = 0.f;
	float m_chunkDeactivationRange = 0.f;
	int m_maxChunkRadiusX = 0;
	int m_maxChunkRadiusY = 0;
	int m_maxChunks = 0;
	std::deque<BlockIterator> m_dirtyLightingQueue;
	bool m_isWorldTimeSpeedUp = false;
	float m_worldDay = 0.5f;
	std::vector<BlockIterator> m_blocksToPushOut;

	ConstantBuffer* m_gameCBO = nullptr;
	GameConstant m_gameConstant;
	bool m_hasDistanceFog = false;

	//profiling variables
	int m_activatingFrames = -1;
	float m_activatingFrametimes = 0.f;
	int m_stableFrames = 0;
	float m_stableFrametimes = 0.f;
	//int m_perlinGenerationCounts = 0;
	//double m_perlinGenerationFrametimes = 0.0;
	//double m_perlinGenerationWorse = 0.0;
	int m_diskLoadCounts = 0;
	double m_diskLoadFrametimes = 0.0;
	double m_diskLoadWorse = 0.0;
	int m_diskSaveCounts = 0;
	double m_diskSaveFrametimes = 0.0;
	double m_diskSaveWorse = 0.0;
	int m_rebuildBufferCounts = 0;
	double m_rebuildBufferFrametimes = 0.0;
	double m_rebuildBufferWorse = 0.0;
	int m_resolveLightingCounts = 0;
	double m_resolveLightingFrametimes = 0.0;
	double m_resolveLightingWorse = 0.0;
};


