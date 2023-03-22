#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Leo.hpp"
#include "Game/Aries.hpp"
#include "Game/Capricorn.hpp"
#include "Game/Bullet.hpp"
#include "Game/World.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Image.hpp"

RaycastResultLibra::RaycastResultLibra(Vec2 startPos, Vec2 forwardNorm, float maxDist, bool didImpact, Vec2 impactPos, float impactDist, Vec2 impactSurfaceNorm)
	: RaycastResult2D(didImpact, impactPos, impactDist, impactSurfaceNorm, startPos, forwardNorm, maxDist)
{
}


Vec2 cameraCenter;

static float enemySightDistance;

Map::Map(World* owner, MapDefinition const& mapDef)
	: m_world(owner)
	, m_mapDef(mapDef)
	, m_dimensions(mapDef.m_dimensions)
	, m_solidTileHeatMap(mapDef.m_dimensions)
{
	m_exit = Vec2(static_cast<float>(m_dimensions.x) - 1.5f, static_cast<float>(m_dimensions.y) - 1.5f);

	constexpr int MAX_MAP_GENERATION_TRIES = 1000;
	constexpr float MAX_COST = 999.f;

	for (int iteration = 0; iteration <= MAX_MAP_GENERATION_TRIES; iteration++)
	{
		CreateMap();
		PopulateDistanceFieldForEntity(m_solidTileHeatMap, IntVec2(1, 1), MAX_COST, nullptr);
		if (m_solidTileHeatMap.GetValue(IntVec2(m_dimensions.x - 2, m_dimensions.y - 2)) < MAX_COST)
		{
			break;
		}
		if (iteration >= MAX_MAP_GENERATION_TRIES)
		{
			ERROR_AND_DIE("No good map was generated.");
		}
	}

	for (int rowIndex = 1; rowIndex < m_dimensions.y - 1; rowIndex++)
	{
		for (int columnIndex = 1; columnIndex < m_dimensions.x - 1; columnIndex++)
		{
			IntVec2 curCoord(columnIndex, rowIndex);

			Tile* curTile = GetTileByCoordinates(curCoord);
			if (m_solidTileHeatMap.GetValue(curCoord) == MAX_COST && (!curTile->IsTileSolid() && !curTile->IsTileWater()))
			{
				GetTileByCoordinates(curCoord)->SetTileType(m_mapDef.m_wallTileType);
			}
		}
	}

	for (int tileIndex = 0; tileIndex < m_solidTileHeatMap.GetSize(); tileIndex++)
	{
		float curCost = m_solidTileHeatMap.GetValue(tileIndex);
		if (curCost != MAX_COST)
		{
			m_solidTileHeatMap.SetValue(tileIndex, 0.f);
		}
		else
		{
			m_solidTileHeatMap.SetValue(tileIndex, 1.f);
		}
	}

	SpawnEnemies();

	enemySightDistance = g_gameConfigBlackboard.GetValue("enemySightDistance", 0.f);
}


void Map::Startup(Player* player)
{
	m_player = player;
	m_player->m_position = Vec2(1.5f, 1.5f);
	AddEntityToMap(*m_player);
}


void Map::Update(float deltaSeconds)
{
	UpdatePlayerDuringFading(deltaSeconds);
	UpdateEntities(deltaSeconds);
	PushEntitiesOutOfEachOther(deltaSeconds);
	PushEntitiesOutOfWall(deltaSeconds);
	CheckProjectileHits(deltaSeconds);
	UpdateTiles();
	DeleteGarbageEntities();
	CheckIfGoToNextLevel();
	UpdateCamera(deltaSeconds);
}


void Map::Render() const
{
	g_theRenderer->BeginCamera(m_worldCamera);
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	RenderTiles();
	RenderEntities();
	RenderFading();
	g_theRenderer->EndCamera(m_worldCamera);
}


void Map::PopulateDistanceFieldForEntity(TileHeatMap& out_distanceField, IntVec2 const& referenceCoords, float maxCost, Entity* e)
{
	out_distanceField.SetAllValues(maxCost);
	out_distanceField.SetValue(referenceCoords, 0.f);

	for (int currentCost = 0; currentCost < maxCost; currentCost++)
	{
		for (int tileY = 0; tileY < m_dimensions.y; tileY++)
		{
			for (int tileX = 0; tileX < m_dimensions.x; tileX++)
			{
				IntVec2 curCoord(tileX, tileY);
				float curCost = out_distanceField.GetValue(curCoord);
				if (curCost == currentCost)
				{
					IntVec2 eastCoord = curCoord + IntVec2::STEP_EAST;
					Tile* eastTile = GetTileByCoordinates(eastCoord);
					if (out_distanceField.GetValue(eastCoord) >= curCost && !IsTileSolidForEntity(e, eastTile)) // EAST
					{
						out_distanceField.SetValue(eastCoord, curCost + 1.f);
					}
					IntVec2 westCoord = curCoord + IntVec2::STEP_WEST;
					Tile* westTile = GetTileByCoordinates(westCoord);
					if (out_distanceField.GetValue(westCoord) >= curCost && !IsTileSolidForEntity(e, westTile)) // WEST
					{
						out_distanceField.SetValue(westCoord, curCost + 1.f);
					}
					IntVec2 northCoord = curCoord + IntVec2::STEP_NORTH;
					Tile* northTile = GetTileByCoordinates(northCoord);
					if (out_distanceField.GetValue(northCoord) >= curCost && !IsTileSolidForEntity(e, northTile)) // NORTH
					{
						out_distanceField.SetValue(northCoord, curCost + 1.f);
					}
					IntVec2 southCoord = curCoord + IntVec2::STEP_SOUTH;
					Tile* southTile = GetTileByCoordinates(southCoord);
					if (out_distanceField.GetValue(southCoord) >= curCost && !IsTileSolidForEntity(e, southTile)) // SOUTH
					{
						out_distanceField.SetValue(southCoord, curCost + 1.f);
					}
				}
			}
		}
	}
}


