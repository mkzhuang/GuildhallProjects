#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <map>
#include <vector>
#include <string>

class Map;
class SpriteAnimDefinition;

enum class CharacterAnim
{
	IDLE,
	COMBAT,
};

struct CharacterDefinition
{
public:
	CharacterDefinition();
	~CharacterDefinition() {}

	static void InitializeCharacterDefs(std::string const& xmlFilePath);
	static void CreateCharacterDef(XmlElement const& element);
	static CharacterDefinition const& GetDefinitionByName(std::string const& name);
	static CharacterDefinition const& GetDefinitionByItem(std::string const& itemName);

	static std::vector<CharacterDefinition> s_characterDefs;

public:
	std::string m_type = "";
	std::string m_name = "";
	std::string m_spawnTileType = "";
	std::map<CharacterAnim, SpriteAnimDefinition*> m_animations;

	// npc
	std::string m_item = "";
	std::string m_wantItem = "";

	//monster
	float m_health = 0.f;
	float m_damage = 0.f;
	std::string m_dropItem = "";
};



class Character
{
public:
	Character(Map* map, CharacterDefinition const* characterDef);
	virtual ~Character() {}

	void Update(float deltaSeconds);
	void Render() const;

public:
	Map* m_map = nullptr;
	int m_characterIndex = 0;
	Vec2 m_position = Vec2::ZERO;

	CharacterDefinition const* m_characterDef = nullptr;
	CharacterAnim m_currentAnimation = CharacterAnim::IDLE;
	float m_animationTimer = 0.f;
};


