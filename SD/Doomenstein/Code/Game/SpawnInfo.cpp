#include "Game/SpawnInfo.hpp"
#include "Game/ActorDefinition.hpp"

SpawnInfo::SpawnInfo()
{
}


SpawnInfo::SpawnInfo(ActorDefinition const* definition, Vec3 const& position, EulerAngles const& orientation, Vec3 const& velocity)
	: m_definition(definition)
	, m_position(position)
	, m_orientation(orientation)
	, m_velocity(velocity)
{
}


SpawnInfo::SpawnInfo(char const* definitionName, Vec3 const& position, EulerAngles const& orientation, Vec3 const& velocity)
	: m_position(position)
	, m_orientation(orientation)
	, m_velocity(velocity)
{
	m_definition = ActorDefinition::GetByName(definitionName);
}


bool SpawnInfo::LoadFromXmlElement(XmlElement const& element)
{
	std::string actorName	= ParseXmlAttribute(element, "actor", "none");
	m_position				= ParseXmlAttribute(element, "position", m_position);
	m_orientation			= ParseXmlAttribute(element, "orientation", m_orientation);

	if (actorName != "none") m_definition = ActorDefinition::GetByName(actorName);
	return true;
}


