#pragma once
#include "Engine/Core/XmlUtils.hpp"
#define UNUSED(x) (void)(x);

class InputSystem;
class Window;
class Renderer;
class AudioSystem;
class App;
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

typedef tinyxml2::XMLDocument XmlDocument;
typedef tinyxml2::XMLElement XmlElement;
typedef tinyxml2::XMLAttribute XmlAttribute;

constexpr int NUM_STARTING_ASTEROIDS = 3;
constexpr int NUM_STARTING_BEETLES = 1;
constexpr int NUM_STARTING_WASPS = 0;
constexpr int MAX_ASTEROIDS = 100;
constexpr int MAX_BULLETS = 300;
constexpr int MAX_BEETLES = 10;
constexpr int MAX_WASPS = 10;
constexpr int MAX_DEBRIS = 300;
constexpr int MAX_STARS = 100;
constexpr int MAX_POWERUPS = 5;
constexpr int MAX_STAR_VERTS = 3 * MAX_STARS;
constexpr int MAX_WAVE = 5;
constexpr float ENDGAME_WAIT_TIME = 3.0f;

constexpr int NUM_LINE_TRIANGLES = 2;
constexpr int NUM_LINE_VERTS = 3 * NUM_LINE_TRIANGLES;
constexpr int NUM_RING_PARTITIONS = 32;
constexpr int NUM_RING_VERTS = 6 * NUM_RING_PARTITIONS;
constexpr float DEBUG_LINE_THICKNESS = 0.2f;
constexpr float HEALTH_BAR_THICKNESS = 0.4f;

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

constexpr float SCREEN_SIZE_X = 1600.f;
constexpr float SCREEN_SIZE_Y = 800.f;

constexpr float MAX_SCREENSHAKE_OFFSET = 3.f;
constexpr float SCREENSHAKE_DECAY = 1.5f;
constexpr float BULLET_COLLISION_SCREENSHAKE_OFFSET = .25f;
constexpr float ENEMY_DEATH_SCREENSHAKE_OFFSET = .75f;
constexpr float PLAYER_DEATH_SCREENSHAKE_OFFSET = 3.f;

constexpr int ASTEROID_HEALTH = 3;
constexpr float ASTEROID_SPEED = 10.f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;

constexpr int BEETLE_HEALTH = 5;
constexpr float BEETLE_SPEED = 8.f;
constexpr float BEETLE_PHYSICS_RADIUS = 1.6f;
constexpr float BEETLE_COSMETIC_RADIUS = 2.0f;

constexpr int WASP_HEALTH = 3;
constexpr float WASP_ACCELERATION = 10.f;
constexpr float WASP_INITIAL_SPEED = 12.f;
constexpr float MAX_WASP_SPEED = 25.f;
constexpr float WASP_PHYSICS_RADIUS = 1.6f;
constexpr float WASP_COSMETIC_RADIUS = 2.0f;

constexpr int DEBRIS_ON_BULLET_IMPACT = 2;
constexpr int DEBRIS_ON_ENTITY_DEATH = 8;
constexpr int DEBRIS_ON_PLAYER_DEATH = 20;
constexpr int DEBRIS_DEFAULT_ALPHA = 127;
constexpr float DEBRIS_PHYSICAL_RADIUS = 0.2f;
constexpr float DEBRIS_COSMETIC_RADIUS = 1.f;
constexpr float DEBRIS_BULLET_MIN_SCALE = 0.2f;
constexpr float DEBRIS_BULLET_MAX_SCALE = 0.5f;
constexpr float DEBRIS_BULLET_MIN_SPEED = 3.f;
constexpr float DEBRIS_BULLET_MAX_SPEED = 8.f;
constexpr float DEBRIS_MIN_SCALE = 1.f;
constexpr float DEBRIS_MAX_SCALE = 2.f;
constexpr float DEBRIS_MIN_SPEED = 5.f;
constexpr float DEBRIS_MAX_SPEED = 15.f;
constexpr double DEBRIS_LIFE_TIME = 2.0;

constexpr float BULLET_LIFETIME_SECONDS = 2.0f;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;

constexpr float POWERUP_LIFETIME_SECONDS = 10.0f;
constexpr float POWERUP_SPEED = 5.f;
constexpr float POWERUP_PHYSICS_RADIUS = 1.25f;
constexpr float POWERUP_COSMETIC_RADIUS = 1.0f;

constexpr int PLAYER_HEALTH = 4;
constexpr float PLAYER_SHIP_ACCELERATION = 30.f;
constexpr float MAX_PLAYER_SPEED = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;

void DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float lineWidth, Rgba8 color);
void DebugDrawRing(Vec2 const& center, float radius, float lineWidth, Rgba8 color);