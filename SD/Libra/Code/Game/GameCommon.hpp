#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#define UNUSED(x) (void)(x);

class InputSystem;
class Window;
class Renderer;
class AudioSystem;
class App;
class Game;
class RandomNumberGenerator;
struct Rgba8;
struct Vec2;

extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudio;
extern App* g_theApp;
extern RandomNumberGenerator RNG;

extern bool g_isQuitting;
extern bool g_isDebugging;
extern bool g_isNoClip;
extern bool g_isInvulnerable;
extern bool g_isDebugCamera;
extern float g_gameSoundVolume;
extern float g_gameSoundPlaySpeed;

extern std::vector<Texture*> g_textures;
extern std::vector<SoundID> g_soundIds;
extern SpriteSheet* g_tileSpriteSheet;
extern SpriteAnimDefinition* g_exlosionAnimation;

typedef tinyxml2::XMLDocument XmlDocument;
typedef tinyxml2::XMLElement XmlElement;
typedef tinyxml2::XMLAttribute XmlAttribute;


enum SoundType
{
	SOUND_TYPE_ATTRACT,
	SOUND_TYPE_IN_GAME,
	SOUND_TYPE_VICTORY,
	SOUND_TYPE_GAME_OVER,
	SOUND_TYPE_LEVEL_END,
	SOUND_TYPE_SELECT,
	SOUND_TYPE_PAUSE,
	SOUND_TYPE_UNPAUSE,
	SOUND_TYPE_PLAYER_SHOOT,
	SOUND_TYPE_ENEMY_SHOOT,
	SOUND_TYPE_PLAYER_HIT,
	SOUND_TYPE_ENEMY_HIT,
	SOUND_TYPE_ENEMY_DIE,
	SOUND_TYPE_BULLET_BOUNCE,
	SOUND_TYPE_BULLET_RICOCHET,
	SOUND_TYPE_PLAYER_DISCOVERY,

	NUM_SOUND_TYPES
};


enum TextureType
{
	//TEXTURE_TYPE_PLAYER_BASE,
	//TEXTURE_TYPE_PLAYER_TURRET,
	//TEXTURE_TYPE_PLAYER_BULLET,
	//TEXTURE_TYPE_SCORPIO_BASE,
	//TEXTURE_TYPE_SCORPOI_TURRET,
	//TEXTURE_TYPE_LEO_TANK,
	//TEXTURE_TYPE_ARIES_TANK,
	//TEXTURE_TYPE_ENEMY_BULLET,
	TEXTURE_TYPE_EXPLOSION_ANIM,
	TEXTURE_TYPE_TERRAIN_TILES,
	TEXTURE_TYPE_ATTRACT_SCREEN,
	TEXTURE_TYPE_VICTORY_SCREEN,
	TEXTURE_TYPE_GAME_OVER_SCREEN,

	NUM_TEXTURE_TYPES
};

SoundPlaybackID PlaySound(SoundID sound, bool loop, float volume);
void DrawLaser(Vec2 const& startPos, Vec2 const& endPos, float lineWidth, float maxLength, Rgba8 color);
void DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float lineWidth, Rgba8 color);
void DebugDrawRing(Vec2 const& center, float radius, float lineWidth, Rgba8 color);
void DebugDrawDot(Vec2 const& center, float radius, Rgba8 color);