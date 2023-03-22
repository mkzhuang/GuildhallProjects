#include "Game/Character.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

std::vector<CharacterDefinition> CharacterDefinition::s_characterDefs;

CharacterDefinition::CharacterDefinition()
{
}


void CharacterDefinition::InitializeCharacterDefs(std::string const& xmlFilePath)
{
	XmlDocument doc;
	doc.LoadFile(xmlFilePath.c_str());

	XmlElement const* child = doc.RootElement()->FirstChildElement();
	while (child)
	{
		CreateCharacterDef(*child);
		child = child->NextSiblingElement();
	}
}


void CharacterDefinition::CreateCharacterDef(XmlElement const& element)
{
	std::string const& type = ParseXmlAttribute(element, "type", "");
	std::string const& name = ParseXmlAttribute(element, "name", "");
	std::string const& terrain = ParseXmlAttribute(element, "terrain", "");
	std::string const& hasItem = ParseXmlAttribute(element, "hasItem", "");
	std::string const& wantsItem = ParseXmlAttribute(element, "wantsItem", "");
	float health = ParseXmlAttribute(element, "health", 0.f);
	float damage = ParseXmlAttribute(element, "damage", 0.f);

	CharacterDefinition def;
	def.m_type = type;
	def.m_name = name;
	def.m_spawnTileType = terrain;
	def.m_item = hasItem;
	def.m_wantItem = wantsItem;
	def.m_health = health;
	def.m_damage = damage;

	XmlElement const& animationInfo = *(element.FirstChildElement());
	std::string const& texturePath = ParseXmlAttribute(animationInfo, "texturePath", "");
	IntVec2 textureGrid = ParseXmlAttribute(animationInfo, "grid", IntVec2::ZERO);
	Texture const* texture = g_theRenderer->CreateOrGetTextureFromFile(texturePath.c_str());
	SpriteSheet* spriteSheet = new SpriteSheet(*texture, textureGrid);

	XmlElement const* animation = animationInfo.FirstChildElement();
	while (animation)
	{
		std::string const& animName = ParseXmlAttribute(*animation, "animName", "");
		int startIndex = ParseXmlAttribute(*animation, "startIndex", 0);
		int endIndex = ParseXmlAttribute(*animation, "endIndex", 0);
		float duration = ParseXmlAttribute(*animation, "duration", 0.f);
		CharacterAnim animType;
		if (animName == "idle")
		{
			animType = CharacterAnim::IDLE;
		}
		else if (animName == "combat")
		{
			animType = CharacterAnim::COMBAT;
		}

		SpriteAnimDefinition* newAnim = new SpriteAnimDefinition(*spriteSheet, startIndex, endIndex, duration, SpriteAnimPlaybackType::LOOP);
		def.m_animations[animType] = newAnim;

		animation = animation->NextSiblingElement();
	}

	CharacterDefinition::s_characterDefs.push_back(def);
}


CharacterDefinition const& CharacterDefinition::GetDefinitionByName(std::string const& name)
{
	for (int index = 0; index < (int)s_characterDefs.size(); index++)
	{
		CharacterDefinition const& characterDef = CharacterDefinition::s_characterDefs[index];
		if (characterDef.m_name == name)
		{
			return characterDef;
		}
	}

	ERROR_AND_DIE("character def does not eixst");
}


CharacterDefinition const& CharacterDefinition::GetDefinitionByItem(std::string const& itemName)
{
	for (int index = 0; index < (int)s_characterDefs.size(); index++)
	{
		CharacterDefinition const& characterDef = CharacterDefinition::s_characterDefs[index];
		if (characterDef.m_item == itemName)
		{
			return characterDef;
		}
	}

	ERROR_AND_DIE("character def does not eixst");
}


Character::Character(Map* map, CharacterDefinition const* characterDef)
	: m_map(map)
	, m_characterDef(characterDef)
{
	m_currentAnimation = CharacterAnim::IDLE;
}


void Character::Update(float deltaSeconds)
{
	m_animationTimer += deltaSeconds;
}


void Character::Render() const
{
	std::vector<Vertex_PCU> verts;
	AABB2 bodyBox(m_position - Vec2(0.5f, 0.5f), m_position + Vec2(0.5f, 0.5f));
	std::map<CharacterAnim, SpriteAnimDefinition*>::const_iterator iter = m_characterDef->m_animations.find(m_currentAnimation);
	AABB2 const& spriteUVs = iter->second->GetSpriteDefAtTime(m_animationTimer).GetUVs();
	AddVertsForAABB2D(verts, bodyBox, Rgba8::WHITE, spriteUVs);
	g_theRenderer->DrawVertexArray(verts);
}