void Map::AddEntityToMap(Entity& e)
{
	e.m_map = this;
	AddEntityToList(e, m_allEntities);
	AddEntityToList(e, m_entityListsByType[e.m_type]);
	if (e.m_isActor) 
	{
		AddEntityToList(e, m_actorListsByFaction[e.m_faction]);
	}
	if (e.m_isProjectile) 
	{
		AddEntityToList(e, m_projectileListsByFaction[e.m_faction]);
	}
}


void Map::AddEntityToList(Entity& e, EntityList& list)
{
	for (int entityIndex = 0; entityIndex < int(list.size()); entityIndex++)
	{
		if (!list[entityIndex])
		{
			list[entityIndex] = &e;
			return;
		}
	}
	list.push_back(&e);
}


void Map::RemoveEntityFromMap(Entity& e)
{
	e.m_map = nullptr;
	RemoveEntityFromList(e, m_allEntities);
	RemoveEntityFromList(e, m_entityListsByType[e.m_type]);
	if (e.m_isActor) 
	{
		RemoveEntityFromList(e, m_actorListsByFaction[e.m_faction]);
	}
	if (e.m_isProjectile) 
	{
		RemoveEntityFromList(e, m_projectileListsByFaction[e.m_faction]);
	}
	if (e.m_type == ENTITY_TYPE_GOOD_PLAYER) 
	{
		m_player = nullptr;
	}
}


void Map::RemoveEntityFromList(Entity& e, EntityList& list)
{
	for (int entityIndex = 0; entityIndex < int(list.size()); entityIndex++)
	{
		if (list[entityIndex] == &e)
		{
			list[entityIndex] = nullptr;
			return;
		}
	}
}


void Map::AddExplosionToMap(Vec2 const& position, float orientation, ExplosionType explosionType)
{
	Explosion* newExplosion = new Explosion(this, position, orientation, explosionType);
	for (int entityIndex = 0; entityIndex < int(m_explosions.size()); entityIndex++)
	{
		if (!m_explosions[entityIndex])
		{
			m_explosions[entityIndex] = newExplosion;
			return;
		}
	}
	m_explosions.push_back(newExplosion);
}


bool Map::IsAlive(Entity* e) const
{
	return (e && !e->m_isDead);
}


bool Map::IsPlayerAlive() const
{
	return IsAlive(m_player);
}


bool Map::IsPointInSolid(Vec2 const& point)
{
	Tile* tile = GetTileByPosition(point);
	if (tile) 
	{
		return IsTileSolid(tile);
	}
	return false;
}


bool Map::IsTileSolidForEntity(Entity* e, Tile const* tile)
{
	if (!e) 
	{
		return IsTileSolid(tile) || IsTileWater(tile);
	}

	if (e->m_canSwim)
	{
		return IsTileSolid(tile);
	}
	else
	{
		EntityList const& scorpios = m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO];
		for (int scorpioIndex = 0; scorpioIndex < int(scorpios.size()); scorpioIndex++)
		{
			Entity* scorpio = scorpios[scorpioIndex];
			if (IsAlive(scorpio))
			{
				Vec2 pos = scorpio->m_position;
				IntVec2 coords(RoundDownToInt(pos.x), RoundDownToInt(pos.y));
				if (coords == tile->m_tileCoords)
				{
					return true;
				}
			}
		}

		return IsTileSolid(tile) || IsTileWater(tile);
	}
}


bool Map::IsTileSolid(Tile const* tile)
{
	return tile && tile->IsTileSolid();
}


bool Map::IsTileWater(Tile const* tile)
{
	return tile && tile->IsTileWater();
}


void Map::CheckIfGoToNextLevel()
{
	if (!IsAlive(m_player)) 
	{
		return;
	}

	if (IsPointInsideDisc2D(m_exit, m_player->m_position, m_player->m_physicsRadius))
	{
		m_isFadingOut = true;
	}
}


IntVec2 Map::RollSpawnLocation(bool treatWaterAsSolid)
{
	IntVec2 pos = IntVec2::ZERO;
	Tile* tile = nullptr;
	while (IsTileSolid(tile) || (treatWaterAsSolid && IsTileWater(tile)) || (pos.x <= 6 && pos.y <= 6) || (pos.x >= m_dimensions.x - 7 && pos.y >= m_dimensions.y - 7))
	{
		pos = RollSpawnLocationWithinWall();
		tile = GetTileByCoordinates(pos);
	}
	return pos;
}


Entity* Map::CreateEntityOfType(EntityType type, Vec2 const& position, float orientaion)
{
	switch (type)
	{
		case ENTITY_TYPE_GOOD_PLAYER:			return new Player(this, position, orientaion);
		case ENTITY_TYPE_EVIL_SCORPIO:			return new Scorpio(this, position, orientaion);
		case ENTITY_TYPE_EVIL_LEO:				return new Leo(this, position, orientaion);
		case ENTITY_TYPE_EVIL_ARIES:			return new Aries(this, position, orientaion);
		case ENTITY_TYPE_EVIL_CAPRICORN:		return new Capricorn(this, position, orientaion);
		case ENTITY_TYPE_GOOD_BULLET:			return new Bullet(this, position, orientaion, ENTITY_FACTION_GOOD, type, false);
		case ENTITY_TYPE_GOOD_FLAME_BULLET:		return new Bullet(this, position, orientaion, ENTITY_FACTION_GOOD, type, false);
		case ENTITY_TYPE_EVIL_BULLET:			return new Bullet(this, position, orientaion, ENTITY_FACTION_EVIL, type, false);
		case ENTITY_TYPE_EVIL_MISSLE:			return new Bullet(this, position, orientaion, ENTITY_FACTION_EVIL, type, true);
	}

	ERROR_AND_DIE("No such entity type exited.")
}


Tile* Map::GetTileByPosition(Vec2 const& position)
{
	IntVec2 coordinates(RoundDownToInt(position.x), RoundDownToInt(position.y));

	return GetTileByCoordinates(coordinates);
}


