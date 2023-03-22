#include "Engine/Renderer/SpriteAnimDefinition.hpp"
 
SpriteAnimDefinition::SpriteAnimDefinition(const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType)
	: m_spriteSheet(sheet)
	, m_startSpriteIndex(startSpriteIndex)
	, m_endSpriteIndex(endSpriteIndex)
	, m_durationSeconds(durationSeconds)
	, m_playbackType(playbackType)
{
	m_numSprites = m_endSpriteIndex - m_startSpriteIndex + 1;
}

const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{
	float secondsIntoCurrentLoop = 0.f;
	float durationEachSprite = 0.f;
	int currentSpriteIndex = 0;

	switch (m_playbackType)
	{
		case SpriteAnimPlaybackType::ONCE:
			if (seconds >= m_durationSeconds)
			{
				return m_spriteSheet.GetSpriteDef(m_endSpriteIndex);
			}
			else
			{
				durationEachSprite = m_durationSeconds / static_cast<float>(m_numSprites);
				currentSpriteIndex = static_cast<int>(seconds / durationEachSprite) + m_startSpriteIndex; 
				return m_spriteSheet.GetSpriteDef(currentSpriteIndex);
			}

		case SpriteAnimPlaybackType::LOOP:
			secondsIntoCurrentLoop = fmodf(seconds, m_durationSeconds);
			durationEachSprite = m_durationSeconds / static_cast<float>(m_numSprites);
			currentSpriteIndex = static_cast<int>(secondsIntoCurrentLoop / durationEachSprite) + m_startSpriteIndex;
			return m_spriteSheet.GetSpriteDef(currentSpriteIndex);
	
		case SpriteAnimPlaybackType::PINGPONG:
			secondsIntoCurrentLoop = fmodf(seconds, m_durationSeconds);
			durationEachSprite = m_durationSeconds / static_cast<float>(2 * m_numSprites - 2);
			currentSpriteIndex = static_cast<int>(secondsIntoCurrentLoop / durationEachSprite) + m_startSpriteIndex;
			if (currentSpriteIndex > m_endSpriteIndex)
			{
				currentSpriteIndex = m_endSpriteIndex - (currentSpriteIndex - m_endSpriteIndex);
			}
			return m_spriteSheet.GetSpriteDef(currentSpriteIndex);
	}

	ERROR_AND_DIE("Invalid Playback Type.");
}


const float SpriteAnimDefinition::GetDuration() const
{
	return m_durationSeconds;
}


