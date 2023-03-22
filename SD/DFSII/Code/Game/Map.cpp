#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/Planner.hpp"
#include "Engine/Core/EventSystem.hpp"

Map::Map(Game* game, Camera* worldCamera, Camera* uiCamera)
	: m_game(game)
	, m_worldCamera(worldCamera)
	, m_uiCamera(uiCamera)
{
}


Map::~Map()
{
	delete m_gameInfo;
}


void Map::Startup()
{
	m_player = new Player(this, Vec2(10.5f, 10.5f), 5.f);
	m_planner = new Planner();
	m_planner->InitializePossibleActions("Data/PlayerActions.xml");
	Image* mapImage = new Image("Data/Images/DFSII/Map.png");
	GenerateMap(mapImage);
	m_gameInfo = new GameInfo(g_theRenderer, m_uiCamera->GetOrthoDimensions());
	EventArgs args;
	args.SetValue("name", "Player");
	CallHandler("updatePlayerName", args);

	AddCharacter("villagerA");
	AddCharacter("villagerB");
	AddCharacter("villagerC");
	AddCharacter("villagerD");
}


void Map::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed('U'))
	{
		m_isDebugDrawCost = !m_isDebugDrawCost;
	}
	if (g_theInput->WasKeyJustPressed('I'))
	{
		m_isDebugDrawPath = !m_isDebugDrawPath;
	}

	UpdatePlayer(deltaSeconds);
	UpdateCharacters(deltaSeconds);
	UpdateMap(deltaSeconds);
	UpdateCamera(deltaSeconds);
	UpdateGameInfo();
}


void Map::Render() const
{
	g_theRenderer->BeginCamera(*m_worldCamera);
	g_theRenderer->ClearScreen(Rgba8::CLEAR);

	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/DFSII/Assets.png");
	g_theRenderer->BindTexture(texture);
	RenderMap();
	RenderDebugPath();
	RenderCharacters();
	RenderPlayer();
	g_theRenderer->EndCamera(*m_worldCamera);

	g_theRenderer->BeginCamera(*m_uiCamera);
	m_gameInfo->Render();
	g_theRenderer->EndCamera(*m_uiCamera);
}


void Map::UpdatePlayer(float deltaSeconds)
{
	if (m_player->m_currentActions.empty())
	{
		m_player->CreateRandomGoal();
		std::vector<GameState> states;
		for (std::string const& item : m_player->m_items)
		{
			GameState state;
			state.key = "hasItem";
			state.item = item;
			if (item == "potion") states.push_back(state);
		}
		GameState state;
		state.key = "hasItem";
		state.item = "gold";
		state.value = m_player->m_golds;
		if (state.value > 0)
		{
			state.value = state.value;
		}
		states.push_back(state);
		m_planner->MakePlan(this, states, m_player->m_currentGoal, m_player->m_currentActions);
	}

	m_player->Update(deltaSeconds);
}


void Map::UpdateCharacters(float deltaSeconds)
{
	for (Character* c : m_characters)
	{
		c->Update(deltaSeconds);
	}
}


void Map::UpdateMap(float deltaSeconds)
{
	float dayFraction = fmodf(m_worldDay, 1.f);
	if (dayFraction < 0.25 || dayFraction > 0.9)
	{
		m_worldDay += deltaSeconds * REAL_TIME_RATIO * DAYS_PER_SECOND * TIME_NIGHT_ACCELERATE_SCALE;
	}
	else
	{
		m_worldDay += deltaSeconds * REAL_TIME_RATIO * DAYS_PER_SECOND * TIME_DAY_ACCELERATE_SCALE;
	}
}


void Map::UpdateCamera(float deltaSeconds)
{
	UNUSED(deltaSeconds)

	if (m_player)
	{
		Vec2 cameraCenter = m_player->m_position;
		Vec2 cameraFullSize = m_worldCamera->GetOrthoSize();
		Vec2 cameraHalfSize = 0.5f * cameraFullSize;
		Vec2 cameraMins = cameraCenter - cameraHalfSize;
		Vec2 cameraMaxs = cameraCenter + cameraHalfSize;
		m_worldCamera->SetOrthoView(cameraMins, cameraMaxs);
		if (cameraMins.x < 0.f)							m_worldCamera->Translate(Vec2(0.f - cameraMins.x, 0.f));
		if (cameraMins.y < 0.f)							m_worldCamera->Translate(Vec2(0.f, 0.f - cameraMins.y));
		if (cameraMaxs.x > (float)m_mapDimensions.x)	m_worldCamera->Translate(Vec2((float)m_mapDimensions.x - cameraMaxs.x, 0.f));
		if (cameraMaxs.y > (float)m_mapDimensions.y)	m_worldCamera->Translate(Vec2(0.f, (float)m_mapDimensions.y - cameraMaxs.y));
	}
}


