#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

RandomNumberGenerator RNG;

Game::Game(App* const& owner)
	: m_theApp(owner)
	, m_gameClock(Clock::GetSystemClock())
{

}


Game::~Game()
{
	delete m_playerShip;
	m_playerShip = nullptr;
	ResetGame();

}


void Game::Startup()
{
	SpawnStarfield();
	SoundID introSound = g_theAudio->CreateOrGetSound("Data/Audio/Intro.wav");
	g_theAudio->StartSound(introSound);
}


void Game::Update()
{
	float deltaSeconds = static_cast<float>(m_gameClock.GetDeltaTime());

	m_gameClock.SetTimeDilation(1.0);
	if (!m_gameClock.IsPaused())
	{
		if (m_isSlowMo)
		{
			m_gameClock.SetTimeDilation(0.1);
		}
	}

	if (m_isAttractMode)
	{
		HandleKeyPressed();
		HandleControllerInput();
		m_joystickMenuDelay -= deltaSeconds;
	} 
	else
	{

		SpawnWaves(deltaSeconds);
		HandleCollision();
		UpdateEntities(deltaSeconds);
		HandleKeyPressed();
		HandleControllerInput();
		DeleteGarbageEntities();
	}
	UpdateCamera(deltaSeconds);
}


void Game::UpdateCamera(float deltaSeconds)
{
	m_attractCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
	m_foregroundCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
	m_worldCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));

	if (m_playerShip && !m_playerShip->m_isGarbage) 
	{
		Vec2 playerDisplacementFromWorldCenter = Vec2(WORLD_CENTER_X, WORLD_CENTER_Y) - m_playerShip->m_position;
		m_foregroundCamera.Translate(-playerDisplacementFromWorldCenter * 0.15f);
	}

	float offsetX = RNG.RollRandomFloatInRange(-m_screenshakeAmount, m_screenshakeAmount);
	float offsetY = RNG.RollRandomFloatInRange(-m_screenshakeAmount, m_screenshakeAmount);
	Vec2 offset = Vec2(offsetX, offsetY);
	m_worldCamera.Translate(offset);

	if (m_screenshakeAmount > 0.f) m_screenshakeAmount -= SCREENSHAKE_DECAY * deltaSeconds;
	m_screenshakeAmount = Clamp(m_screenshakeAmount, 0.f, MAX_SCREENSHAKE_OFFSET);
}


void Game::UpdateEntities(float deltaSeconds)
{
	for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
	{
		Asteroid* curAsteroid = m_asteroids[asteroidIndex];
		if (curAsteroid) curAsteroid->Update(deltaSeconds);
	}

	for (int beetleIndex = 0; beetleIndex < MAX_BEETLES; beetleIndex++)
	{
		Beetle* curBeetle = m_beetles[beetleIndex];
		if (curBeetle) curBeetle->Update(deltaSeconds);
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; waspIndex++)
	{
		Wasp* curWasp = m_wasps[waspIndex];
		if (curWasp) curWasp->Update(deltaSeconds);
	}

	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		Bullet* curBullet = m_bullets[bulletIndex];
		if (curBullet) curBullet->Update(deltaSeconds);
	}

	for (int debrisIndex = 0; debrisIndex < MAX_DEBRIS; debrisIndex++)
	{
		Debris* curDebris = m_debris[debrisIndex];
		if (curDebris) curDebris->Update(deltaSeconds);
	}

	for (int powerupIndex = 0; powerupIndex < MAX_POWERUPS; powerupIndex++)
	{
		Powerup* curPowerup = m_powerups[powerupIndex];
		if (curPowerup) curPowerup->Update(deltaSeconds);
	}

	m_playerShip->Update(deltaSeconds);
}


void Game::Render() const
{
	g_theRenderer->BindTexture(nullptr);

	g_theRenderer->BeginCamera(m_foregroundCamera);
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	g_theRenderer->DrawVertexArray(MAX_STAR_VERTS, m_starfieldForeground);
	g_theRenderer->EndCamera(m_foregroundCamera);

	if (m_isAttractMode)
	{
		g_theRenderer->BeginCamera(m_attractCamera);
		g_theRenderer->DrawVertexArray(MAX_STAR_VERTS, m_starfieldBackground);
		DrawAttractScreen();
		g_theRenderer->EndCamera(m_attractCamera);
	}
	else 
	{
		g_theRenderer->BeginCamera(m_worldCamera);
		g_theRenderer->DrawVertexArray(MAX_STAR_VERTS, m_starfieldBackground);
		RenderEntities();
		DrawEnemiesHealthBar();
		if (m_isDebugging)
		{
			DebugRender();
		}
		g_theRenderer->EndCamera(m_worldCamera);
	}

	g_theRenderer->BeginCamera(m_screenCamera);
	if (!m_isAttractMode) 
	{
		DrawLevelTextUI();
		DrawPlayerLivesUI();
		DrawWeaponUI();
		DrawEnemyCountsUI();
		if (m_isWinning) DrawWinningUI();
		if (m_isLosing) DrawLosingUI();
	}
	g_theRenderer->EndCamera(m_screenCamera);
}


void Game::RenderEntities() const
{
	for (int debrisIndex = 0; debrisIndex < MAX_DEBRIS; debrisIndex++)
	{
		Debris* curDebris = m_debris[debrisIndex];
		if (curDebris) curDebris->Render();
	}

	for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
	{
		Asteroid* curAsteroid = m_asteroids[asteroidIndex];
		if (curAsteroid) curAsteroid->Render();
	}

	for (int beetleIndex = 0; beetleIndex < MAX_BEETLES; beetleIndex++)
	{
		Beetle* curBeetle = m_beetles[beetleIndex];
		if (curBeetle) curBeetle->Render();
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; waspIndex++)
	{
		Wasp* curWasp = m_wasps[waspIndex];
		if (curWasp) curWasp->Render();
	}

	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		Bullet* curBullet = m_bullets[bulletIndex];
		if (curBullet) curBullet->Render();
	}

	for (int powerupIndex = 0; powerupIndex < MAX_POWERUPS; powerupIndex++)
	{
		Powerup* curPowerup = m_powerups[powerupIndex];
		if (curPowerup) curPowerup->Render();
	}

	m_playerShip->Render();
}


void Game::SpawnWaves(float deltaSeconds)
{
	int totalCounter = m_beetleCounter + m_waspCounter;
	if (totalCounter == 0 && m_wave < MAX_WAVE)
	{
		m_wave++;
		if (m_wave == MAX_WAVE) return;
		SpawnEnemies(NUM_STARTING_ASTEROIDS, NUM_STARTING_BEETLES + m_wave, NUM_STARTING_WASPS + m_wave);
		SoundID newLevelSound = g_theAudio->CreateOrGetSound("Data/Audio/NewLevel.wav");
		g_theAudio->StartSound(newLevelSound);
	}

	if ((m_wave == MAX_WAVE && totalCounter == 0) || (m_playerShip->health == 0 && m_playerShip->m_isGarbage))
	{
		if (m_returnTimer == 0.f)
		{
			if (m_wave == MAX_WAVE)
			{
				SoundID winSound = g_theAudio->CreateOrGetSound("Data/Audio/Win.wav");
				g_theAudio->StartSound(winSound);
				m_isWinning = true;
			}
			else
			{
				SoundID loseSound = g_theAudio->CreateOrGetSound("Data/Audio/Lose.wav");
				g_theAudio->StartSound(loseSound);
				m_isLosing = true;
			}
		}
		m_returnTimer += deltaSeconds;
		if (m_returnTimer >= ENDGAME_WAIT_TIME)
		{
			ResetGame();
		}
	}
}


