#include "Game/Weapon.hpp"
#include "Game/Map.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

Weapon::Weapon(Clock const& clock, WeaponDefinition const* definition)
	:m_definition(definition)
{
	m_refireStopwatch.Start(&clock, m_definition->m_refireTime);
}


Weapon::~Weapon()
{

}


bool Weapon::Fire(Vec3 const& position, Vec3 const& forward, Actor* owner)
{
	if (m_refireStopwatch.HasDurationElapsed())
	{
		SoundID weaponSound = g_theAudio->CreateOrGetSound(m_definition->m_fireSoundName);
		Vec3 soundPosition = position + forward * 0.001f;
		g_theAudio->StartSoundAt(weaponSound, soundPosition, false, g_gameSoundVolume);
		for (int rayCount = 0; rayCount < m_definition->m_numRays; rayCount++)
		{
			Vec3 randomRayDirection = GetRandomDirectionInCone(position, forward, m_definition->m_rayCone).GetNormalized();
			// raycast vs actor and if hit, damage the actor
			RaycastFilter filter;
			filter.m_ignoreActor = owner;
			RaycastResultDoomenstein raycastResult = m_map->RaycastAll(position, randomRayDirection, m_definition->m_rayRange, filter);
			//DebugAddWorldPoint(raycastResult.m_impactPosition, 0.025f, m_definition->m_refireTime, Rgba8::MAGENTA, Rgba8::MAGENTA, DebugRenderMode::USEDEPTH);
			Actor* impactedActor = raycastResult.m_impactActor;
			ActorDefinition const* bulletImapctActor = nullptr;
			if (impactedActor)
			{
				if (m_definition->m_canShrink)
				{
					impactedActor->Shrink(m_definition->m_shrinkTime);
				}
				float damage = RNG.RollRandomFloatInFloatRange(m_definition->m_rayDamage);
				impactedActor->Damage(damage);
				randomRayDirection.z = 0.f;
				randomRayDirection = randomRayDirection.GetNormalized();
				impactedActor->AddImpulse(m_definition->m_rayImpulse * randomRayDirection);
				bulletImapctActor = ActorDefinition::GetByName("BloodSplatter");
				if (m_definition->m_canShrink)
				{
					bulletImapctActor = ActorDefinition::GetByName("ShrinkEffect");
				}
			}
			else
			{
				bulletImapctActor = ActorDefinition::GetByName("BulletHit");
			}

			SpawnInfo impactSpawnInfo(bulletImapctActor, raycastResult.m_impactPosition, EulerAngles(), Vec3::ZERO);
			m_map->SpawnActor(impactSpawnInfo);
		}

		for (int projectileCount = 0; projectileCount < m_definition->m_numProjectiles; projectileCount++)
		{
			Vec3 randomProjectileDirection = GetRandomDirectionInCone(position, forward, m_definition->m_projectileCone).GetNormalized();
			SpawnInfo projectileSpawnInfo(m_definition->m_projectileActorDefinition, position, EulerAngles(), randomProjectileDirection * m_definition->m_projectileSpeed);
			Actor* actor = m_map->SpawnActor(projectileSpawnInfo);
			actor->m_owner = owner;
		}
		m_refireStopwatch.Start(&m_map->GetGameClock(), m_definition->m_refireTime);

		return true;
	}
	return false;
}


Vec3 Weapon::GetRandomDirectionInCone(Vec3 const& position, Vec3 const& forward, float angle) const
{
	UNUSED(position)

	float maxDiameter = 0.5f * (SinDegrees(angle) / CosDegrees(angle));

	float randomJLength = RNG.RollRandomFloatInRange(-maxDiameter, maxDiameter);
	float randomKLength = RNG.RollRandomFloatInRange(-maxDiameter, maxDiameter);

	Vec3 left, up;
	if (fabsf(DotProduct3D(forward, Vec3(0.f, 1.f, 0.f))) < .99f)
	{
		up = CrossProduct3D(forward, Vec3(0.f, 1.f, 0.f)).GetNormalized();
		left = CrossProduct3D(up, forward).GetNormalized();
	}
	else
	{
		up = CrossProduct3D(forward, Vec3(0.f, 0.f, 1.f)).GetNormalized();
		left = CrossProduct3D(up, forward).GetNormalized();
	}

	return forward + left * randomJLength + up * randomKLength;
}