Tile* Map::GetTileByCoordinates(IntVec2 const& coordinates)
{
	if (coordinates.x < 0 || coordinates.x > m_dimensions.x - 1 || coordinates.y < 0 || coordinates.y > m_dimensions.y - 1) 
	{
		return NULL;
	}

	int tileIndex = GetTileIndexByCoordinates(coordinates);
	return &m_tiles[tileIndex];
}


int Map::GetTileIndexByCoordinates(IntVec2 const& coordinates)
{
	return coordinates.x + coordinates.y * m_dimensions.x;
}


IntVec2 Map::GetMapDimensions() const
{
	return m_dimensions;
}


Player* Map::SpawnPlayer()
{
	float playerOrientation = g_gameConfigBlackboard.GetValue("playerStartOrientation", 0.f);
	m_player = new Player(this, m_lastPlayerPosition, playerOrientation);
	AddEntityToMap(*m_player);
	return m_player;
}


Player* Map::GetPlayer() const
{
	return m_player;
}


void Map::PopulateDistanceFieldForEntityPathToGoal(TileHeatMap& out_distanceField, float maxCost, Entity* e)
{
	out_distanceField.SetAllValues(maxCost);
	Vec2 const& position = e->m_position;
	IntVec2 entityCoords(RoundDownToInt(position.x), RoundDownToInt(position.y));
	out_distanceField.SetValue(entityCoords, 0.f);

	for (int currentCost = 0; currentCost < maxCost; currentCost++)
	{
		for (int tileY = 0; tileY < m_dimensions.y; tileY++)
		{
			for (int tileX = 0; tileX < m_dimensions.x; tileX++)
			{
				IntVec2 curCoord(tileX, tileY);
				float curCost = out_distanceField.GetValue(curCoord);
				if (curCost == currentCost)
				{
					IntVec2 eastCoord = curCoord + IntVec2::STEP_EAST;
					Tile* eastTile = GetTileByCoordinates(eastCoord);
					if (out_distanceField.GetValue(eastCoord) >= curCost && !IsTileSolidForEntity(e, eastTile)) // EAST
					{
						out_distanceField.SetValue(eastCoord, curCost + 1.f);
					}

					IntVec2 westCoord = curCoord + IntVec2::STEP_WEST;
					Tile* westTile = GetTileByCoordinates(westCoord);
					if (out_distanceField.GetValue(westCoord) >= curCost && !IsTileSolidForEntity(e, westTile)) // WEST
					{
						out_distanceField.SetValue(westCoord, curCost + 1.f);
					}

					IntVec2 northCoord = curCoord + IntVec2::STEP_NORTH;
					Tile* northTile = GetTileByCoordinates(northCoord);
					if (out_distanceField.GetValue(northCoord) >= curCost && !IsTileSolidForEntity(e, northTile)) // NORTH
					{
						out_distanceField.SetValue(northCoord, curCost + 1.f);
					}

					IntVec2 southCoord = curCoord + IntVec2::STEP_SOUTH;
					Tile* southTile = GetTileByCoordinates(southCoord);
					if (out_distanceField.GetValue(southCoord) >= curCost && !IsTileSolidForEntity(e, southTile)) // SOUTH
					{
						out_distanceField.SetValue(southCoord, curCost + 1.f);
					}
				}
			}
		}
	}
}


std::vector<Vec2> Map::GenerateEntityPathToGoal(TileHeatMap const& distanceField, IntVec2 const& goalCoords)
{
	
	std::vector<Vec2> path;
	float curCost = distanceField.GetValue(goalCoords);
	IntVec2 curCoords = goalCoords;
	path.emplace_back(Vec2(curCoords) + Vec2(0.5f, 0.5f));
	for (int cost = static_cast<int>(curCost); cost > 0; cost--)
	{
		IntVec2 eastCoords = curCoords + IntVec2::STEP_EAST;
		float eastCost = distanceField.GetValue(eastCoords);
		if (eastCost == curCost - 1.f)
		{
			path.emplace_back(Vec2(eastCoords) + Vec2(0.5f, 0.5f));
			curCoords = eastCoords;
			curCost = eastCost;
			continue;
		}

		IntVec2 westCoords = curCoords + IntVec2::STEP_WEST;
		float westCost = distanceField.GetValue(westCoords);
		if (westCost == curCost - 1.f)
		{
			path.emplace_back(Vec2(westCoords) + Vec2(0.5f, 0.5f));
			curCoords = westCoords;
			curCost = westCost;
			continue;
		}

		IntVec2 northCoords = curCoords + IntVec2::STEP_NORTH;
		float northCost = distanceField.GetValue(northCoords);
		if (northCost == curCost - 1.f)
		{
			path.emplace_back(Vec2(northCoords) + Vec2(0.5f, 0.5f));
			curCoords = northCoords;
			curCost = northCost;
			continue;
		}

		IntVec2 southCoords = curCoords + IntVec2::STEP_SOUTH;
		float southCost = distanceField.GetValue(southCoords);
		if (southCost == curCost - 1.f)
		{
			path.emplace_back(Vec2(southCoords) + Vec2(0.5f, 0.5f));
			curCoords = southCoords;
			curCost = southCost;
			continue;
		}
	}

	return path;
}


bool Map::HasLineOfSight(Vec2 const& startPos, Vec2 const& targetPos)
{
	Vec2 displacement = targetPos - startPos;
	float distance = displacement.GetLength();
	if (distance > enemySightDistance)
	{
		return false;
	}

	Vec2 forward = displacement.GetNormalized();
	RaycastResultLibra result = FastRaycastVsTile(startPos, forward, distance);
	return !result.m_didImpact;
}


