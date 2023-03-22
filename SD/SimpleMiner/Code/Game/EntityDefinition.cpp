#include "Game/EntityDefinition.hpp"

std::vector<EntityDefinition*> EntityDefinition::s_definitions;

bool EntityDefinition::LoadFromXmlElement(XmlElement const& element)
{
	UNUSED(element)
	return false;
}


void EntityDefinition::InitializeDefinitions()
{
	CreateNewEntityDef("player", AABB3(Vec3(-0.3f, -0.3f, 0.f), Vec3(0.3f, 0.3f, 1.8f)), 36.f, 9.f, 1.65f);
}


EntityDefinition const* EntityDefinition::GetByName(std::string const& name)
{
	for (int entityDefIndex = 0; entityDefIndex < (int)s_definitions.size(); entityDefIndex++)
	{
		EntityDefinition*& entityDef = EntityDefinition::s_definitions[entityDefIndex];
		if (entityDef && entityDef->m_name == name)
		{
			return entityDef;
		}
	}

	ERROR_AND_DIE("Error: invalid entity definition name.");
}


void EntityDefinition::CreateNewEntityDef(std::string const& name, AABB3 const& bounds, float speed, float drag, float eyeHeight)
{
	EntityDefinition* newEntityDef = new EntityDefinition();
	newEntityDef->m_name = name;
	newEntityDef->m_bounds = bounds;
	newEntityDef->m_speed = speed;
	newEntityDef->m_drag = drag;
	newEntityDef->m_eyeHeight = eyeHeight;
	EntityDefinition::s_definitions.push_back(newEntityDef);
}


