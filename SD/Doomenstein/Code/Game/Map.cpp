#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/AI.hpp"
#include "TileMaterialDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/TileSetDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"

constexpr int MAX_INDEX = 0x0000ffff;
constexpr int MAX_SALT	= 0x0000fffe;

constexpr float COUNTDOWN_TIMER = 5.f;

RaycastResultDoomenstein::RaycastResultDoomenstein(bool didImpact, Vec3 impactPosition, float impactDistance, Vec3 impactSurtaceNormal, Vec3 startPosition, Vec3 forwardNormal, float maxDistance)
	: RaycastResult3D(didImpact, impactPosition, impactDistance, impactSurtaceNormal, startPosition, forwardNormal, maxDistance)
{
}


Map::Map(Game* game, const MapDefinition* definition)
	: m_game(game)
	, m_definition(definition)
{
	m_dimensions = m_definition->m_image->GetDimensions();

	CreateTiles();
	CreateGeometry();
	CreateBuffers();
	AddWorldBasis();

	m_shader = m_tiles[0].m_definition->m_wallMaterialDefinition->m_shader;
	m_texture = m_tiles[0].m_definition->m_wallMaterialDefinition->m_texture;

	for (int spawnIndex = 0; spawnIndex < (int)m_definition->m_spawnInfos.size(); spawnIndex++)
	{
		SpawnInfo const& spawnInfo = m_definition->m_spawnInfos[spawnIndex];
		if (spawnInfo.m_definition->m_name == "SpawnPoint") m_marineSpawnInfos.push_back(spawnInfo);
		if (spawnInfo.m_definition->m_name == "DemonSpawnPoint") m_demonSpawnInfos.push_back(spawnInfo);
	}

	std::vector<Player*> players = GetAllPlayerControllers();

	for (int playerIndex = 0; playerIndex < (int)players.size(); playerIndex++)
	{
		Player*& player = players[playerIndex];
		if (!player) continue;

		int spawnIndex = RNG.RollRandomIntInRange(0, (int)m_marineSpawnInfos.size() - 1);
		SpawnInfo spawnInfo = m_marineSpawnInfos[spawnIndex];
		spawnInfo.m_definition = ActorDefinition::GetByName("Marine");
		Actor* actor = SpawnActor(spawnInfo);
		GetPlayerController(player->m_playerIndex)->Possess(actor);
	}

	m_waveTimer.Start(&GetGameClock(), COUNTDOWN_TIMER);
}


Map::~Map()
{
	delete m_vertexBuffer;
	delete m_indexBuffer;
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (actor)
		{
			delete actor;
			actor = nullptr;
		}
	}
}


void Map::Update(float deltaSeconds)
{
	SpawnWave();
	UpdateActors(deltaSeconds);
	CollideActors();
	CollideActorsWithMap();

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_displayLightSettings = !m_displayLightSettings;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_ambientIntensity -= 0.1f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_ambientIntensity += 0.1f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_sunIntensity -= 0.1f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_sunIntensity += 0.1f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_sunDirection.m_pitchDegrees -= 5.f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_sunDirection.m_pitchDegrees += 5.f;
	}

	m_ambientIntensity = Clamp(m_ambientIntensity, 0.f, 1.f);
	m_sunIntensity = Clamp(m_sunIntensity, 0.f, 1.f);
	m_sunDirection.m_pitchDegrees = Clamp(m_sunDirection.m_pitchDegrees, 0.f, 180.f);

	if (m_displayLightSettings)
	{
		std::string sunInfo = Stringf("Ambient Intensity: %.1f, Sun Intensity: %.1f, Sun Pitch: %.1f", m_ambientIntensity, m_sunIntensity, m_sunDirection.m_pitchDegrees);
		EventArgs infoArgs;
		infoArgs.SetValue("text", sunInfo);
		infoArgs.SetValue("duration", "0.0");
		FireEvent("debugSpawnScreenMessage", infoArgs);
	}
}


void Map::Render(Camera const& camera)
{
 	g_theRenderer->SetAmbientIntensity(m_ambientIntensity);
	g_theRenderer->SetSunIntensity(m_sunIntensity);
	g_theRenderer->SetSunDirection(m_sunDirection.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D());

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
	g_theRenderer->SetSamplerState(SamplerMode::POINTCLAMP);

	g_theRenderer->BindShader(m_shader);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->BindLightConstantBuffer();
	g_theRenderer->DrawIndexBuffer(m_vertexBuffer, m_indexBuffer, (int)m_indices.size());

	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (actor)
		{
			actor->Render(camera);
		}
	}
	g_theRenderer->BindShader(nullptr);
}


