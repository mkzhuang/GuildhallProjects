#include "PlayerShip.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#pragma comment( lib, "xinput9_1_0" )

PlayerShip::PlayerShip(Game* owner, Vec2 const& startPos)
	:Entity(owner, startPos)
{
	m_angularVelocity = PLAYER_SHIP_TURN_SPEED;
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	health = PLAYER_HEALTH;
	m_velocity = Vec2(0.f, 0.f);
	m_mainColor = Rgba8(102, 153, 204, 255);
	InitializeLocalVerts();
}


PlayerShip::~PlayerShip()
{

}


void PlayerShip::Update(float deltaSeconds)
{
	m_velocity.ClampLength(MAX_PLAYER_SPEED);
	m_position += (m_velocity * deltaSeconds);
	HandleKeyPressed(deltaSeconds);
	HandleControllerInput(deltaSeconds);
	BounceBackWhenOutbounds();
}


void PlayerShip::Render() const 
{
	if (m_isGarbage) return;
	Vertex_PCU tempShipVerts[NUM_SHIP_VERTICES];
	for (int vertIndex = 0; vertIndex < NUM_SHIP_VERTICES; vertIndex++)
	{
		tempShipVerts[vertIndex] = m_verts[vertIndex];
	}
	float flameFraction = RNG.RollRandomFloatInRange(0.f, m_thrustFraction);
	tempShipVerts[17].m_position.x -= flameFraction * 3.f;

	TransformVertexArrayXY3D(NUM_SHIP_VERTICES, tempShipVerts, m_scale, m_orientationDegree, m_position);

	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTICES, tempShipVerts);

}


void PlayerShip::Die()
{
	m_isGarbage = true;
}


