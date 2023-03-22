#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"

#define UNUSED(x) (void)(x);

class InputSystem;
class Window;
class Renderer;
class AudioSystem;
class App;
class RandomNumberGenerator;

extern bool g_isQuitting;
extern bool g_isDebugging;
extern float g_gameMusicVolume;
extern float g_gameSoundVolume;
extern float g_gameSoundPlaySpeed;

extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudio;
extern App* g_theApp;
extern RandomNumberGenerator RNG;

extern std::vector<Texture*> g_textures;
extern std::vector<SoundID> g_soundIDs;
extern SpriteSheet* g_tileSpriteSheet;

typedef tinyxml2::XMLDocument XmlDocument;
typedef tinyxml2::XMLElement XmlElement;
typedef tinyxml2::XMLAttribute XmlAttribute;

enum TextureType
{
	TEXTURE_TILES,
	TEXTURE_TEST,

	NUM_TEXTURES
};

enum SoundIDType
{
	SOUND_ATTRACT,
	SOUND_GAME,
	SOUND_PAUSE,
	SOUND_UNPAUSE,
	SOUND_SELECT,
	SOUND_PISTOL,
	SOUND_PLASMA,
	SOUND_SHRINKGUN,
	SOUND_PLAYER_HURT,
	SOUND_PLAYER_DEATH,
	SOUND_DEMON_ATTACK,
	SOUND_DEMON_HURT,
	SOUND_DEMON_DEATH,
	SOUND_MAGMA_ATTACK,
	SOUND_MAGMA_HURT,
	SOUND_MAGMA_DEATH,
	SOUND_FIREBALL_HIT,

	NUM_SOUNDS
};

SoundPlaybackID PlaySound(SoundID sound, bool loop, float volume);