void Map::CreateTiles()
{
	Image* mapImage = m_definition->m_image;
	for (int row = 0; row < m_dimensions.y; row++)
	{
		for (int column = 0; column < m_dimensions.x; column++)
		{
			Vec3 boundMins(static_cast<float>(column), static_cast<float>(row), 0.f);
			Vec3 boundMaxs = boundMins + Vec3::ONE;
			AABB3 tileBounds(boundMins, boundMaxs);
			Rgba8 const& texelColor = mapImage->GetTexelColor(IntVec2(column, row));
			TileDefinition const* tileDef = m_definition->m_tileSetDefinition->GetTileDefinitionByColor(texelColor);
			Tile newTile(tileBounds, tileDef);
			m_tiles.push_back(newTile);
		}
	}
}


void Map::CreateGeometry()
{
	for (int row = 0; row < m_dimensions.y; row++)
	{
		for (int column = 0; column < m_dimensions.x; column++)
		{
			IntVec2 curCoord(column, row);
			AddVertsForTile(curCoord);
		}
	}
}


void Map::CreateBuffers()
{
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PNCU) * m_vertices.size(), sizeof(Vertex_PNCU));
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int) * m_indices.size());
	g_theRenderer->CopyCPUToGPU(m_vertices.data(), sizeof(Vertex_PNCU) * m_vertices.size(), m_vertexBuffer);
	g_theRenderer->CopyCPUToGPU(m_indices.data(), sizeof(unsigned int) * m_indices.size(), m_indexBuffer);
}


RaycastResultDoomenstein Map::RaycastAll(Vec3 const& start, Vec3 const& direction, float distance, RaycastFilter filter) const
{
	RaycastResultDoomenstein best(false, Vec3::ZERO, 9999.f, Vec3::ZERO, start, direction, distance);
	RaycastResultDoomenstein vsWorldXYBest = RaycastWorldXY(start, direction, distance);
	if (vsWorldXYBest.m_didImpact && vsWorldXYBest.m_impactDistance < best.m_impactDistance) best = vsWorldXYBest;
	RaycastResultDoomenstein vsWorldZBest = RaycastWorldZ(start, direction, distance);
	if (vsWorldZBest.m_didImpact && vsWorldZBest.m_impactDistance < best.m_impactDistance) best = vsWorldZBest;
	RaycastResultDoomenstein vsWorldActorsBest = RaycastWorldActors(start, direction, distance, filter);
	if (vsWorldActorsBest.m_didImpact && vsWorldActorsBest.m_impactDistance < best.m_impactDistance) best = vsWorldActorsBest;

	return best;
}


void Map::SpawnWave()
{
	int demonCount = GetDemonCount();
	if (demonCount == 0)
	{
		std::vector<Player*> players = GetAllPlayerControllers();
		for (int playerIndex = 0; playerIndex < (int)players.size(); playerIndex++)
		{
			Player*& player = players[playerIndex];
			if (!player) continue;
			if (!player->GetActor()) continue;
			player->GetActor()->m_health = player->GetActor()->m_definition->m_health;
		}

		if (m_waveTimer.GetElapsedTime() == 0.f)
		{
			m_currentWaveDemonCounts += m_demonMultiplierPerWave;
			m_waveTimer.Start(&GetGameClock(), COUNTDOWN_TIMER);
			return;
		}

		if (m_waveTimer.HasDurationElapsed())
		{
			m_waveCounter += 1;
			for (int demonIndex = 0; demonIndex < m_currentWaveDemonCounts; demonIndex++)
			{
				int spawnInfoIndex = RNG.RollRandomIntInRange(0, static_cast<int>(m_demonSpawnInfos.size()) - 1);
				SpawnInfo spawnInfo = m_demonSpawnInfos[spawnInfoIndex];
				int demonTypeIndex = RNG.RollRandomIntInRange(0, 1);
				if (demonTypeIndex == 0)
				{
					spawnInfo.m_definition = ActorDefinition::GetByName("Demon");
				}
				else if (demonTypeIndex == 1)
				{
					spawnInfo.m_definition = ActorDefinition::GetByName("Magma");
				}
				Actor* actor = SpawnActor(spawnInfo);
				if (actor->m_definition->m_aiEnabled)
				{
					AI* aiController = new AI();
					aiController->m_meleeStopwatch.Start(&GetGameClock(), actor->m_definition->m_meleeDelay);
					actor->m_aiController = aiController;
					aiController->Possess(actor);
				}
			}
			m_waveTimer.Stop();
		}
	}
}


