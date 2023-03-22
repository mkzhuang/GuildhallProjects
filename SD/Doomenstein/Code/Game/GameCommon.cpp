#include "Game/GameCommon.hpp"

bool g_isQuitting = false;
bool g_isDebugging = false;
float g_gameMusicVolume = 0.1f;
float g_gameSoundVolume = 0.5f;
float g_gameSoundPlaySpeed = 1.f;

SoundPlaybackID PlaySound(SoundID sound, bool loop, float volume)
{
	if (sound == MISSING_SOUND_ID)
	{
		return NULL;
	}

	return g_theAudio->StartSound(sound, loop, volume, 0.f, g_gameSoundPlaySpeed);
}


