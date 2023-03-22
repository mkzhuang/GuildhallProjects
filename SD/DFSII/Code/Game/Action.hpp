#pragma once
#include "Game/GameState.hpp"

#include <vector>

struct Action 
{
public:
	Action() {}
	~Action() {}

public:
	std::string type = "";
	GameState preCondition;
	GameState effect;
	
	// move to related
	std::string targetName = "";
	Vec2 targetLocation = Vec2::ZERO;

	// kill relate
	int targetIndex = 0;
};