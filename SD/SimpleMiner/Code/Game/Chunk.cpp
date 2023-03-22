#include "Game/Chunk.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Game/TemplateDefinition.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"

Chunk::Chunk(World* world, IntVec2 const& chunckCoords)
	: m_world(world)
	, m_coordinates(chunckCoords)
{
	IntVec3 mins(m_coordinates.x << CHUNK_BITS_X, m_coordinates.y << CHUNK_BITS_Y, 0);
	IntVec3 maxs((m_coordinates.x + 1) << CHUNK_BITS_X, (m_coordinates.y + 1) << CHUNK_BITS_Y, CHUNK_SIZE_Z);
	m_bounds.m_mins = Vec3(mins);
	m_bounds.m_maxs = Vec3(maxs);
	m_isHiddenSurfaceRemovedDisable = g_gameConfigBlackboard.GetValue("disableHiddenSurfaceRemoved", false);
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int));
	m_waterVertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	m_waterIndexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int));
	m_vertices.reserve(RESERVED_VERTICES);
	m_indices.reserve(RESERVED_INDICES);
	m_waterVertices.reserve(RESERVED_VERTICES);
	m_waterIndices.reserve(RESERVED_INDICES);
	m_worldSeed = m_world->GetWorldSeed();
}


Chunk::~Chunk()
{
	if (m_needSaving)
	{
		m_world->m_diskSaveCounts++;
		double start = GetCurrentTimeSeconds();

		SaveBlocksToDisk();

		double end = GetCurrentTimeSeconds();
		double duration = end - start;
		if (duration > m_world->m_diskSaveWorse) m_world->m_diskSaveWorse = duration;
		m_world->m_diskSaveFrametimes += duration;

	}

	delete m_debugVertexBuffer;
	delete m_vertexBuffer;
	delete m_indexBuffer;
	delete m_waterVertexBuffer;
	delete m_waterIndexBuffer;
}


void Chunk::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds)
}


void Chunk::Render() const
{
	g_theRenderer->DrawIndexBuffer(m_vertexBuffer, m_indexBuffer, (int)m_indices.size());
}


void Chunk::RenderDebug() const
{
	g_theRenderer->DrawVertexBuffer(m_debugVertexBuffer, (int)m_debugVertices.size());
}


void Chunk::RenderWater() const
{
	g_theRenderer->DrawIndexBuffer(m_waterVertexBuffer, m_waterIndexBuffer, (int)m_waterIndices.size());
}


void Chunk::PopulateBlocksFromDisk()
{
	std::string fileName = Stringf("Saves/World_%i/Chunk(%i,%i).chunk", m_worldSeed, m_coordinates.x, m_coordinates.y);
	std::vector<uint8_t> buffer;
	FileReadToBuffer(buffer, fileName);
	uint8_t g = buffer[0];
	uint8_t c = buffer[1];
	uint8_t h = buffer[2];
	uint8_t k = buffer[3];
	uint8_t version = buffer[4];
	uint8_t chunkBitsX = buffer[5];
	uint8_t chunkBitsY = buffer[6];
	uint8_t chunkBitsZ = buffer[7];
	if (g != 'G' || c != 'C' || h != 'H' || k != 'K' || version != 1 || chunkBitsX != CHUNK_BITS_X || chunkBitsY != CHUNK_BITS_Y || chunkBitsZ != CHUNK_BITS_Z)
	{
		GenerateBlocks();
		return;
	}

	int blockIndex = 0;
	for (int bufferIndex = 8; bufferIndex < (int)buffer.size(); bufferIndex += 2)
	{
		uint8_t currentBlockType = buffer[bufferIndex];
		uint8_t currentBlockCount = buffer[bufferIndex + 1];
		for (int blockCount = 0; blockCount < currentBlockCount; blockCount++)
		{
			m_blocks[blockIndex].m_type = currentBlockType;
			if (BlockDefinition::GetById(currentBlockType)->m_isOpaque)
			{
				m_blocks[blockIndex].SetBlockOpaque();
			}
			blockIndex++;
		}
	}
}


