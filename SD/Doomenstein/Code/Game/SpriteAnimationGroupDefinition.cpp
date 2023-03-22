#include "Game/SpriteAnimationGroupDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"

bool SpriteAnimationGroupDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name							= ParseXmlAttribute(element, "name", m_name);
	std::string shaderName			= ParseXmlAttribute(element, "shader", "none");
	std::string spriteTextureName	= ParseXmlAttribute(element, "spriteSheet", "none");
	IntVec2 spriteSheetLayout		= ParseXmlAttribute(element, "cellCount", IntVec2::ZERO);
	if (spriteTextureName != "none")
	{
		Texture const* spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteTextureName.c_str());
		m_spriteSheet = new SpriteSheet(*spriteSheetTexture, spriteSheetLayout);
	}
	m_fps							= ParseXmlAttribute(element, "secondsPerFrame", 1.f);
	std::string playbackMode		= ParseXmlAttribute(element, "playbackMode", "none");
	m_scaleBySpeed					= ParseXmlAttribute(element, "scaleBySpeed", m_scaleBySpeed);
	m_shader = g_theRenderer->GetShaderForName(shaderName.c_str());
	if (playbackMode == "Once") m_playbackMode = SpriteAnimPlaybackType::ONCE;
	if (playbackMode == "Loop") m_playbackMode = SpriteAnimPlaybackType::LOOP;
	if (playbackMode == "Pingpong") m_playbackMode = SpriteAnimPlaybackType::PINGPONG;

	XmlElement const* direction = element.FirstChildElement();
	while (direction)
	{
		if (std::string(direction->Name()) == "Direction")
		{
			Vec3 vector = ParseXmlAttribute(*direction, "vector", Vec3::ZERO).GetNormalized();
			XmlElement const* animation = direction->FirstChildElement();
			if (animation && std::string(animation->Name()) == "Animation")
			{
				int startIndex = ParseXmlAttribute(*animation, "startFrame", 0);
				int endIndex = ParseXmlAttribute(*animation, "endFrame", 0);
				float duration = static_cast<float>(endIndex - startIndex + 1) * m_fps;
				m_duration = duration;
				SpriteAnimDefinition* spriteAnimDef = new SpriteAnimDefinition(*m_spriteSheet, startIndex, endIndex, duration, m_playbackMode);
				m_directions.push_back(vector);
				m_spriteAnimationDefinitions.push_back(spriteAnimDef);
			}
		}
		direction = direction->NextSiblingElement();
	}
	return true;
}


SpriteAnimDefinition const& SpriteAnimationGroupDefinition::GetAnimationForDirection(Vec3 const& direction) const
{
	for (int directionIndex = 0; directionIndex < (int)m_directions.size(); directionIndex++)
	{
		if (m_directions[directionIndex] == direction)
		{
			return *m_spriteAnimationDefinitions[directionIndex];
		}
	}

	ERROR_AND_DIE("Can't find sprite animation definition.");
}


float SpriteAnimationGroupDefinition::GetDuration() const
{
	return m_duration;
}


