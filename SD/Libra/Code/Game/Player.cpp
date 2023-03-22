#include "Game/Player.hpp"
#include "Game/Bullet.hpp"

Player::Player(Map* owner, Vec2 const& startPos, float orientation)
	: Entity(owner, startPos, orientation)
{
	m_faction = ENTITY_FACTION_GOOD;
	m_type = ENTITY_TYPE_GOOD_PLAYER;
	m_maxHealth = g_gameConfigBlackboard.GetValue("playerHealth", 0);
	m_health = g_gameConfigBlackboard.GetValue("playerHealth", 0);
	m_shootDelay = g_gameConfigBlackboard.GetValue("playerFireDelay", 0.f);
	m_isActor = true;
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_position = startPos;
	m_angularVelocity = g_gameConfigBlackboard.GetValue("playerTurnRate", 0.f);
	m_turretAngularVelocity = g_gameConfigBlackboard.GetValue("playerTurretTurnRate", 0.f);
	m_physicsRadius =  g_gameConfigBlackboard.GetValue("playerPhysicsRadius", 0.f);
	m_cosmeticRadius =  g_gameConfigBlackboard.GetValue("playerCosmeticRadius", 0.f);
	m_bodyTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankBase.png");
	m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankTop.png");
}


void Player::Update(float deltaSeconds)
{
	m_shootTimer += deltaSeconds;
	UpdateTransform(deltaSeconds);
	HandleKeyboardInput();
	HandleControllerInput();
}


void Player::Render() const
{
	RenderEntity();
	if (g_isDebugging)
	{
		DebugRender();
	}
}


void Player::UpdateTransform(float deltaSeconds)
{
	float speed = g_gameConfigBlackboard.GetValue("playerSpeed", 0.f);
	//body rotation & movement
	if (m_bodyMovementInput.GetLengthSquared() != 0)
	{
		m_targetBodyOrientation = m_bodyMovementInput.GetOrientationDegrees();
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, m_targetBodyOrientation, m_angularVelocity * deltaSeconds);

	}

	m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_bodyMovementInput.GetLength()) * speed;
	m_position += m_velocity * deltaSeconds;

	//turret rotation
	if (m_turretMovementInput.GetLengthSquared() != 0)
	{
		m_targetTurretOrientation = m_turretMovementInput.GetOrientationDegrees() - m_orientationDegrees;
		m_turretOrientationDegrees = GetTurnedTowardDegrees(m_turretOrientationDegrees, m_targetTurretOrientation, m_turretAngularVelocity * deltaSeconds);
	}
}


void Player::ReactToBulletHit(Bullet& bullet)
{
	Entity::ReactToBulletHit(bullet);
	PlaySound(g_soundIds[SOUND_TYPE_PLAYER_HIT], false, g_gameSoundVolume);
}


void Player::TakeDamage(int damage)
{
	if (g_isInvulnerable) damage = 0;
	Entity::TakeDamage(damage);
	if (m_health <= 0)
	{
		PlaySound(g_soundIds[SOUND_TYPE_ENEMY_DIE], false, g_gameSoundVolume);
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		m_map->AddExplosionToMap(m_position, randomOrientation, ExplosionType::PLAYER_DEATH);
	}
}


void Player::RenderEntity() const
{
	std::vector<Vertex_PCU> tankBaseVertexArray;
	OBB2 bodyBox(m_position, Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.5f, 0.5f) * m_scale);
	AddVertsForOBB2D(tankBaseVertexArray, bodyBox, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_bodyTexture);
	g_theRenderer->DrawVertexArray(int(tankBaseVertexArray.size()), tankBaseVertexArray.data());

	std::vector<Vertex_PCU> tankTopVertexArray;
	OBB2 turretBox(m_position, Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees + m_orientationDegrees), Vec2(0.5f, 0.5f) * m_scale);
	AddVertsForOBB2D(tankTopVertexArray, turretBox, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_turretTexture);
	g_theRenderer->DrawVertexArray(int(tankTopVertexArray.size()), tankTopVertexArray.data());
}


void Player::DebugRender() const
{
	Vec2 bodyForward = GetForwardNormal() * m_cosmeticRadius;
	Vec2 bodyLeft = bodyForward.GetRotated90Degrees();
	Vec2 turretForward = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * m_cosmeticRadius;
	float debugLineThickness = g_gameConfigBlackboard.GetValue("debugLineThickness", 0.f);
	DebugDrawRing(m_position, m_cosmeticRadius * m_scale, debugLineThickness, Rgba8(255, 0, 255, 255)); // draw cosmetic ring
	DebugDrawRing(m_position, m_physicsRadius * m_scale, debugLineThickness, Rgba8(0, 255, 255, 255)); // draw physics ring
	DebugDrawLine(m_position, m_position + bodyForward, debugLineThickness, Rgba8(255, 0, 0, 255)); // draw body forward vector
	DebugDrawLine(m_position, m_position + bodyLeft, debugLineThickness, Rgba8(0, 255, 0, 255)); // draw body left vector
	DebugDrawLine(m_position, m_position + m_velocity, debugLineThickness, Rgba8(255, 255, 0, 255)); // draw velocity vector
	DebugDrawLine(m_position, m_position + turretForward, debugLineThickness * 3.f, Rgba8(50, 50, 200, 175)); // draw turret forward vector

	//draw targeted body orientation
	Vec2 targetBodyOrientation = Vec2::MakeFromPolarDegrees(m_targetBodyOrientation);
	Vec2 targetBodyOrientationStart = m_position + targetBodyOrientation * (0.05f + m_cosmeticRadius);
	Vec2 targetBodyOrietationEnd = m_position + targetBodyOrientation * (0.1f + m_cosmeticRadius);
	DebugDrawLine(targetBodyOrientationStart, targetBodyOrietationEnd, debugLineThickness, Rgba8(255, 0, 0, 175));

	// draw target turret orientation
	Vec2 targetTurretOrientation = Vec2::MakeFromPolarDegrees(m_targetTurretOrientation + m_orientationDegrees);
	Vec2 targetTurretOrientationStart = m_position + targetTurretOrientation * (0.05f + m_cosmeticRadius);
	Vec2 targetTurretOrietationEnd = m_position + targetTurretOrientation * (0.1f + m_cosmeticRadius);
	DebugDrawLine(targetTurretOrientationStart, targetTurretOrietationEnd, debugLineThickness * 3.f, Rgba8(50, 50, 200, 175));

	if (g_isNoClip)
	{
		DebugDrawRing(m_position, (m_physicsRadius - 0.05f) * m_scale, debugLineThickness, Rgba8(0, 0, 0, 255));
	}

	if (g_isInvulnerable)
	{
		DebugDrawRing(m_position, (m_physicsRadius + 0.05f) * m_scale, debugLineThickness, Rgba8(255, 255, 255, 255));
	}
}


