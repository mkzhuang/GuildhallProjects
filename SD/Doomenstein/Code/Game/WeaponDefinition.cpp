#include "Game/WeaponDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

std::vector<WeaponDefinition*> WeaponDefinition::s_definitions;

bool WeaponDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name							= ParseXmlAttribute(element, "name", "none");
	m_refireTime					= ParseXmlAttribute(element, "refireTime", m_refireTime);

	m_numRays						= ParseXmlAttribute(element, "numRays", m_numRays);
	m_rayCone						= ParseXmlAttribute(element, "rayCone", m_rayCone);
	m_rayRange						= ParseXmlAttribute(element, "rayRange", m_rayRange);
	m_rayDamage						= ParseXmlAttribute(element, "rayDamage", m_rayDamage);
	m_rayImpulse					= ParseXmlAttribute(element, "rayImpulse", m_rayImpulse);
	m_canShrink						= ParseXmlAttribute(element, "canShrink", m_canShrink);
	m_shrinkTime					= ParseXmlAttribute(element, "shrinkTime", m_shrinkTime);

	m_numProjectiles				= ParseXmlAttribute(element, "numProjectiles", m_numProjectiles);
	std::string projectileActorName = ParseXmlAttribute(element, "projectileActor", "none");
	m_projectileCone				= ParseXmlAttribute(element, "projectileCone", m_projectileCone);
	m_projectileSpeed				= ParseXmlAttribute(element, "projectileSpeed", m_projectileSpeed);

	if (projectileActorName != "none") m_projectileActorDefinition = ActorDefinition::GetByName(projectileActorName);

	XmlElement const* child = element.FirstChildElement();
	while (child)
	{
		if (std::string(child->Name()) == "HUD")
		{
			std::string shaderName		= ParseXmlAttribute(*child, "shader", "none");
			std::string hudBaseName		= ParseXmlAttribute(*child, "baseTexture", "none");
			std::string reticleName		= ParseXmlAttribute(*child, "reticleTexture", "none");
			m_reticeSize				= ParseXmlAttribute(*child, "reticleSize", m_reticeSize);
			m_spriteSize				= ParseXmlAttribute(*child, "spriteSize", m_spriteSize);
			m_spritePivot				= ParseXmlAttribute(*child, "spritePivot", m_spritePivot);
			m_shader					= g_theRenderer->GetShaderForName(shaderName.c_str());
			m_hubBaseTexture			= g_theRenderer->CreateOrGetTextureFromFile(hudBaseName.c_str());
			m_reticleTexture			= g_theRenderer->CreateOrGetTextureFromFile(reticleName.c_str());

			XmlElement const* animation = child->FirstChildElement();
			while (animation)
			{
				if (std::string(animation->Name()) == "Animation")
				{
					std::string animationName			= ParseXmlAttribute(*animation, "name", "none");
					std::string spriteTextureName		= ParseXmlAttribute(*animation, "spriteSheet", "none");
					IntVec2 spriteSheetLayout			= ParseXmlAttribute(*animation, "cellCount", IntVec2::ZERO);
					if (spriteTextureName != "none")
					{
						Texture const* spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteTextureName.c_str());
						m_spriteSheet = new SpriteSheet(*spriteSheetTexture, spriteSheetLayout);
					}
					float secondsPerFrame = ParseXmlAttribute(*animation, "secondsPerFrame", 1.f);
					int startIndex = ParseXmlAttribute(*animation, "startFrame", 0);
					int endIndex = ParseXmlAttribute(*animation, "endFrame", 0);
					float duration = static_cast<float>(endIndex - startIndex + 1) * secondsPerFrame;
					if (animationName == "Idle")
					{
						m_idleAnimationDefinition = new SpriteAnimDefinition(*m_spriteSheet, startIndex, endIndex, duration, SpriteAnimPlaybackType::LOOP);
					}
					else if (animationName == "Attack")
					{
						m_attackAnimationDefinition = new SpriteAnimDefinition(*m_spriteSheet, startIndex, endIndex, duration, SpriteAnimPlaybackType::ONCE);
					}
				}
				animation = animation->NextSiblingElement();
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
					if (soundName == "Fire")
					{
						m_fireSoundName = ParseXmlAttribute(*sound, "name", m_fireSoundName);
					}
				}
				sound = sound->NextSiblingElement();
			}
		}

		child = child->NextSiblingElement();
	}
	return true;
}


void WeaponDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument doc;
	doc.LoadFile(path);
	XmlElement const* root = doc.RootElement();

	XmlElement const* element = root->FirstChildElement();
	while (element)
	{
		if (std::string(element->Name()) == "WeaponDefinition")
		{
			WeaponDefinition* newWeaponDef = new WeaponDefinition();
			newWeaponDef->LoadFromXmlElement(*element);
			WeaponDefinition::s_definitions.push_back(newWeaponDef);
			element = element->NextSiblingElement();
		}
	}
}


void WeaponDefinition::ClearDefinitions()
{
	for (int weaponDefIndex = 0; weaponDefIndex < (int)s_definitions.size(); weaponDefIndex++)
	{
		WeaponDefinition*& weaponDef = WeaponDefinition::s_definitions[weaponDefIndex];
		if (weaponDef)
		{
			delete weaponDef;
		}
	}

	s_definitions.clear();
}


const WeaponDefinition* WeaponDefinition::GetByName(const std::string& name)
{
	for (int weaponDefIndex = 0; weaponDefIndex < (int)s_definitions.size(); weaponDefIndex++)
	{
		WeaponDefinition*& weaponDef = WeaponDefinition::s_definitions[weaponDefIndex];
		if (weaponDef && weaponDef->m_name == name)
		{
			return weaponDef;
		}
	}

	ERROR_AND_DIE("Error: weapon definition.");
}


