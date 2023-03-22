#pragma once
#include "Game/Block.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/BlockIterator.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

constexpr int CHUNK_BITS_X = 4;
constexpr int CHUNK_BITS_Y = 4;
constexpr int CHUNK_BITS_Z = 7;

constexpr int CHUNK_SIZE_X = (1 << CHUNK_BITS_X);
constexpr int CHUNK_SIZE_Y = (1 << CHUNK_BITS_Y);
constexpr int CHUNK_SIZE_Z = (1 << CHUNK_BITS_Z);
constexpr int CHUNK_MAX_X = CHUNK_SIZE_X - 1;
constexpr int CHUNK_MAX_Y = CHUNK_SIZE_Y - 1;
constexpr int CHUNK_MAX_Z = CHUNK_SIZE_Z - 1;

constexpr int CHUNK_MASK_X = CHUNK_MAX_X;
constexpr int CHUNK_MASK_Y = CHUNK_MAX_Y;
constexpr int CHUNK_MASK_Z = CHUNK_MAX_Z;
constexpr int CHUNK_BITSHIFT_X = 0;
constexpr int CHUNK_BITSHIFT_Y = CHUNK_BITS_X;
constexpr int CHUNK_BITSHIFT_Z = CHUNK_BITS_Y + CHUNK_BITS_X;

constexpr int CHUNK_BLOCKS_PER_LAYER = CHUNK_SIZE_X * CHUNK_SIZE_Y;
constexpr int CHUNK_BLOCKS_TOTAL = CHUNK_BLOCKS_PER_LAYER * CHUNK_SIZE_Z;

constexpr int SEA_LEVEL = CHUNK_SIZE_Z / 2;
constexpr int MAX_OCEAN_DEPTH = SEA_LEVEL - 20;
constexpr int RIVER_WIDTH = 1;
constexpr float TERRAIN_HEIGHT_VARIANCE = 50.0f;
constexpr float COAL_PERCENTAGE = 5.f;
constexpr float IRON_PERCENTAGE = 2.f;
constexpr float GOLD_PERCENTAGE = 0.5f;
constexpr float DIAMOND_PERCENTAGE = 0.1f;
constexpr int RESERVED_VERTICES = 20'000;
constexpr int RESERVED_INDICES = RESERVED_VERTICES * 2 / 3;
constexpr float EAST_WEST_LIGHT_SCALING = 0.95f;
constexpr float NORTH_SOUTH_LIGHT_SCALING = 0.85f;
constexpr float HUMIDITY_THRESHOLD = 0.3f;
constexpr float BEACH_THRESHOLD = 0.6f;
constexpr float TEMPERATURE_THRESHOLD = 0.3f;
constexpr int SAND_MAX_DEPTH = 8;

constexpr int TREE_RADIUS = 3;
constexpr int LOCAL_MAX_RADIUS = 2;
constexpr int TREE_NOISE_SIZE = TREE_RADIUS + LOCAL_MAX_RADIUS;

constexpr int VILLAGE_NOISE_RADIUS = 5;

class VertexBuffer;
class IndexBuffer;
class World;

enum class ChunkState
{
	INITIALIZING,

	ACTIVATING_QUEUE_GENERATE,
	ACTIVATING_GENERATING,
	ACTIVATING_GENERATE_DONE,

	ACTIVATING_QUEUE_LOAD,
	ACTIVATING_LOADING,
	ACTIVATING_LOAD_DONE,

	DEACTIVATING_QUEUE_SAVE,
	DEACTIVATING_SAVTING,
	DEACTIVATING_SAVE_DONE,

	ACTIVE,

	NUM_CHUNK_STATES
};

class Chunk
{

public:
	Chunk() {}
	Chunk(World* world, IntVec2 const& chunckCoords);
	~Chunk();

	void Update(float deltaSeconds);
	void Render() const;
	void RenderDebug() const;
	void RenderWater() const;

	void PopulateBlocksFromDisk();
	void SaveBlocksToDisk();
	void GenerateBlocks();
	void AddDebugDrawing();
	bool RefreshVertexBuffer();
	int GetIndexForLocalCoords(IntVec3 const& coords) const;
	int GetIndexForWorldCoords(IntVec3 const& coords) const;
	IntVec3 GetLocalCoordsForWorldCoords(IntVec3 const& coords) const;
	IntVec3 GetLocalCoordsForIndex(int index) const;
	IntVec3 GetWorldCoordsForIndex(int index) const;
	int GetVerticesCounts() const;
	bool AllNeighborsAreActivated() const;
	uint8_t GetBlockTypeAtCoords(IntVec3 const& coords) const;
	void DigBlockAt(BlockIterator const& blockItr);
	void PlaceBlockAt(BlockIterator const& blockItr, uint8_t blockType);
	void SetBufferDirty();
	void InitializeLighting();

private:
	bool IsLocalCoordsTreenessLocalMax(IntVec2 const& coords, std::map<IntVec2, float> const& treenessNoise) const;
	void GenerateTree(IntVec2 const& coords);
	bool IsCoordsVillageLocalMax(std::map<IntVec2, float> const& villageNoise) const;
	void GenerateVillage();
	void AddIndexedVertsForBlock(BlockIterator const& blockItr, std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, AABB3 const& bounds, BlockDefinition const* blockDef, 
								bool renderWest = true, bool renderEast = true, bool renderNorth = true, bool renderSouth = true, bool renderTop = true, bool renderBottom = true);
	void GetBlockLighting(unsigned char& outdoorLight, unsigned char& indoorLight, BlockIterator const& blockIterator);

public:
	World* m_world = nullptr;
	IntVec2 m_coordinates = IntVec2::ZERO;
	AABB3 m_bounds = AABB3::ZERO_TO_ONE;
	Block m_blocks[CHUNK_BLOCKS_TOTAL] = { };
	std::vector<Vertex_PCU> m_debugVertices;
	std::vector<Vertex_PCU> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Vertex_PCU> m_waterVertices;
	std::vector<unsigned int> m_waterIndices;
	VertexBuffer* m_debugVertexBuffer = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	VertexBuffer* m_waterVertexBuffer = nullptr;
	IndexBuffer* m_waterIndexBuffer = nullptr;
	bool m_isBufferDirty = true;
	bool m_isLightingDirty = false;
	bool m_needSaving = false;
	bool m_isHiddenSurfaceRemovedDisable = false;
	int m_worldSeed = 0;

	Chunk* m_westChunk = nullptr;
	Chunk* m_eastChunk = nullptr;
	Chunk* m_northChunk = nullptr;
	Chunk* m_southChunk = nullptr;

	std::atomic<ChunkState> m_state = ChunkState::INITIALIZING;
};