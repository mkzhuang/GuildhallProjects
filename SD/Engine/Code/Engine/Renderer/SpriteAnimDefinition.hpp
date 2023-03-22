#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/EngineCommon.hpp"

enum class SpriteAnimPlaybackType
{
	ONCE,		// for 5-frame anim, plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4...
	LOOP,		// for 5-frame anim, plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0...
	PINGPONG,	// for 5-frame anim, plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1...
};


class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition(const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex,
		float durationSeconds, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP);

	const SpriteDefinition& GetSpriteDefAtTime(float seconds) const;
	const float GetDuration() const;

private:
	SpriteSheet const& m_spriteSheet;
	int m_startSpriteIndex = -1;
	int m_endSpriteIndex = -1;
	int m_numSprites = 0;
	float m_durationSeconds = 1.f;
	SpriteAnimPlaybackType	m_playbackType = SpriteAnimPlaybackType::LOOP;
};


