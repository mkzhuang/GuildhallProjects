#pragma once
#include "Game/GameCommon.hpp"

class EntityDefinition
{
public:
	bool LoadFromXmlElement(XmlElement const& element);

public:
	std::string m_name;
	AABB3 m_bounds;
	float m_speed = 0.f;
	float m_drag = 0.f;
	float m_eyeHeight = 0.f;

	static void InitializeDefinitions();
	static EntityDefinition const* GetByName(std::string const& name);
	static void CreateNewEntityDef(std::string const& m_name, AABB3 const& bounds, float speed, float drag, float eyeHeight);
	static std::vector<EntityDefinition*> s_definitions;
};