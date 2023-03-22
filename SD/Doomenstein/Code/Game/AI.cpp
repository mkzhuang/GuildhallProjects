#include "Game/AI.hpp"
#include "Game/Map.hpp"
#include "Game/ActorDefinition.hpp"

AI::AI()
{
}


AI::~AI()
{
}


void AI::Update(float deltaSeconds)
{
	if (GetActor()->m_isDead) return;
	Actor* controllerActor = m_map->FindActorByUID(m_actorUID);
	ActorDefinition const* controllerActorDefinition = controllerActor->m_definition;
	Actor* targetActor = m_map->GetClosestVisibleEnemy(controllerActor);
	if (!targetActor) return;

	Vec3 displacement = targetActor->m_position - controllerActor->m_position;
	float distance = displacement.GetLength() - (controllerActorDefinition->m_physicsRadius + targetActor->m_definition->m_physicsRadius);
	if (m_meleeStopwatch.CheckDurationElapsedAndDecrement() && distance <= controllerActorDefinition->m_meleeRange && !GetActor()->m_isShrinked)
	{
		SoundID attackSound = g_theAudio->CreateOrGetSound(GetActor()->m_definition->m_attackSoundName);
		g_theAudio->StartSoundAt(attackSound, GetActor()->m_position, false, g_gameSoundVolume);
		float damage = RNG.RollRandomFloatInFloatRange(controllerActorDefinition->m_meleeDamage);
		targetActor->Damage(damage);
		GetActor()->ChangeAnimationType(AnimationType::ATTACK);
		std::string projectileName = GetActor()->m_definition->m_projectileDefName;
		if (projectileName != "")
		{
			ActorDefinition const* projectile = ActorDefinition::GetByName(projectileName);
			Vec3 displacementNormal = displacement.GetNormalized();
			Vec3 velocity = displacementNormal * 3.0f;
			SpawnInfo projectileInfo(projectile, GetActor()->GetEyePosition() + displacementNormal, EulerAngles::ZERO, velocity);
			Actor* actor = m_map->SpawnActor(projectileInfo);
			actor->m_owner = GetActor();
		}
		m_meleeStopwatch.Start(&m_map->GetGameClock(), m_meleeStopwatch.m_duration);
	}
	float targetDegrees = displacement.GetAngleAboutZDegrees();
	Vec3 forward = controllerActor->GetForward();
	float forwardDegrees = forward.GetAngleAboutZDegrees();
	controllerActor->m_orientation.m_yawDegrees = GetTurnedTowardDegrees(forwardDegrees, targetDegrees, controllerActor->m_definition->m_turnSpeed * deltaSeconds);
	forward = controllerActor->GetForward();
	Vec2 forward2D = Vec2(forward.x, forward.y).GetNormalized();
	forward = Vec3(forward2D);
	if (distance > GetActor()->m_definition->m_meleeRange - 0.01f)
	{
		float speed = RangeMapClamped(distance, controllerActorDefinition->m_meleeRange, 3.f, controllerActorDefinition->m_walkSpeed, controllerActorDefinition->m_runSpeed);
		controllerActor->AddForce(controllerActorDefinition->m_drag * forward * speed);
	}
}