void PlayerShip::InitializeLocalVerts()
{
	m_verts[0] = Vertex_PCU(Vec3(0.f, 2.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 1
	m_verts[1] = Vertex_PCU(Vec3(-2.f, 1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 2
	m_verts[2] = Vertex_PCU(Vec3(2.f, 1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 3

	m_verts[3] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle B vertex 1
	m_verts[4] = Vertex_PCU(Vec3(-2.f, 1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle B vertex 2
	m_verts[5] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle B vertex 3

	m_verts[6] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle C vertex 1
	m_verts[7] = Vertex_PCU(Vec3(0.f, -1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle C vertex 2
	m_verts[8] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle C vertex 3

	m_verts[9] = Vertex_PCU(Vec3(1.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle D vertex 1
	m_verts[10] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle D vertex 2
	m_verts[11] = Vertex_PCU(Vec3(0.f, -1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle D vertex 3

	m_verts[12] = Vertex_PCU(Vec3(0.f, -2.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle E vertex 1
	m_verts[13] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle E vertex 2
	m_verts[14] = Vertex_PCU(Vec3(2.f, -1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle E vertex 3

	m_verts[15] = Vertex_PCU(Vec3(-2., 1.f, 0.f), m_flameColor, Vec2(0.f, 0.f)); //triangle F vertex 1
	m_verts[16] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), m_flameColor, Vec2(0.f, 0.f)); //triangle F vertex 2
	m_verts[17] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), Rgba8(0, 0, 0, 255), Vec2(0.f, 0.f)); //triangle F vertex 3
}


void PlayerShip::HandleKeyPressed(float deltaSeconds)
{
	if (!m_isGarbage)
	{
		if (g_theInput->IsKeyDown('S'))
		{
			m_orientationDegree += (m_angularVelocity * deltaSeconds);
		}
		if (g_theInput->IsKeyDown('F'))
		{
			m_orientationDegree -= (m_angularVelocity * deltaSeconds);
		}
		if (g_theInput->IsKeyDown('E'))
		{
			m_thrustFraction = 1.f;
			m_velocity += (PLAYER_SHIP_ACCELERATION * deltaSeconds * GetForwardNormal() * m_thrustFraction);
		}
		else 
		{
			m_thrustFraction = 0.f;
		}
		if (g_theInput->WasKeyJustPressed(' '))
		{
			FireBullet();
		}
	}
	else 
	{
		if (g_theInput->IsKeyDown('N') && health > 0)
		{
			SoundID playerSpawnSound = g_theAudio->CreateOrGetSound("Data/Audio/Respawn.wav");
			g_theAudio->StartSound(playerSpawnSound);
			Respawn();
		}
	}
}


void PlayerShip::HandleControllerInput(float deltaSeconds)
{
	XboxController const& controller = g_theInput->GetController(0);

	if (m_isGarbage)
	{
		if (controller.WasButtonJustPressed(XboxButtonID::START) && health > 0)
		{
			SoundID playerSpawnSound = g_theAudio->CreateOrGetSound("Data/Audio/Respawn.wav");
			g_theAudio->StartSound(playerSpawnSound);
			Respawn();
		}
		return;
	}

	float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
	if (leftStickMagnitude > 0.f && deltaSeconds > 0.f)
	{
		m_thrustFraction = leftStickMagnitude;
		m_orientationDegree = controller.GetLeftStick().GetOrientationDegrees();
		m_velocity += (PLAYER_SHIP_ACCELERATION * m_thrustFraction * deltaSeconds * GetForwardNormal());
	}

	if (controller.WasButtonJustPressed(XboxButtonID::BUTTON_A))
	{
		FireBullet();
	}
}


void PlayerShip::BounceBackWhenOutbounds()
{
	//check against east wall
	if (m_position.x >= WORLD_SIZE_X - m_physicsRadius)
	{
		m_position.x = WORLD_SIZE_X - m_physicsRadius;
		m_velocity.x *= -1.f;
	}

	//check against west wall
	if (m_position.x <= m_physicsRadius)
	{
		m_position.x = m_physicsRadius;
		m_velocity.x *= -1.f;
	}

	//check against north wall
	if (m_position.y >= WORLD_SIZE_Y - m_physicsRadius)
	{
		m_position.y = WORLD_SIZE_Y - m_physicsRadius;
		m_velocity.y *= -1.f;
	}

	//check against south wall
	if (m_position.y <= m_physicsRadius)
	{
		m_position.y = m_physicsRadius;
		m_velocity.y *= -1.f;
	}
}


void PlayerShip::Respawn()
{
	health--;
	m_position = Vec2(WORLD_CENTER_X, WORLD_CENTER_Y);
	m_velocity = Vec2(0.f, 0.f);
	m_orientationDegree = 0.f;
	m_isGarbage = false;
	m_bulletCount = 1;
}


void PlayerShip::FireBullet()
{
	Vec2 nosePosition = m_position + (GetForwardNormal() * 2.f);
	Vec2 leftMuzzlePosition = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegree).GetRotatedMinus90Degrees() + GetForwardNormal() * 2.f;
	Vec2 rightMuzzlePosition = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegree).GetRotated90Degrees() + GetForwardNormal() * 2.f;
	if (m_bulletCount == 1)
	{
		m_game->SpawnBullet(nosePosition, m_orientationDegree);
	}
	if (m_bulletCount == 2)
	{
		m_game->SpawnBullet(leftMuzzlePosition, m_orientationDegree);
		m_game->SpawnBullet(rightMuzzlePosition, m_orientationDegree);
	}
	if (m_bulletCount == 3)
	{
		m_game->SpawnBullet(leftMuzzlePosition, m_orientationDegree - 2.5f);
		m_game->SpawnBullet(nosePosition, m_orientationDegree);
		m_game->SpawnBullet(rightMuzzlePosition, m_orientationDegree + 2.5f);
	}
	SoundID fireSound = g_theAudio->CreateOrGetSound("Data/Audio/PlayerShoot.wav");
	g_theAudio->StartSound(fireSound);
}