RaycastResultLibra Map::FastRaycastVsTile(Vec2 const& start, Vec2 const& forwardNormal, float maxLength)
{
	Tile* startTile = GetTileByPosition(start);
	if (IsTileSolid(startTile))
	{
		return RaycastResultLibra(start, forwardNormal, maxLength, true, start, 0.f, forwardNormal.GetRotatedDegrees(180.f));
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

	if (forwardNormal.x == 0.f)
	{
		fwdDistAtNextXCrossing = maxLength + 1.f;
	}
	else
	{
		fwdDistPerXCrossing = 1.f / abs(forwardNormal.x);
		tileStepDirectionX = forwardNormal.x < 0.f ? -1 : 1;
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
	if (forwardNormal.y == 0.f)
	{
		fwdDistAtNextYCrossing = maxLength + 1.f;
	}
	else
	{
		fwdDistPerYCrossing = 1.f / abs(forwardNormal.y);
		tileStepDirectionY = forwardNormal.y < 0.f ? -1 : 1;
		yAtFirstYCrossing = static_cast<float>(tileY + (tileStepDirectionY + 1) / 2);
		yDistToFirstYCrossing = yAtFirstYCrossing - start.y;
		fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;
	}

	for(; ;)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)  // x crossing
		{
			if (fwdDistAtNextXCrossing > maxLength)
			{
				Vec2 endPosition = start + maxLength * forwardNormal;
				return RaycastResultLibra(start, forwardNormal, maxLength, false, endPosition);
			}

			tileX += tileStepDirectionX;
			IntVec2 tileCoords(tileX, tileY);
			if (IsTileSolid(GetTileByCoordinates(tileCoords)))
			{
				Vec2 impactPosition = start + fwdDistAtNextXCrossing * forwardNormal;
				return RaycastResultLibra(start, forwardNormal, maxLength, true, impactPosition, fwdDistAtNextXCrossing, Vec2(static_cast<float>(-tileStepDirectionX), 0.f));
			}
			else
			{
				fwdDistAtNextXCrossing += fwdDistPerXCrossing;
			}
		}
		else // y crossing
		{
			if (fwdDistAtNextYCrossing > maxLength)
			{
				Vec2 endPosition = start + maxLength * forwardNormal;
				return RaycastResultLibra(start, forwardNormal, maxLength, false, endPosition);
			}

			tileY += tileStepDirectionY;
			IntVec2 tileCoords(tileX, tileY);
			if (IsTileSolid(GetTileByCoordinates(tileCoords)))
			{
				Vec2 impactPosition = start + fwdDistAtNextYCrossing * forwardNormal;
				return RaycastResultLibra(start, forwardNormal, maxLength, true, impactPosition, fwdDistAtNextYCrossing, Vec2(0.f, static_cast<float>(-tileStepDirectionY)));
			}
			else
			{
				fwdDistAtNextYCrossing += fwdDistPerYCrossing;
			}
		}
	}
}


void Map::UpdatePlayerDuringFading(float deltaSeconds)
{
	if (m_isFadingIn)
	{
		m_fadeTimer -= deltaSeconds;
		m_player->RotateAndScale(deltaSeconds, 1, 1.f - m_fadeTimer);
		if (m_fadeTimer <= 0.f)
		{
			m_player->m_orientationDegrees = g_gameConfigBlackboard.GetValue("playerStartOrientation", 0.f);
			m_player->m_turretOrientationDegrees = 0.f;
			m_isFadingIn = false;
			m_fadeTimer = 0.f;
		}
	}
	else if (m_isFadingOut)
	{
		m_fadeTimer += deltaSeconds;
		m_player->RotateAndScale(deltaSeconds, -1, 1.f - m_fadeTimer);
		if (m_fadeTimer >= m_fadeDelay)
		{
			m_isFadingOut = false;
			m_fadeTimer = 1.f;
			m_world->GoToNextLevel();
		}
	}
}


void Map::UpdateEntities(float deltaSeconds)
{
	if (m_isPlayerDiscovered)
	{
		if (m_discoveryTimer >= m_discoveryDelay)
		{
			m_isPlayerDiscovered = false;
			PlaySound(g_soundIds[SOUND_TYPE_PLAYER_DISCOVERY], false, g_gameSoundVolume);
		}
		else
		{
			m_discoveryTimer += deltaSeconds;
		}
	}

	if (IsPlayerAlive()) 
	{
		m_lastPlayerPosition = m_player->m_position;
	}

	for (int entityIndex = 0; entityIndex < int(m_allEntities.size()); entityIndex++)
	{
		Entity* e = m_allEntities[entityIndex];
		if (IsAlive(e)) 
		{
			e->Update(deltaSeconds);
		}
	}

	EntityList enemyList = m_actorListsByFaction[ENTITY_FACTION_EVIL];
	for (int entityIndex = 0; entityIndex < int(enemyList.size()); entityIndex++)
	{
		Entity* e = enemyList[entityIndex];
		if (IsAlive(e))
		{
			if (IsPlayerAlive() && HasLineOfSight(e->m_position, m_player->m_position))
			{
				if (!e->m_isPursuing && !m_isPlayerDiscovered)
				{
					m_isPlayerDiscovered = true;
				}
				e->m_isPursuing = true;
				e->m_hasSightOfPlayer = true;
				e->m_targetPosition = m_player->m_position;
			}
			else
			{
				e->m_hasSightOfPlayer = false;
				if (e->m_type == ENTITY_TYPE_EVIL_SCORPIO)
				{
					e->m_isPursuing = false;
				}
			}

			if (e->m_type == ENTITY_TYPE_EVIL_SCORPIO)
			{
				Scorpio* s = dynamic_cast<Scorpio*>(e);
				RaycastResultLibra r = FastRaycastVsTile(s->m_position, s->GetForwardNormal(), enemySightDistance);
				s->m_laserImpactPoint = r.m_impactPosition;
			}
		}
	}

	for (int explosionIndex = 0; explosionIndex < int(m_explosions.size()); explosionIndex++)
	{
		Entity* e = m_explosions[explosionIndex];
		if (IsAlive(e))
		{
			e->Update(deltaSeconds);
		}
	}
}