void Map::UpdateActors(float deltaSeconds)
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (actor && actor->m_controller && dynamic_cast<AI*>(actor->m_controller))
		{
			actor->m_controller->Update(deltaSeconds);
		}
	}

	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (actor)
		{
			actor->Update(deltaSeconds);
		}
	}
}


void Map::CollideActors()
{
	for (int actorAIndex = 0; actorAIndex < (int)m_actors.size() - 1; actorAIndex++)
	{
		Actor*& actorA = m_actors[actorAIndex];
		if (!actorA) continue;
		if (actorA->m_isDead || !actorA->m_definition->m_simulated) continue;
		if (!actorA->m_definition->m_collidesWithActors) continue;

		for (int actorBIndex = actorAIndex + 1; actorBIndex < (int)m_actors.size(); actorBIndex++)
		{
			Actor*& actorB = m_actors[actorBIndex];
			if (!actorB) continue;
			if (actorB->m_isDead || !actorB->m_definition->m_simulated) continue;
			if (!actorB->m_definition->m_collidesWithActors) continue;
			if (actorA->m_owner == actorB || actorB->m_owner == actorA) continue;

			CollideActors(actorA, actorB);
		}
	}
}


void Map::CollideActors(Actor* actorA, Actor* actorB)
{
	Vec2 actorAPosXY(actorA->m_position.x, actorA->m_position.y);
	Vec2 actorBPosXY(actorB->m_position.x, actorB->m_position.y);
	float actorARadius = actorA->m_definition->m_physicsRadius;
	float actorBRadius = actorB->m_definition->m_physicsRadius;

	if (PushDiscsOutOfEachOther2D(actorAPosXY, actorARadius, actorBPosXY, actorBRadius))
	{
		actorA->m_position.x = actorAPosXY.x;
		actorA->m_position.y = actorAPosXY.y;
		actorB->m_position.x = actorBPosXY.x;
		actorB->m_position.y = actorBPosXY.y;
		actorA->OnCollide(actorB);
	}
}

void Map::CollideActorsWithMap()
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (!actor) continue;
		if (actor->m_isDead || !actor->m_definition->m_simulated) continue;
		if (!actor->m_definition->m_collidesWithWorld) continue;

		CollideActorWithMap(actor);
	}
}

void Map::CollideActorWithMap(Actor* actor)
{
	if (actor->m_position.z < 0.f || actor->m_position.z > 1.f - actor->m_definition->m_physicsHeight)
	{
		if (actor->m_definition->m_dieOnCollide) actor->Die();
		return;
	}

	Vec2 const& actorPos = Vec2(actor->m_position.x, actor->m_position.y);
	Tile const* curTile = GetTileByPosition(actorPos);
	if(!curTile) return;
	
	Tile const* northTile = GetTileByPosition(actorPos + Vec2::NORTH);
	if (northTile) PushActorOutOfWall(actor, northTile);

	Tile const* southTile = GetTileByPosition(actorPos + Vec2::SOUTH);
	if (southTile) PushActorOutOfWall(actor, southTile);

	Tile const* westTile = GetTileByPosition(actorPos + Vec2::WEST);
	if (westTile) PushActorOutOfWall(actor, westTile);

	Tile const* eastTile = GetTileByPosition(actorPos + Vec2::EAST);
	if (eastTile) PushActorOutOfWall(actor, eastTile);

	Tile const* NWTile = GetTileByPosition(actorPos + Vec2::NORTH + Vec2::WEST);
	if (NWTile) PushActorOutOfWall(actor, NWTile);

	Tile const* NETile = GetTileByPosition(actorPos + Vec2::NORTH + Vec2::EAST);
	if (NETile) PushActorOutOfWall(actor, NETile);

	Tile const* SWTile = GetTileByPosition(actorPos + Vec2::SOUTH + Vec2::WEST);
	if (SWTile) PushActorOutOfWall(actor, SWTile);

	Tile const* SETile = GetTileByPosition(actorPos + Vec2::SOUTH + Vec2::EAST);
	if (SETile) PushActorOutOfWall(actor, SETile);
}