void Chunk::SaveBlocksToDisk()
{
	std::string fileName = Stringf("Saves/World_%i/Chunk(%i,%i).chunk", m_worldSeed, m_coordinates.x, m_coordinates.y);
	std::vector<uint8_t> buffer;
	buffer.reserve(10'000);
	buffer.push_back('G');
	buffer.push_back('C');
	buffer.push_back('H');
	buffer.push_back('K');
	buffer.push_back(1);
	buffer.push_back(CHUNK_BITS_X);
	buffer.push_back(CHUNK_BITS_Y);
	buffer.push_back(CHUNK_BITS_Z);

	uint8_t currentBlockType = m_blocks[0].m_type;
	uint8_t currentBlockCount = 1;
	for (int blockIndex = 1; blockIndex < CHUNK_BLOCKS_TOTAL; blockIndex++)
	{
		uint8_t type = m_blocks[blockIndex].m_type;

		if (currentBlockType == type)
		{
			if (currentBlockCount == 255)
			{
				buffer.push_back(currentBlockType);
				buffer.push_back(currentBlockCount);
				currentBlockCount = 0;
			}
			currentBlockCount++;
		}
		else
		{
			buffer.push_back(currentBlockType);
			buffer.push_back(currentBlockCount);
			currentBlockType = type;
			currentBlockCount = 1;
		}
	}

	buffer.push_back(currentBlockType);
	buffer.push_back(currentBlockCount);

	FileWriteFromBuffer(buffer, fileName);
}


void Chunk::GenerateBlocks()
{
	float heightsNoise[CHUNK_BLOCKS_PER_LAYER] = {};
	float humidityNoise[CHUNK_BLOCKS_PER_LAYER] = {};
	float temperatureNoise[CHUNK_BLOCKS_PER_LAYER] = {};
	float hillinessNoise[CHUNK_BLOCKS_PER_LAYER] = {};
	float oceannessNoise[CHUNK_BLOCKS_PER_LAYER] = {};
	std::map<IntVec2, float> treenessNoise;
	std::map<IntVec2, float> villageNoise;

	uint8_t air			= (uint8_t)BlockDefinition::GetIndexByName("air");
	uint8_t water		= (uint8_t)BlockDefinition::GetIndexByName("water");
	uint8_t grass		= (uint8_t)BlockDefinition::GetIndexByName("grass");
	uint8_t dirt		= (uint8_t)BlockDefinition::GetIndexByName("dirt");
	uint8_t stone		= (uint8_t)BlockDefinition::GetIndexByName("stone");
	uint8_t coal		= (uint8_t)BlockDefinition::GetIndexByName("coal");
	uint8_t iron		= (uint8_t)BlockDefinition::GetIndexByName("iron");
	uint8_t gold		= (uint8_t)BlockDefinition::GetIndexByName("gold");
	uint8_t diamond		= (uint8_t)BlockDefinition::GetIndexByName("diamond");
	uint8_t sand		= (uint8_t)BlockDefinition::GetIndexByName("sand");
	uint8_t ice			= (uint8_t)BlockDefinition::GetIndexByName("ice");
	uint8_t	snow_grass	= (uint8_t)BlockDefinition::GetIndexByName("snow_grass");

	int chunkGlobalX = m_coordinates.x << CHUNK_BITS_X;
	int chunkGlobalY = m_coordinates.y << CHUNK_BITS_Y;

	// calculate terrain height
	for (int localY = 0; localY < CHUNK_SIZE_Y; localY++)
	{
		for (int localX = 0; localX < CHUNK_SIZE_X; localX++)
		{
			int index = localY * CHUNK_SIZE_X + localX;
			float globalX = static_cast<float>(chunkGlobalX + localX);
			float globalY = static_cast<float>(chunkGlobalY + localY);
			heightsNoise[index] = Compute2dPerlinNoise(globalX, globalY, 275.f, 5, 0.75f, 2.0f, true, m_worldSeed);
			humidityNoise[index] = 0.5f + 0.5f * Compute2dPerlinNoise(globalX, globalY, 800.f, 3, 0.75f, 3.0f, true, m_worldSeed + 1);
			temperatureNoise[index] = 0.5f + 0.5f * Compute2dPerlinNoise(globalX, globalY, 500.f, 3, 0.5f, 2.0f, true, m_worldSeed + 2);
			hillinessNoise[index] = SmoothStep3(0.5f + 0.5f * Compute2dPerlinNoise(globalX, globalY, 1000.f, 2, 2.0f, 0.5f, true, m_worldSeed + 3));
			oceannessNoise[index] = SmoothStart3(Compute2dPerlinNoise(globalX, globalY, 1200.f, 2, 0.5f, 0.5f, true, m_worldSeed + 4));
			heightsNoise[index] *= hillinessNoise[index];
		}
	}

	//set individual block type
	for (int localZ = 0; localZ < CHUNK_SIZE_Z; localZ++)
	{
		for (int localY = 0; localY < CHUNK_SIZE_Y; localY++)
		{
			for (int localX = 0; localX < CHUNK_SIZE_X; localX++)
			{
				int columnIndex = localY * CHUNK_SIZE_X + localX;
				IntVec3 localCoords(localX, localY, localZ);
				int localIndex = GetIndexForLocalCoords(localCoords);
				float terrainRawHeight = static_cast<float>(SEA_LEVEL - RIVER_WIDTH) + TERRAIN_HEIGHT_VARIANCE * fabsf(heightsNoise[columnIndex]);
				int terrainHeight = static_cast<int>(RangeMapClamped(oceannessNoise[columnIndex], 0.0f, 0.5f, terrainRawHeight, MAX_OCEAN_DEPTH));
				int dirtCount = RNG.RollRandomIntInRange(3, 4);
				if (localZ > terrainHeight)
				{
					if (localZ <= SEA_LEVEL)
					{
						if (temperatureNoise[columnIndex] < TEMPERATURE_THRESHOLD)
						{
							m_blocks[localIndex].m_type = ice;
						}
						else
						{
							m_blocks[localIndex].m_type = water;
						}
					}
					else
					{
						m_blocks[localIndex].m_type = air;
					}
				}
				else if (localZ == terrainHeight)
				{
					if (humidityNoise[columnIndex] < HUMIDITY_THRESHOLD)
					{
						m_blocks[localIndex].m_type = sand;
					}
					else
					{
						float snowChance = RangeMapClamped(static_cast<float>(terrainHeight), 75.f, 128.f, 0.f, 1.f);
						if (temperatureNoise[columnIndex] - snowChance < TEMPERATURE_THRESHOLD)
						{
							m_blocks[localIndex].m_type = snow_grass;
						}
						else
						{
							m_blocks[localIndex].m_type = grass;
						}	
					}
				}
				else if (localZ < terrainHeight && localZ >= (terrainHeight - dirtCount))
				{
					m_blocks[localIndex].m_type = dirt;
				}
				else
				{
					float chanceOfCoal = RNG.RollRandomFloatInRange(0.f, 100.f);
					if (chanceOfCoal <= COAL_PERCENTAGE)
					{
						m_blocks[localIndex].m_type = coal;
					}
					else
					{
						float chanceOfIron = RNG.RollRandomFloatInRange(0.f, 100.f);
						if (chanceOfIron <= IRON_PERCENTAGE)
						{
							m_blocks[localIndex].m_type = iron;
						}
						else
						{
							float chanceOfGold = RNG.RollRandomFloatInRange(0.f, 100.f);
							if (chanceOfGold <= GOLD_PERCENTAGE)
							{
								m_blocks[localIndex].m_type = gold;
							}
							else
							{
								float chanceOfDiamond = RNG.RollRandomFloatInRange(0.f, 100.f);
								if (chanceOfDiamond <= DIAMOND_PERCENTAGE)
								{
									m_blocks[localIndex].m_type = diamond;
								}
								else
								{
									m_blocks[localIndex].m_type = stone;
								}
							}
						}
					}
				}

				if (localZ == SEA_LEVEL && m_blocks[localIndex].m_type == grass && humidityNoise[columnIndex] < BEACH_THRESHOLD)
				{
					m_blocks[localIndex].m_type = sand;
				}

				if (localZ < terrainHeight && humidityNoise[columnIndex] < HUMIDITY_THRESHOLD)
				{
					int sandBlocks = static_cast<int>(RangeMapClamped(humidityNoise[columnIndex], 0.f, HUMIDITY_THRESHOLD, static_cast<float>(SAND_MAX_DEPTH), 0.f));
					if (localZ > terrainHeight - sandBlocks)
					{
						m_blocks[localIndex].m_type = sand;
					}
				}

				if (BlockDefinition::GetById(m_blocks[localIndex].m_type)->m_isOpaque)
				{
					m_blocks[localIndex].SetBlockOpaque();
				}
			}
		}
	}

	for (int localY = -TREE_NOISE_SIZE; localY < CHUNK_SIZE_Y + TREE_NOISE_SIZE; localY++)
	{
		for (int localX = -TREE_NOISE_SIZE; localX < CHUNK_SIZE_X + TREE_NOISE_SIZE; localX++)
		{
			float globalX = static_cast<float>(chunkGlobalX + localX);
			float globalY = static_cast<float>(chunkGlobalY + localY);
			float treeDensity = 0.5f + 0.5f * Compute2dPerlinNoise(globalX, globalY, 1000.f, 2, 0.5f, 2.0f, true, m_worldSeed + 5);
			treenessNoise[IntVec2(localX, localY)] = 0.5f + 0.5f * Compute2dPerlinNoise(globalX, globalY, 250.f, 5, treeDensity, 2.0f, true, m_worldSeed + 6);
		}
	}

	for (int localY = -TREE_RADIUS; localY < CHUNK_SIZE_Y + TREE_RADIUS; localY++)
	{
		for (int localX = -TREE_RADIUS; localX < CHUNK_SIZE_X + TREE_RADIUS; localX++)
		{
			IntVec2 currentCoords(localX, localY);
			bool isLocalMax = IsLocalCoordsTreenessLocalMax(currentCoords, treenessNoise);
			if (isLocalMax)
			{
				GenerateTree(currentCoords);
			}
		}
	}


	for (int chunkY = -VILLAGE_NOISE_RADIUS; chunkY < VILLAGE_NOISE_RADIUS; chunkY++)
	{
		for (int chunkX = -VILLAGE_NOISE_RADIUS; chunkX < VILLAGE_NOISE_RADIUS; chunkX++)
		{
			IntVec2 currentCoords = m_coordinates + IntVec2(chunkX, chunkY);
			villageNoise[currentCoords] = 0.5f + 0.5f * Compute2dPerlinNoise(static_cast<float>(currentCoords.x), static_cast<float>(currentCoords.y), 20.f, 2, 0.5f, 2.0f, true, m_worldSeed + 7);
		}
	}

	bool isLocalMax = IsCoordsVillageLocalMax(villageNoise);
	if (isLocalMax)
	{
		GenerateVillage();
	}
}


void Chunk::AddDebugDrawing()
{
	// add debug verts
	AddVertsForWireAABB3D(m_debugVertices, m_bounds, 0.025f);
	m_debugVertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * m_debugVertices.size(), sizeof(Vertex_PCU));
	g_theRenderer->CopyCPUToGPU(m_debugVertices.data(), sizeof(Vertex_PCU) * m_debugVertices.size(), m_debugVertexBuffer);
}


