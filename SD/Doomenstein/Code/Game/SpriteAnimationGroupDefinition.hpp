#pragma once
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include <string>

//------------------------------------------------------------------------------------------------
class Camera;
class Shader;
class SpriteSheet;
class Texture;
struct Vec3;

//------------------------------------------------------------------------------------------------
class SpriteAnimationGroupDefinition
{
public:
	bool LoadFromXmlElement(XmlElement const& element);

	SpriteAnimDefinition const& GetAnimationForDirection(Vec3 const& direction) const;
	float GetDuration() const;

	std::string m_name;

	Shader* m_shader;
	SpriteSheet const* m_spriteSheet;
	float m_fps = 1.0f;
	float m_duration = 0.f;
	SpriteAnimPlaybackType m_playbackMode = SpriteAnimPlaybackType::LOOP;
	bool m_scaleBySpeed = false;

	std::vector<Vec3> m_directions;
	std::vector<SpriteAnimDefinition*> m_spriteAnimationDefinitions;
};


