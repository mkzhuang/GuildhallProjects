#pragma once
#include "Game/Tile.hpp"
#include "Game/GameState.hpp"
#include "Game/GameInfo.hpp"
#include "Game/Character.hpp"

#include <vector>
#include <deque>

constexpr float REAL_TIME_RATIO = 200.f;
constexpr float DAYS_PER_SECOND = 1.f / (60.f * 60.f * 24.f);
constexpr float TIME_NIGHT_ACCELERATE_SCALE = 30.f;
constexpr float TIME_DAY_ACCELERATE_SCALE = 5.f;

class Game;
class Image;
class Player;
class Planner;
class Camera;

struct AStarNode
{
	IntVec2 coordinate = IntVec2::ZERO;
	AStarNode* parent = nullptr;

	float cost = 0;
	float f = 0;
	float g = 0;
	float h = 0;
};


class Map
{
	friend class Player;
	friend class Planner;

public:
	Map(Game* game, Camera* worldCamera, Camera* uiCamera);
	~Map();

	void Startup();
	void Update(float deltaSeconds);
	void Render() const;

private:
	void UpdatePlayer(float deltaSeconds);
	void UpdateCharacters(float deltaSeconds);
	void UpdateMap(float deltaSeconds);
	void UpdateCamera(float deltaSeconds);
	void UpdateGameInfo();

	void RenderMap() const;
	void RenderDebugPath() const;
	void RenderCharacters() const;
	void RenderPlayer() const;

	void GenerateMap(Image* fromImage = nullptr);
	Character const* AddCharacter(std::string const& name);
	Character const* FindCharacter(std::string const& name, int characterIndex);
	void RemoveCharacter(std::string const& name, int characterIndex);
	Character const* FindCharacterWithItem(std::string const& itemName);
	IntVec2 GetCoordinateForPosition(Vec2 const& position);
	int GetTileIndexForCoordinate(IntVec2 const& coordinate);
	int GetTileIndexForPosition(Vec2 const& position);
	Tile const& GetRandomFreeTile(std::vector<Tile> const& tiles);
	Tile const& GetTileForCoordinate(IntVec2 const& coordinate);
	Tile const& GetTileForPosition(Vec2 const& position);
	bool IsCoordinateInBound(IntVec2 const& coordinate);
	float GetDistanceBetweenCoodinates(IntVec2 const& from, IntVec2 const& to);
	std::deque<Vec2> GetPathBetweenPositions(Vec2 const& start, Vec2 const& end);


public:
	Game* m_game = nullptr;
	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	Player* m_player = nullptr;
	std::vector<Character*> m_characters;
	Planner* m_planner = nullptr;
	IntVec2 m_mapDimensions = IntVec2::ZERO;
	std::vector<Tile> m_tiles;
	std::vector<Vertex_PCU> m_mapVerts;
	float m_worldDay = 0.25f;
	int m_characterCounter = 0;
	bool m_isDebugDrawCost = false;
	bool m_isDebugDrawPath = false;

	GameInfo* m_gameInfo = nullptr;
};