int Chunk::GetIndexForLocalCoords(IntVec3 const& coords) const
{
	return coords.x | coords.y << CHUNK_BITSHIFT_Y | coords.z << CHUNK_BITSHIFT_Z;
}


int Chunk::GetIndexForWorldCoords(IntVec3 const& coords) const
{
	IntVec3 const& localCoords = GetLocalCoordsForWorldCoords(coords);
	return GetIndexForLocalCoords(localCoords);
}


IntVec3 Chunk::GetLocalCoordsForWorldCoords(IntVec3 const& coords) const
{
	int x = coords.x & CHUNK_MASK_X;
	int y = coords.y & CHUNK_MASK_Y;
	return IntVec3(x, y, coords.z);
}


IntVec3 Chunk::GetLocalCoordsForIndex(int index) const
{
	int x = index & CHUNK_MASK_X;
	int y = index >> CHUNK_BITSHIFT_Y & CHUNK_MASK_Y;
	int z = index >> CHUNK_BITSHIFT_Z & CHUNK_MASK_Z;
	return IntVec3(x, y, z);
}


IntVec3 Chunk::GetWorldCoordsForIndex(int index) const
{
	IntVec3 localCoords = GetLocalCoordsForIndex(index);
	int x = (m_coordinates.x << CHUNK_BITS_X) + localCoords.x;
	int y = (m_coordinates.y << CHUNK_BITS_Y) + localCoords.y;
	int z = localCoords.z;
	return IntVec3(x, y, z);
}