void Map::PushEntitiesOutOfEachOther(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	for (int entityAIndex = 0; entityAIndex < int(m_allEntities.size()); entityAIndex++)
	{
		Entity* a = m_allEntities[entityAIndex];
		if (!IsAlive(a)) 
		{
			continue;
		}
		if (a->m_type == ENTITY_TYPE_GOOD_PLAYER && g_isNoClip) 
		{
			continue;
		}
		for (int entityBIndex = entityAIndex + 1; entityBIndex < int(m_allEntities.size()); entityBIndex++)
		{
			Entity* b = m_allEntities[entityBIndex];
			if (!IsAlive(b)) 
			{
				continue;
			}
			PushTwoEntitiesOutOfEachOther(*a, *b);
		}
	}
}


void Map::PushEntitiesOutOfWall(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	for (int entityIndex = 0; entityIndex < m_allEntities.size(); entityIndex++)
	{
		Entity* e = m_allEntities[entityIndex];
		if (!IsAlive(e)) 
		{
			continue;
		}

		if (e->m_type == ENTITY_TYPE_GOOD_PLAYER && g_isNoClip) 
		{
			continue;
		}

		Tile* curTile = GetTileByPosition(e->m_position);
		if (!curTile)
		{
			continue;
		}

		if (IsAlive(e) && e->m_isPushedByWalls)
		{
			PushEntityOutOfWalls(*e, *curTile);
		}
	}
}


void Map::CheckProjectileHits(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	CheckProjectilesWithEntities(m_projectileListsByFaction[ENTITY_FACTION_GOOD], m_actorListsByFaction[ENTITY_FACTION_EVIL]);
	CheckProjectilesWithEntities(m_projectileListsByFaction[ENTITY_FACTION_EVIL], m_actorListsByFaction[ENTITY_FACTION_GOOD]);
}


void Map::CheckProjectilesWithEntities(EntityList projectiles, EntityList actors)
{
	for (int bulletIndex = 0; bulletIndex < int(projectiles.size()); bulletIndex++)
	{
		Bullet* bullet = dynamic_cast<Bullet*>(projectiles[bulletIndex]);
		if (!IsAlive(bullet))
		{
			continue;
		}

		for (int actorIndex = 0; actorIndex < int(actors.size()); actorIndex++)
		{
			Entity* actor = actors[actorIndex];
			if (!IsAlive(actor)) 
			{
				continue;
			}

			CheckBulletVsActor(*bullet, *actor);
		}
	}

}


void Map::CheckBulletVsActor(Bullet& bullet, Entity& actor)
{
	if (!DoDiscsOverlap2D(bullet.m_position, bullet.m_physicsRadius, actor.m_position, actor.m_physicsRadius))
	{
		return;
	}

	actor.ReactToBulletHit(bullet);
}


void Map::UpdateTiles()
{
	for (int tileIndex = 0; tileIndex < int(m_tiles.size()); tileIndex++)
	{
		Tile& curTile = m_tiles[tileIndex];
		if (curTile.m_tileDef->m_isDestructible && curTile.m_health == 0)
		{
			curTile.SetTileType(curTile.m_tileDef->m_tileAfterDestructed);
		}
	}
}


void Map::DeleteGarbageEntities()
{
	for (int entityIndex = 0; entityIndex < int(m_allEntities.size()); entityIndex++)
	{
		Entity* e = m_allEntities[entityIndex];
		if (e && e->m_isGarbage)
		{
			RemoveEntityFromMap(*e);
			delete e;
		}
	}

	for (int explosionIndex = 0; explosionIndex < int(m_explosions.size()); explosionIndex++)
	{
		Entity* e = m_explosions[explosionIndex];
		if (e && e->m_isGarbage)
		{
			m_explosions[explosionIndex] = nullptr;
			delete e;
		}
	}
}


void Map::PushTwoEntitiesOutOfEachOther(Entity& a, Entity& b)
{
	bool canAPushB = a.m_doesPushEntities && b.m_isPushedByEntities;
	bool canBPushA = b.m_doesPushEntities && a.m_isPushedByEntities;

	if (!canAPushB && !canBPushA)
	{
		return;
	}

	if (canAPushB && canBPushA)
	{
		PushDiscsOutOfEachOther2D(a.m_position, a.m_physicsRadius, b.m_position, b.m_physicsRadius);
	}
	else if (canAPushB && !canBPushA)
	{
		PushDiscOutOfDisc2D(b.m_position, b.m_physicsRadius, a.m_position, a.m_physicsRadius);
	}
	else if (!canAPushB && canBPushA)
	{
		PushDiscOutOfDisc2D(a.m_position, a.m_physicsRadius, b.m_position, b.m_physicsRadius);
	}
}


void Map::PushEntityOutOfWalls(Entity& e, Tile& tile)
{
	Tile* northTile = GetTileByCoordinates(tile.m_tileCoords + IntVec2::STEP_NORTH);
	if (northTile) 
	{
		PushEntityOutOfTile(e, *northTile);
	}

	Tile* southTile = GetTileByCoordinates(tile.m_tileCoords + IntVec2::STEP_SOUTH);
	if (southTile) 
	{
		PushEntityOutOfTile(e, *southTile);
	}

	Tile* westTile = GetTileByCoordinates(tile.m_tileCoords + IntVec2::STEP_WEST);
	if (westTile)
	{
		PushEntityOutOfTile(e, *westTile);
	}

	Tile* eastTile = GetTileByCoordinates(tile.m_tileCoords + IntVec2::STEP_EAST);
	if (eastTile) 
	{
		PushEntityOutOfTile(e, *eastTile);
	}

	Tile* NWTile = GetTileByCoordinates(tile.m_tileCoords + IntVec2::STEP_NW);
	if (NWTile)
	{
		PushEntityOutOfTile(e, *NWTile);
	}

	Tile* NETile = GetTileByCoordinates(tile.m_tileCoords + IntVec2::STEP_NE);
	if (NETile) 
	{
		PushEntityOutOfTile(e, *NETile);
	}

	Tile* SWTile = GetTileByCoordinates(tile.m_tileCoords + IntVec2::STEP_SW);
	if (SWTile) 
	{
		PushEntityOutOfTile(e, *SWTile);
	}

	Tile* SETile = GetTileByCoordinates(tile.m_tileCoords + IntVec2::STEP_SE);
	if (SETile) 
	{
		PushEntityOutOfTile(e, *SETile);
	}
}