void Map::PushActorOutOfWall(Actor* actor, Tile const* tile)
{
	if (!tile->m_definition->m_isSolid) return;

	AABB3 const& tileBounds = tile->m_bounds;
	AABB2 tileBounds2D(Vec2(tileBounds.m_mins.x, tileBounds.m_mins.y), Vec2(tileBounds.m_maxs.x, tileBounds.m_maxs.y));
	Vec2 actorPosXY(actor->m_position.x, actor->m_position.y);
	if (PushDiscOutOfAABB2D(actorPosXY, actor->m_definition->m_physicsRadius, tileBounds2D))
	{
		actor->m_position.x = actorPosXY.x;
		actor->m_position.y = actorPosXY.y;
		if (actor->m_definition->m_dieOnCollide) actor->Die();
	}

}


Clock& Map::GetGameClock() const
{
	return m_game->GetGameClock();
}


void Map::RespawnPlayer(Player* controller)
{
	int spawnIndex = RNG.RollRandomIntInRange(0, (int)m_marineSpawnInfos.size() - 1);
	SpawnInfo spawnInfo = m_marineSpawnInfos[spawnIndex];
	spawnInfo.m_definition = ActorDefinition::GetByName("Marine");
	Actor* actor = SpawnActor(spawnInfo);
	controller->Possess(actor);
	std::vector<Player*> players = GetAllPlayerControllers();

	for (int playerIndex = 0; playerIndex < (int)players.size(); playerIndex++)
	{
		Player*& player = players[playerIndex];
		if (!player) continue;
		if (player == controller) continue;
		player->m_kills++;
	}
}


Actor* Map::SpawnActor(SpawnInfo const& spawnInfo)
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (!actor)
		{
			m_actorSalt++;
			if (m_actorSalt > MAX_SALT) m_actorSalt = 0;
			ActorUID uid(actorIndex, m_actorSalt);
			Actor* newActor = new Actor(this, spawnInfo);
			newActor->m_uid = uid;
			m_actors[actorIndex] = newActor;
			return newActor;
		}
	}

	if ((int)m_actors.size() < MAX_INDEX)
	{
		m_actorSalt++;
		if (m_actorSalt > MAX_SALT) m_actorSalt = 0;
		ActorUID uid((int)m_actors.size(), m_actorSalt);
		Actor* newActor = new Actor(this, spawnInfo);
		newActor->m_uid = uid;
		m_actors.push_back(newActor);
		return newActor;
	}
	else
	{
		ERROR_AND_DIE("exceeded maximum actors.");
	}
}


void Map::DestroyActor(ActorUID const uid)
{
	if (!uid.IsValid()) return;

	int index = uid.GetIndex();
	Actor*& actor = m_actors[index];
	if (!actor) return;
	if (actor->m_uid != uid) return;

	delete actor;
	m_actors[index] = nullptr;
}


Actor* Map::FindActorByUID(ActorUID const uid) const
{
	if (!uid.IsValid()) return nullptr;

	int index = uid.GetIndex();
	Actor* actor = m_actors[index];
	if (!actor) return nullptr;
	if (actor->m_uid != uid) return nullptr;

	return actor;
}