int Chunk::GetVerticesCounts() const
{
	return (int)m_vertices.size() + (int)m_waterVertices.size();
}


bool Chunk::AllNeighborsAreActivated() const
{
	return m_northChunk && m_southChunk && m_westChunk && m_eastChunk;
}


uint8_t Chunk::GetBlockTypeAtCoords(IntVec3 const& coords) const
{
	int index = GetIndexForLocalCoords(coords);
	return m_blocks[index].m_type;
}


void Chunk::DigBlockAt(BlockIterator const& blockItr)
{
	uint8_t air = BlockDefinition::GetIndexByName("air");

	m_blocks[blockItr.m_blockIndex].m_type = air;
	m_blocks[blockItr.m_blockIndex].SetBlockNotOpaque();
	m_isBufferDirty = true;
	m_needSaving = true;

	m_world->MarkLightingDirty(blockItr);

	// mark block sky
	BlockIterator topNeightbor = blockItr.GetTopNeighbor(); // top
	Block* topBlock = topNeightbor.GetBlock();
	if (topBlock && topBlock->IsBlockSky())
	{
		Block* currentBlock = blockItr.GetBlock();
		currentBlock->SetBlockSky();
	}

	// marking neighbors dirty
	BlockIterator westNeighbor = blockItr.GetWestNeighbor(); // west
	Block* westBlock = westNeighbor.GetBlock();
	if (westBlock && westBlock->IsBlockOpaque())
	{
		westNeighbor.m_chunk->SetBufferDirty();
	}

	BlockIterator eastNeighbor = blockItr.GetEastNeighbor(); // east
	Block* eastBlock = eastNeighbor.GetBlock();
	if (eastBlock && eastBlock->IsBlockOpaque())
	{
		eastNeighbor.m_chunk->SetBufferDirty();
	}

	BlockIterator northNeighbor = blockItr.GetNorthNeighbor(); // north
	Block* northBlock = northNeighbor.GetBlock();
	if (northBlock && northBlock->IsBlockOpaque())
	{
		northNeighbor.m_chunk->SetBufferDirty();
	}

	BlockIterator southNeighbor = blockItr.GetSouthNeighbor(); // south
	Block* southBlock = southNeighbor.GetBlock();
	if (southBlock && southBlock->IsBlockOpaque())
	{
		southNeighbor.m_chunk->SetBufferDirty();
	}
}


void Chunk::PlaceBlockAt(BlockIterator const& blockItr, uint8_t blockType)
{
	m_blocks[blockItr.m_blockIndex].m_type = blockType;
	m_blocks[blockItr.m_blockIndex].SetBlockOpaque();
	m_isBufferDirty = true;
	m_needSaving = true;

	m_world->MarkLightingDirty(blockItr);

	// marking neighbors dirty
	BlockIterator westNeighbor = blockItr.GetWestNeighbor(); // west
	Block* westBlock = westNeighbor.GetBlock();
	if (westBlock && westBlock->IsBlockOpaque())
	{
		westNeighbor.m_chunk->SetBufferDirty();
	}

	BlockIterator eastNeighbor = blockItr.GetEastNeighbor(); // east
	Block* eastBlock = eastNeighbor.GetBlock();
	if (eastBlock && eastBlock->IsBlockOpaque())
	{
		eastNeighbor.m_chunk->SetBufferDirty();
	}

	BlockIterator northNeighbor = blockItr.GetNorthNeighbor(); // north
	Block* northBlock = northNeighbor.GetBlock();
	if (northBlock && northBlock->IsBlockOpaque())
	{
		northNeighbor.m_chunk->SetBufferDirty();
	}

	BlockIterator southNeighbor = blockItr.GetSouthNeighbor(); // south
	Block* southBlock = southNeighbor.GetBlock();
	if (southBlock && southBlock->IsBlockOpaque())
	{
		southNeighbor.m_chunk->SetBufferDirty();
	}
}


void Chunk::SetBufferDirty()
{
	m_isBufferDirty = true;
}