void Map::PushEntityOutOfTile(Entity& e, Tile& tile)
{
	if (!IsTileSolidForEntity(&e, &tile)) 
	{
		return;
	}

	IntVec2 coordinate = tile.m_tileCoords;
	Vec2 bottomLeft(static_cast<float>(coordinate.x), static_cast<float>(coordinate.y));
	Vec2 topRight = bottomLeft + Vec2::ONE;
	AABB2 box(bottomLeft, topRight);
	PushDiscOutOfAABB2D(e.m_position, e.m_physicsRadius, box);
}


void Map::RenderFading() const
{
	if (m_isFadingIn || m_isFadingOut)
	{
		std::vector<Vertex_PCU> verts;
		verts.reserve(6);
		static float cameraWdith = g_gameConfigBlackboard.GetValue("screenCameraDimensionX", 200.f);
		static float cameraHeight = g_gameConfigBlackboard.GetValue("screenCameraDimensionY", 100.f);
		AABB2 box(Vec2::ZERO, Vec2(cameraWdith, cameraHeight));
		unsigned char alpha = static_cast<unsigned char>(Interpolate(0.f, 255.f, m_fadeTimer));
		AddVertsForAABB2D(verts, box, Rgba8(0, 0, 0, alpha));
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
	}
}


void Map::RenderEntities() const
{
	for (int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; entityTypeIndex++)
	{
		RenderEntitiesOfTypes(EntityType(entityTypeIndex));
	}

	for (int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; entityTypeIndex++)
	{
		if (EntityType(entityTypeIndex) == EntityType::ENTITY_TYPE_GOOD_BULLET) continue;
		if (EntityType(entityTypeIndex) == EntityType::ENTITY_TYPE_GOOD_FLAME_BULLET) continue;
		if (EntityType(entityTypeIndex) == EntityType::ENTITY_TYPE_EVIL_BULLET) continue;
		if (EntityType(entityTypeIndex) == EntityType::ENTITY_TYPE_EVIL_MISSLE) continue;
		RenderEntitiesHealth(EntityType(entityTypeIndex));
	}

	g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
	for (int explosionIndex = 0; explosionIndex < int(m_explosions.size()); explosionIndex++)
	{
		Entity* e = m_explosions[explosionIndex];
		if (IsAlive(e))
		{
			e->Render();
		}
	}
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
}


void Map::RenderEntitiesOfTypes(EntityType entityTypeIndex) const
{
	EntityList const& list = m_entityListsByType[entityTypeIndex];
	for (int entityIndex = 0; entityIndex < int(list.size()); entityIndex++)
	{
		Entity* e = list[entityIndex];
		if (IsAlive(e))
		{
			e->Render();
		}
	}
}


void Map::RenderEntitiesHealth(EntityType entityTypeIndex) const
{
	EntityList const& list = m_entityListsByType[entityTypeIndex];
	for (int entityIndex = 0; entityIndex < int(list.size()); entityIndex++)
	{
		Entity* e = list[entityIndex];
		if (IsAlive(e))
		{
			e->RenderHealth();
		}
	}
}


void Map::RenderTiles() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve(6 * m_tiles.size());
	g_theRenderer->BindTexture(&g_tileSpriteSheet->GetTexture());
	for (int tileIndex = 0; tileIndex < int(m_tiles.size()); tileIndex++)
	{
		Tile const& curTile = m_tiles[tileIndex];
		Vec2 bottomLeft(curTile.m_tileCoords);
		Vec2 topRight = bottomLeft + Vec2::ONE;
		AABB2 tileBox(bottomLeft, topRight);
		TileDefinition const* curTileDef = curTile.m_tileDef;
		AddVertsForAABB2D(verts, tileBox, curTileDef->m_tint, curTileDef->m_uvBounds.m_mins, curTileDef->m_uvBounds.m_maxs);
	}
	g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
}


void Map::UpdateCamera(float deltaSeconds)
{
	SetCameraOrtho();

	float offsetX = RNG.RollRandomFloatInRange(-m_screenshakeAmount, m_screenshakeAmount);
	float offsetY = RNG.RollRandomFloatInRange(-m_screenshakeAmount, m_screenshakeAmount);
	Vec2 offset = Vec2(offsetX, offsetY);
	m_worldCamera.Translate(offset);

	static float maxScreenshakeOffset = g_gameConfigBlackboard.GetValue("screenshakeOffsetMax", 0.f);
	static float screenshakeDecayRate = g_gameConfigBlackboard.GetValue("screenshakeDecayRate", 0.f);

	if (m_screenshakeAmount > 0.f) 
	{
		m_screenshakeAmount -= screenshakeDecayRate * deltaSeconds;
	}

	m_screenshakeAmount = Clamp(m_screenshakeAmount, 0.f, maxScreenshakeOffset);
}


void Map::AddScreenshake(float deltaScreenshake)
{
	static float maxScreenshakeOffset = g_gameConfigBlackboard.GetValue("screenshakeOffsetMax", 0.f);

	m_screenshakeAmount += deltaScreenshake;
	m_screenshakeAmount = Clamp(m_screenshakeAmount, 0.f, maxScreenshakeOffset);
}


