#include "Game/ActorDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "SpriteAnimationGroupDefinition.hpp"

std::vector<ActorDefinition*> ActorDefinition::s_definitions;

bool ActorDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name					= ParseXmlAttribute(element, "name", "none");
	std::string factionName = ParseXmlAttribute(element, "faction", "none");
	m_health				= ParseXmlAttribute(element, "health", m_health);
	m_dieOnSpawn			= ParseXmlAttribute(element, "dieOnSpawn", m_dieOnSpawn);
	m_canBePossessed		= ParseXmlAttribute(element, "canBePossessed", m_canBePossessed);
	m_corpseLifetime		= ParseXmlAttribute(element, "corpseLifetime", m_corpseLifetime);
	if (factionName == "Neutral") m_faction = Faction::NEUTRAL;
	if (factionName == "Marine") m_faction = Faction::MARINE;
	if (factionName == "Demon") m_faction = Faction::DEMON;

	XmlElement const* child = element.FirstChildElement();
	while (child)
	{
		if (std::string(child->Name()) == "Collision")
		{
			m_physicsRadius			= ParseXmlAttribute(*child, "radius", m_physicsRadius);
			m_physicsHeight			= ParseXmlAttribute(*child, "height", m_physicsHeight);
			m_floatingHeight		= ParseXmlAttribute(*child, "floatingHeight", m_floatingHeight);
			m_collidesWithWorld		= ParseXmlAttribute(*child, "collidesWithWorld", m_collidesWithWorld);
			m_collidesWithActors	= ParseXmlAttribute(*child, "collidesWithActors", m_collidesWithActors);
			m_damageOnCollide		= ParseXmlAttribute(*child, "damageOnCollide", m_damageOnCollide);
			m_impulseOnCollide		= ParseXmlAttribute(*child, "impulseOnCollide", m_impulseOnCollide);
			m_dieOnCollide			= ParseXmlAttribute(*child, "dieOnCollide", m_dieOnCollide);
		}
		else if (std::string(child->Name()) == "Physics")
		{
			m_simulated				= ParseXmlAttribute(*child, "simulated", m_simulated);
			m_walkSpeed				= ParseXmlAttribute(*child, "walkSpeed", m_walkSpeed);
			m_runSpeed				= ParseXmlAttribute(*child, "runSpeed", m_runSpeed);
			m_turnSpeed				= ParseXmlAttribute(*child, "turnSpeed", m_turnSpeed);
			m_drag					= ParseXmlAttribute(*child, "drag", m_drag);
			m_flying				= ParseXmlAttribute(*child, "flying", m_flying);
		}
		else if (std::string(child->Name()) == "Camera")
		{
			m_eyeHeight				= ParseXmlAttribute(*child, "eyeHeight", m_eyeHeight);
			m_cameraFOVDegrees		= ParseXmlAttribute(*child, "cameraFOV", m_cameraFOVDegrees);
		}
		else if (std::string(child->Name()) == "AI")
		{
			m_aiEnabled				= ParseXmlAttribute(*child, "aiEnabled", m_aiEnabled);
			m_sightRadius			= ParseXmlAttribute(*child, "sightRadius", m_sightRadius);
			m_sightAngle			= ParseXmlAttribute(*child, "sightAngle", m_sightAngle);
			m_meleeDamage			= ParseXmlAttribute(*child, "meleeDamage", m_meleeDamage);
			m_meleeDelay			= ParseXmlAttribute(*child, "meleeDelay", m_meleeDelay);
			m_meleeRange			= ParseXmlAttribute(*child, "meleeRange", m_meleeRange);
			m_projectileDefName		= ParseXmlAttribute(*child, "projectileName", m_projectileDefName);
		}
		else if (std::string(child->Name()) == "Inventory")
		{
			XmlElement const* weapon = child->FirstChildElement();
			while (weapon)
			{
				if (std::string(weapon->Name()) == "Weapon")
				{
					std::string weaponName = ParseXmlAttribute(*weapon, "name", "none");
					if (weaponName != "none") m_weaponDefinitions.push_back(WeaponDefinition::GetByName(weaponName));
					weapon = weapon->NextSiblingElement();
				}
			}
		}
		else if (std::string(child->Name()) == "Appearance")
		{
			m_spriteSize			= ParseXmlAttribute(*child, "size", m_spriteSize);
			m_spritePivot			= ParseXmlAttribute(*child, "pivot", m_spritePivot);
			std::string billboard	= ParseXmlAttribute(*child, "billboardType", "none");
			m_renderDepth			= ParseXmlAttribute(*child, "renderDepth", m_renderDepth);
			m_renderLit				= ParseXmlAttribute(*child, "renderLit", m_renderLit);
			m_renderRounded			= ParseXmlAttribute(*child, "renderRoundedNormals", m_renderRounded);
			if (billboard == "None")	m_billboardType = BillboardType::NONE;
			if (billboard == "Facing")	m_billboardType = BillboardType::FACING;
			if (billboard == "Aligned") m_billboardType = BillboardType::ALIGNED;
			
			XmlElement const* animationGroup = child->FirstChildElement();
			while (animationGroup)
			{
				if (std::string(animationGroup->Name()) == "AnimationGroup")
				{
					SpriteAnimationGroupDefinition animationGroupDef;
					animationGroupDef.LoadFromXmlElement(*animationGroup);
					m_spriteAnimationGroupDefinitions.push_back(animationGroupDef);
					animationGroup = animationGroup->NextSiblingElement();
				}
			}
		}
		else if (std::string(child->Name()) == "Sounds")
		{
			XmlElement const* sound = child->FirstChildElement();
			while (sound)
			{
				if (std::string(sound->Name()) == "Sound")
				{
					std::string soundName = ParseXmlAttribute(*sound, "sound", "none");
					if (soundName == "Hurt")
					{
						m_hurtSoundName = ParseXmlAttribute(*sound, "name", m_hurtSoundName);
					}
					if (soundName == "Death")
					{
						m_deathSoundName = ParseXmlAttribute(*sound, "name", m_deathSoundName);
					}
					if (soundName == "Attack")
					{
						m_attackSoundName = ParseXmlAttribute(*sound, "name", m_attackSoundName);
					}
				}

				sound = sound->NextSiblingElement();
			}
		}

		child = child->NextSiblingElement();
	}
	return true;
}


