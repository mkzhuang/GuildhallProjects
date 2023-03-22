#include "Game/World.hpp"
#include "Game/Chunk.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/BlockIterator.hpp"
#include "Game/ChunkGenerationJob.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"

#include <algorithm>

struct{
	bool operator()(IntVec2 const& a, IntVec2 const& b) const 
	{
		return (abs(a.x) + abs(a.y)) > (abs(b.x) + abs(b.y));
	}
} comparator;

GameRaycastResult3D::GameRaycastResult3D(Vec3 const& rayStart, Vec3 const forwardNormal, float maxLength, bool didImpact, Vec3 const& impactPosition, float impactDistance, Vec3 const& impactSurfaceNormal)
	: RaycastResult3D(didImpact, impactPosition, impactDistance, impactSurfaceNormal, rayStart, forwardNormal, maxLength)
{
}


World::World(Game* game)
	:m_game(game)
{
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BasicSprites_64x64.png");
	if (!g_gameConfigBlackboard.GetValue("disableShader", false))
	{
		m_shader = g_theRenderer->CreateOrGetShader("Data/Shaders/World");
	}
	m_worldSeed = g_gameConfigBlackboard.GetValue("worldSeed", 0);
	m_chunkActivationRange = g_gameConfigBlackboard.GetValue("chunkActivationRange", 0.f);
	m_chunkDeactivationRange = m_chunkActivationRange + static_cast<float>(CHUNK_SIZE_X + CHUNK_SIZE_Y);
	m_maxChunkRadiusX = 1 + static_cast<int>(m_chunkActivationRange) / CHUNK_SIZE_X;
	m_maxChunkRadiusY = 1 + static_cast<int>(m_chunkActivationRange) / CHUNK_SIZE_Y;
	m_maxChunks =  (2 * m_maxChunkRadiusX) * (2 * m_maxChunkRadiusY);

	m_gameCBO = g_theRenderer->CreateConstantBuffer((unsigned int)sizeof(GameConstant));
	if (!g_gameConfigBlackboard.GetValue("disableDistanceFog", false))
	{
		m_gameConstant.FogStartDistance = m_chunkActivationRange * 0.5f;
		m_gameConstant.FogEndDistance = m_chunkActivationRange - 16.f;
		m_hasDistanceFog = true;
	}
	else
	{
		m_gameConstant.FogStartDistance = m_chunkActivationRange * 999.f;
		m_gameConstant.FogEndDistance = m_gameConstant.FogStartDistance + 1.f;
		m_hasDistanceFog = false;
	}

	for (int yOffset = -m_maxChunkRadiusY; yOffset < m_maxChunkRadiusY; yOffset++)
	{
		for (int xOffset = -m_maxChunkRadiusX; xOffset < m_maxChunkRadiusX; xOffset++)
		{
			m_offsets.push_back(IntVec2(xOffset, yOffset));
		}
	}
	std::sort(m_offsets.begin(), m_offsets.end(), comparator);

	m_offsetsReversed = m_offsets;
	std::reverse(m_offsetsReversed.begin(), m_offsetsReversed.end());
}


World::~World()
{
	std::map<IntVec2, Chunk*>::iterator generateItr;
	for (generateItr = m_generationChunks.begin(); generateItr != m_generationChunks.end(); generateItr++)
	{
		Chunk*& chunk = generateItr->second;
		if (chunk)
		{
			delete chunk;
		}
	}
	m_generationChunks.clear();

	std::map<IntVec2, Chunk*>::iterator activateItr;
	for (activateItr = m_activeChunks.begin(); activateItr != m_activeChunks.end(); activateItr++)
	{
		Chunk*& chunk = activateItr->second;
		if (chunk)
		{
			delete chunk;
		}
	}
	m_activeChunks.clear();

	delete m_gameCBO;
	m_gameCBO = nullptr;
}


void World::Startup(Vec3 const& pos, EulerAngles const& orientation)
{
	m_player = new Player(this, pos, orientation, m_game->GetCamera());
}


void World::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed('L'))
	{
		m_hasDistanceFog = !m_hasDistanceFog;
	}

	if (m_hasDistanceFog)
	{
		m_gameConstant.FogStartDistance = m_chunkActivationRange * 0.5f;
		m_gameConstant.FogEndDistance = m_chunkActivationRange - 16.f;
	}
	else
	{
		m_gameConstant.FogStartDistance = m_chunkActivationRange * 999.f;
		m_gameConstant.FogEndDistance = m_gameConstant.FogStartDistance + 1.f;
	}

	UpdateWorld(deltaSeconds);
	RetrieveCompletedJobs();
	if (m_player->m_physicsMode != GamePhysicsMode::NOCLIP) 
	{
		PushEntityOutOfBlocks(m_player);
	}
	m_player->Update(deltaSeconds);
	UpdateDebugInfo(deltaSeconds);
}