void Game::SpawnRandomAsteroid()
{
	if (m_asteroidCounter == MAX_ASTEROIDS)
	{
		ERROR_RECOVERABLE("Can not spawn an asteroid. All slots are full!");
		return;
	}

	for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
	{
		if (!m_asteroids[asteroidIndex])
		{
			Vec2 spawnPos = RollSpawnPos(BEETLE_COSMETIC_RADIUS);
			Asteroid* newAsteroid = new Asteroid(this, spawnPos);
			float randomOrientationDegrees = RNG.RollRandomFloatInRange(0.f, 360.f);
			newAsteroid->m_orientationDegree = randomOrientationDegrees;
			float randomVelocityX = RNG.RollRandomFloatInRange(-1.f, 1.f);
			float randomVelocityY = RNG.RollRandomFloatInRange(-1.f, 1.f);
			Vec2 randomVeclocity = Vec2(randomVelocityX, randomVelocityY).GetNormalized() * ASTEROID_SPEED;
			newAsteroid->m_velocity = (randomVeclocity);
			float randomAngularVeclocity = RNG.RollRandomFloatInRange(-200.f, 200.f);
			newAsteroid->m_angularVelocity = randomAngularVeclocity;
			
			m_asteroids[asteroidIndex] = newAsteroid;
			m_asteroidCounter++;
			return;
		}
	}
}


void Game::SpawnRandomBeetle()
{
	if (m_beetleCounter == MAX_BEETLES)
	{
		ERROR_RECOVERABLE("Can not spawn a beetle. All slots are full!");
		return;
	}

	for (int beetleIndex = 0; beetleIndex < MAX_BEETLES; beetleIndex++)
	{
		if (!m_beetles[beetleIndex])
		{
			Vec2 spawnPos = RollSpawnPos(BEETLE_COSMETIC_RADIUS);
			Beetle* newBeetle = new Beetle(this, spawnPos);

			m_beetles[beetleIndex] = newBeetle;
			m_beetleCounter++;
			return;
		}
	}
}


void Game::SpawnRandomWasp()
{
	if (m_waspCounter == MAX_WASPS)
	{
		ERROR_RECOVERABLE("Can not spawn a wasp. All slots are full!");
		return;
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; waspIndex++)
	{
		if (!m_wasps[waspIndex])
		{
			Vec2 spawnPos = RollSpawnPos(WASP_COSMETIC_RADIUS);
			Wasp* newWasp = new Wasp(this, spawnPos);

			m_wasps[waspIndex] = newWasp;
			m_waspCounter++;
			return;
		}
	}
}


void Game::SpawnPowerup(Vec2 const& pos)
{
	if (m_powerupCounter == MAX_POWERUPS)
	{
		ERROR_RECOVERABLE("Can not spawn a powerup. All slots are full!");
		return;
	}

	for (int powerupIndex = 0; powerupIndex < MAX_POWERUPS; powerupIndex++)
	{
		if (!m_powerups[powerupIndex])
		{
			Powerup* newPowerup = new Powerup(this, pos);

			m_powerups[powerupIndex] = newPowerup;
			m_powerupCounter++;
			return;
		}
	}
}


void Game::SpawnBullet(Vec2 const& pos, float forwardDegrees)
{
	if (m_bulletCounter == MAX_BULLETS)
	{
		ERROR_RECOVERABLE("Can not spawn a bullet. All slots are full!");
		return;
	}

	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		if (!m_bullets[bulletIndex])
		{
			Bullet* newBullet = new Bullet(this, pos);
			newBullet->m_orientationDegree = forwardDegrees;
			newBullet->m_velocity = newBullet->GetForwardNormal() * BULLET_SPEED;

			m_bullets[bulletIndex] = newBullet;
			m_bulletCounter++;
			return;
		}
	}
}


void Game::SpawnDebrisCluster(int count, Vec2 pos, Rgba8 color, Vec2 baseVelocity,
	float minRndScale, float maxRndScale, float minSpeed, float maxSpeed)
{
	for (int debrisIndex = 0; debrisIndex < count; debrisIndex++)
	{
		float scale = RNG.RollRandomFloatInRange(minRndScale, maxRndScale);
		float speed = RNG.RollRandomFloatInRange(minSpeed, maxSpeed);
		color.a = DEBRIS_DEFAULT_ALPHA;
		SpawnRandomDebris(pos, color, baseVelocity, scale, speed);
	}
}


void Game::SpawnRandomDebris(Vec2 const& pos, Rgba8 const& color,
	Vec2 const& baseVelocity, float scale, float speed)
{
	if (m_debrisCounter == MAX_DEBRIS)
	{
		ERROR_RECOVERABLE("Can not spawn debris. All slots are full!");
		return;
	}

	for (int debrisIndex = 0; debrisIndex < MAX_DEBRIS; debrisIndex++)
	{
		if (!m_debris[debrisIndex])
		{
			Debris* newDebris = new Debris(this, pos);
			float randomOrientationDegrees = RNG.RollRandomFloatInRange(0.f, 360.f);
			newDebris->m_orientationDegree = randomOrientationDegrees;
			float randomAngularVeclocity = RNG.RollRandomFloatInRange(-200.f, 200.f);
			newDebris->m_angularVelocity = randomAngularVeclocity;
			float randomVelocityX = RNG.RollRandomFloatInRange(-1.f, 1.f);
			float randomVelocityY = RNG.RollRandomFloatInRange(-1.f, 1.f);
			Vec2 randomVeclocity = Vec2(randomVelocityX, randomVelocityY).GetNormalized() * speed;
			Vec2 velocity = baseVelocity + randomVeclocity;
			newDebris->InitializeLocalVerts(color, scale, velocity);

			m_debris[debrisIndex] = newDebris;
			m_debrisCounter++;
			return;
		}
	}
}


void Game::SpawnEnemies(int asteroids, int beetles, int wasps)
{
	for (int asteroidIndex = 0; asteroidIndex < asteroids; asteroidIndex++)
	{
		SpawnRandomAsteroid();
	}

	for (int beetleIndex = 0; beetleIndex < beetles; beetleIndex++)
	{
		SpawnRandomBeetle();
	}

	for (int waspIndex = 0; waspIndex < wasps; waspIndex++)
	{
		SpawnRandomWasp();
	}
}


