#pragma once
#include "Game/GameCommon.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Beetle.hpp"
#include "Game/Wasp.hpp"
#include "Game/Bullet.hpp"
#include "Game/Debris.hpp"
#include "Game/Powerup.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"

class Game
{
public:
	Game(App* const& owner);
	~Game();
	void Startup();

	void Update();
	void UpdateCamera(float deltaSeconds);
	void UpdateEntities(float deltaSeconds);
	void Render() const;
	void RenderEntities() const;
	void SpawnWaves(float deltaSeconds);
	void SpawnRandomAsteroid();
	void SpawnRandomBeetle();
	void SpawnRandomWasp();
	void SpawnPowerup(Vec2 const& pos);
	void SpawnBullet(Vec2 const& pos, float forwardDegrees);
	void SpawnDebrisCluster(int count, Vec2 pos, Rgba8 color, Vec2 baseVelocity, 
							float minRndScale, float maxRndScale, float minSpeed, float maxSpeed);
	void SpawnRandomDebris(Vec2 const& pos, Rgba8 const& color,
						   Vec2 const& baseVelocity, float scale, float speed);
	void SpawnEnemies(int asteroids, int beetles, int wasps);
	void HandleKeyPressed();
	void HandleControllerInput();
	void HandleCollision();
	void AddScreenshake(float deltaScreenshake);
	void DeleteGarbageEntities();
	void ResetGame();
	void DebugRender() const;
	PlayerShip* GetNearestPlayerShip() const;
	Vec2 RollSpawnPos(float cosmeticRadius);
	void SpawnStarfield();
	void DrawLevelTextUI() const;
	void DrawPlayerLivesUI() const;
	void DrawWeaponUI() const;
	void DrawEnemyCountsUI() const;
	void DrawWinningUI() const;
	void DrawLosingUI() const;
	void DrawEnemiesHealthBar() const;
	void DrawAttractScreen() const;
	void DrawSelectTriangle(Vec2 pos, Rgba8 color) const;
	void DrawBeetle(Vec2 pos, Rgba8 color, float scale, float rotation) const;
	void DrawWasp(Vec2 pos, Rgba8 color, float scale, float rotation) const;
	void DrawPlayerShip(Vec2 pos, Rgba8 color, float scale, float rotation) const;
	void DrawBullet(Vec2 pos, Rgba8 color, float scale, float rotation) const;

public:
	bool m_isAttractMode = true;

private:
	App* m_theApp = nullptr;
	PlayerShip* m_playerShip = nullptr;
	Asteroid* m_asteroids[MAX_ASTEROIDS] = {};
	Beetle* m_beetles[MAX_BEETLES] = {};
	Wasp* m_wasps[MAX_WASPS] = {};
	Bullet* m_bullets[MAX_BULLETS] = {};
	Debris* m_debris[MAX_DEBRIS] = {};
	Powerup* m_powerups[MAX_POWERUPS] = {};
	Vertex_PCU m_starfieldBackground[MAX_STAR_VERTS] = {};
	Vertex_PCU m_starfieldForeground[MAX_STAR_VERTS] = {};
	int m_asteroidCounter = 0;
	int m_beetleCounter = 0;
	int m_bulletCounter = 0;
	int m_waspCounter = 0;
	int m_debrisCounter = 0;
	int m_powerupCounter = 0;
	int m_wave = 0;
	bool m_isStartOrExit = true;
	bool m_isSlowMo = false;
	bool m_isDebugging = false;
	bool m_isWinning = false;
	bool m_isLosing = false;
	float m_joystickMenuDelay = 0.2f;
	float m_returnTimer = 0.f;
	float m_screenshakeAmount = 0.f;
	Camera m_worldCamera;
	Camera m_screenCamera;
	Camera m_foregroundCamera;
	Camera m_attractCamera;
	Clock m_gameClock;
};


