#pragma once
#include "Engine/Math/Vec2.hpp"

#include <string>

struct GameState
{
public:
	GameState() {}
	~GameState() {}

public:
	std::string key = "";
	int value = 0;
	
	// item related
	std::string item = "";	
};