void Game::HandleKeyPressed()
{
	if (m_isAttractMode)
	{
		if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N'))
		{
			if (m_isStartOrExit)
			{
				Vec2 worldCenter(WORLD_CENTER_X, WORLD_CENTER_Y);
				m_playerShip = new PlayerShip(this, worldCenter);
				m_playerShip->health--;
				ResetGame();
				SpawnEnemies(NUM_STARTING_ASTEROIDS, NUM_STARTING_BEETLES, NUM_STARTING_WASPS);
				m_isAttractMode = false;
				m_wave = 0;
				SoundID selectSound = g_theAudio->CreateOrGetSound("Data/Audio/Select.wav");
				g_theAudio->StartSound(selectSound);
				return;
			}
			else
			{
				g_theApp->HandleQuitRequested();
			}
		}

		if (g_theInput->WasKeyJustPressed('S') || g_theInput->WasKeyJustPressed('W')) 
		{
			m_isStartOrExit = !m_isStartOrExit;
			SoundID selectSound = g_theAudio->CreateOrGetSound("Data/Audio/Select.wav");
			g_theAudio->StartSound(selectSound);
		}

		if (g_theInput->WasKeyJustPressed(VK_ESCAPE))
		{
			m_theApp->HandleQuitRequested();
		}

		return;
	}

	if (g_theInput->WasKeyJustPressed('I'))
	{
		SpawnRandomAsteroid();
	}
	
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock.StepFrame();
		SoundID pauseSound = g_theAudio->CreateOrGetSound("Data/Audio/Pause.wav");
		g_theAudio->StartSound(pauseSound, false, .25f);
	}

	if (g_theInput->WasKeyJustPressed(VK_F1))
	{
		m_isDebugging = !m_isDebugging;
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		if (m_gameClock.IsPaused())
		{
			m_gameClock.Unpause();
			m_gameClock.Unpause();
			SoundID unpauseSound = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.wav");
			g_theAudio->StartSound(unpauseSound, false, .25f);
		}
		else
		{
			m_gameClock.Pause();
			m_gameClock.Pause();
			SoundID pauseSound = g_theAudio->CreateOrGetSound("Data/Audio/Pause.wav");
			g_theAudio->StartSound(pauseSound, false, .25f);
		}
	}

	if (g_theInput->WasKeyJustPressed(VK_ESCAPE)) 
	{
		m_isAttractMode = true;
		m_isStartOrExit = true;
		SoundID selectSound = g_theAudio->CreateOrGetSound("Data/Audio/Select.wav");
		g_theAudio->StartSound(selectSound);
		ResetGame();
	}

	if (g_theInput->WasKeyJustPressed(VK_F5) && m_playerShip->m_bulletCount < 3)
	{
		m_playerShip->m_bulletCount++;
	}

	if (g_theInput->WasKeyJustPressed(VK_F6) && m_playerShip->m_bulletCount > 1)
	{
		m_playerShip->m_bulletCount--;
	}
}


void Game::HandleControllerInput()
{
	XboxController const& controller = g_theInput->GetController(0);

	if (m_isAttractMode)
	{
		if (controller.WasButtonJustPressed(XboxButtonID::START) || controller.WasButtonJustPressed(XboxButtonID::BUTTON_A))
		{
			if (m_isStartOrExit)
			{
				Vec2 worldCenter(WORLD_CENTER_X, WORLD_CENTER_Y);
				m_playerShip = new PlayerShip(this, worldCenter);
				m_playerShip->health--;
				ResetGame();
				SpawnEnemies(NUM_STARTING_ASTEROIDS, NUM_STARTING_BEETLES, NUM_STARTING_WASPS);
				m_isAttractMode = false;
				m_wave = 0;
				SoundID selectSound = g_theAudio->CreateOrGetSound("Data/Audio/Select.wav");
				g_theAudio->StartSound(selectSound);
				return;
			}
			else
			{
				g_theApp->HandleQuitRequested();
			}
		}

		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.5f && m_joystickMenuDelay < 0.f)
		{
			m_isStartOrExit = !m_isStartOrExit;
			m_joystickMenuDelay = 0.2f;
			SoundID selectSound = g_theAudio->CreateOrGetSound("Data/Audio/Select.wav");
			g_theAudio->StartSound(selectSound);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::BACK))
		{
			m_theApp->HandleQuitRequested();
		}

		return;
	}

	if (controller.WasButtonJustPressed(XboxButtonID::BACK)) 
	{
		m_isAttractMode = true;
		m_isStartOrExit = true;
		SoundID selectSound = g_theAudio->CreateOrGetSound("Data/Audio/Select.wav");
		g_theAudio->StartSound(selectSound);
		ResetGame();

		return;
	}
	
	if (controller.WasButtonJustPressed(XboxButtonID::BUTTON_B))
	{
		if (m_gameClock.IsPaused())
		{
			m_gameClock.Unpause();
			SoundID unpauseSound = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.wav");
			g_theAudio->StartSound(unpauseSound, false, .25f);
		}
		else
		{
			m_gameClock.Pause();
			SoundID pauseSound = g_theAudio->CreateOrGetSound("Data/Audio/Pause.wav");
			g_theAudio->StartSound(pauseSound, false, .25f);
		}
	}
	
	if (g_theInput->IsKeyDown('T') || controller.IsButtonDown(XboxButtonID::BUTTON_X))
	{
		m_isSlowMo = true;
	}
	else if (!g_theInput->IsKeyDown('T') || !controller.IsButtonDown(XboxButtonID::BUTTON_X))
	{
		m_isSlowMo = false;
	}

	if (controller.WasButtonJustPressed(XboxButtonID::RIGHT_SHOULDER) && m_playerShip->m_bulletCount < 3)
	{
		m_playerShip->m_bulletCount++;
	}

	if (controller.WasButtonJustPressed(XboxButtonID::LEFT_SHOULDER) && m_playerShip->m_bulletCount > 1)
	{
		m_playerShip->m_bulletCount--;
	}
}