Actor* Map::GetClosestVisibleEnemy(Actor* actor)
{
	Vec3 start = actor->GetEyePosition();
	Vec3 direction = actor->GetForward();
	float distance = actor->m_definition->m_sightRadius;

	RaycastResultDoomenstein best(false, Vec3::ZERO, 9999.f, Vec3::ZERO, start, direction, distance);

	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor* other = m_actors[actorIndex];

		if (!other) continue;
		if (other->m_isDead) continue;
		if (other == actor) continue;
		if (other->m_definition->m_faction == actor->m_definition->m_faction || !other->m_definition->m_canBePossessed) continue;

		float forwardDegrees = direction.GetAngleAboutZDegrees();
		float sightAngle = actor->m_definition->m_sightAngle;

		if (!IsPointInsideOrientedSector2D(Vec2(other->m_position.x, other->m_position.y), Vec2(start.x, start.y), forwardDegrees, sightAngle, distance)) continue;

		Vec3 displacementNormal = (other->GetEyePosition() - start).GetNormalized();
		RaycastResult3D vsWall = RaycastWorldXY(start, displacementNormal, distance);
		RaycastResult3D result = RaycastVsZCylinder3D(start, displacementNormal, distance, other->m_position, other->m_definition->m_physicsRadius, other->m_definition->m_physicsHeight);

		if (vsWall.m_didImpact && result.m_didImpact)
		{
			if (vsWall.m_impactDistance < result.m_impactDistance) continue;
		}

		if (result.m_didImpact && result.m_impactDistance < best.m_impactDistance)
		{
			best = RaycastResultDoomenstein(true, result.m_impactPosition, result.m_impactDistance, result.m_impactSurfaceNormal, start, direction, distance);
			best.m_impactActor = other;
		}
	}

	return best.m_impactActor;
}


Player* Map::GetPlayerController(int index) const
{
	return m_game->GetPlayer(index);
}


std::vector<Player*> Map::GetAllPlayerControllers() const
{
	return m_game->GetAllPlayer();
}


void Map::IncreaseKills()
{
	std::vector<Player*> players = GetAllPlayerControllers();

	for (int playerIndex = 0; playerIndex < (int)players.size(); playerIndex++)
	{
		Player*& player = players[playerIndex];
		if (!player) continue;
		player->m_kills++;
	}
}


Game* Map::GetGame()
{
	return m_game;
}


void Map::PossessNextActor()
{
	Actor* currentPlayerActor = GetPlayerController(0)->GetActor();
	int currentPlayerIndex = -1;
	if (currentPlayerActor) 
	{
		currentPlayerIndex = currentPlayerActor->m_uid.GetIndex();
		GetPlayerController(0)->UnPossess(currentPlayerActor);
	}
	while (true)
	{
		currentPlayerIndex++;
		if (currentPlayerIndex == (int)m_actors.size()) currentPlayerIndex = 0;
		Actor *& actor = m_actors[currentPlayerIndex];
		if (!actor) continue;
		if (actor->m_definition->m_canBePossessed)
		{
			GetPlayerController(0)->Possess(actor);
			return;
		}
	}
}


int Map::GetDemonCount() const
{
	int count = 0;
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor* actor = m_actors[actorIndex];
		if (!actor) continue;
		std::string defName = actor->m_definition->m_name;
		if (defName == "Demon" || defName == "Magma")
		{
			count++;
		}
	}

	return count;
}


void Map::AddVertsForTile(IntVec2 const& tileCoord)
{
	Tile const* curTile = GetTileByCoordinate(IntVec2(tileCoord));

	if (curTile->IsAir()) return;

	if (curTile->HasFloor()) AddVertsForFloorTile(tileCoord);

	if (curTile->IsSolid()) AddVertsForSolidTile(tileCoord);
}


void Map::AddVertsForFloorTile(IntVec2 const& tileCoord)
{
	Tile const* curTile = GetTileByCoordinate(IntVec2(tileCoord));

	AABB3 const& tileBounds = curTile->m_bounds;

	//ceiling
	Vec3 ceilingBottomLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
	Vec3 ceilingBottomRight = tileBounds.m_maxs;
	Vec3 ceilingTopRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
	Vec3 ceilingTopLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
	AABB2 const& ceilinguvs = curTile->m_definition->m_ceilingMaterialDefinition->m_uv;
	AddIndexedVertsForQuad3D(m_vertices, m_indices, ceilingBottomLeft, ceilingBottomRight, ceilingTopRight, ceilingTopLeft, Rgba8::WHITE, ceilinguvs);

	//floor
	Vec3 floorBottomLeft = tileBounds.m_mins;
	Vec3 floorBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
	Vec3 floorTopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
	Vec3 floorTopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
	AABB2 const& flooruvs = curTile->m_definition->m_floorMaterialDefinition->m_uv;
	AddIndexedVertsForQuad3D(m_vertices, m_indices, floorBottomLeft, floorBottomRight, floorTopRight, floorTopLeft, Rgba8::WHITE, flooruvs);
}


