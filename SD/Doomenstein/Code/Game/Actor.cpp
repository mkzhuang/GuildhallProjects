#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/AI.hpp"
#include "Game/Player.hpp"
#include "Game/SpriteAnimationGroupDefinition.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"

#include <vector>

char const* g_factionNames[] = {"Neutral", "Marine", "Demon"};

Actor::Actor(Map* map, SpawnInfo const& spawnInfo)
	: m_map(map)
{
	m_definition = spawnInfo.m_definition;
	m_position = spawnInfo.m_position;
	m_orientation = spawnInfo.m_orientation;
	m_velocity = spawnInfo.m_velocity;
	m_health = m_definition->m_health;

	if (m_definition->m_faction == NEUTRAL || m_definition->m_dieOnCollide) m_solidColor = Rgba8::BLUE;
	if (m_definition->m_faction == MARINE) m_solidColor = Rgba8::GREEN;
	if (m_definition->m_faction == DEMON) m_solidColor = Rgba8::RED;

	for (int weaponIndex = 0; weaponIndex < (int)m_definition->m_weaponDefinitions.size(); weaponIndex++)
	{
		Weapon* newWeapon = new Weapon(m_map->GetGameClock(), m_definition->m_weaponDefinitions[weaponIndex]);
		newWeapon->m_map = m_map;
		m_weapons.push_back(newWeapon);
	}

	if ((int)m_weapons.size() > 0) m_equippedWeaponIndex = 0;
	m_currentAnimationDef = m_definition->GetDefaultAnimationGroup();

	if (m_definition->m_dieOnSpawn) Die();
}


void Actor::Update(float deltaSeconds)
{
	if (m_definition->m_simulated)
	{
		UpdatePhysics(deltaSeconds);
	}

	if (!m_isDead && m_animationType != AnimationType::WALK)
	{
		if (!m_currentAnimationDef || m_animationTimer >= m_currentAnimationDef->m_duration)
		{
			ChangeAnimationType(AnimationType::WALK);
		}
	}

	float scale = 1.f;
	if (m_currentAnimationDef && m_currentAnimationDef->m_scaleBySpeed) scale *= RangeMapClamped(m_velocity.GetLength(), 0.f, m_definition->m_runSpeed, 0.f, 3.f);
	m_animationTimer += (scale * deltaSeconds);

	if (m_lifetimeStopwatch.CheckDurationElapsedAndDecrement())
	{
		m_map->DestroyActor(m_uid);
	}

	if (m_shirnkStopwatch.CheckDurationElapsedAndDecrement() && !m_isDead)
	{
		m_isShrinked = false;
	}
}


void Actor::UpdatePhysics(float deltaSeconds)
{
	if (!m_isDead)
	{
		AddForce(-m_definition->m_drag * m_velocity);
		m_velocity += m_acceleration * deltaSeconds;
		m_position += m_velocity * deltaSeconds;
		m_acceleration = Vec3::ZERO;
		if (!m_definition->m_flying) m_position.z = m_definition->m_floatingHeight;
	}
	else
	{
		if (!m_isShrinked && !m_definition->m_flying) m_position.z = 0.f;
	}
}