void Game::HandleCollision()
{
	//bullet collision
	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		Bullet* curBullet = m_bullets[bulletIndex];
		if (!curBullet) continue;
		Vec2 bulletCenter = curBullet->m_position;
		float bulletRadius = curBullet->m_physicsRadius;

		// bullet asteroid collision
		for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
		{
			Asteroid* curAsteroid = m_asteroids[asteroidIndex];
			if (!curAsteroid) continue;
			Vec2 asteroidCenter = curAsteroid->m_position;
			float asteroidRadius = curAsteroid->m_physicsRadius;
			if (DoDiscsOverlap2D(bulletCenter, bulletRadius, asteroidCenter, asteroidRadius))
			{
				curAsteroid->health--;
				Rgba8 color = curBullet->m_mainColor;
				Vec2 velocity = curBullet->m_velocity;
				SpawnDebrisCluster(DEBRIS_ON_BULLET_IMPACT, curBullet->m_position, color,
								   -(velocity.GetClamped(DEBRIS_BULLET_MAX_SPEED)),
								   DEBRIS_BULLET_MIN_SCALE, DEBRIS_BULLET_MAX_SCALE,
								   DEBRIS_BULLET_MIN_SPEED, DEBRIS_BULLET_MAX_SPEED);
				if (curAsteroid->health == 0) 
				{
					curAsteroid->Die();
					AddScreenshake(ENEMY_DEATH_SCREENSHAKE_OFFSET);
					SoundID explosionSound = g_theAudio->CreateOrGetSound("Data/Audio/Explosion.wav");
					g_theAudio->StartSound(explosionSound);
					//get a 1/2 chance spawn a power up if asteroid is dead
					if (m_powerupCounter < MAX_POWERUPS && m_playerShip->m_bulletCount < 3)
					{
						int roll = RNG.RollRandomIntInRange(1, 2);
						if (roll == 1)
						{
							SpawnPowerup(curAsteroid->m_position);
						}
					}
				}
				curBullet->Die();
				AddScreenshake(BULLET_COLLISION_SCREENSHAKE_OFFSET);
				SoundID enemyHitSound = g_theAudio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
				g_theAudio->StartSound(enemyHitSound);
			}
		}

		//bullet beetle collision
		for (int beetleIndex = 0; beetleIndex < MAX_BEETLES; beetleIndex++)
		{
			Beetle* curBeetle = m_beetles[beetleIndex];
			if (!curBeetle) continue;
			Vec2 beetleCenter = curBeetle->m_position;
			float beetleRadius = curBeetle->m_physicsRadius;
			if (DoDiscsOverlap2D(bulletCenter, bulletRadius, beetleCenter, beetleRadius))
			{
				curBeetle->health--;
				Rgba8 color = curBullet->m_mainColor;
				Vec2 velocity = curBullet->m_velocity;
				SpawnDebrisCluster(DEBRIS_ON_BULLET_IMPACT, curBullet->m_position, color, 
								   -(velocity.GetClamped(DEBRIS_BULLET_MAX_SPEED)),
								   DEBRIS_BULLET_MIN_SCALE, DEBRIS_BULLET_MAX_SCALE,
								   DEBRIS_BULLET_MIN_SPEED, DEBRIS_BULLET_MAX_SPEED);
				if (curBeetle->health == 0) 
				{
					curBeetle->Die();
					AddScreenshake(ENEMY_DEATH_SCREENSHAKE_OFFSET);
					SoundID explosionSound = g_theAudio->CreateOrGetSound("Data/Audio/Explosion.wav");
					g_theAudio->StartSound(explosionSound);
				}
				curBullet->Die();
				AddScreenshake(BULLET_COLLISION_SCREENSHAKE_OFFSET);
				SoundID enemyHitSound = g_theAudio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
				g_theAudio->StartSound(enemyHitSound);
			}
		}

		//bullet wasp collision
		for (int waspIndex = 0; waspIndex < MAX_WASPS; waspIndex++)
		{
			Wasp* curWasp = m_wasps[waspIndex];
			if (!curWasp) continue;
			Vec2 waspCenter = curWasp->m_position;
			float waspRadius = curWasp->m_physicsRadius;
			if (DoDiscsOverlap2D(bulletCenter, bulletRadius, waspCenter, waspRadius))
			{
				curWasp->health--;
				Rgba8 color = curBullet->m_mainColor;
				Vec2 velocity = curBullet->m_velocity;
				SpawnDebrisCluster(DEBRIS_ON_BULLET_IMPACT, curBullet->m_position, color, 
								   -(velocity.GetClamped(DEBRIS_BULLET_MAX_SPEED)),
								   DEBRIS_BULLET_MIN_SCALE, DEBRIS_BULLET_MAX_SCALE,
								   DEBRIS_BULLET_MIN_SPEED, DEBRIS_BULLET_MAX_SPEED);
				if (curWasp->health == 0) 
				{
					curWasp->Die();
					AddScreenshake(ENEMY_DEATH_SCREENSHAKE_OFFSET);
					SoundID explosionSound = g_theAudio->CreateOrGetSound("Data/Audio/Explosion.wav");
					g_theAudio->StartSound(explosionSound);
				}
				curBullet->Die();
				AddScreenshake(BULLET_COLLISION_SCREENSHAKE_OFFSET);
				SoundID enemyHitSound = g_theAudio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
				g_theAudio->StartSound(enemyHitSound);
			}
		}
	}

	//asteroid collision with player
	for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
	{
		Asteroid* curAsteroid = m_asteroids[asteroidIndex];
		if (!curAsteroid) continue;
		Vec2 asteroidCenter = curAsteroid->m_position;
		float asteroidRadius = curAsteroid->m_physicsRadius;
		if (!m_playerShip->m_isGarbage)
		{
			Vec2 playerCenter = m_playerShip->m_position;
			float playerRadius = m_playerShip->m_physicsRadius;
			if (DoDiscsOverlap2D(asteroidCenter, asteroidRadius, playerCenter, playerRadius))
			{
				curAsteroid->Die();
				m_playerShip->Die();
				AddScreenshake(PLAYER_DEATH_SCREENSHAKE_OFFSET);
				SoundID playerDiedSound = g_theAudio->CreateOrGetSound("Data/Audio/PlayerDied.wav");
				g_theAudio->StartSound(playerDiedSound);
				Vec2 pos = m_playerShip->m_position;
				Rgba8 color = m_playerShip->m_mainColor;
				Vec2 velocity = m_playerShip->m_velocity;
				SpawnDebrisCluster(DEBRIS_ON_PLAYER_DEATH, pos, color, velocity,
					DEBRIS_MIN_SCALE, DEBRIS_MAX_SCALE,
					DEBRIS_MIN_SPEED, DEBRIS_MAX_SPEED);
			}
		}
	}

	//beetle collision with player
	for (int beetleIndex = 0; beetleIndex < MAX_BEETLES; beetleIndex++)
	{
		Beetle* curBeetle = m_beetles[beetleIndex];
		if (!curBeetle) continue;
		Vec2 beetleCenter = curBeetle->m_position;
		float beetleRadius = curBeetle->m_physicsRadius;
		if (!m_playerShip->m_isGarbage)
		{
			Vec2 playerCenter = m_playerShip->m_position;
			float playerRadius = m_playerShip->m_physicsRadius;
			if (DoDiscsOverlap2D(beetleCenter, beetleRadius, playerCenter, playerRadius))
			{
				curBeetle->Die();
				m_playerShip->Die();
				AddScreenshake(PLAYER_DEATH_SCREENSHAKE_OFFSET);
				SoundID playerDiedSound = g_theAudio->CreateOrGetSound("Data/Audio/PlayerDied.wav");
				g_theAudio->StartSound(playerDiedSound);
				Vec2 pos = m_playerShip->m_position;
				Rgba8 color = m_playerShip->m_mainColor;
				Vec2 velocity = m_playerShip->m_velocity;
				SpawnDebrisCluster(DEBRIS_ON_PLAYER_DEATH, pos, color, velocity,
					DEBRIS_MIN_SCALE, DEBRIS_MAX_SCALE,
					DEBRIS_MIN_SPEED, DEBRIS_MAX_SPEED);
			}
		}
	}

	//wasp collision with player
	for (int waspIndex = 0; waspIndex < MAX_WASPS; waspIndex++)
	{
		Wasp* curWasp = m_wasps[waspIndex];
		if (!curWasp) continue;
		Vec2 waspCenter = curWasp->m_position;
		float waspRadius = curWasp->m_physicsRadius;
		if (!m_playerShip->m_isGarbage)
		{
			Vec2 playerCenter = m_playerShip->m_position;
			float playerRadius = m_playerShip->m_physicsRadius;
			if (DoDiscsOverlap2D(waspCenter, waspRadius, playerCenter, playerRadius))
			{
				curWasp->Die();
				m_playerShip->Die();
				AddScreenshake(PLAYER_DEATH_SCREENSHAKE_OFFSET);
				SoundID playerDiedSound = g_theAudio->CreateOrGetSound("Data/Audio/PlayerDied.wav");
				g_theAudio->StartSound(playerDiedSound);
				Vec2 pos = m_playerShip->m_position;
				Rgba8 color = m_playerShip->m_mainColor;
				Vec2 velocity = m_playerShip->m_velocity;
				SpawnDebrisCluster(DEBRIS_ON_PLAYER_DEATH, pos, color, velocity,
					DEBRIS_MIN_SCALE, DEBRIS_MAX_SCALE,
					DEBRIS_MIN_SPEED, DEBRIS_MAX_SPEED);
			}
		}
	}

	//power up collision with player
	for (int powerupIndex = 0; powerupIndex < MAX_POWERUPS; powerupIndex++)
	{
		Powerup* curPowerup = m_powerups[powerupIndex];
		if (!curPowerup) continue;
		Vec2 powerupCenter = curPowerup->m_position;
		float powerupRadius = curPowerup->m_physicsRadius;
		if (!m_playerShip->m_isGarbage)
		{
			Vec2 playerCenter = m_playerShip->m_position;
			float playerRadius = m_playerShip->m_physicsRadius;
			if (DoDiscsOverlap2D(powerupCenter, powerupRadius, playerCenter, playerRadius))
			{
				curPowerup->Die();
				SpawnDebrisCluster(DEBRIS_ON_PLAYER_DEATH, powerupCenter, Rgba8(255, 255, 255, 255), curPowerup->m_velocity,
					DEBRIS_BULLET_MIN_SCALE, DEBRIS_BULLET_MAX_SCALE,
					DEBRIS_BULLET_MIN_SPEED, DEBRIS_BULLET_MAX_SPEED);
				if (m_playerShip->m_bulletCount <3) m_playerShip->m_bulletCount += 1;
				SoundID powerupSound = g_theAudio->CreateOrGetSound("Data/Audio/Powerup.wav");
				g_theAudio->StartSound(powerupSound);
			}
		}
	}
}