void Map::AddVertsForSolidTile(IntVec2 const& tileCoord)
{
	Tile const* curTile = GetTileByCoordinate(IntVec2(tileCoord));

	//east
	IntVec2 eastCoord = tileCoord + IntVec2::STEP_EAST;
	Tile const* eastTile = GetTileByCoordinate(eastCoord);
	if (eastTile && eastTile->HasFloor())
	{
		AABB3 const& tileBounds = curTile->m_bounds;
		Vec3 bottomLeft(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
		Vec3 bottomRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
		Vec3 topRight = tileBounds.m_maxs;
		Vec3 topLeft(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
		AABB2 const& uvs = curTile->m_definition->m_wallMaterialDefinition->m_uv;
		AddIndexedVertsForQuad3D(m_vertices, m_indices, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, uvs);
	}

	//west
	IntVec2 westCoord = tileCoord + IntVec2::STEP_WEST;
	Tile const* westTile = GetTileByCoordinate(westCoord);
	if (westTile && westTile->HasFloor())
	{
		AABB3 const& tileBounds = curTile->m_bounds;
		Vec3 bottomLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
		Vec3 bottomRight = tileBounds.m_mins;
		Vec3 topRight(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
		Vec3 topLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
		AABB2 const& uvs = curTile->m_definition->m_wallMaterialDefinition->m_uv;
		AddIndexedVertsForQuad3D(m_vertices, m_indices, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, uvs);
	}

	//north
	IntVec2 northCoord = tileCoord + IntVec2::STEP_NORTH;
	Tile const* northTile = GetTileByCoordinate(northCoord);
	if (northTile && northTile->HasFloor())
	{
		AABB3 const& tileBounds = curTile->m_bounds;
		Vec3 bottomLeft(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
		Vec3 bottomRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
		Vec3 topRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
		Vec3 topLeft = tileBounds.m_maxs;
		AABB2 const& uvs = curTile->m_definition->m_wallMaterialDefinition->m_uv;
		AddIndexedVertsForQuad3D(m_vertices, m_indices, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, uvs);
	}

	//south
	IntVec2 southCoord = tileCoord + IntVec2::STEP_SOUTH;
	Tile const* southTile = GetTileByCoordinate(southCoord);
	if (southTile && southTile->HasFloor())
	{
		AABB3 const& tileBounds = curTile->m_bounds;
		Vec3 bottomLeft = tileBounds.m_mins;
		Vec3 bottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
		Vec3 topRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
		Vec3 topLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
		AABB2 const& uvs = curTile->m_definition->m_wallMaterialDefinition->m_uv;
		AddIndexedVertsForQuad3D(m_vertices, m_indices, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, uvs);
	}
}


Tile const* Map::GetTileByPosition(Vec2 const& position) const
{
	IntVec2 coordinates(RoundDownToInt(position.x), RoundDownToInt(position.y));

	return GetTileByCoordinate(coordinates);
}


Tile const* Map::GetTileByCoordinate(IntVec2 const& coordinate) const
{
	if (coordinate.x < 0 || coordinate.y < 0 || coordinate.x >= m_dimensions.x || coordinate.y >= m_dimensions.y)
	{
		return NULL;
	}

	int tileIndex = coordinate.x + coordinate.y * m_dimensions.x;
	return &m_tiles[tileIndex];
}


void Map::AddWorldBasis() const
{
	DebugAddWorldBasis(Mat44(), -1.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);

	Mat44 xAxis;
	xAxis.Append(Mat44::CreateTranslation3D(Vec3(0.1f, 0.f, 0.1f)));
	xAxis.Append(Mat44::CreateXRotationDegrees(90.f));
	DebugAddWorldText("x - forward", xAxis, 0.1f, Vec2::ZERO, -1.f, Rgba8::RED, Rgba8::RED, DebugRenderMode::USEDEPTH);

	Mat44 yAxis;
	yAxis.Append(Mat44::CreateTranslation3D(Vec3(-0.1f, 0.1f, 0.f)));
	yAxis.Append(Mat44::CreateZRotationDegrees(90.f));
	DebugAddWorldText("y - left", yAxis, 0.1f, Vec2::ZERO, -1.f, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USEDEPTH);

	Mat44 zAxis;
	zAxis.Append(Mat44::CreateTranslation3D(Vec3(0.f, 0.1f, 0.1f)));
	zAxis.Append(Mat44::CreateYRotationDegrees(-90.f));
	DebugAddWorldText("z - up", zAxis, 0.1f, Vec2::ZERO, -1.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USEDEPTH);
}


RaycastResultDoomenstein Map::RaycastWorldXY(Vec3 const& start, Vec3 const& direction, float distance) const
{
	Vec3 end = start + direction * distance;
	Vec2 startXY(start.x, start.y);
	Vec2 endXY(end.x, end.y);
	float distanceXY = (endXY - startXY).GetLength();
	float distanceZ = end.z - start.z;
	Vec2 forwardNormalXY = Vec2(direction.x, direction.y).GetNormalized();
	AABB3 bounds(Vec3(-.001f, -.001f, 0.f), Vec3(static_cast<float>(m_dimensions.x) + .001f, static_cast<float>(m_dimensions.y) + .001f, 1.f));
	Tile const* startTile = GetTileByPosition(startXY);
	if (startTile && startTile->IsSolid() && bounds.IsPointInside(start))
	{
		return RaycastResultDoomenstein(true, start, 0.f, -direction, start, direction, distance);
	}

	IntVec2 startTileCoords(RoundDownToInt(start.x), RoundDownToInt(start.y));
	int tileX = startTileCoords.x;
	int tileY = startTileCoords.y;

	// x crossing calculation
	float fwdDistPerXCrossing = 0.f;
	int tileStepDirectionX = 0;
	float xAtFirstXCrossing = 0.f;
	float xDistToFirstXCrossing = 0.f;
	float fwdDistAtNextXCrossing = 0.f;

	if (forwardNormalXY.x == 0.f)
	{
		fwdDistAtNextXCrossing = distanceXY + 1.f;
	}
	else
	{
		fwdDistPerXCrossing = 1.f / fabsf(forwardNormalXY.x);
		tileStepDirectionX = forwardNormalXY.x < 0.f ? -1 : 1;
		xAtFirstXCrossing = static_cast<float>(tileX + (tileStepDirectionX + 1) / 2);
		xDistToFirstXCrossing = xAtFirstXCrossing - start.x;
		fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;
	}

	// y crossing calculation
	float fwdDistPerYCrossing = 0.f;
	int tileStepDirectionY = 0;
	float yAtFirstYCrossing = 0.f;
	float yDistToFirstYCrossing = 0.f;
	float fwdDistAtNextYCrossing = 0.f;
	if (forwardNormalXY.y == 0.f)
	{
		fwdDistAtNextYCrossing = distanceXY + 1.f;
	}
	else
	{
		fwdDistPerYCrossing = 1.f / fabsf(forwardNormalXY.y);
		tileStepDirectionY = forwardNormalXY.y < 0.f ? -1 : 1;
		yAtFirstYCrossing = static_cast<float>(tileY + (tileStepDirectionY + 1) / 2);
		yDistToFirstYCrossing = yAtFirstYCrossing - start.y;
		fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;
	}

	for (; ;)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)  // x crossing
		{
			if (fwdDistAtNextXCrossing > distanceXY)
			{
				return RaycastResultDoomenstein(false, Vec3::ZERO, 0.f, Vec3::ZERO, start, direction, distance);
			}

			tileX += tileStepDirectionX;
			IntVec2 tileCoords(tileX, tileY);
			Tile const* tile = GetTileByCoordinate(tileCoords);
			if (tile && tile->IsSolid())
			{
				Vec2 impactPositionXY = startXY + fwdDistAtNextXCrossing * forwardNormalXY;
				float impactDistanceZ = (fwdDistAtNextXCrossing * distanceZ) / distanceXY;
				float impactDistance = sqrtf((fwdDistAtNextXCrossing * fwdDistAtNextXCrossing) + (impactDistanceZ * impactDistanceZ));
				Vec3 impactPosition = start + direction * impactDistance;
				if (bounds.IsPointInside(impactPosition))
				{
					return RaycastResultDoomenstein(true, impactPosition, impactDistance, Vec3(static_cast<float>(-tileStepDirectionX), 0.f, 0.f), start, direction, distance);
				}
				else
				{
					fwdDistAtNextXCrossing += fwdDistPerXCrossing;
				}
			}
			else
			{
				fwdDistAtNextXCrossing += fwdDistPerXCrossing;
			}
		}
		else // y crossing
		{
			if (fwdDistAtNextYCrossing > distanceXY)
			{
				return RaycastResultDoomenstein(false, Vec3::ZERO, 0.f, Vec3::ZERO, start, direction, distance);
			}

			tileY += tileStepDirectionY;
			IntVec2 tileCoords(tileX, tileY);
			Tile const* tile = GetTileByCoordinate(tileCoords);
			if (tile && tile->IsSolid())
			{
				Vec2 impactPositionXY = startXY + fwdDistAtNextYCrossing * forwardNormalXY;
				float impactDistanceZ = (fwdDistAtNextYCrossing * distanceZ) / distanceXY;
				float impactDistance = sqrtf((fwdDistAtNextYCrossing * fwdDistAtNextYCrossing) + (impactDistanceZ * impactDistanceZ));
				Vec3 impactPosition = start + direction * impactDistance;
				if (bounds.IsPointInside(impactPosition))
				{
					return RaycastResultDoomenstein(true, impactPosition, impactDistance, Vec3(0.f, static_cast<float>(-tileStepDirectionY), 0.f), start, direction, distance);
				}
				else
				{
					fwdDistAtNextYCrossing += fwdDistPerYCrossing;
				}
			}
			else
			{
				fwdDistAtNextYCrossing += fwdDistPerYCrossing;
			}
		}
	}
}


RaycastResultDoomenstein Map::RaycastWorldZ(Vec3 const& start, Vec3 const& direction, float distance) const
{
	Vec3 end = start + direction * distance;
	float distanceZ = end.z - start.z;
	float impactLength = 0.f;
	Vec3 impactSurfaceNormal = Vec3::ZERO;

	if (distanceZ > 0.f)
	{
		float impactDistanceZ = 1.f - start.z;
		impactLength = fabsf((distance * impactDistanceZ) / distanceZ);
		impactSurfaceNormal = Vec3(0.f, 0.f, -1.f);
	}
	else if (distanceZ < 0.f)
	{
		float impactDistanceZ = start.z - 0.f;
		impactLength = fabsf((distance * impactDistanceZ) / distanceZ);
		impactSurfaceNormal = Vec3(0.f, 0.f, 1.f);
	}
	else
	{
		return RaycastResultDoomenstein(false, Vec3::ZERO, 0.f, Vec3::ZERO, start, direction, distance);
	}
	
	if (impactLength < 0.f || impactLength > distance)
	{
		return RaycastResultDoomenstein(false, Vec3::ZERO, 0.f, Vec3::ZERO, start, direction, distance);
	}

	Vec3 impactPosition = start + direction * impactLength;
	FloatRange xBounds(0.f, static_cast<float>(m_dimensions.x));
	FloatRange yBounds(0.f, static_cast<float>(m_dimensions.y));
	if (xBounds.IsOnRange(impactPosition.x) && yBounds.IsOnRange(impactPosition.y))
	{
		RaycastResultDoomenstein result = RaycastResultDoomenstein(true, impactPosition, impactLength, impactSurfaceNormal, start, direction, distance);
		if (distanceZ < 0.f) result.m_hitFloor = true;
		return result;
	}
	else
	{
		return RaycastResultDoomenstein(false, Vec3::ZERO, 0.f, Vec3::ZERO, start, direction, distance);
	}
}


RaycastResultDoomenstein Map::RaycastWorldActors(Vec3 const& start, Vec3 const& direction, float distance, RaycastFilter filter) const
{
	RaycastResultDoomenstein best(false, Vec3::ZERO, 9999.f, Vec3::ZERO, start, direction, distance);

	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor* actor = m_actors[actorIndex];

		if (!actor) continue;
		if (actor == filter.m_ignoreActor) continue;

		Vec3 actorPosition = actor->m_position;
		float actorHeight = actor->m_definition->m_physicsHeight;
		if (actor->m_isShrinked)
		{
			actorHeight *= actor->m_shrinkScale;
		}
		RaycastResult3D result = RaycastVsZCylinder3D(start, direction, distance, actorPosition, actor->m_definition->m_physicsRadius, actorHeight);
		if (result.m_didImpact && result.m_impactDistance < best.m_impactDistance)
		{
			best = RaycastResultDoomenstein(true, result.m_impactPosition, result.m_impactDistance, result.m_impactSurfaceNormal, start, direction, distance);
			best.m_impactActor = actor;
		}
	}

	return best;
}