void Chunk::InitializeLighting()
{
	SetBufferDirty();

	uint8_t water = (uint8_t) BlockDefinition::GetIndexByName("water");
	// set sky blocks
	for (int localY = 0; localY < CHUNK_SIZE_Y; localY++)
	{
		for (int localX = 0; localX < CHUNK_SIZE_X; localX++)
		{
			int localIndex = GetIndexForLocalCoords(IntVec3(localX, localY, CHUNK_MAX_Z));
			while (!m_blocks[localIndex].IsBlockOpaque())
			{
				m_blocks[localIndex].SetBlockSky();
				m_blocks[localIndex].SetOutdoorLightInfluence(MAX_LIGHT_STRENGTH);
				localIndex -= CHUNK_BLOCKS_PER_LAYER;
			}
		}
	}

	//marking block dirty
	for (int localZ = 0; localZ < CHUNK_SIZE_Z; localZ++)
	{
		for (int localY = 0; localY < CHUNK_SIZE_Y; localY++)
		{
			for (int localX = 0; localX < CHUNK_SIZE_X; localX++)
			{
				int localIndex = GetIndexForLocalCoords(IntVec3(localX, localY, localZ));
				BlockIterator currentBlockItr(this, localIndex);
				Block* block = currentBlockItr.GetBlock();

				// mark border blocks dirty
				if (localX == 0 || localY == 0 || localX == CHUNK_MAX_X || localY == CHUNK_MAX_Y)
				{
					if (!block->IsBlockOpaque() || block->m_type == water)
					{
						m_world->MarkLightingDirty(currentBlockItr);
					}
				}

				// mark light sources dirty
				if (BlockDefinition::GetById(m_blocks[localIndex].m_type)->m_light != 0)
				{
					m_world->MarkLightingDirty(currentBlockItr);
				}

				// mark sky neighbors dirty
				if (block->IsBlockSky())
				{
					BlockIterator eastNeighbor = currentBlockItr.GetEastNeighbor();
					Block* eastBlock = eastNeighbor.GetBlock();
					if (eastBlock)
					{
						eastNeighbor.m_chunk->SetBufferDirty();
						if (!eastBlock->IsBlockOpaque() && !eastBlock->IsBlockSky() || block->m_type == water)
						{
							m_world->MarkLightingDirty(eastNeighbor);
						}
					}

					BlockIterator westNeighbor = currentBlockItr.GetWestNeighbor();
					Block* westBlock = westNeighbor.GetBlock();
					if (westBlock)
					{
						westNeighbor.m_chunk->SetBufferDirty();
						if (!westBlock->IsBlockOpaque() && !westBlock->IsBlockSky() || block->m_type == water)
						{
							m_world->MarkLightingDirty(westNeighbor);
						}
					}

					BlockIterator northNeighbor = currentBlockItr.GetNorthNeighbor();
					Block* northBlock = northNeighbor.GetBlock();
					if (northBlock)
					{
						northNeighbor.m_chunk->SetBufferDirty();
						if (!northBlock->IsBlockOpaque() && !northBlock->IsBlockSky() || block->m_type == water)
						{
							m_world->MarkLightingDirty(northNeighbor);
						}

					}

					BlockIterator southNeighbor = currentBlockItr.GetSouthNeighbor();
					Block* southBlock = southNeighbor.GetBlock();
					if (southBlock)
					{
						southNeighbor.m_chunk->SetBufferDirty();
						if (!southBlock->IsBlockOpaque() && !southBlock->IsBlockSky() || block->m_type == water)
						{
							m_world->MarkLightingDirty(southNeighbor);
						}
					}
				}
			}
		}
	}
}


bool Chunk::IsLocalCoordsTreenessLocalMax(IntVec2 const& coords, std::map<IntVec2, float> const& treenessNoise) const
{
	int currentX = coords.x;
	int currentY = coords.y;
	int startX = currentX - LOCAL_MAX_RADIUS;
	int startY = currentY - LOCAL_MAX_RADIUS;
	int endX = currentX + LOCAL_MAX_RADIUS;
	int endY = currentY + LOCAL_MAX_RADIUS;
	std::map<IntVec2, float>::const_iterator curItr = treenessNoise.find(coords);
	float currentTreeNoise = curItr->second;
	for (int searchY = startY; searchY < endY; searchY++)
	{
		for (int searchX = startX; searchX < endX; searchX++)
		{
			IntVec2 searchCoords(searchX, searchY);
			if (searchCoords == coords) continue;
			std::map<IntVec2, float>::const_iterator searchItr = treenessNoise.find(searchCoords);
			float searchTreeNoise = searchItr->second;
			if (searchTreeNoise >= currentTreeNoise)
			{
				return false;
			}
		}
	}
	return true;
}