void Map::UpdateGameInfo()
{
	EventArgs statusArgs;
	statusArgs.SetValue("worldDay", m_worldDay);
	statusArgs.SetValue("currentHealth", m_player->m_currentHealth);
	statusArgs.SetValue("maxHealth", m_player->m_maxHealth);
	std::string goal = m_player->m_currentGoal.item;
	statusArgs.SetValue("goal", goal);
	std::string action = m_player->m_currentActions.empty() ? "" : m_player->m_currentActions.front().type;
	statusArgs.SetValue("action", action);

	CallHandler("updateStatus", statusArgs);

	EventArgs itemsArgs;
	itemsArgs.SetValue("gold", m_player->m_golds);
	int itemSize = (int)m_player->m_items.size();
	if (itemSize > 0)
	{
		std::string item = m_player->m_items[0];
		for (int index = 1; index < (int)m_player->m_items.size(); index++)
		{
			item += " " + m_player->m_items[index];
		}
		itemsArgs.SetValue("items", item);
	}

	CallHandler("updateItems", itemsArgs);

	m_gameInfo->Update();
}


void Map::RenderMap() const
{
	float dayFraction = fmodf(m_worldDay, 1.f);
	float tintValue = 1.f;
	if (dayFraction < 0.5f)
	{
		tintValue = RangeMapClamped(dayFraction, 0.15f, 0.35f, 0.f, 1.f);
	}
	else
	{
		tintValue = RangeMapClamped(dayFraction, 0.65f, 0.85f, 1.f, 0.f);
	}

	Rgba8 tintColor = InterpolateBetweenColor(Rgba8(50, 50, 80, 255), Rgba8(255, 255, 255, 255), tintValue);

	std::vector<Vertex_PCU> verts;
	verts.reserve(6 * m_tiles.size());
	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); tileIndex++)
	{
		Tile const& currentTile = m_tiles[tileIndex];
		Vec2 mins(currentTile.m_tileCoordinates);
		Vec2 maxs = mins + Vec2::ONE;
		AABB2 tileBox(mins, maxs);
		AABB2 baseUVs = currentTile.m_tileDef->m_baseUVs;
		AABB2 topUVs = currentTile.m_tileDef->m_topUVs;
		AddVertsForAABB2D(verts, tileBox, tintColor, baseUVs);
		AddVertsForAABB2D(verts, tileBox, tintColor, topUVs);
	}

	g_theRenderer->DrawVertexArray(verts);
}


void Map::RenderDebugPath() const
{
	if (!m_isDebugDrawCost && !m_isDebugDrawPath) return;
	if (m_isDebugDrawCost)
	{
		// whether the tile is passable
		std::vector<Vertex_PCU> solidVerts;
		solidVerts.reserve(6 * m_tiles.size());
		for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); tileIndex++)
		{
			Tile const& currentTile = m_tiles[tileIndex];
			Vec2 mins(currentTile.m_tileCoordinates);
			Vec2 maxs = mins + Vec2::ONE;
			AABB2 tileBox(mins, maxs);
			Rgba8 color = currentTile.m_tileDef->m_isObstacle ? Rgba8(50, 50, 50, 150) : Rgba8::CLEAR;
			AddVertsForAABB2D(solidVerts, tileBox, color);
		}
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(solidVerts);

		//cost for each cell
		BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/InputMonoRegular_44x_64p_aa");
		std::vector<Vertex_PCU> costVerts;
		costVerts.reserve(6 * m_tiles.size());
		for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); tileIndex++)
		{
			Tile const& currentTile = m_tiles[tileIndex];
			Vec2 mins(currentTile.m_tileCoordinates);
			Vec2 maxs = mins + Vec2::ONE;
			AABB2 tileBox(mins, maxs);
			std::string cost = Stringf("%.1f", currentTile.m_tileDef->m_cost);
			font->AddVertsForTextInBox2D(costVerts, tileBox, 0.3f, cost);
		}
		g_theRenderer->BindTexture(&font->GetTexture());
		g_theRenderer->DrawVertexArray(costVerts);
	}

	//path to destination
	if (m_isDebugDrawPath)
	{
		std::deque<Vec2> const& waypoints = m_player->m_wayPoints;

		std::vector<Vertex_PCU> waypointVerts;
		waypointVerts.reserve(6 * waypoints.size());
		for (Vec2 const& p : waypoints)
		{
			AABB2 box(p + Vec2(-0.5f, -0.5f), p + Vec2(0.5f, 0.5f));
			AddVertsForAABB2D(waypointVerts, box, Rgba8(255, 100, 100, 100));
		}
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(waypointVerts);
	}

	//rebind asset texture
	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/DFSII/Assets.png");
	g_theRenderer->BindTexture(texture);
}


