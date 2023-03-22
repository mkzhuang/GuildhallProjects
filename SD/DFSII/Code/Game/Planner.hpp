#pragma once
#include "Game/Action.hpp"
#include "Game/GameState.hpp"

#include <deque>

class Map;

class Planner
{
public:
	Planner() {}
	~Planner() {}

	void InitializePossibleActions(std::string const& xmlFilePath);

	void MakePlan(Map* map, std::vector<GameState> const& currentState, GameState const& targetState, std::deque<Action>& actions);

public:
	std::vector<Action> m_possibleActions;
};