void Chunk::GenerateTree(IntVec2 const& coords)
{
	int chunkGlobalX = m_coordinates.x << CHUNK_BITS_X;
	int chunkGlobalY = m_coordinates.y << CHUNK_BITS_Y;
	float globalX = static_cast<float>(chunkGlobalX + coords.x);
	float globalY = static_cast<float>(chunkGlobalY + coords.y);
	float humidity = 0.5f + 0.5f * Compute2dPerlinNoise(globalX, globalY, 800.f, 3, 0.75f, 3.0f, true, m_worldSeed + 1);
	float temperature = 0.5f + 0.5f * Compute2dPerlinNoise(globalX, globalY, 500.f, 3, 0.5f, 2.0f, true, m_worldSeed + 2);
	float heightNoise = Compute2dPerlinNoise(globalX, globalY, 275.f, 5, 0.75f, 2.0f, true, m_worldSeed);
	float hillNoise = SmoothStep3(0.5f + 0.5f * Compute2dPerlinNoise(globalX, globalY, 1000.f, 2, 2.0f, 0.5f, true, m_worldSeed + 3));
	heightNoise *= hillNoise;
	float terrainRawHeight = static_cast<float>(SEA_LEVEL - RIVER_WIDTH) + TERRAIN_HEIGHT_VARIANCE * fabsf(heightNoise);
	float oceanNoise = SmoothStart3(Compute2dPerlinNoise(globalX, globalY, 1200.f, 2, 0.5f, 0.5f, true, m_worldSeed + 4));
	int terrainHeight = static_cast<int>(RangeMapClamped(oceanNoise, 0.0f, 0.5f, terrainRawHeight, MAX_OCEAN_DEPTH));

	if (terrainHeight >= SEA_LEVEL)
	{
		TemplateDefinition const* templateDef = TemplateDefinition::GetByName("oak_3d");
		if (humidity < HUMIDITY_THRESHOLD)
		{
			templateDef = TemplateDefinition::GetByName("cactus");
		}
		else if (temperature < TEMPERATURE_THRESHOLD)
		{
			templateDef = TemplateDefinition::GetByName("spruce_3d");
		}
		IntVec3 startCoords(coords.x, coords.y, terrainHeight + 1);
		for (int templateBlockIndex = 0; templateBlockIndex < (int)templateDef->m_blocks.size(); templateBlockIndex++)
		{
			TemplateBlock const& templateBlock = templateDef->m_blocks[templateBlockIndex];
			uint8_t blockType = BlockDefinition::GetIndexByName(templateBlock.m_blockName);
			IntVec3 blockCoords = startCoords + templateBlock.m_offset;
			if (blockCoords.x < 0 || blockCoords.x > CHUNK_MASK_X || blockCoords.y < 0 || blockCoords.y > CHUNK_MASK_Y) continue;
			int blockIndex = GetIndexForLocalCoords(blockCoords);
			m_blocks[blockIndex].m_type = blockType;
			if (BlockDefinition::GetById(m_blocks[blockIndex].m_type)->m_isOpaque)
			{
				m_blocks[blockIndex].SetBlockOpaque();
			}
		}
	}
}


bool Chunk::IsCoordsVillageLocalMax(std::map<IntVec2, float> const& villageNoise) const
{
	std::map<IntVec2, float>::const_iterator curItr = villageNoise.find(m_coordinates);
	float currentTreeNoise = curItr->second;
	for (int chunkY = -VILLAGE_NOISE_RADIUS; chunkY < VILLAGE_NOISE_RADIUS; chunkY++)
	{
		for (int chunkX = -VILLAGE_NOISE_RADIUS; chunkX < VILLAGE_NOISE_RADIUS; chunkX++)
		{
			IntVec2 searchCoords = m_coordinates + IntVec2(chunkX, chunkY);
			if (searchCoords == m_coordinates) continue;
			std::map<IntVec2, float>::const_iterator searchItr = villageNoise.find(searchCoords);
			float searchTreeNoise = searchItr->second;
			if (searchTreeNoise >= currentTreeNoise)
			{
				return false;
			}
		}
	}
	return true;
}



void Chunk::GenerateVillage()
{
	int chunkGlobalX = m_coordinates.x << CHUNK_BITS_X;
	int chunkGlobalY = m_coordinates.y << CHUNK_BITS_Y;
	float globalX = static_cast<float>(chunkGlobalX + 8);
	float globalY = static_cast<float>(chunkGlobalY + 8);
	float heightNoise = Compute2dPerlinNoise(globalX, globalY, 275.f, 5, 0.75f, 2.0f, true, m_worldSeed);
	float hillNoise = SmoothStep3(0.5f + 0.5f * Compute2dPerlinNoise(globalX, globalY, 1000.f, 2, 2.0f, 0.5f, true, m_worldSeed + 3));
	heightNoise *= hillNoise;
	float terrainRawHeight = static_cast<float>(SEA_LEVEL - RIVER_WIDTH) + TERRAIN_HEIGHT_VARIANCE * fabsf(heightNoise);
	float oceanNoise = SmoothStart3(Compute2dPerlinNoise(globalX, globalY, 1200.f, 2, 0.5f, 0.5f, true, m_worldSeed + 4));
	int terrainHeight = static_cast<int>(RangeMapClamped(oceanNoise, 0.0f, 0.5f, terrainRawHeight, MAX_OCEAN_DEPTH));

	if (terrainHeight >= SEA_LEVEL)
	{
		TemplateDefinition const* templateDef = TemplateDefinition::GetByName("house_3d");
		IntVec3 startCoords(8, 8, terrainHeight + 1);
		for (int templateBlockIndex = 0; templateBlockIndex < (int)templateDef->m_blocks.size(); templateBlockIndex++)
		{
			TemplateBlock const& templateBlock = templateDef->m_blocks[templateBlockIndex];
			uint8_t blockType = BlockDefinition::GetIndexByName(templateBlock.m_blockName);
			IntVec3 blockCoords = startCoords + templateBlock.m_offset;
			if (blockCoords.x < 0 || blockCoords.x > CHUNK_MASK_X || blockCoords.y < 0 || blockCoords.y > CHUNK_MASK_Y) continue;
			int blockIndex = GetIndexForLocalCoords(blockCoords);
			m_blocks[blockIndex].m_type = blockType;
			if (BlockDefinition::GetById(m_blocks[blockIndex].m_type)->m_isOpaque)
			{
				m_blocks[blockIndex].SetBlockOpaque();
			}
		}
	}
}


