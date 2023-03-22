#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Stopwatch.hpp"

class WeaponDefinition;
class Actor;
class Map;

class Weapon
{
public:
	Weapon(Clock const& clock, WeaponDefinition const* definition);
	~Weapon();

	bool Fire(Vec3 const& position, Vec3 const& forward, Actor* owner);
	Vec3 GetRandomDirectionInCone(Vec3 const& position, Vec3 const& forward, float angle) const;

	Map* m_map = nullptr;
	WeaponDefinition const* m_definition = nullptr;
	Stopwatch m_refireStopwatch;
};