void Actor::Render(Camera const& camera) const
{
	Player* playerController = dynamic_cast<Player*>(m_controller);
	if (playerController)
	{
		if (playerController->m_camera == &camera) return;
	}

	Vec3 displacement = m_position - camera.GetCameraPosition();
	displacement.z = 0.f;
	displacement = displacement.GetNormalized();
	//transform this vector in the actor's local space
	displacement = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetOrthonormalInverse().TransformVectorQuantity3D(displacement).GetNormalized();
	if (!m_currentAnimationDef) return;
	int bestIndex = -1;
	float bestDot = -999.f;
	for (int animationIndex = 0; animationIndex < (int)m_currentAnimationDef->m_directions.size(); animationIndex++)
	{
		Vec3 direction = m_currentAnimationDef->m_directions[animationIndex];
		float dot = DotProduct3D(displacement, direction);
		if (dot >= bestDot)
		{
			bestIndex = animationIndex;
			bestDot = dot;
		}
	}
	SpriteAnimDefinition const* spriteAnimDef = m_currentAnimationDef->m_spriteAnimationDefinitions[bestIndex];

	SpriteDefinition const& spriteDef = spriteAnimDef->GetSpriteDefAtTime(m_animationTimer);

	Vec2 localMaxs = m_definition->m_spriteSize;
	if (m_isShrinked) localMaxs *= m_shrinkScale;
	Vec2 pivot = m_definition->m_spritePivot * localMaxs;
	Vec3 bl(0.f, pivot.x, -pivot.y);
	Vec3 br(0.f, pivot.x - localMaxs.x, -pivot.y);
	Vec3 tr(0.f, pivot.x - localMaxs.x, localMaxs.y - pivot.y);
	Vec3 tl(0.f, pivot.x, localMaxs.y - pivot.y);


	if (m_definition->m_renderLit)
	{
		std::vector<Vertex_PNCU> actorVecrts;
		AddVertsForRoundedQuad3D(actorVecrts, bl, br, tr, tl, Rgba8::WHITE, spriteDef.GetUVs());
		g_theRenderer->BindShader(m_currentAnimationDef->m_shader);
		g_theRenderer->SetModelMatrix(GetModelMatrix(camera));
		g_theRenderer->BindTexture(&spriteDef.GetTexture());
		g_theRenderer->DrawVertexArray(actorVecrts);
	}
	else
	{
		std::vector<Vertex_PCU> actorVecrts;
		AddVertsForQuad3D(actorVecrts, bl, br, tr, tl, Rgba8::WHITE, spriteDef.GetUVs());
		g_theRenderer->BindShader(m_currentAnimationDef->m_shader);
		g_theRenderer->SetModelMatrix(GetModelMatrix(camera));
		g_theRenderer->BindTexture(&spriteDef.GetTexture());
		g_theRenderer->DrawVertexArray(actorVecrts);
	}
}


Mat44 Actor::GetModelMatrix(Camera const& camera) const
{
	if (m_definition->m_billboardType == BillboardType::FACING)
	{
		Mat44 model;
		model.SetTranslation3D(m_position);
		Vec3 displacement = m_position - camera.GetCameraPosition();
		displacement.z = 0.f;
		Vec3 iBasis = displacement.GetNormalized();
		Vec3 jBasis(-iBasis.y, iBasis.x, 0.f);
		Vec3 kBasis(0.f, 0.f, 1.f);
		model.SetIJK3D(iBasis, jBasis, kBasis);
		return model;
	}
	else if (m_definition->m_billboardType == BillboardType::ALIGNED)
	{
		Mat44 model;
		model.SetTranslation3D(m_position);
		Vec3 forward = camera.GetViewOrientation().GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
		forward.z = 0.f;
		Vec3 iBasis = forward.GetNormalized();
		Vec3 jBasis(-iBasis.y, iBasis.x, 0.f);
		Vec3 kBasis(0.f, 0.f, 1.f);
		model.SetIJK3D(iBasis, jBasis, kBasis);
		return model;
	}
	else
	{
		Mat44 model;
		model.SetTranslation3D(m_position);
		EulerAngles orientation = m_orientation;
		orientation.m_pitchDegrees = 0.f;
		orientation.m_rollDegrees = 0.f;
		Mat44 rotation = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
		model.Append(rotation);
		return model;
	}
}


Vec3 Actor::GetForward() const
{
	return m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
}


Vec3 Actor::GetEyePosition() const
{
	float eyeHeight = m_definition->m_eyeHeight;
	if (m_isShrinked) eyeHeight *= m_shrinkScale;
	return m_position + Vec3(0.f, 0.f, eyeHeight);
}


void Actor::Damage(float damage)
{
	if (m_isDead) return;
	if (damage > 0.f) 
	{
		if (m_isShrinked) 
		{
			Die();
			return;
		}
		ChangeAnimationType(AnimationType::PAIN);
		SoundID hurtSound = g_theAudio->CreateOrGetSound(m_definition->m_hurtSoundName);
		g_theAudio->StartSoundAt(hurtSound, m_position, false, g_gameSoundVolume);
	}
	m_health -= damage;
	if (m_health <= 0.f) 
	{
		m_health = 0.f;
		Die();
	}
}


void Actor::Shrink(float time)
{
	m_isShrinked = true;
	m_shirnkStopwatch.Start(&m_map->GetGameClock(), time);
}