void Game::AddScreenshake(float deltaScreenshake)
{
	m_screenshakeAmount += deltaScreenshake;
	m_screenshakeAmount = Clamp(m_screenshakeAmount, 0.f, MAX_SCREENSHAKE_OFFSET);
}


void Game::DeleteGarbageEntities()
{
	for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
	{
		Asteroid*& curAsteroid = m_asteroids[asteroidIndex];
		if (curAsteroid)
		{
			if (curAsteroid->m_isGarbage) 
			{
				Vec2 pos = curAsteroid->m_position;
				Rgba8 color = curAsteroid->m_mainColor;
				Vec2 velocity = curAsteroid->m_velocity;
				SpawnDebrisCluster(DEBRIS_ON_ENTITY_DEATH, pos, color, velocity,
								   DEBRIS_MIN_SCALE, DEBRIS_MAX_SCALE, 
								   DEBRIS_MIN_SPEED, DEBRIS_MAX_SPEED);
				delete curAsteroid;
				curAsteroid = nullptr;
				m_asteroidCounter--;
			}
		}
	}

	for (int beetleIndex = 0; beetleIndex < MAX_BEETLES; beetleIndex++)
	{
		Beetle*& curBeetle = m_beetles[beetleIndex];
		if (curBeetle)
		{
			if (curBeetle->m_isGarbage) 
			{
				Vec2 pos = curBeetle->m_position;
				Rgba8 color = curBeetle->m_mainColor;
				Vec2 velocity = curBeetle->m_velocity;
				SpawnDebrisCluster(DEBRIS_ON_ENTITY_DEATH, pos, color, velocity,
								   DEBRIS_MIN_SCALE, DEBRIS_MAX_SCALE,
								   DEBRIS_MIN_SPEED, DEBRIS_MAX_SPEED);
				delete curBeetle;
				curBeetle = nullptr;
				m_beetleCounter--;
			}
		}
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; waspIndex++)
	{
		Wasp*& curWasp = m_wasps[waspIndex];
		if (curWasp)
		{
			if (curWasp->m_isGarbage) 
			{
				Vec2 pos = curWasp->m_position;
				Rgba8 color = curWasp->m_mainColor;
				Vec2 velocity = curWasp->m_velocity;
				SpawnDebrisCluster(DEBRIS_ON_ENTITY_DEATH, pos, color, velocity,
								   DEBRIS_MIN_SCALE, DEBRIS_MAX_SCALE,
								   DEBRIS_MIN_SPEED, DEBRIS_MAX_SPEED);
				delete curWasp;
				curWasp = nullptr;
				m_waspCounter--;
			}
		}
	}

	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		Bullet*& curBullet = m_bullets[bulletIndex];
		if (curBullet)
		{
			if (curBullet->m_isGarbage) 
			{
				delete curBullet;
				curBullet = nullptr;
				m_bulletCounter--;
			}
		}
	}

	for (int debrisIndex = 0; debrisIndex < MAX_DEBRIS; debrisIndex++)
	{
		Debris*& curDebris = m_debris[debrisIndex];
		if (curDebris)
		{
			if (curDebris->m_isGarbage) 
			{
				delete curDebris;
				curDebris = nullptr;
				m_debrisCounter--;
			}
		}
	}

	for (int powerupIndex = 0; powerupIndex < MAX_POWERUPS; powerupIndex++)
	{
		Powerup*& curPowerup = m_powerups[powerupIndex];
		if (curPowerup)
		{
			if (curPowerup->m_isGarbage)
			{
				delete curPowerup;
				curPowerup = nullptr;
				m_powerupCounter--;
			}
		}
	}
}


void Game::ResetGame()
{
	m_isAttractMode = true;
	m_isStartOrExit = true;
	m_isWinning = false;
	m_isLosing = false;
	m_wave = 0;
	m_returnTimer = 0.f;

	for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
	{
		Asteroid*& curAsteroid = m_asteroids[asteroidIndex];
		if (curAsteroid)
		{
			m_asteroidCounter--;
			delete curAsteroid;
			curAsteroid = nullptr;
		}
	}

	for (int beetleIndex = 0; beetleIndex < MAX_BEETLES; beetleIndex++)
	{
		Beetle*& curBeetle = m_beetles[beetleIndex];
		if (curBeetle)
		{
			m_beetleCounter--;
			delete curBeetle;
			curBeetle = nullptr;
		}
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; waspIndex++)
	{
		Wasp*& curWasp = m_wasps[waspIndex];
		if (curWasp)
		{
			m_waspCounter--;
			delete curWasp;
			curWasp = nullptr;
		}
	}

	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		Bullet*& curBullet = m_bullets[bulletIndex];
		if (curBullet)
		{
			m_bulletCounter--;
			delete curBullet;
			curBullet = nullptr;
		}
	}

	for (int debrisIndex = 0; debrisIndex < MAX_DEBRIS; debrisIndex++)
	{
		Debris*& curDebris = m_debris[debrisIndex];
		if (curDebris)
		{
			m_debrisCounter--;
			delete curDebris;
			curDebris = nullptr;
		}
	}

	for (int powerupIndex = 0; powerupIndex < MAX_POWERUPS; powerupIndex++)
	{
		Powerup*& curPowerup = m_powerups[powerupIndex];
		if (curPowerup)
		{
			m_powerupCounter--;
			delete curPowerup;
			curPowerup = nullptr;
		}
	}
}