void Map::SetCameraOrtho()
{
	static float cameraCenterX = g_gameConfigBlackboard.GetValue("gameCameraCenterX", 8.f);
	static float cameraCenterY = g_gameConfigBlackboard.GetValue("gameCameraCenterY", 4.f);

	Vec2 dimensions(m_dimensions);
	if (g_isDebugCamera)
	{
		float aspectRatio = g_theWindow->GetConfig().m_clientAspect;
		if (m_dimensions.y * g_theWindow->GetConfig().m_clientAspect < m_dimensions.x)
		{
			m_worldCamera.SetOrthoView(Vec2::ZERO, Vec2(dimensions.x, dimensions.x / aspectRatio));
		}
		else
		{
			m_worldCamera.SetOrthoView(Vec2::ZERO, Vec2(aspectRatio * dimensions.y, dimensions.y));
		}
		return;
	}

	if (IsAlive(m_player)) 
	{
		cameraCenter = m_player->m_position;
	}

	Vec2 bottomLeft(cameraCenter - Vec2(cameraCenterX, cameraCenterY));
	Vec2 topRight(cameraCenter + Vec2(cameraCenterX, cameraCenterY));
	m_worldCamera.SetOrthoView(bottomLeft, topRight);
	if (bottomLeft.x < 0.f) m_worldCamera.Translate(Vec2(0.f - bottomLeft.x, 0.f));
	if (bottomLeft.y < 0.f) m_worldCamera.Translate(Vec2(0.f, 0.f - bottomLeft.y));
	if (topRight.x > dimensions.x) m_worldCamera.Translate(Vec2(dimensions.x - topRight.x, 0.f));
	if (topRight.y > dimensions.y) m_worldCamera.Translate(Vec2(0.f, dimensions.y - topRight.y));
}


IntVec2 Map::RollSpawnLocationWithinWall()
{
	int x = RNG.RollRandomIntInRange(1, m_mapDef.m_dimensions.x - 2);
	int y = RNG.RollRandomIntInRange(1, m_mapDef.m_dimensions.y - 2);
	return IntVec2(x, y);
}


IntVec2 Map::RollRandomDirection()
{
	int direction = RNG.RollRandomIntInRange(0, 3);
	if (direction == 0) // EAST
	{
		return IntVec2::STEP_EAST;
	}
	else if (direction == 1) // WEST
	{
		return IntVec2::STEP_WEST;
	}
	else if (direction == 2) // NORTH
	{
		return IntVec2::STEP_NORTH;
	}
	else if (direction == 3) // SOUTH
	{
		return IntVec2::STEP_SOUTH;
	}

	return IntVec2::ZERO;
}


void Map::CreateMap()
{
	m_tiles.reserve(m_dimensions.x * m_dimensions.y);
	for (int rowIndex = 0; rowIndex < m_dimensions.y; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < m_dimensions.x; columnIndex++)
		{
			Tile newTile;

			newTile.SetTileType(m_mapDef.m_fillTileType);

			if (columnIndex == 0 || rowIndex == 0 || columnIndex == m_dimensions.x - 1 || rowIndex == m_dimensions.y - 1)
			{
				newTile.SetTileType(m_mapDef.m_wallTileType);
			}
			newTile.m_tileCoords = IntVec2(columnIndex, rowIndex);
			m_tiles.push_back(newTile);
		}
	}

	for (int wormIndex = 0; wormIndex < m_mapDef.m_worm1Num; wormIndex++)
	{
		IntVec2 curPos = RollSpawnLocationWithinWall();
		Tile* tile = GetTileByCoordinates(curPos);
		tile->SetTileType(m_mapDef.m_worm1TileType);

		for (int wormLength = 0; wormLength < m_mapDef.m_worm1Length - 1; wormLength++)
		{
			IntVec2 nextPos = RollRandomDirection() + curPos;
			if (nextPos.x > 0 && nextPos.y > 0 && nextPos.x < m_dimensions.x - 1 && nextPos.y < m_dimensions.y - 1)
			{
				Tile* next = GetTileByCoordinates(nextPos);
				next->SetTileType(m_mapDef.m_worm1TileType);
				curPos = nextPos;
			}
		}
	}

	for (int wormIndex = 0; wormIndex < m_mapDef.m_worm2Num; wormIndex++)
	{
		IntVec2 curPos = RollSpawnLocationWithinWall();
		Tile* tile = GetTileByCoordinates(curPos);
		tile->SetTileType(m_mapDef.m_worm2TileType);

		for (int wormLength = 0; wormLength < m_mapDef.m_worm2Length - 1; wormLength++)
		{
			IntVec2 nextPos = RollRandomDirection() + curPos;
			if (nextPos.x > 0 && nextPos.y > 0 && nextPos.x < m_dimensions.x - 1 && nextPos.y < m_dimensions.y - 1)
			{
				Tile* next = GetTileByCoordinates(nextPos);
				next->SetTileType(m_mapDef.m_worm2TileType);
				curPos = nextPos;
			}
		}
	}

	for (int wormIndex = 0; wormIndex < m_mapDef.m_worm3Num; wormIndex++)
	{
		IntVec2 curPos = RollSpawnLocationWithinWall();
		Tile* tile = GetTileByCoordinates(curPos);
		tile->SetTileType(m_mapDef.m_worm3TileType);

		for (int wormLength = 0; wormLength < m_mapDef.m_worm3Length - 1; wormLength++)
		{
			IntVec2 nextPos = RollRandomDirection() + curPos;
			if (nextPos.x > 0 && nextPos.y > 0 && nextPos.x < m_dimensions.x - 1 && nextPos.y < m_dimensions.y - 1)
			{
				Tile* next = GetTileByCoordinates(nextPos);
				next->SetTileType(m_mapDef.m_worm3TileType);
				curPos = nextPos;
			}
		}
	}

	GenerateMapImage();
	GenerateSpawn();
	GenerateGoal();
}


void Map::GenerateMapImage()
{
	if (m_mapDef.m_mapImageName == "") return;

	Image* mapImage = new Image(m_mapDef.m_mapImageName.c_str());
	IntVec2 mapImageDimension = mapImage->GetDimensions();
	if (mapImageDimension.x >= m_dimensions.x || mapImageDimension.y >= m_dimensions.y)
	{
		ERROR_AND_DIE("Map image dimensions need to be smaller than map dimensions");
	}

	IntVec2 mapImageOffset = m_mapDef.m_mapImageOffset;
	for (int rowIndex = 0; rowIndex < mapImageDimension.y; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < mapImageDimension.x; columnIndex++)
		{
			IntVec2 currentCoords = IntVec2(columnIndex, rowIndex);
			Rgba8 color = mapImage->GetTexelColor(currentCoords);
			if (color.a == 0)
			{
				continue;
			}
			else if (color.a == 255)
			{
				Tile* tile = GetTileByCoordinates(currentCoords + mapImageOffset);
				for (int i = 0; i < int(TileDefinition::s_tileDefs.size()); i++)
				{
					TileDefinition& tileDef = TileDefinition::s_tileDefs[i];
					if (color == tileDef.m_mapImageColor)
					{
						tile->SetTileType(tileDef.m_name);
						break;
					}
				}
			}
			else
			{
				int result = RNG.RollRandomIntInRange(0, 254);
				if (result < color.a)
				{
					Tile* tile = GetTileByCoordinates(currentCoords + mapImageOffset);
					for (int i = 0; i < int(TileDefinition::s_tileDefs.size()); i++)
					{
						TileDefinition& tileDef = TileDefinition::s_tileDefs[i];
						if (color == tileDef.m_mapImageColor)
						{
							tile->SetTileType(tileDef.m_name);
							break;
						}
					}
				}
			}
		}
	}
}