void Player::RotateAndScale(float deltaSeconds, int direction, float scale)
{
	m_orientationDegrees += static_cast<float>(direction) * m_angularVelocity * deltaSeconds * 5.f;
	m_turretOrientationDegrees += static_cast<float>(direction) * m_angularVelocity * deltaSeconds * 5.f;
	m_scale = scale;
}


void Player::HandleKeyboardInput()
{
	m_bodyMovementInput = g_theInput->IsKeyDown('E') ? Vec2::NORTH : Vec2::ZERO; // North
	m_bodyMovementInput += g_theInput->IsKeyDown('D') ? Vec2::SOUTH : Vec2::ZERO; // South
	m_bodyMovementInput += g_theInput->IsKeyDown('S') ? Vec2::WEST : Vec2::ZERO; // East
	m_bodyMovementInput += g_theInput->IsKeyDown('F') ? Vec2::EAST : Vec2::ZERO; // West
	m_bodyMovementInput.ClampLength(1.f); // clamp body input vector length to 1

	m_turretMovementInput = g_theInput->IsKeyDown('I') ? Vec2::NORTH : Vec2::ZERO; // North
	m_turretMovementInput += g_theInput->IsKeyDown('K') ? Vec2::SOUTH : Vec2::ZERO; // South
	m_turretMovementInput += g_theInput->IsKeyDown('J') ? Vec2::WEST : Vec2::ZERO; // East
	m_turretMovementInput += g_theInput->IsKeyDown('L') ? Vec2::EAST : Vec2::ZERO; // West
	m_turretMovementInput.ClampLength(1.f); // clamp turret input vector length to 1

	if (g_theInput->WasKeyJustPressed('Q'))
	{
		m_isUsingFlameThrower = !m_isUsingFlameThrower;
	}

	if (g_theInput->IsKeyDown(' ') && m_shootTimer > m_shootDelay)
	{
		ShootBullet();
	}
}


void Player::HandleControllerInput()
{
	XboxController controller = g_theInput->GetController(0);

	m_bodyMovementInput += controller.GetLeftStick().GetPosition();
	m_turretMovementInput += controller.GetRightStick().GetPosition();

	if (controller.GetRightStick().GetMagnitude() > 0.f && m_shootTimer > m_shootDelay)
	{
		ShootBullet();
	}
}

void Player::ShootBullet()
{
	static float bulletPhysicsRadius = g_gameConfigBlackboard.GetValue("bulletPhysicsRadius", 0.f);

	
	float absTurretOrientationDegrees = m_turretOrientationDegrees + m_orientationDegrees;
	Vec2 spawnPos = m_position + Vec2::MakeFromPolarDegrees(absTurretOrientationDegrees) * (m_physicsRadius - bulletPhysicsRadius);
	if (m_isUsingFlameThrower)
	{
		static constexpr int maxFlamePerBullet = 10;
		for (int numOfFlame = 0; numOfFlame < maxFlamePerBullet; numOfFlame++)
		{
			Vec2 flameSpawnPos = spawnPos + Vec2::MakeFromPolarDegrees(absTurretOrientationDegrees) * static_cast<float>(numOfFlame) * 0.05f;
			Entity* newBullet = m_map->CreateEntityOfType(ENTITY_TYPE_GOOD_FLAME_BULLET, flameSpawnPos, absTurretOrientationDegrees);
			m_map->AddEntityToMap(*newBullet);
		}
	}
	else
	{
		PlaySound(g_soundIds[SOUND_TYPE_PLAYER_SHOOT], false, g_gameSoundVolume);
		Entity* newBullet = m_map->CreateEntityOfType(ENTITY_TYPE_GOOD_BULLET, spawnPos, absTurretOrientationDegrees);
		m_map->AddEntityToMap(*newBullet);
	}
	m_shootTimer = 0.f;
	Vec2 muzzlePos = m_position + Vec2::MakeFromPolarDegrees(absTurretOrientationDegrees) * (m_physicsRadius + bulletPhysicsRadius);
	float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
	m_map->AddExplosionToMap(muzzlePos, randomOrientation, ExplosionType::BULLET);
}