bool Chunk::RefreshVertexBuffer()
{
	if (m_isBufferDirty && AllNeighborsAreActivated())
	{
		m_world->m_rebuildBufferCounts++;
		double start = GetCurrentTimeSeconds();

		m_vertices.clear();
		m_indices.clear();
		m_waterVertices.clear();
		m_waterIndices.clear();
		m_vertices.reserve(RESERVED_VERTICES);
		m_indices.reserve(RESERVED_INDICES);
		m_waterVertices.reserve(RESERVED_VERTICES);
		m_waterIndices.reserve(RESERVED_INDICES);

		uint8_t water = (uint8_t) BlockDefinition::GetIndexByName("water");
		uint8_t air = (uint8_t) BlockDefinition::GetIndexByName("air");
		for (int blockIndex = 0; blockIndex < CHUNK_BLOCKS_TOTAL; blockIndex++)
		{
			if (m_blocks[blockIndex].m_type == air) continue;
			if (!m_blocks[blockIndex].IsBlockOpaque() && !m_blocks->m_type == water) continue;
			BlockDefinition const* blockDef = BlockDefinition::GetById(m_blocks[blockIndex].m_type);

			IntVec3 mins = GetWorldCoordsForIndex(blockIndex);
			IntVec3 maxs = mins + IntVec3::ONE;
			AABB3 blockBounds = AABB3(Vec3(mins), Vec3(maxs));
			BlockIterator currentBlockItr(this, blockIndex);
			if (m_isHiddenSurfaceRemovedDisable)
			{
				AddIndexedVertsForBlock(currentBlockItr, m_vertices, m_indices, blockBounds, blockDef);
			}
			else
			{
				bool renderWest = true;
				bool renderEast = true;
				bool renderNorth = true;
				bool renderSouth = true;
				bool renderTop = true;
				bool renderBottom = true;
				BlockIterator westBlockItr = currentBlockItr.GetWestNeighbor();
				BlockIterator eastBlockItr = currentBlockItr.GetEastNeighbor();
				BlockIterator northBlockItr = currentBlockItr.GetNorthNeighbor();
				BlockIterator southBlockItr = currentBlockItr.GetSouthNeighbor();
				BlockIterator topBlockItr = currentBlockItr.GetTopNeighbor();
				BlockIterator bottomBlockItr = currentBlockItr.GetBottomNeighbor();

				Block* currentBlock = currentBlockItr.GetBlock();

				if (currentBlock->m_type == water)
				{
					Block* westBlock = westBlockItr.GetBlock();
					if (!westBlock || westBlock->IsBlockOpaque() || westBlock->m_type == water)
					{
						renderWest = false;
					}

					Block* eastBlock = eastBlockItr.GetBlock();
					if (!eastBlock || eastBlock->IsBlockOpaque() || eastBlock->m_type == water)
					{
						renderEast = false;
					}

					Block* northBlock = northBlockItr.GetBlock();
					if (!northBlock || northBlock->IsBlockOpaque() || northBlock->m_type == water)
					{
						renderNorth = false;
					}

					Block* southBlock = southBlockItr.GetBlock();
					if (!southBlock || southBlock->IsBlockOpaque() || southBlock->m_type == water)
					{
						renderSouth = false;
					}
					renderBottom = false;
					AddIndexedVertsForBlock(currentBlockItr, m_waterVertices, m_waterIndices, blockBounds, blockDef, renderWest, renderEast, renderNorth, renderSouth, renderTop, renderBottom);
				}
				else
				{
					Block* westBlock = westBlockItr.GetBlock();
					if (!westBlock || westBlock->IsBlockOpaque())
					{
						renderWest = false;
					}

					Block* eastBlock = eastBlockItr.GetBlock();
					if (!eastBlock || eastBlock->IsBlockOpaque())
					{
						renderEast = false;
					}

					Block* northBlock = northBlockItr.GetBlock();
					if (!northBlock || northBlock->IsBlockOpaque())
					{
						renderNorth = false;
					}

					Block* southBlock = southBlockItr.GetBlock();
					if (!southBlock || southBlock->IsBlockOpaque())
					{
						renderSouth = false;
					}

					Block* topBlock = topBlockItr.GetBlock();
					if (!topBlock || topBlock->IsBlockOpaque())
					{
						renderTop = false;
					}

					Block* bottomBlock = bottomBlockItr.GetBlock();
					if (!bottomBlock || bottomBlock->IsBlockOpaque())
					{
						renderBottom = false;
					}
					AddIndexedVertsForBlock(currentBlockItr, m_vertices, m_indices, blockBounds, blockDef, renderWest, renderEast, renderNorth, renderSouth, renderTop, renderBottom);
				}
			}
		}

		g_theRenderer->CopyCPUToGPU(m_vertices.data(), sizeof(Vertex_PCU)* m_vertices.size(), m_vertexBuffer);
		g_theRenderer->CopyCPUToGPU(m_indices.data(), sizeof(unsigned int)* m_indices.size(), m_indexBuffer);

		g_theRenderer->CopyCPUToGPU(m_waterVertices.data(), sizeof(Vertex_PCU)* m_waterVertices.size(), m_waterVertexBuffer);
		g_theRenderer->CopyCPUToGPU(m_waterIndices.data(), sizeof(unsigned int)* m_waterIndices.size(), m_waterIndexBuffer);

		m_isBufferDirty = false;

		double end = GetCurrentTimeSeconds();
		double duration = end - start;
		if (duration > m_world->m_rebuildBufferWorse) m_world->m_rebuildBufferWorse = duration;
		m_world->m_rebuildBufferFrametimes += duration;
		return true;
	}
	
	return false;
}