void Game::DebugRender() const
{
	m_playerShip->DebugRender();

	for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
	{
		Asteroid* curAsteroid = m_asteroids[asteroidIndex];
		if (curAsteroid)
		{
			curAsteroid->DebugRender();
			DebugDrawLine(m_playerShip->m_position, curAsteroid->m_position, 
						  DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
		}
	}

	for (int beetleIndex = 0; beetleIndex < MAX_BEETLES; beetleIndex++)
	{
		Beetle* curBeetle = m_beetles[beetleIndex];
		if (curBeetle)
		{
			curBeetle->DebugRender();
			DebugDrawLine(m_playerShip->m_position, curBeetle->m_position, 
						  DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
		}
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; waspIndex++)
	{
		Wasp* curWasp = m_wasps[waspIndex];
		if (curWasp)
		{
			curWasp->DebugRender();
			DebugDrawLine(m_playerShip->m_position, curWasp->m_position, 
						  DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
		}
	}

	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		Bullet* curBullet = m_bullets[bulletIndex];
		if (curBullet)
		{
			curBullet->DebugRender();
			DebugDrawLine(m_playerShip->m_position, curBullet->m_position,
					      DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
		}
	}

	for (int debrisIndex = 0; debrisIndex < MAX_DEBRIS; debrisIndex++)
	{
		Debris* curDebris = m_debris[debrisIndex];
		if (curDebris)
		{
			curDebris->DebugRender();
			DebugDrawLine(m_playerShip->m_position, curDebris->m_position, 
						  DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
		}
	}

	for (int powerupIndex = 0; powerupIndex < MAX_POWERUPS; powerupIndex++)
	{
		Powerup* curPowerup = m_powerups[powerupIndex];
		if (curPowerup)
		{
			curPowerup->DebugRender();
			DebugDrawLine(m_playerShip->m_position, curPowerup->m_position,
				DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
		}
	}
}


PlayerShip* Game::GetNearestPlayerShip() const
{
	return m_playerShip;
}


Vec2 Game::RollSpawnPos(float cosmeticRadius)
{
	int spawningArea = RNG.RollRandomIntInRange(0, 3);
	if (spawningArea == 0) // left
	{
		float startY = RNG.RollRandomFloatInRange(cosmeticRadius, WORLD_SIZE_Y - cosmeticRadius);
		return Vec2(-cosmeticRadius, startY);
	}
	else if (spawningArea == 1) //right
	{
		float startY = RNG.RollRandomFloatInRange(cosmeticRadius, WORLD_SIZE_Y - cosmeticRadius);
		return Vec2(WORLD_SIZE_X + cosmeticRadius, startY);
	}
	else if (spawningArea == 2) //top
	{
		float startX = RNG.RollRandomFloatInRange(cosmeticRadius, WORLD_SIZE_X - cosmeticRadius);
		return Vec2(startX, WORLD_SIZE_Y + cosmeticRadius);
	}
	else if (spawningArea == 3) //bottom
	{
		float startX = RNG.RollRandomFloatInRange(cosmeticRadius, WORLD_SIZE_X - cosmeticRadius);
		return Vec2(startX, -cosmeticRadius);
	}
	else { //error
		return Vec2(0.f, 0.f);
	}
}


void Game::SpawnStarfield()
{
	Rgba8 colorStar = Rgba8(255, 255, 255, 150);
	for (int starIndex = 0; starIndex < MAX_STARS; starIndex++)
	{
		float backgroundStarX = RNG.RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		float backgroundStarY = RNG.RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
		m_starfieldBackground[3 * starIndex] = Vertex_PCU(Vec3(backgroundStarX + .3f, backgroundStarY, 0.f), colorStar, Vec2(0.f, 0.f));
		m_starfieldBackground[3 * starIndex + 1] = Vertex_PCU(Vec3(backgroundStarX - .15f, backgroundStarY + .15f, 0.f), colorStar, Vec2(0.f, 0.f));
		m_starfieldBackground[3 * starIndex + 2] = Vertex_PCU(Vec3(backgroundStarX - .15f, backgroundStarY - .15f, 0.f), colorStar, Vec2(0.f, 0.f));

		float foregroundStarX = RNG.RollRandomFloatInRange(-.25f * WORLD_SIZE_X, 1.25f * WORLD_SIZE_X);
		float foregroundStarY = RNG.RollRandomFloatInRange(-.25f * WORLD_SIZE_Y, 1.25f * WORLD_SIZE_Y);
		m_starfieldForeground[3 * starIndex] = Vertex_PCU(Vec3(foregroundStarX + .3f, foregroundStarY, 0.f), colorStar, Vec2(0.f, 0.f));
		m_starfieldForeground[3 * starIndex + 1] = Vertex_PCU(Vec3(foregroundStarX - .15f, foregroundStarY + .15f, 0.f), colorStar, Vec2(0.f, 0.f));
		m_starfieldForeground[3 * starIndex + 2] = Vertex_PCU(Vec3(foregroundStarX - .15f, foregroundStarY - .15f, 0.f), colorStar, Vec2(0.f, 0.f));
	}
}


void Game::DrawLevelTextUI() const
{
	int level = (m_wave == MAX_WAVE) ? m_wave : m_wave + 1;
	std::vector<Vertex_PCU> levelTextVertexArray;
	AddVertsForTextTriangles2D(levelTextVertexArray, "Level: " + std::to_string(level), Vec2(30.f, 755.f), 20.f, Rgba8(255, 255, 255, 255));
	g_theRenderer->DrawVertexArray(int(levelTextVertexArray.size()), &levelTextVertexArray[0]);
}


void Game::DrawPlayerLivesUI() const
{
	std::vector<Vertex_PCU> livesTextVertexArray;
	AddVertsForTextTriangles2D(livesTextVertexArray, "Lives: ", Vec2(30.f, 715.f), 20.f, Rgba8(255, 255, 255, 255));
	g_theRenderer->DrawVertexArray(int(livesTextVertexArray.size()), &livesTextVertexArray[0]);

	for (int liveIndex = 0; liveIndex < m_playerShip->health; liveIndex++)
	{
		Rgba8 shipColor = m_playerShip->m_mainColor;
		DrawPlayerShip(Vec2(135.f, 730.f) + (Vec2(40.f, 0.f) * static_cast<float>(liveIndex)), shipColor, 8.f, 90.f);
	}
}


void Game::DrawWeaponUI() const
{
	std::vector<Vertex_PCU> weaponTextVertexArray;
	AddVertsForTextTriangles2D(weaponTextVertexArray, "Weapon: ", Vec2(30.f, 675.f), 20.f, Rgba8(255, 255, 255, 255));
	g_theRenderer->DrawVertexArray(int(weaponTextVertexArray.size()), &weaponTextVertexArray[0]);

	for (int bulletIndex = 0; bulletIndex < m_playerShip->m_bulletCount; bulletIndex++)
	{
		DrawBullet(Vec2(135.f, 690.f) + (Vec2(10.f, 0.f) * static_cast<float>(bulletIndex)), Rgba8(255, 255, 0, 255), 8.f, 90.f);
	}
}


void Game::DrawEnemyCountsUI() const
{
	std::vector<Vertex_PCU> enemyTextVertexArray;
	AddVertsForTextTriangles2D(enemyTextVertexArray, "Enemies: ", Vec2(30.f, 635.f), 20.f, Rgba8(255, 255, 255, 255));
	g_theRenderer->DrawVertexArray(int(enemyTextVertexArray.size()), &enemyTextVertexArray[0]);

	for (int beetleIndex = 0; beetleIndex < m_beetleCounter; beetleIndex++)
	{
		DrawBeetle(Vec2(150.f, 655.f) + (Vec2(20.f, 0.f) * static_cast<float>(beetleIndex)), Rgba8(0, 175, 0, 255), 4.f, 90.f);
	}

	for (int waspIndex = 0; waspIndex < m_waspCounter; waspIndex++)
	{
		DrawWasp(Vec2(150.f, 635.f) + (Vec2(20.f, 0.f) * static_cast<float>(waspIndex)), Rgba8(255, 255, 0, 255), 4.f, 90.f);
	}
}


void Game::DrawEnemiesHealthBar() const
{
	Vec2 static offset = Vec2(-2.f, 3.f);
	for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
	{
		Asteroid* curAsteroid = m_asteroids[asteroidIndex];
		if (curAsteroid)
		{
			float percentage = static_cast<float>(curAsteroid->health) / static_cast<float>(curAsteroid->maxHealth);
			Vec2 healthBarStartPos = curAsteroid->m_position + offset;
			float healthBarLength = Interpolate(0.f, 4.f, percentage);
			Vec2 healthBarEndPos = healthBarStartPos + Vec2(healthBarLength, 0.f);
			DebugDrawLine(healthBarStartPos, healthBarEndPos, HEALTH_BAR_THICKNESS, Rgba8(255, 0, 0, 255));
		}
	}

	for (int beetleIndex = 0; beetleIndex < MAX_BEETLES; beetleIndex++)
	{
		Beetle* curBeetle = m_beetles[beetleIndex];
		if (curBeetle)
		{
			float percentage = static_cast<float>(curBeetle->health) / static_cast<float>(curBeetle->maxHealth);
			Vec2 healthBarStartPos = curBeetle->m_position + offset;
			float healthBarLength = Interpolate(0.f, 4.f, percentage);
			Vec2 healthBarEndPos = healthBarStartPos + Vec2(healthBarLength, 0.f);
			DebugDrawLine(healthBarStartPos, healthBarEndPos, HEALTH_BAR_THICKNESS, Rgba8(255, 0, 0, 255));
		}
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; waspIndex++)
	{
		Wasp* curWasp = m_wasps[waspIndex];
		if (curWasp)
		{
			float percentage = static_cast<float>(curWasp->health) / static_cast<float>(curWasp->maxHealth);
			Vec2 healthBarStartPos = curWasp->m_position + offset;
			float healthBarLength = Interpolate(0.f, 4.f, percentage);
			Vec2 healthBarEndPos = healthBarStartPos + Vec2(healthBarLength, 0.f);
			DebugDrawLine(healthBarStartPos, healthBarEndPos, HEALTH_BAR_THICKNESS, Rgba8(255, 0, 0, 255));
		}
	}
}


void Game::DrawWinningUI() const
{
	float percentage = Clamp(m_returnTimer, 0.f, 1.5f);
	std::vector<Vertex_PCU> winTextVertexArray;
	AddVertsForTextTriangles2D(winTextVertexArray, "You Win", Vec2(775.f - 100.f * percentage, 415.f - 50.f * percentage), 50.f * percentage, Rgba8(0, 0, 255, 255));
	g_theRenderer->DrawVertexArray(int(winTextVertexArray.size()), &winTextVertexArray[0]);
}


void Game::DrawLosingUI() const
{
	float percentage = Clamp(m_returnTimer, 0.f, 1.5f);
	std::vector<Vertex_PCU> loseTextVertexArray;
	AddVertsForTextTriangles2D(loseTextVertexArray, "You Lose", Vec2(775.f - 100.f * percentage, 415.f - 50.f * percentage), 50.f * percentage, Rgba8(255, 0, 0, 255));
	g_theRenderer->DrawVertexArray(int(loseTextVertexArray.size()), &loseTextVertexArray[0]);
}


void Game::DrawAttractScreen() const
{
	static int frames = 0;
	float offset = SinDegrees(static_cast<float>(frames * 2.f)) * 75.f;
	Vec2 veclocity = Vec2::MakeFromPolarDegrees(200.f) * 2.f;
	DrawBeetle(Vec2(200.f, 110.f) + static_cast<float>(frames) * veclocity, Rgba8(0, 175, 0, 255), 1.5f, 200.f);
	DrawWasp(Vec2(223.f, 105.f) + static_cast<float>(frames) * veclocity, Rgba8(255, 255, 0, 255), 1.5f, 200.f);
	DrawPlayerShip(Vec2(200.f, 80.f) + static_cast<float>(frames) * veclocity, Rgba8(102, 153, 204, 255), 1.5f, 200.f);
	DrawBeetle(Vec2(250.f, 95.f) + static_cast<float>(frames) * veclocity, Rgba8(0, 175, 0, 255), 1.5f, 200.f);
	DrawWasp(Vec2(240.f, 75.f) + static_cast<float>(frames) * veclocity, Rgba8(255, 255, 0, 255), 1.5f, 200.f);
	frames++;
	if (frames > 180) frames = 0;

	std::vector<Vertex_PCU> starshipTitleShadowVertexArray;
	AddVertsForTextTriangles2D(starshipTitleShadowVertexArray, "Starship", Vec2(14.f, 64.f), 20.f, Rgba8(50, 50, 50, 100 - static_cast<unsigned char>(offset)));
	g_theRenderer->DrawVertexArray(int(starshipTitleShadowVertexArray.size()), &starshipTitleShadowVertexArray[0]);

	std::vector<Vertex_PCU> starshipTitleVertexArray;
	AddVertsForTextTriangles2D(starshipTitleVertexArray, "Starship", Vec2(15.f, 63.f), 20.f, Rgba8(100, 100, 255, 150 - static_cast<unsigned char>(offset)));
	g_theRenderer->DrawVertexArray(int(starshipTitleVertexArray.size()), &starshipTitleVertexArray[0]);

	std::vector<Vertex_PCU> startGameVertexArray;
	AddVertsForTextTriangles2D(startGameVertexArray, "START", Vec2(165.f, 20.f), 5.f, Rgba8(255, 255, 255, 255));
	g_theRenderer->DrawVertexArray(int(startGameVertexArray.size()), &startGameVertexArray[0]);

	std::vector<Vertex_PCU> endGameVertexArray;
	AddVertsForTextTriangles2D(endGameVertexArray, "EXIT", Vec2(165.f, 10.f), 5.f, Rgba8(255, 255, 255, 255));
	g_theRenderer->DrawVertexArray(int(endGameVertexArray.size()), &endGameVertexArray[0]);
	
	if (m_isStartOrExit)
	{
		DrawSelectTriangle(Vec2(162.f, 23.5f), Rgba8(255, 255, 255, 255));
	}
	else
	{
		DrawSelectTriangle(Vec2(162.f, 13.5f), Rgba8(255, 255, 255, 255));
	}
}


void Game::DrawSelectTriangle(Vec2 pos, Rgba8 color) const
{
	Vertex_PCU selectTriangleVerts[3] = {};
	selectTriangleVerts[0] = Vertex_PCU(Vec3(0.f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 1
	selectTriangleVerts[1] = Vertex_PCU(Vec3(-2.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 2
	selectTriangleVerts[2] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 3

	TransformVertexArrayXY3D(3, selectTriangleVerts, 1.5f, 0.f, pos);
	g_theRenderer->DrawVertexArray(3, selectTriangleVerts);
}


void Game::DrawBeetle(Vec2 pos, Rgba8 color, float scale, float rotation) const
{
	Vertex_PCU beetleVerts[NUM_BEETLE_VERTICES] = {};
	beetleVerts[0] = Vertex_PCU(Vec3(0.f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 1
	beetleVerts[1] = Vertex_PCU(Vec3(-2.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 2
	beetleVerts[2] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 3

	beetleVerts[3] = Vertex_PCU(Vec3(-1.f, 2.f, 0.f), color, Vec2(0.f, 0.f)); //triangle B vertex 1
	beetleVerts[4] = Vertex_PCU(Vec3(-1.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle B vertex 2
	beetleVerts[5] = Vertex_PCU(Vec3(2.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle B vertex 3

	beetleVerts[6] = Vertex_PCU(Vec3(-1.f, -2.f, 0.f), color, Vec2(0.f, 0.f)); //triangle C vertex 1
	beetleVerts[7] = Vertex_PCU(Vec3(-1.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle C vertex 2
	beetleVerts[8] = Vertex_PCU(Vec3(2.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle C vertex 3

	TransformVertexArrayXY3D(NUM_BEETLE_VERTICES, beetleVerts, scale, rotation, pos);
	g_theRenderer->DrawVertexArray(NUM_BEETLE_VERTICES, beetleVerts);
}


void Game::DrawWasp(Vec2 pos, Rgba8 color, float scale, float rotation) const
{
	Vertex_PCU waspVerts[NUM_WASP_VERTICES] = {};
	waspVerts[0] = Vertex_PCU(Vec3(2.f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 1
	waspVerts[1] = Vertex_PCU(Vec3(0.5f, 1.5f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 2
	waspVerts[2] = Vertex_PCU(Vec3(0.5f, -1.5f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 3

	waspVerts[3] = Vertex_PCU(Vec3(2.f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle B vertex 1
	waspVerts[4] = Vertex_PCU(Vec3(-1.5f, 1.5f, 0.f), color, Vec2(0.f, 0.f)); //triangle B vertex 2
	waspVerts[5] = Vertex_PCU(Vec3(-1.f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle B vertex 3

	waspVerts[6] = Vertex_PCU(Vec3(2.f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle C vertex 1
	waspVerts[7] = Vertex_PCU(Vec3(-1.5f, -1.5f, 0.f), color, Vec2(0.f, 0.f)); //triangle C vertex 2
	waspVerts[8] = Vertex_PCU(Vec3(-1.f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle C vertex 3

	waspVerts[9] = Vertex_PCU(Vec3(0.f, .75f, 0.f), color, Vec2(0.f, 0.f)); //triangle D vertex 1
	waspVerts[10] = Vertex_PCU(Vec3(0.f, -.75f, 0.f), color, Vec2(0.f, 0.f)); //triangle D vertex 2
	waspVerts[11] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle D vertex 3

	TransformVertexArrayXY3D(NUM_WASP_VERTICES, waspVerts, scale, rotation, pos);
	g_theRenderer->DrawVertexArray(NUM_WASP_VERTICES, waspVerts);
}


void Game::DrawPlayerShip(Vec2 pos, Rgba8 color, float scale, float rotation) const
{
	Vertex_PCU shipVerts[NUM_SHIP_VERTICES] = {};
	shipVerts[0] = Vertex_PCU(Vec3(0.f, 2.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 1
	shipVerts[1] = Vertex_PCU(Vec3(-2.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 2
	shipVerts[2] = Vertex_PCU(Vec3(2.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 3

	shipVerts[3] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle B vertex 1
	shipVerts[4] = Vertex_PCU(Vec3(-2.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle B vertex 2
	shipVerts[5] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle B vertex 3

	shipVerts[6] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle C vertex 1
	shipVerts[7] = Vertex_PCU(Vec3(0.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle C vertex 2
	shipVerts[8] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle C vertex 3

	shipVerts[9] = Vertex_PCU(Vec3(1.f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle D vertex 1
	shipVerts[10] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle D vertex 2
	shipVerts[11] = Vertex_PCU(Vec3(0.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle D vertex 3

	shipVerts[12] = Vertex_PCU(Vec3(0.f, -2.f, 0.f), color, Vec2(0.f, 0.f)); //triangle E vertex 1
	shipVerts[13] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle E vertex 2
	shipVerts[14] = Vertex_PCU(Vec3(2.f, -1.f, 0.f), color, Vec2(0.f, 0.f)); //triangle E vertex 3

	TransformVertexArrayXY3D(NUM_SHIP_VERTICES, shipVerts, scale, rotation, pos);
	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTICES, shipVerts);
}


void Game::DrawBullet(Vec2 pos, Rgba8 color, float scale, float rotation) const
{
	Vertex_PCU bulletVerts[NUM_BULLET_VERTICES] = {};
	bulletVerts[0] = Vertex_PCU(Vec3(0.f, 0.5f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 1
	bulletVerts[1] = Vertex_PCU(Vec3(0.5f, 0.f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 2
	bulletVerts[2] = Vertex_PCU(Vec3(0.f, -0.5f, 0.f), color, Vec2(0.f, 0.f)); //triangle A vertex 3

	Rgba8 bulletTailColor = Rgba8(255, 0, 0, 255);
	bulletVerts[3] = Vertex_PCU(Vec3(0.f, -0.5f, 0.f), bulletTailColor, Vec2(0.f, 0.f)); //triangle B vertex 1
	bulletVerts[4] = Vertex_PCU(Vec3(0.f, 0.5f, 0.f), bulletTailColor, Vec2(0.f, 0.f)); //triangle B vertex 2
	Rgba8 bulletFadeColor = Rgba8(255, 0, 0, 0);
	bulletVerts[5] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), bulletFadeColor, Vec2(0.f, 0.f)); //triangle B vertex 3

	TransformVertexArrayXY3D(NUM_BULLET_VERTICES, bulletVerts, scale, rotation, pos);
	g_theRenderer->DrawVertexArray(NUM_BULLET_VERTICES, bulletVerts);
}