void World::Render() const
{
	unsigned char r = DenormalizeByte(m_gameConstant.SkyColor.x);
	unsigned char g = DenormalizeByte(m_gameConstant.SkyColor.y);
	unsigned char b = DenormalizeByte(m_gameConstant.SkyColor.z);
	g_theRenderer->ClearScreen(Rgba8(r, g, b, 255));
	g_theRenderer->BindShader(m_shader);

	std::map<IntVec2, Chunk*>::const_iterator itr;
	g_theRenderer->BindTexture(m_texture);
	for (itr = m_activeChunks.begin(); itr != m_activeChunks.end(); itr++)
	{
		Chunk* chunk = itr->second;
		if (chunk && chunk->m_state == ChunkState::ACTIVE)
		{
			chunk->Render();
		}
	}

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	Vec3 playerPos = m_player->m_camera->GetCameraPosition();
	IntVec2 playerChunk = GetChunkCoordinatesForPosition(playerPos);
	for (int index = 0; index < (int)m_offsets.size(); index++)
	{
		std::map<IntVec2, Chunk*>::const_iterator chunkItr = m_activeChunks.find(playerChunk + m_offsets[index]);
		if (chunkItr == m_activeChunks.end()) continue;
		Chunk* chunk = chunkItr->second;
		if (chunk && chunk->m_state == ChunkState::ACTIVE)
		{
			chunk->RenderWater();
		}
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindShader(nullptr);

	m_player->Render();

	if (!g_isDebugging) return;
	for (itr = m_activeChunks.begin(); itr != m_activeChunks.end(); itr++)
	{
		Chunk* chunk = itr->second;
		if (chunk && chunk->m_state == ChunkState::ACTIVE)
		{
			chunk->RenderDebug();
		}
	}
	std::vector<Vertex_PCU> verts;
	for (int index = 0; index < (int)m_blocksToPushOut.size(); index++)
	{
		Block* block = m_blocksToPushOut[index].GetBlock();
		if (!block) continue;
		AABB3 bounds = m_blocksToPushOut[index].GetBlockBounds();
		AddVertsForWireAABB3D(verts, bounds, 0.01f, Rgba8::RED);
	}
	g_theRenderer->DrawVertexArray(verts);
}


void World::RenderUI(Camera camera) const
{
	m_player->RenderUI(camera);
}


std::string World::GetChunksInfo() const
{
	int numChunks = 0;
	int numVertices = 0;
	std::map<IntVec2, Chunk*>::const_iterator itr;
	for (itr = m_activeChunks.begin(); itr != m_activeChunks.end(); itr++)
	{
		Chunk* chunk = itr->second;
		if (chunk)
		{
			numChunks ++;
			numVertices += chunk->GetVerticesCounts();
		}
	}
	std::string chunksInfo = Stringf("Chunks=%i, Verts=%i", numChunks, numVertices);
	return chunksInfo;
}


int World::GetWorldSeed() const
{
	return m_worldSeed;
}


GameRaycastResult3D World::RaycastVsBlocks(Vec3 const& rayStart, Vec3 const& forwardNormal, float maxLength) const
{
	IntVec2 const& chunkCoords = GetChunkCoordinatesForPosition(rayStart);
	Chunk* chunk = GetChunkForCoordinate(chunkCoords);
	if (!chunk) return GameRaycastResult3D();
	BlockIterator currentBlockIterator(chunk, rayStart);
	Block* currentBlock = currentBlockIterator.GetBlock();

	//check if raycast inside a opaque block
	if (currentBlock && currentBlock->IsBlockOpaque())
	{
		GameRaycastResult3D result(rayStart, forwardNormal, maxLength, true, rayStart, 0.f, -forwardNormal);
		result.m_impactBlock = currentBlockIterator;
		return result;
	}

	IntVec3 currentBlockCoords = currentBlockIterator.m_chunk->GetWorldCoordsForIndex(currentBlockIterator.m_blockIndex);

	// x crossing calculation
	float fwdDistPerXCrossing = 0.f;
	int blockStepDirectionX = 0;
	float xAtFirstXCrossing = 0.f;
	float xDistToFirstXCrossing = 0.f;
	float fwdDistAtNextXCrossing = 0.f;

	if (forwardNormal.x == 0.f)
	{
		fwdDistAtNextXCrossing = maxLength + 1.f;
	}
	else
	{
		fwdDistPerXCrossing = 1.f / abs(forwardNormal.x);
		blockStepDirectionX = forwardNormal.x < 0.f ? -1 : 1;
		xAtFirstXCrossing = static_cast<float>(currentBlockCoords.x + (blockStepDirectionX + 1) / 2);
		xDistToFirstXCrossing = xAtFirstXCrossing - rayStart.x;
		fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;
	}
	
	// y crossing calculation
	float fwdDistPerYCrossing = 0.f;
	int blockStepDirectionY = 0;
	float yAtFirstYCrossing = 0.f;
	float yDistToFirstYCrossing = 0.f;
	float fwdDistAtNextYCrossing = 0.f;
	if (forwardNormal.y == 0.f)
	{
		fwdDistAtNextYCrossing = maxLength + 1.f;
	}
	else
	{
		fwdDistPerYCrossing = 1.f / abs(forwardNormal.y);
		blockStepDirectionY = forwardNormal.y < 0.f ? -1 : 1;
		yAtFirstYCrossing = static_cast<float>(currentBlockCoords.y + (blockStepDirectionY + 1) / 2);
		yDistToFirstYCrossing = yAtFirstYCrossing - rayStart.y;
		fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;
	}

	// z crossing calculation
	float fwdDistPerZCrossing = 0.f;
	int blockStepDirectionZ = 0;
	float zAtFirstZCrossing = 0.f;
	float zDistToFirstZCrossing = 0.f;
	float fwdDistAtNextZCrossing = 0.f;
	if (forwardNormal.z == 0.f)
	{
		fwdDistAtNextZCrossing = maxLength + 1.f;
	}
	else
	{
		fwdDistPerZCrossing = 1.f / abs(forwardNormal.z);
		blockStepDirectionZ = forwardNormal.z < 0.f ? -1 : 1;
		zAtFirstZCrossing = static_cast<float>(currentBlockCoords.z + (blockStepDirectionZ + 1) / 2);
		zDistToFirstZCrossing = zAtFirstZCrossing - rayStart.z;
		fwdDistAtNextZCrossing = fabsf(zDistToFirstZCrossing) * fwdDistPerZCrossing;
	}

	for (; ;)
	{
		if (fwdDistAtNextXCrossing >= maxLength && fwdDistAtNextYCrossing >= maxLength && fwdDistAtNextZCrossing >= maxLength)
		{
			Vec3 endPosition = rayStart + maxLength * forwardNormal;
			return GameRaycastResult3D(rayStart, forwardNormal, maxLength, false, endPosition, maxLength, forwardNormal);
		}

		if (fwdDistAtNextXCrossing <= fwdDistAtNextYCrossing && fwdDistAtNextXCrossing <= fwdDistAtNextZCrossing)  // x crossing
		{
			if (blockStepDirectionX > 0)
			{
				currentBlockIterator = currentBlockIterator.GetEastNeighbor();
			}
			else
			{
				currentBlockIterator = currentBlockIterator.GetWestNeighbor();
			}

			currentBlock = currentBlockIterator.GetBlock();
			if (currentBlock && currentBlock->IsBlockOpaque())
			{
				Vec3 impactPosition = rayStart + fwdDistAtNextXCrossing * forwardNormal;
				GameRaycastResult3D result(rayStart, forwardNormal, maxLength, true, impactPosition, fwdDistAtNextXCrossing, Vec3(static_cast<float>(-blockStepDirectionX), 0.f, 0.f));
				result.m_impactBlock = currentBlockIterator;
				return result;
			}
			else
			{
				fwdDistAtNextXCrossing += fwdDistPerXCrossing;
			}
		}
		else if (fwdDistAtNextYCrossing <= fwdDistAtNextXCrossing && fwdDistAtNextYCrossing <= fwdDistAtNextZCrossing) // y crossing
		{
			if (blockStepDirectionY > 0)
			{
				currentBlockIterator = currentBlockIterator.GetNorthNeighbor();
			}
			else
			{
				currentBlockIterator = currentBlockIterator.GetSouthNeighbor();
			}

			currentBlock = currentBlockIterator.GetBlock();
			if (currentBlock && currentBlock->IsBlockOpaque())
			{
				Vec3 impactPosition = rayStart + fwdDistAtNextYCrossing * forwardNormal;
				GameRaycastResult3D result(rayStart, forwardNormal, maxLength, true, impactPosition, fwdDistAtNextYCrossing, Vec3(0.f, static_cast<float>(-blockStepDirectionY), 0.f));
				result.m_impactBlock = currentBlockIterator;
				return result;
			}
			else
			{
				fwdDistAtNextYCrossing += fwdDistPerYCrossing;
			}
		}
		else // z crossing
		{
			if (blockStepDirectionZ > 0)
			{
				currentBlockIterator = currentBlockIterator.GetTopNeighbor();
			}
			else
			{
				currentBlockIterator = currentBlockIterator.GetBottomNeighbor();
			}

			currentBlock = currentBlockIterator.GetBlock();
			if (currentBlock && currentBlock->IsBlockOpaque())
			{
				Vec3 impactPosition = rayStart + fwdDistAtNextZCrossing * forwardNormal;
				GameRaycastResult3D result(rayStart, forwardNormal, maxLength, true, impactPosition, fwdDistAtNextZCrossing, Vec3(0.f, 0.f, static_cast<float>(-blockStepDirectionZ)));
				result.m_impactBlock = currentBlockIterator;
				return result;
			}
			else
			{
				fwdDistAtNextZCrossing += fwdDistPerZCrossing;
			}
		}
	}
}


void World::UpdateWorld(float deltaSeconds)
{
	if (!g_gameConfigBlackboard.GetValue("disableShader", false))
	{
		SetGameConstant();
		BindGameConstantBuffer();
	}

	m_isWorldTimeSpeedUp = g_theInput->IsKeyDown('Y') ? true : false;

	if (m_isWorldTimeSpeedUp)
	{
		m_worldDay += (deltaSeconds * REAL_TIME_RATIO * TIME_ACCELERATE_SCALE) * DAYS_PER_SECOND;
	}
	else
	{
		m_worldDay += (deltaSeconds * REAL_TIME_RATIO) * DAYS_PER_SECOND;
	}

	bool activateThisFrame = false;
	if ((int)m_activeChunks.size() < m_maxChunks)
	{
		activateThisFrame = ActivateNearestChunk();
	}

	if (!activateThisFrame)
	{
		DeactivateFurthestChunk();
	}

	// profiling start
	if (activateThisFrame)
	{
		m_activatingFrames++;
		if (m_activatingFrames == 0) return;
		m_activatingFrametimes += deltaSeconds;
	}
	else
	{
		m_stableFrames++;
		m_stableFrametimes += deltaSeconds;
	}
	// profiling end

	m_resolveLightingCounts++;
	double start = GetCurrentTimeSeconds();

	ProcessDirtyLighting();

	double end = GetCurrentTimeSeconds();
	double duration = end - start;
	if (duration > m_resolveLightingWorse) m_resolveLightingWorse = duration;
	m_resolveLightingFrametimes += duration;

	std::map<IntVec2, Chunk*>::iterator itr;
	for (itr = m_activeChunks.begin(); itr != m_activeChunks.end(); itr++)
	{
		Chunk*& chunk = itr->second;
		if (chunk && chunk->m_state == ChunkState::ACTIVE)
		{
			chunk->Update(deltaSeconds);
		}
	}

	Vec3 playerPos = m_player->m_camera->GetCameraPosition();
	IntVec2 playerChunk = GetChunkCoordinatesForPosition(playerPos);
	int refreshCount = NUM_BUILD_CHUNK_PER_FRAME;
	for (int index = 0; index < (int)m_offsetsReversed.size(); index++)
	{
		std::map<IntVec2, Chunk*>::const_iterator chunkItr = m_activeChunks.find(playerChunk + m_offsetsReversed[index]);
		if (chunkItr == m_activeChunks.end()) continue;
		Chunk* chunk = chunkItr->second;
		if (chunk && chunk->m_state == ChunkState::ACTIVE)
		{
			if (chunk->RefreshVertexBuffer())
			{
				refreshCount--;
			}

			if (refreshCount == 0)
			{
				break;
			}
		}
	}
}


bool World::ActivateNearestChunk()
{
	Vec3 playerPos = m_player->m_camera->GetCameraPosition();
	IntVec2 playerChunk = GetChunkCoordinatesForPosition(playerPos);
	int minChunkX = playerChunk.x - m_maxChunkRadiusX;
	int maxChunkX = playerChunk.x + m_maxChunkRadiusX;
	int minChunkY = playerChunk.y - m_maxChunkRadiusY;
	int maxChunkY = playerChunk.y + m_maxChunkRadiusY;
	float nearnestDistanceSquared = 999'999.f;
	IntVec2 nearestChunk = IntVec2::ZERO;
	for (int chunkY = minChunkY; chunkY < maxChunkY; chunkY++)
	{
		float chunkCenterY = static_cast<float>(chunkY << CHUNK_BITS_Y) + 0.5f * static_cast<float>(CHUNK_SIZE_Y);
		for (int chunkX = minChunkX; chunkX < maxChunkX; chunkX++)
		{
			std::map<IntVec2, Chunk*>::const_iterator genItr = m_generationChunks.find(IntVec2(chunkX, chunkY));
			std::map<IntVec2, Chunk*>::const_iterator activeItr = m_activeChunks.find(IntVec2(chunkX, chunkY));
			if (genItr != m_generationChunks.end() || activeItr != m_activeChunks.end()) continue;
			float chunkCenterX = static_cast<float>(chunkX << CHUNK_BITS_X) + 0.5f * static_cast<float>(CHUNK_SIZE_X);
			float distanceSquared = GetDistanceSquared2D(Vec2(playerPos.x, playerPos.y), Vec2(chunkCenterX, chunkCenterY));
			if (distanceSquared <= (m_chunkActivationRange * m_chunkActivationRange) && distanceSquared < nearnestDistanceSquared)
			{
				nearnestDistanceSquared = distanceSquared;
				nearestChunk = IntVec2(chunkX, chunkY);
			}
		}
	}

	if (nearnestDistanceSquared != 999'999.f)
	{
		ActivateChunk(nearestChunk);
		return true;
	}

	return false;
}


bool World::DeactivateFurthestChunk()
{
	Vec3 playerPos = m_player->m_camera->GetCameraPosition();
	IntVec2 playerChunk = GetChunkCoordinatesForPosition(playerPos);
	float furthestDistanceSquared = 0.f;
	IntVec2 furthestChunk = IntVec2::ZERO;
	std::map<IntVec2, Chunk*>::const_iterator itr;
	for (itr = m_activeChunks.begin(); itr != m_activeChunks.end(); itr++)
	{
		Chunk* chunk = itr->second;
		if (chunk)
		{
			IntVec2 const& chunkCoords = itr->first;
			float chunkCenterY = static_cast<float>(chunkCoords.y << CHUNK_BITS_Y) + 0.5f * static_cast<float>(CHUNK_SIZE_Y);
			float chunkCenterX = static_cast<float>(chunkCoords.x << CHUNK_BITS_X) + 0.5f * static_cast<float>(CHUNK_SIZE_X);
			float distanceSquared = GetDistanceSquared2D(Vec2(playerPos.x, playerPos.y), Vec2(chunkCenterX, chunkCenterY));
			if (distanceSquared >= (m_chunkDeactivationRange * m_chunkDeactivationRange) && distanceSquared > furthestDistanceSquared)
			{
				furthestDistanceSquared = distanceSquared;
				furthestChunk = chunkCoords;
			}
		}
	}

	if (furthestDistanceSquared != 0.f)
	{
		DeactivateChunk(furthestChunk);
		return true;
	}

	return false;
}


void World::MarkLightingDirty(BlockIterator const& blockItr)
{
	if (blockItr.GetBlock()->IsLightDirty()) return;
	blockItr.GetBlock()->SetLightDirty();
	m_dirtyLightingQueue.push_back(blockItr);
}


void World::SetGameConstant()
{
	m_gameConstant.CameraWorldPosition = Vec4(m_player->m_camera->GetCameraPosition());
	float dayFraction = fmodf(m_worldDay, 1.f);
	float colorValue;
	if (dayFraction < 0.5f)
	{
		colorValue = RangeMapClamped(dayFraction, 0.25f, 0.5f, 0.f, 1.f);
	}
	else
	{
		colorValue = RangeMapClamped(dayFraction, 0.5f, 0.75f, 1.f, 0.f);
	}
	float outdoorLightingFactor = Compute1dPerlinNoise(m_worldDay, .008f, 6);
	outdoorLightingFactor = RangeMapClamped(outdoorLightingFactor, 0.6f, 0.9f, 0.f, 1.f);
	Rgba8 skyColor = InterpolateBetweenColor(Rgba8(20, 20, 40, 255), Rgba8(200, 230, 255, 255), colorValue);
	skyColor = InterpolateBetweenColor(skyColor, Rgba8::WHITE, outdoorLightingFactor);
	float skyColorFloat[4] = {};
	skyColor.GetAsFloats(skyColorFloat);
	m_gameConstant.SkyColor = Vec4(skyColorFloat[0], skyColorFloat[1], skyColorFloat[2], skyColorFloat[3]);

	Rgba8 outdoorColor = InterpolateBetweenColor(Rgba8(40, 40, 60, 255), Rgba8::WHITE, colorValue);
	outdoorColor = InterpolateBetweenColor(outdoorColor, Rgba8::WHITE, outdoorLightingFactor);
	float outdoorColorFloat[4] = {};
	outdoorColor.GetAsFloats(outdoorColorFloat);
	m_gameConstant.OutdoorLightColor = Vec4(outdoorColorFloat[0], outdoorColorFloat[1], outdoorColorFloat[2], outdoorColorFloat[3]);

	float indoorLightStrength = Compute1dPerlinNoise(m_worldDay, .01f, 5);
	indoorLightStrength = RangeMapClamped(indoorLightStrength, -1.f, 1.f, 0.8f, 1.f);
	m_gameConstant.IndoorLightColor = Vec4(1.f, 0.9f, 0.8f, 1.f) * indoorLightStrength;

	m_gameConstant.CurrentTime = m_worldDay;
}


void World::BindGameConstantBuffer()
{
	g_theRenderer->CopyCPUToGPU(&m_gameConstant, sizeof(m_gameConstant), m_gameCBO);
	g_theRenderer->BindConstantBuffer(GAME_CONSTANT_BUFFER_SLOT, m_gameCBO);
}


Vec3 const& World::GetPlayerPos() const
{
	return m_player->m_position;
}


EulerAngles const& World::GetPlayerOrientation() const
{
	return m_player->m_orientationDegree;
}


void World::ActivateChunk(IntVec2 const& chunkCoords)
{
	Chunk* newChunk = new Chunk(this, chunkCoords);
	m_generationChunks[chunkCoords] = newChunk;

	std::string fileName = Stringf("Saves/World_%i/Chunk(%i,%i).chunk", m_worldSeed, chunkCoords.x, chunkCoords.y);
	if (FileExists(fileName))
	{
		m_diskLoadCounts++;
		double start = GetCurrentTimeSeconds();

		newChunk->PopulateBlocksFromDisk();
		AddActiveChunkToWorld(newChunk);

		double end = GetCurrentTimeSeconds();
		double duration = end - start;
		if (duration > m_diskLoadWorse) m_diskLoadWorse = duration;
		m_diskLoadFrametimes += duration;
	}
	else
	{
		//m_perlinGenerationCounts++;
		//double start = GetCurrentTimeSeconds();

		//newChunk->GenerateBlocks();

		//double end = GetCurrentTimeSeconds();
		//double duration = end - start;
		//if (duration > m_perlinGenerationWorse) m_perlinGenerationWorse = duration;
		//m_perlinGenerationFrametimes += duration;

		ChunkGenerationJob* job = new ChunkGenerationJob(newChunk);
		g_theJobSystem->QueueJob(job);
	}
}


void World::DeactivateChunk(IntVec2 const& chunkCoords)
{
	Chunk*& chunk = m_activeChunks.find(chunkCoords)->second;
	if (chunk->m_westChunk)
	{
		chunk->m_westChunk->m_eastChunk = nullptr;
	}
	if (chunk->m_eastChunk)
	{
		chunk->m_eastChunk->m_westChunk = nullptr;
	}
	if (chunk->m_northChunk)
	{
		chunk->m_northChunk->m_southChunk = nullptr;
	}
	if (chunk->m_southChunk)
	{
		chunk->m_southChunk->m_northChunk = nullptr;
	}

	UndirtyAllBlocksInChunk(chunk);
	delete chunk;
	m_activeChunks.erase(chunkCoords);
}


void World::RetrieveCompletedJobs()
{
	Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
	while (completedJob)
	{
		if (ChunkGenerationJob* job = dynamic_cast<ChunkGenerationJob*>(completedJob))
		{
			Chunk* chunk = job->m_chunk;
			AddActiveChunkToWorld(chunk);
		}

		delete completedJob;
		completedJob = g_theJobSystem->RetrieveCompletedJob();
	}
}


void World::AddActiveChunkToWorld(Chunk* chunk)
{
	IntVec2 chunkCoords = chunk->m_coordinates;
	m_generationChunks.erase(chunkCoords);

	chunk->m_westChunk = GetChunkForCoordinate(chunkCoords + IntVec2::STEP_WEST);
	chunk->m_eastChunk = GetChunkForCoordinate(chunkCoords + IntVec2::STEP_EAST);
	chunk->m_northChunk = GetChunkForCoordinate(chunkCoords + IntVec2::STEP_NORTH);
	chunk->m_southChunk = GetChunkForCoordinate(chunkCoords + IntVec2::STEP_SOUTH);

	if (chunk->m_westChunk)
	{
		chunk->m_westChunk->m_eastChunk = chunk;
	}
	if (chunk->m_eastChunk)
	{
		chunk->m_eastChunk->m_westChunk = chunk;
	}
	if (chunk->m_northChunk)
	{
		chunk->m_northChunk->m_southChunk = chunk;
	}
	if (chunk->m_southChunk)
	{
		chunk->m_southChunk->m_northChunk = chunk;
	}

	chunk->InitializeLighting();
	chunk->AddDebugDrawing();
	chunk->m_state = ChunkState::ACTIVE;

	m_activeChunks[chunkCoords] = chunk;
}


IntVec2 World::GetChunkCoordinatesForPosition(Vec3 const& pos) const
{
	int x = RoundDownToInt(pos.x);
	int y = RoundDownToInt(pos.y);
	int chunkCoordsX = x >> CHUNK_BITS_X;
	int chunkCoordsY = y >> CHUNK_BITS_Y;
	return IntVec2(chunkCoordsX, chunkCoordsY);
}


Chunk* World::GetChunkForCoordinate(IntVec2 const& coordinate) const
{
	std::map<IntVec2, Chunk*>::const_iterator itr = m_activeChunks.find(coordinate);
	if (itr != m_activeChunks.end()) return itr->second;

	return nullptr;
}


BlockIterator World::GetBlockIteratorForPosition(Vec3 const& pos) const
{
	IntVec2 chunkCoords = GetChunkCoordinatesForPosition(pos);
	Chunk* chunk = GetChunkForCoordinate(chunkCoords);
	return BlockIterator(chunk, pos);
}


void World::ProcessDirtyLighting()
{
	while (!m_dirtyLightingQueue.empty())
	{
		BlockIterator blockItr = m_dirtyLightingQueue.front();
		m_dirtyLightingQueue.pop_front();
		ProcessNextDirtyLightBlock(blockItr);
	}
}


void World::ProcessNextDirtyLightBlock(BlockIterator const& blockItr)
{
	Block* currentBlock = blockItr.GetBlock();
	currentBlock->SetLightNotDirty();
	blockItr.m_chunk->SetBufferDirty();

	int maxIndoorNeighboringLight = 0;
	int maxOutdoorNeighboringLight = 0;
	bool isValueIncorrect = false;

	BlockIterator eastNeighbor = blockItr.GetEastNeighbor();
	Block* eastNeighborBlock = eastNeighbor.GetBlock();

	BlockIterator westNeighbor = blockItr.GetWestNeighbor();
	Block* westNeighborBlock = westNeighbor.GetBlock();

	BlockIterator northNeighbor = blockItr.GetNorthNeighbor();
	Block* northNeighborBlock = northNeighbor.GetBlock();

	BlockIterator southNeighbor = blockItr.GetSouthNeighbor();
	Block* southNeighborBlock = southNeighbor.GetBlock();

	BlockIterator topNeighbor = blockItr.GetTopNeighbor();
	Block* topNeighborBlock = topNeighbor.GetBlock();

	BlockIterator bottomNeighbor = blockItr.GetBottomNeighbor();
	Block* bottomNeighborBlock = bottomNeighbor.GetBlock();

	if (currentBlock->IsBlockOpaque()) // check if it is light source block
	{
		int blockIndoorLight = BlockDefinition::GetById(currentBlock->m_type)->m_light;
		if (currentBlock->GetIndoorLightInfluence() != blockIndoorLight || currentBlock->GetOutdoorLightInfluence() != 0)
		{
			currentBlock->SetIndoorLightInfluence(blockIndoorLight);
			maxIndoorNeighboringLight = blockIndoorLight;
			currentBlock->SetOutdoorLightInfluence(0);
			maxOutdoorNeighboringLight = 0;

			while (bottomNeighborBlock && bottomNeighborBlock->IsBlockSky()) // bottom
			{
				bottomNeighborBlock->SetBlockNotSky();
				MarkLightingDirty(bottomNeighbor);
				bottomNeighbor = bottomNeighbor.GetBottomNeighbor();
				bottomNeighborBlock = bottomNeighbor.GetBlock();
			}
			isValueIncorrect = true;
		}
	}
	else
	{
		if (currentBlock->IsBlockSky()) // check if it is sky
		{
			if (currentBlock->GetOutdoorLightInfluence() != MAX_LIGHT_STRENGTH)
			{
				currentBlock->SetOutdoorLightInfluence(MAX_LIGHT_STRENGTH);
				maxOutdoorNeighboringLight = MAX_LIGHT_STRENGTH;

				while (bottomNeighborBlock && !bottomNeighborBlock->IsBlockOpaque())
				{
					bottomNeighborBlock->SetBlockSky();
					MarkLightingDirty(bottomNeighbor);
					bottomNeighbor = bottomNeighbor.GetBottomNeighbor();
					bottomNeighborBlock = bottomNeighbor.GetBlock();
				}
				isValueIncorrect = true;
			}
		}

		if (eastNeighborBlock) // check against east block
		{
			GetMaxLight(maxIndoorNeighboringLight, maxOutdoorNeighboringLight, eastNeighborBlock);
		}

		if (westNeighborBlock) // check against west block
		{
			GetMaxLight(maxIndoorNeighboringLight, maxOutdoorNeighboringLight, westNeighborBlock);
		}

		if (northNeighborBlock) // check against north block
		{
			GetMaxLight(maxIndoorNeighboringLight, maxOutdoorNeighboringLight, northNeighborBlock);
		}

		if (southNeighborBlock) // check against south block
		{
			GetMaxLight(maxIndoorNeighboringLight, maxOutdoorNeighboringLight, southNeighborBlock);
		}

		if (topNeighborBlock) // check against top block
		{
			GetMaxLight(maxIndoorNeighboringLight, maxOutdoorNeighboringLight, topNeighborBlock);
		}

		if (bottomNeighborBlock) // check against bottom block
		{
			GetMaxLight(maxIndoorNeighboringLight, maxOutdoorNeighboringLight, bottomNeighborBlock);
		}

		int maxIndoorLight = Clamp(maxIndoorNeighboringLight - 1, 0, 15);
		if (currentBlock->GetIndoorLightInfluence() != maxIndoorLight) // adjust indoor lighting
		{
			currentBlock->SetIndoorLightInfluence(maxIndoorLight);
			isValueIncorrect = true;
		}

		int maxOutdoorLight = Clamp(maxOutdoorNeighboringLight - 1, 0, 15);
		if (currentBlock->GetOutdoorLightInfluence() != maxOutdoorLight && !currentBlock->IsBlockSky()) // adjust outdoor lighting
		{
			currentBlock->SetOutdoorLightInfluence(maxOutdoorLight);
			isValueIncorrect = true;
		}
	}

	if (isValueIncorrect) // mark neighbor lighting dirty
	{
		blockItr.m_chunk->SetBufferDirty();
		if (eastNeighborBlock)
		{
			eastNeighbor.m_chunk->SetBufferDirty();
			if (!eastNeighborBlock->IsBlockOpaque()) MarkLightingDirty(eastNeighbor);
		}
		if (westNeighborBlock)
		{
			westNeighbor.m_chunk->SetBufferDirty();
			if (!westNeighborBlock->IsBlockOpaque()) MarkLightingDirty(westNeighbor);
		}
		if (northNeighborBlock)
		{
			northNeighbor.m_chunk->SetBufferDirty();
			if (!northNeighborBlock->IsBlockOpaque()) MarkLightingDirty(northNeighbor);
		}
		if (southNeighborBlock)
		{
			southNeighbor.m_chunk->SetBufferDirty();
			if (!southNeighborBlock->IsBlockOpaque()) MarkLightingDirty(southNeighbor);
		}
		if (topNeighborBlock)
		{
			topNeighbor.m_chunk->SetBufferDirty();
			if (!topNeighborBlock->IsBlockOpaque()) MarkLightingDirty(topNeighbor);
		}
		if (bottomNeighborBlock)
		{
			bottomNeighbor.m_chunk->SetBufferDirty();
			if (!bottomNeighborBlock->IsBlockOpaque()) MarkLightingDirty(bottomNeighbor);
		}
	}
}


void World::GetMaxLight(int& maxIndoorNeighborLight, int& maxOutdoorNeighborLight, Block const* neighborBlock)
{
	int blockIndoorLighting = neighborBlock->GetIndoorLightInfluence();
	if (blockIndoorLighting > maxIndoorNeighborLight)
	{
		maxIndoorNeighborLight = blockIndoorLighting;
	}

	int blockOutDoorLighting = neighborBlock->GetOutdoorLightInfluence();
	if (blockOutDoorLighting > maxOutdoorNeighborLight)
	{
		maxOutdoorNeighborLight = blockOutDoorLighting;
	}
}


void World::UndirtyAllBlocksInChunk(Chunk* chunk)
{
	std::deque<BlockIterator>::iterator itr;
	for (itr = m_dirtyLightingQueue.begin(); itr != m_dirtyLightingQueue.end(); itr++)
	{
		if (itr->m_chunk == chunk)
		{
			m_dirtyLightingQueue.erase(itr);
			itr--;
		}
	}
}


void World::UpdateDebugInfo(float deltaSeconds)
{
	std::string controlGuide = Stringf("WASD=Horizontal, QE=Vertical, Shift=Speed, F8=Regenerate, F1=Debug, H=Origin, Y=Time, Mid Mouse=Inventory Function");
	EventArgs guideArgs;
	guideArgs.SetValue("text", controlGuide);
	guideArgs.SetValue("duration", "0.0");
	guideArgs.SetValue("color", "255, 255, 0");
	FireEvent("debugSpawnScreenMessage", guideArgs);

	Vec3 position = m_player->m_position;
	EulerAngles orientation = m_player->m_orientationDegree;
	float microSeconds = deltaSeconds * 1000.f;
	std::string chunksInfo = GetChunksInfo();
	std::string cameraPos = Stringf("XYZ=(%.1f, %.1f, %.1f)", position.x, position.y, position.z);
	std::string cameraOrientation = Stringf("YPR=(%.0f, %.0f, %.0f)", orientation.m_yawDegrees, orientation.m_pitchDegrees, orientation.m_rollDegrees);
	std::string dayInfo = Stringf("Current Day=%.2f", m_worldDay);
	std::string framesInfo = Stringf("Frames=%.2fms(%.0fFPS)", microSeconds, 1.f / deltaSeconds);
	std::string gameStatus = chunksInfo + ", " + cameraPos + ", " + cameraOrientation + ", " + dayInfo + ", " + framesInfo;
	EventArgs debugArgs;
	debugArgs.SetValue("text", gameStatus);
	debugArgs.SetValue("duration", "0.0");
	debugArgs.SetValue("color", "100, 255, 100");
	FireEvent("debugSpawnScreenMessage", debugArgs);
	
	std::string cameraModeInfo = "";
	switch (m_player->m_cameraMode)
	{
		case GameCameraMode::FIRST_PERSON:			cameraModeInfo = "First Person";			break;
		case GameCameraMode::FIXED_ANGLE_TRACKING:	cameraModeInfo = "Fixed Angle Tracking";	break;
		case GameCameraMode::OVER_SHOULDER:			cameraModeInfo = "Over Shoulder";			break;
		case GameCameraMode::SPECTATOR:				cameraModeInfo = "Spectator";				break;
		case GameCameraMode::INDEPENDENT:			cameraModeInfo = "Independent";				break;
	}

	std::string physicsModeInfo = "";
	switch (m_player->m_physicsMode)
	{
		case GamePhysicsMode::WALKING:	physicsModeInfo = "Walking";	break;
		case GamePhysicsMode::FLYING:	physicsModeInfo = "Flying";		break;
		case GamePhysicsMode::NOCLIP:	physicsModeInfo	= "No Clip";		break;
	}

	std::string velocity = Stringf("%.2f, %.2f, %.2f", m_player->m_velocity.x, m_player->m_velocity.y, m_player->m_velocity.z);
	std::string modeInfo = "F2=" + cameraModeInfo + ", F3=" + physicsModeInfo + " || " + velocity;
	EventArgs modeArgs;
	modeArgs.SetValue("text", modeInfo);
	modeArgs.SetValue("duration", "0.0");
	modeArgs.SetValue("color", "255, 100, 100");
	FireEvent("debugSpawnScreenMessage", modeArgs);

	if (!g_isDebugging || g_gameConfigBlackboard.GetValue("disableProfiling", true)) return;
	// fps profiling
	float activatingFPS = static_cast<float>(m_activatingFrames) / m_activatingFrametimes;
	float stableFPS = static_cast<float>(m_stableFrames) / m_stableFrametimes;
	std::string fpsInfo = Stringf("Activating Chunk FPS=%.f, Stable FPS=%.f", activatingFPS, stableFPS);
	EventArgs fpsArgs;
	fpsArgs.SetValue("text", fpsInfo);
	fpsArgs.SetValue("duration", "0.0");
	fpsArgs.SetValue("color", "100, 255, 255");
	FireEvent("debugSpawnScreenMessage", fpsArgs);

	// Perlin generation profiling
	//double perlinGenAverage = m_perlinGenerationFrametimes / static_cast<double>(m_perlinGenerationCounts) * 1000.0;
	//std::string perlinGenInfo = Stringf("Perlin Generation - worst=%.2fms, average %.2fms", m_perlinGenerationWorse * 1000.0, perlinGenAverage);
	//EventArgs perlinGenArgs;
	//perlinGenArgs.SetValue("text", perlinGenInfo);
	//perlinGenArgs.SetValue("duration", "0.0");
	//perlinGenArgs.SetValue("color", "100, 255, 255");
	//FireEvent("debugSpawnScreenMessage", perlinGenArgs);

	// disk load profiling
	double diskLoadAverage = m_diskLoadFrametimes / static_cast<double>(m_diskLoadCounts) * 1000.0;
	std::string diskLoadInfo = Stringf("Disk Load         - worst=%.2fms, average %.2fms", m_diskLoadWorse * 1000.0, diskLoadAverage);
	EventArgs diskLoadArgs;
	diskLoadArgs.SetValue("text", diskLoadInfo);
	diskLoadArgs.SetValue("duration", "0.0");
	diskLoadArgs.SetValue("color", "100, 255, 255");
	FireEvent("debugSpawnScreenMessage", diskLoadArgs);

	// disk save profiling
	double diskSaveAverage = m_diskSaveFrametimes / static_cast<double>(m_diskSaveCounts) * 1000.0;
	std::string diskSaveInfo = Stringf("Disk Save         - worst=%.2fms, average %.2fms", m_diskSaveWorse * 1000.0, diskSaveAverage);
	EventArgs diskSaveArgs;
	diskSaveArgs.SetValue("text", diskSaveInfo);
	diskSaveArgs.SetValue("duration", "0.0");
	diskSaveArgs.SetValue("color", "100, 255, 255");
	FireEvent("debugSpawnScreenMessage", diskSaveArgs);

	// chunk rebuild profiling
	double rebuildBufferAverage = m_rebuildBufferFrametimes / static_cast<double>(m_rebuildBufferCounts) * 1000.0;
	std::string rebuildBufferInfo = Stringf("Chunk Rebuild     - worst=%.2fms, average %.2fms", m_rebuildBufferWorse * 1000.0, rebuildBufferAverage);
	EventArgs rebuildBufferArgs;
	rebuildBufferArgs.SetValue("text", rebuildBufferInfo);
	rebuildBufferArgs.SetValue("duration", "0.0");
	rebuildBufferArgs.SetValue("color", "100, 255, 255");
	FireEvent("debugSpawnScreenMessage", rebuildBufferArgs);

	// light resolve profiling
	double resolveLightingAverage = m_resolveLightingFrametimes / static_cast<double>(m_resolveLightingCounts) * 1000.0;
	std::string resolveLightingInfo = Stringf("Resolve Lighting  - worst=%.2fms, average %.2fms", m_resolveLightingWorse * 1000.0, resolveLightingAverage);
	EventArgs resolveLightingArgs;
	resolveLightingArgs.SetValue("text", resolveLightingInfo);
	resolveLightingArgs.SetValue("duration", "0.0");
	resolveLightingArgs.SetValue("color", "100, 255, 255");
	FireEvent("debugSpawnScreenMessage", resolveLightingArgs);
}


void World::PushEntityOutOfBlocks(Entity* entity)
{
	Vec3 pos = entity->m_position;
	BlockIterator currentBlockItr = GetBlockIteratorForPosition(pos);
	float xDecimalPlace = fmodf(pos.x, 1.f);
	float yDecimalPlace = fmodf(pos.y, 1.f);
	xDecimalPlace = pos.x > 0.f ? xDecimalPlace : 1.f + xDecimalPlace;
	yDecimalPlace = pos.y > 0.f ? yDecimalPlace : 1.f + yDecimalPlace;

	m_blocksToPushOut.clear();
	BlockIterator bottomNeighbor = currentBlockItr.GetBottomNeighbor();
	BlockIterator topNeighbor = currentBlockItr.GetTopNeighbor();
	m_blocksToPushOut.push_back(bottomNeighbor);
	m_blocksToPushOut.push_back(topNeighbor);
	if (xDecimalPlace <= 0.5f && yDecimalPlace <= 0.5f) // check with west and south neighbors
	{
		m_blocksToPushOut.push_back(currentBlockItr.GetSouthNeighbor());
		m_blocksToPushOut.push_back(currentBlockItr.GetWestNeighbor());
		m_blocksToPushOut.push_back(bottomNeighbor.GetSouthNeighbor());
		m_blocksToPushOut.push_back(bottomNeighbor.GetWestNeighbor());
		m_blocksToPushOut.push_back(topNeighbor.GetSouthNeighbor());
		m_blocksToPushOut.push_back(topNeighbor.GetWestNeighbor());
		BlockIterator southWestNeighbor = currentBlockItr.GetWestNeighbor().GetSouthNeighbor();
		m_blocksToPushOut.push_back(southWestNeighbor);
		m_blocksToPushOut.push_back(southWestNeighbor.GetBottomNeighbor());
		m_blocksToPushOut.push_back(southWestNeighbor.GetTopNeighbor());
	}
	else if (xDecimalPlace <= 0.5f && yDecimalPlace > 0.5f) // check with west and north neighbors
	{
		m_blocksToPushOut.push_back(currentBlockItr.GetNorthNeighbor());
		m_blocksToPushOut.push_back(currentBlockItr.GetWestNeighbor());
		m_blocksToPushOut.push_back(bottomNeighbor.GetNorthNeighbor());
		m_blocksToPushOut.push_back(bottomNeighbor.GetWestNeighbor());
		m_blocksToPushOut.push_back(topNeighbor.GetNorthNeighbor());
		m_blocksToPushOut.push_back(topNeighbor.GetWestNeighbor());
		BlockIterator northWestNeighbor = currentBlockItr.GetWestNeighbor().GetNorthNeighbor();
		m_blocksToPushOut.push_back(northWestNeighbor);
		m_blocksToPushOut.push_back(northWestNeighbor.GetBottomNeighbor());
		m_blocksToPushOut.push_back(northWestNeighbor.GetTopNeighbor());
	}
	else if (xDecimalPlace > 0.5f && yDecimalPlace <= 0.5f) // check with east and south neighbors
	{
		m_blocksToPushOut.push_back(currentBlockItr.GetSouthNeighbor());
		m_blocksToPushOut.push_back(currentBlockItr.GetEastNeighbor());
		m_blocksToPushOut.push_back(bottomNeighbor.GetSouthNeighbor());
		m_blocksToPushOut.push_back(bottomNeighbor.GetEastNeighbor());
		m_blocksToPushOut.push_back(topNeighbor.GetSouthNeighbor());
		m_blocksToPushOut.push_back(topNeighbor.GetEastNeighbor());
		BlockIterator southEastNeighbor = currentBlockItr.GetEastNeighbor().GetSouthNeighbor();
		m_blocksToPushOut.push_back(southEastNeighbor);
		m_blocksToPushOut.push_back(southEastNeighbor.GetBottomNeighbor());
		m_blocksToPushOut.push_back(southEastNeighbor.GetTopNeighbor());
	}
	else if (xDecimalPlace > 0.5f && yDecimalPlace > 0.5f) // check with east and north neighbors
	{
		m_blocksToPushOut.push_back(currentBlockItr.GetNorthNeighbor());
		m_blocksToPushOut.push_back(currentBlockItr.GetEastNeighbor());
		m_blocksToPushOut.push_back(bottomNeighbor.GetNorthNeighbor());
		m_blocksToPushOut.push_back(bottomNeighbor.GetEastNeighbor());
		m_blocksToPushOut.push_back(topNeighbor.GetNorthNeighbor());
		m_blocksToPushOut.push_back(topNeighbor.GetEastNeighbor());
		BlockIterator northEastNeighbor = currentBlockItr.GetEastNeighbor().GetNorthNeighbor();
		m_blocksToPushOut.push_back(northEastNeighbor);
		m_blocksToPushOut.push_back(northEastNeighbor.GetBottomNeighbor());
		m_blocksToPushOut.push_back(northEastNeighbor.GetTopNeighbor());
	}

	for (int index = 0; index < (int)m_blocksToPushOut.size(); index++)
	{
		Block* block = m_blocksToPushOut[index].GetBlock();
		if (!block || !block->IsBlockOpaque()) continue;
		Vec3 direction = PushAABB3OutOfAABB3D(m_player->m_bounds, m_blocksToPushOut[index].GetBlockBounds());
		m_player->m_position = m_player->m_bounds.GetCenter();
		m_player->m_velocity *= direction;
	}
}


bool operator<(IntVec2 const& a, IntVec2 const& b)
{
	if (a.y < b.y)
	{
		return true;
	}
	else if (a.y > b.y)
	{
		return false;
	}
	else
	{
		return a.x < b.x;
	}
}