void Map::GenerateSpawn()
{
	for (int rowIndex = 1; rowIndex < 6; rowIndex++)
	{
		for (int columnIndex = 1; columnIndex < 6; columnIndex++)
		{
			GetTileByCoordinates(IntVec2(columnIndex, rowIndex))->SetTileType(m_mapDef.m_startFloorTileType);
		}
	}

	GetTileByCoordinates(IntVec2(4, 2))->SetTileType(m_mapDef.m_startBunkerTileType);
	GetTileByCoordinates(IntVec2(4, 3))->SetTileType(m_mapDef.m_startBunkerTileType);
	GetTileByCoordinates(IntVec2(4, 4))->SetTileType(m_mapDef.m_startBunkerTileType);
	GetTileByCoordinates(IntVec2(3, 4))->SetTileType(m_mapDef.m_startBunkerTileType);
	GetTileByCoordinates(IntVec2(2, 4))->SetTileType(m_mapDef.m_startBunkerTileType);
	GetTileByCoordinates(IntVec2(1, 1))->SetTileType("MapEntry");
}


void Map::GenerateGoal()
{
	for (int rowIndex = m_dimensions.y - 7; rowIndex < m_dimensions.y - 1; rowIndex++)
	{
		for (int columnIndex = m_dimensions.x - 7; columnIndex < m_dimensions.x - 1; columnIndex++)
		{
			GetTileByCoordinates(IntVec2(columnIndex, rowIndex))->SetTileType(m_mapDef.m_endFloorTileType);
		}
	}

	GetTileByCoordinates(IntVec2(m_dimensions.x - 3, m_dimensions.y - 6))->SetTileType(m_mapDef.m_endBunkerTileType);
	GetTileByCoordinates(IntVec2(m_dimensions.x - 4, m_dimensions.y - 6))->SetTileType(m_mapDef.m_endBunkerTileType);
	GetTileByCoordinates(IntVec2(m_dimensions.x - 5, m_dimensions.y - 6))->SetTileType(m_mapDef.m_endBunkerTileType);
	GetTileByCoordinates(IntVec2(m_dimensions.x - 6, m_dimensions.y - 6))->SetTileType(m_mapDef.m_endBunkerTileType);
	GetTileByCoordinates(IntVec2(m_dimensions.x - 6, m_dimensions.y - 5))->SetTileType(m_mapDef.m_endBunkerTileType);
	GetTileByCoordinates(IntVec2(m_dimensions.x - 6, m_dimensions.y - 4))->SetTileType(m_mapDef.m_endBunkerTileType);
	GetTileByCoordinates(IntVec2(m_dimensions.x - 6, m_dimensions.y - 3))->SetTileType(m_mapDef.m_endBunkerTileType);
	GetTileByCoordinates(IntVec2(m_dimensions.x - 2, m_dimensions.y - 2))->SetTileType("MapExit");
}


void Map::SpawnEnemies()
{
	for (int scorpioIndex = 0; scorpioIndex < m_mapDef.m_enemyCounts[ENEMY_TYPE_SCORPIO]; scorpioIndex++)
	{
		IntVec2 tileToSpawn = RollSpawnLocation(true);
		Vec2 posToSpawn = Vec2(tileToSpawn) + Vec2(0.5f, 0.5f);
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		Entity* newScorpio = CreateEntityOfType(EntityType::ENTITY_TYPE_EVIL_SCORPIO, posToSpawn, randomOrientation);
		AddEntityToMap(*newScorpio);
	}

	for (int leoIndex = 0; leoIndex < m_mapDef.m_enemyCounts[ENEMY_TYPE_LEO]; leoIndex++)
	{
		IntVec2 tileToSpawn = RollSpawnLocation(true);
		Vec2 posToSpawn = Vec2(tileToSpawn) + Vec2(0.5f, 0.5f);
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		Entity* newLeo = CreateEntityOfType(EntityType::ENTITY_TYPE_EVIL_LEO, posToSpawn, randomOrientation);
		AddEntityToMap(*newLeo);
	}

	for (int ariesIndex = 0; ariesIndex < m_mapDef.m_enemyCounts[ENEMY_TYPE_ARIES]; ariesIndex++)
	{
		IntVec2 tileToSpawn = RollSpawnLocation(true);
		Vec2 posToSpawn = Vec2(tileToSpawn) + Vec2(0.5f, 0.5f);
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		Entity* newAries = CreateEntityOfType(EntityType::ENTITY_TYPE_EVIL_ARIES, posToSpawn, randomOrientation);
		AddEntityToMap(*newAries);
	}

	for (int capricornIndex = 0; capricornIndex < m_mapDef.m_enemyCounts[ENEMY_TYPE_CAPRICORN]; capricornIndex++)
	{
		IntVec2 tileToSpawn = RollSpawnLocation(false);
		Vec2 posToSpawn = Vec2(tileToSpawn) + Vec2(0.5f, 0.5f);
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		Entity* newCapricorn = CreateEntityOfType(EntityType::ENTITY_TYPE_EVIL_CAPRICORN, posToSpawn, randomOrientation);
		AddEntityToMap(*newCapricorn);
	}
}


