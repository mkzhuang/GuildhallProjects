#pragma once
#include "Game/GameCommon.hpp"

class ActorDefinition;

//------------------------------------------------------------------------------------------------
class SpawnInfo
{
public:
	SpawnInfo();
	SpawnInfo( ActorDefinition const* definition, Vec3 const& position = Vec3::ZERO, EulerAngles const& orientation = EulerAngles::ZERO, Vec3 const& velocity = Vec3::ZERO );
	SpawnInfo( char const* definitionName, Vec3 const& position = Vec3::ZERO, EulerAngles const& orientation = EulerAngles::ZERO, Vec3 const& velocity = Vec3::ZERO );

	bool LoadFromXmlElement( XmlElement const& element );

	const ActorDefinition* m_definition = nullptr;
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
};