void Map::RenderCharacters() const
{
	for (Character* c : m_characters)
	{
		c->Render();
	}
}

void Map::RenderPlayer() const
{
	m_player->Render();
}


void Map::GenerateMap(Image* fromImage)
{
	if (!fromImage)
	{
		m_tiles.reserve(1000);
		for (int rowIndex = 0; rowIndex < 50; rowIndex++)
		{
			for (int columnIndex = 0; columnIndex < 20; columnIndex++)
			{
				Tile newTile;
				newTile.SetTile("grass");
				newTile.SetCoordinate(IntVec2(columnIndex, rowIndex));
				m_tiles.push_back(newTile);
			}
		}
		return;
	}

	m_mapDimensions = fromImage->GetDimensions();
	m_tiles.reserve((size_t)(m_mapDimensions.x) * (size_t)(m_mapDimensions.y));
	for (int rowIndex = 0; rowIndex < m_mapDimensions.y; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < m_mapDimensions.x; columnIndex++)
		{
			IntVec2 currentCoord(columnIndex, rowIndex);
			Rgba8 color = fromImage->GetTexelColor(currentCoord);
			TileDefinition const& tileDef = TileDefinition::GetDefinitionByColor(color);
			Tile newTile;
			newTile.SetCoordinate(currentCoord);
			newTile.SetTileDefinition(&tileDef);
			m_tiles.push_back(newTile);
		}
	}
}


Character const* Map::AddCharacter(std::string const& name)
{
	CharacterDefinition const& characterDef = CharacterDefinition::GetDefinitionByName(name);
	Character* character = new Character(this, &characterDef);
	std::vector<Tile> possibleSpawns;
	for (int index = 0; index < (int)m_tiles.size(); index++)
	{
		Tile const& tile = m_tiles[index];
		if (tile.m_tileDef->m_name == characterDef.m_spawnTileType)
		{
			possibleSpawns.push_back(tile);
		}
	}
	Tile const& randomTile = GetRandomFreeTile(possibleSpawns);
	character->m_position = Vec2(randomTile.m_tileCoordinates) + Vec2(0.5f, 0.5f);
	character->m_characterIndex = m_characterCounter;
	m_characterCounter++;

	m_characters.push_back(character);

	return character;
}


Character const* Map::FindCharacter(std::string const& name, int characterIndex)
{
	for (Character* c : m_characters)
	{
		if (c->m_characterDef->m_name == name && c->m_characterIndex == characterIndex)
		{
			return c;
		}
	}

	return nullptr;
}


void Map::RemoveCharacter(std::string const& name, int characterIndex)
{
	for (int index = 0; index < (int)m_characters.size(); index++)
	{
		Character*& c = m_characters[index];
		if (c->m_characterDef->m_name == name && c->m_characterIndex == characterIndex)
		{
			delete c;
			c = nullptr;
			m_characters.erase(m_characters.begin() + index);
			return;
		}
	}
}


Character const* Map::FindCharacterWithItem(std::string const& itemName)
{
	for (Character* c : m_characters)
	{
		if (c->m_characterDef->m_item == itemName)
		{
			return c;
		}
	}

	return nullptr;
}


IntVec2 Map::GetCoordinateForPosition(Vec2 const& position)
{
	return IntVec2(RoundDownToInt(position.x), RoundDownToInt(position.y));
}


int Map::GetTileIndexForCoordinate(IntVec2 const& coordinate)
{
	return coordinate.x + m_mapDimensions.x * coordinate.y;
}


int Map::GetTileIndexForPosition(Vec2 const& position)
{
	return GetTileIndexForCoordinate(GetCoordinateForPosition(position));
}


Tile const& Map::GetRandomFreeTile(std::vector<Tile> const& tiles)
{
	int randomTileIndex = RNG.RollRandomIntInRange(0, (int)tiles.size() - 1);
	Tile const& randomTile = tiles[randomTileIndex];
	for (Character const* c : m_characters)
	{
		if (GetCoordinateForPosition(c->m_position) == randomTile.m_tileCoordinates)
		{
			return GetRandomFreeTile(tiles);
		}
	}
	return randomTile;
}


Tile const& Map::GetTileForCoordinate(IntVec2 const& coordinate)
{
	return m_tiles[GetTileIndexForCoordinate(coordinate)];
}


Tile const& Map::GetTileForPosition(Vec2 const& position)
{
	return m_tiles[GetTileIndexForPosition(position)];
}


bool Map::IsCoordinateInBound(IntVec2 const& coordinate)
{
	if (coordinate.x < 0 || coordinate.x >= m_mapDimensions.x) return false;
	if (coordinate.y < 0 || coordinate.y >= m_mapDimensions.y) return false;
	return true;
}