SpriteAnimationGroupDefinition const* ActorDefinition::GetDefaultAnimationGroup() const
{
	if (m_spriteAnimationGroupDefinitions.size() == 0) return nullptr;

	return &m_spriteAnimationGroupDefinitions[0];
}


SpriteAnimationGroupDefinition const* ActorDefinition::GetAnimationGroup(std::string const& name) const
{
	for (int animationGroupIndex = 0; animationGroupIndex < (int)m_spriteAnimationGroupDefinitions.size(); animationGroupIndex++)
	{
		SpriteAnimationGroupDefinition const* animationGroupDef = &m_spriteAnimationGroupDefinitions[animationGroupIndex];

		if (animationGroupDef->m_name == name)
		{
			return animationGroupDef;
		}
	}

	return nullptr;
}


void ActorDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument doc;
	doc.LoadFile(path);
	XmlElement const* root = doc.RootElement();

	XmlElement const* element = root->FirstChildElement();
	while (element)
	{
		if (std::string(element->Name()) == "ActorDefinition")
		{
			ActorDefinition* newActorDef = new ActorDefinition();
			newActorDef->LoadFromXmlElement(*element);
			ActorDefinition::s_definitions.push_back(newActorDef);
			element = element->NextSiblingElement();
		}
	}
}


void ActorDefinition::ClearDefinitions()
{
	for (int actorDefIndex = 0; actorDefIndex < (int)s_definitions.size(); actorDefIndex++)
	{
		ActorDefinition*& actorDef = ActorDefinition::s_definitions[actorDefIndex];
		if (actorDef)
		{
			delete actorDef;
		}
	}

	s_definitions.clear();
}


const ActorDefinition* ActorDefinition::GetByName(const std::string& name)
{
	for (int actorDefIndex = 0; actorDefIndex < (int)s_definitions.size(); actorDefIndex++)
	{
		ActorDefinition*& actorDef = ActorDefinition::s_definitions[actorDefIndex];
		if (actorDef && actorDef->m_name == name)
		{
			return actorDef;
		}
	}

	ERROR_AND_DIE("Error: actor definition.");
}


