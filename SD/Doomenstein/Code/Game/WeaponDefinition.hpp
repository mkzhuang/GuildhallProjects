#pragma once
#include "Game/GameCommon.hpp"

class ActorDefinition;
class SpriteAnimDefinition;

class WeaponDefinition
{
public:
	bool LoadFromXmlElement( const XmlElement& element );

	std::string m_name;
	float m_refireTime = 0.5f;

	int m_numRays = 0;
	float m_rayCone = 0.0f;
	float m_rayRange = 40.0f;
	FloatRange m_rayDamage = FloatRange( 0.0f, 1.0f );
	float m_rayImpulse = 4.0f;
	bool m_canShrink = false;
	float m_shrinkTime = 0.f;

	int m_numProjectiles = 0;
	ActorDefinition const* m_projectileActorDefinition = nullptr;
	float m_projectileCone = 1.0f;
	float m_projectileSpeed = 40.0f;

	SpriteAnimDefinition const* m_idleAnimationDefinition = nullptr;
	SpriteAnimDefinition const* m_attackAnimationDefinition = nullptr;

	Shader* m_shader = nullptr;
	Texture const* m_hubBaseTexture = nullptr;
	SpriteSheet const* m_spriteSheet = nullptr;
	Vec2 m_spriteSize = Vec2::ONE;
	Vec2 m_spritePivot = Vec2(0.5f, 0.f);
	Texture* m_reticleTexture = nullptr;
	Vec2 m_reticeSize = Vec2::ONE;

	std::string m_fireSoundName;

	static void InitializeDefinitions( const char* path );
	static void ClearDefinitions();
	static const WeaponDefinition* GetByName( const std::string& name );
	static std::vector<WeaponDefinition*> s_definitions;
};


