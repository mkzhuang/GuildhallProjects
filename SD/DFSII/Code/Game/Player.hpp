#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Action.hpp"

#include <map>
#include <deque>

class Map;
class SpriteAnimDefinition;

enum class PlayerAnim
{
	MOVE_EAST,
	MOVE_WEST,
	MOVE_NORTH,
	MOVE_SOUTH,
};

class Player
{
public:
	Player(Map* map, Vec2 const& position, float speed);
	~Player() {}

	void Update(float deltaSeconds);
	void Render() const;

	void HandleInput();
	void UpdateTransform(float deltaSeconds);
	void ExecutePlan(float deltaSeconds);

	void CreateRandomGoal();

public:
	Map* m_map = nullptr;
	Vec2 m_position = Vec2::ZERO;
	float m_speed = 0.f;
	Vec2 m_velocity = Vec2::ZERO;
	Vec2 m_movementInput = Vec2::ZERO;
	AABB2 m_spriteUV = AABB2::ZERO_TO_ONE;
	std::map<PlayerAnim, SpriteAnimDefinition*> m_animations;
	PlayerAnim m_currentAnimation = PlayerAnim::MOVE_SOUTH;
	float m_animationTimer = 0.f;
	GameState m_currentGoal;
	std::vector<GameState> m_currentStates;
	std::deque<Action> m_currentActions;
	float m_actionTimer = 0.f;
	std::deque<Vec2> m_wayPoints;

	int m_golds = 0;
	std::vector<std::string> m_items;

	float m_currentHealth = 100.f;
	float m_maxHealth = 100.f;
};