float Map::GetDistanceBetweenCoodinates(IntVec2 const& from, IntVec2 const& to)
{
	return (from - to).GetLength();
}


std::deque<Vec2> Map::GetPathBetweenPositions(Vec2 const& start, Vec2 const& end)
{
	AStarNode* startNode = new AStarNode();
	startNode->coordinate = GetCoordinateForPosition(start);
	startNode->cost = GetTileForCoordinate(startNode->coordinate).m_tileDef->m_cost;
	AStarNode* endNode = new AStarNode();
	endNode->coordinate = GetCoordinateForPosition(end);
	endNode->cost = GetTileForCoordinate(endNode->coordinate).m_tileDef->m_cost;

	std::vector<AStarNode*> openList;
	std::vector<AStarNode*> closedList;

	openList.push_back(startNode);

	while (!openList.empty())
	{
		AStarNode* currentNode = openList[0];
		int currentIndex = 0;
		for (int index = 1; index < (int)openList.size(); index++)
		{
			AStarNode* node = openList[index];
			if (node->f < currentNode->f)
			{
				currentNode = node;
				currentIndex = index;
			}
		}

		openList.erase(openList.begin() + currentIndex);
		closedList.push_back(currentNode);

		if (currentNode->coordinate == endNode->coordinate)
		{
			std::deque<Vec2> path;
			AStarNode* current = currentNode;
			while (current)
			{
				path.push_front(Vec2(current->coordinate) + Vec2(0.5f, 0.5f));
				current = current->parent;
			}

			for (AStarNode* node : openList)
			{
				delete node;
			}
			openList.clear();
			for (AStarNode* node : closedList)
			{
				delete node;
			}
			closedList.clear();

			return path;
		}

		std::vector<AStarNode*> children;
		if (IsCoordinateInBound(currentNode->coordinate + IntVec2::STEP_NORTH))
		{
			Tile const& northTile = GetTileForCoordinate(currentNode->coordinate + IntVec2::STEP_NORTH);
			if (!northTile.m_tileDef->m_isObstacle)
			{
				AStarNode* child = new AStarNode();
				child->coordinate = northTile.m_tileCoordinates;
				child->cost = northTile.m_tileDef->m_cost;
				child->parent = currentNode;
				children.push_back(child);
			}
		}

		if (IsCoordinateInBound(currentNode->coordinate + IntVec2::STEP_SOUTH))
		{
			Tile const& southTile = GetTileForCoordinate(currentNode->coordinate + IntVec2::STEP_SOUTH);
			if (!southTile.m_tileDef->m_isObstacle)
			{
				AStarNode* child = new AStarNode();
				child->coordinate = southTile.m_tileCoordinates;
				child->cost = southTile.m_tileDef->m_cost;
				child->parent = currentNode;
				children.push_back(child);
			}
		}

		if (IsCoordinateInBound(currentNode->coordinate + IntVec2::STEP_EAST))
		{
			Tile const& eastTile = GetTileForCoordinate(currentNode->coordinate + IntVec2::STEP_EAST);
			if (!eastTile.m_tileDef->m_isObstacle)
			{
				AStarNode* child = new AStarNode();
				child->coordinate = eastTile.m_tileCoordinates;
				child->cost = eastTile.m_tileDef->m_cost;
				child->parent = currentNode;
				children.push_back(child);
			}
		}

		if (IsCoordinateInBound(currentNode->coordinate + IntVec2::STEP_WEST))
		{
			Tile const& westTile = GetTileForCoordinate(currentNode->coordinate + IntVec2::STEP_WEST);
			if (!westTile.m_tileDef->m_isObstacle)
			{
				AStarNode* child = new AStarNode();
				child->coordinate = westTile.m_tileCoordinates;
				child->cost = westTile.m_tileDef->m_cost;
				child->parent = currentNode;
				children.push_back(child);
			}
		}

		for (AStarNode* child : children)
		{
			bool isInCloseList = false;
			for (AStarNode* node : closedList)
			{
				if (node->coordinate == child->coordinate)
				{
					isInCloseList = true;
					break;
				}
			}
			if (isInCloseList) continue;

			child->g = currentNode->g + child->cost;
			child->h = GetDistanceBetweenCoodinates(endNode->coordinate, child->coordinate);
			child->f = child->g + child->h;

			bool isInOpenList = false;
			for (AStarNode* node : openList)
			{
				if (node->coordinate == child->coordinate)
				{
					if (child->g < node->g)
					{
						node->g = child->g;
						node->h = child->h;
						node->f = child->f;
						node->parent = child->parent;
					}
					delete child;
					isInOpenList = true;
					break;
				}
			}

			if (isInOpenList) continue;

			openList.push_back(child);
		}
	}

	return std::deque<Vec2>();
}