void Actor::Die()
{
	m_isDead = true;
	if (m_definition->m_faction == NEUTRAL || m_definition->m_dieOnCollide) m_solidColor.b = 100;
	if (m_definition->m_faction == MARINE) m_solidColor.g = 100;
	if (m_definition->m_faction == DEMON) m_solidColor.r = 100;
	m_lifetimeStopwatch.Start(&m_map->GetGameClock(), m_definition->m_corpseLifetime);
	ChangeAnimationType(AnimationType::DEATH);
	SoundID deathSound = g_theAudio->CreateOrGetSound(m_definition->m_deathSoundName);
	g_theAudio->StartSoundAt(deathSound, m_position, false, g_gameSoundVolume);
	if (m_definition->m_name == "Demon" || m_definition->m_name == "Magma")
	{
		m_map->IncreaseKills();
	}
}


void Actor::AddForce(Vec3 const& force)
{
	m_acceleration += force;
}


void Actor::AddImpulse(Vec3 const& impulse)
{
	m_velocity += impulse;
}


void Actor::OnCollide(Actor* other)
{
	if (m_isShrinked)
	{
		if (other->m_definition->m_name == "Marine")
		{
			Die();
			return;
		}
	}

	if (other->m_isShrinked)
	{
		if (m_definition->m_name == "Marine")
		{
			other->Die();
			return;
		}
	}

	Vec3 myVelocity = m_velocity;
	myVelocity.z = 0.f;
	myVelocity = myVelocity.GetNormalized();

	Vec3 otherVelocity = other->m_velocity;
	otherVelocity.z = 0.f;
	otherVelocity = otherVelocity.GetNormalized();
	
	AddImpulse(other->m_definition->m_impulseOnCollide * otherVelocity);
	other->AddImpulse(m_definition->m_impulseOnCollide * myVelocity);

	Damage(RNG.RollRandomFloatInFloatRange(other->m_definition->m_damageOnCollide));
	other->Damage(RNG.RollRandomFloatInFloatRange(m_definition->m_damageOnCollide));

	if (m_definition->m_dieOnCollide) Die();
	if (other->m_definition->m_dieOnCollide) other->Die();
}


void Actor::ChangeAnimationType(AnimationType type)
{
	if (m_animationType == type) return;
	m_animationType = type;
	m_animationTimer = 0.f;
	switch (type)
	{
	case WALK:
		m_currentAnimationDef = m_definition->GetAnimationGroup("Walk");
		break;
	case ATTACK:
		m_currentAnimationDef = m_definition->GetAnimationGroup("Attack");
		break;
	case PAIN:
		m_currentAnimationDef = m_definition->GetAnimationGroup("Pain");
		break;
	case DEATH:
		m_currentAnimationDef = m_definition->GetAnimationGroup("Death");
		break;
	}
}


void Actor::OnPossessed(Controller* controller)
{
	m_controller = controller;
	controller->m_actorUID = m_uid;
	controller->m_map = m_map;
}


void Actor::OnUnpossessed(Controller* controller)
{
	controller->m_actorUID = ActorUID::INVALID;
	controller->m_map = nullptr;
	m_controller = m_aiController;
}


void Actor::MoveInDirection(Vec3 direction, float speed)
{
	m_position += direction * speed;
}


void Actor::TurnInDirection(EulerAngles direction)
{
	m_orientation += direction;
}


Weapon* Actor::GetEquippedWeapon()
{
	return m_weapons[m_equippedWeaponIndex];
}


void Actor::EquipWeapon(int weaponIndex)
{
	m_equippedWeaponIndex = weaponIndex;
}


void Actor::EquipNextWeapon()
{
	m_equippedWeaponIndex++;
	if (m_equippedWeaponIndex >= (int)m_weapons.size())
	{
		m_equippedWeaponIndex = 0;
	}
}


void Actor::EquipPreviousWeapon()
{
	m_equippedWeaponIndex--;
	if (m_equippedWeaponIndex == -1)
	{
		m_equippedWeaponIndex = (int)m_weapons.size() - 1;
	}
}


void Actor::Attack()
{
	if (m_equippedWeaponIndex < 0 || m_equippedWeaponIndex > (int)m_weapons.size() - 1) return;
	if (!m_weapons[m_equippedWeaponIndex]) return;

	Vec3 forward = GetForward() * m_definition->m_physicsRadius;
	Vec3 firePos = GetEyePosition() + forward;
	if (m_weapons[m_equippedWeaponIndex]->Fire(firePos, forward, this))
	{
		ChangeAnimationType(AnimationType::ATTACK);
	}
}