void Chunk::AddIndexedVertsForBlock(BlockIterator const& blockItr, std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, AABB3 const& bounds, BlockDefinition const* blockDef, 
									bool renderWest, bool renderEast, bool renderNorth, bool renderSouth, bool renderTop, bool renderBottom)
{
	Vec3 corners[8];
	bounds.GetCorners(corners);
	Vec3 nearBL = corners[0];
	Vec3 nearBR = corners[1];
	Vec3 nearTR = corners[2];
	Vec3 nearTL = corners[3];
	Vec3 farBL = corners[4];
	Vec3 farBR = corners[5];
	Vec3 farTR = corners[6];
	Vec3 farTL = corners[7];

	bool isWater = blockItr.GetBlock()->m_type == BlockDefinition::GetIndexByName("water");
	unsigned char alpha = isWater ? 75 : 255;
	// x forward, y left, z up
	if (renderWest) 
	{
		BlockIterator westNeighborItr = blockItr.GetWestNeighbor();
		unsigned char outdoorLightChannelValue = 0;
		unsigned char indoorLightChannelValue = 0;
		GetBlockLighting(outdoorLightChannelValue, indoorLightChannelValue, westNeighborItr);
		AddIndexedVertsForQuad3D(verts, indices, farBL, nearBL, nearTL, farTL, Rgba8(outdoorLightChannelValue, indoorLightChannelValue, 0, alpha), blockDef->m_sideUVs); //left quad -x
	}

	if (renderEast) 
	{
		BlockIterator eastNeighborItr = blockItr.GetEastNeighbor();
		unsigned char outdoorLightChannelValue = 0;
		unsigned char indoorLightChannelValue = 0;
		GetBlockLighting(outdoorLightChannelValue, indoorLightChannelValue, eastNeighborItr);
		AddIndexedVertsForQuad3D(verts, indices, nearBR, farBR, farTR, nearTR, Rgba8(outdoorLightChannelValue, indoorLightChannelValue, 0, alpha), blockDef->m_sideUVs); //right quad +x
	}

	if (renderNorth) 
	{
		BlockIterator northNeighborItr = blockItr.GetNorthNeighbor();
		unsigned char outdoorLightChannelValue = 0;
		unsigned char indoorLightChannelValue = 0;
		GetBlockLighting(outdoorLightChannelValue, indoorLightChannelValue, northNeighborItr);
		AddIndexedVertsForQuad3D(verts, indices, farBR, farBL, farTL, farTR, Rgba8(outdoorLightChannelValue, indoorLightChannelValue, 0, alpha), blockDef->m_sideUVs); //back quad +y
	}

	if (renderSouth)
	{
		BlockIterator southNeighborItr = blockItr.GetSouthNeighbor();
		unsigned char outdoorLightChannelValue = 0;
		unsigned char indoorLightChannelValue = 0;
		GetBlockLighting(outdoorLightChannelValue, indoorLightChannelValue, southNeighborItr);
		AddIndexedVertsForQuad3D(verts, indices, nearBL, nearBR, nearTR, nearTL, Rgba8(outdoorLightChannelValue, indoorLightChannelValue, 0, alpha), blockDef->m_sideUVs); //front quad -y
	}

	if (renderTop) 
	{
		unsigned char waterChannelValue = 0;
		if (isWater)
		{
			waterChannelValue = 255;
		}
		BlockIterator topNeighborItr = blockItr.GetTopNeighbor();
		unsigned char outdoorLightChannelValue = 0;
		unsigned char indoorLightChannelValue = 0;
		GetBlockLighting(outdoorLightChannelValue, indoorLightChannelValue, topNeighborItr);
		AddIndexedVertsForQuad3D(verts, indices, nearTL, nearTR, farTR, farTL, Rgba8(outdoorLightChannelValue, indoorLightChannelValue, waterChannelValue, alpha), blockDef->m_topUVs); //top quad +z
	}

	if (renderBottom)
	{
		BlockIterator bottomNeighborItr = blockItr.GetBottomNeighbor();
		unsigned char outdoorLightChannelValue = 0;
		unsigned char indoorLightChannelValue = 0;
		GetBlockLighting(outdoorLightChannelValue, indoorLightChannelValue, bottomNeighborItr);
		AddIndexedVertsForQuad3D(verts, indices, farBL, farBR, nearBR, nearBL, Rgba8(outdoorLightChannelValue, indoorLightChannelValue, 0, alpha), blockDef->m_bottomUVs); //bottom quad -z
	}
}


void Chunk::GetBlockLighting(unsigned char& outdoorLight, unsigned char& indoorLight, BlockIterator const& blockIterator)
{
	Block* block = blockIterator.GetBlock();
	int outdoorLightingInfluence = block->GetOutdoorLightInfluence();
	float normalizedOutdoorLighting = static_cast<float>(outdoorLightingInfluence) / static_cast<float>(MAX_LIGHT_STRENGTH);
	outdoorLight = DenormalizeByte(normalizedOutdoorLighting);
	int indoorLightingInfluence = block->GetIndoorLightInfluence();
	float normalizedIndoorLightingInfluence = static_cast<float>(indoorLightingInfluence) / static_cast<float>(MAX_LIGHT_STRENGTH);
	indoorLight = DenormalizeByte(normalizedIndoorLightingInfluence);
}


