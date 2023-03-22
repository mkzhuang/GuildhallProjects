#pragma once
#include "Game/GameCommon.hpp"

class BlockDefinition
{
public:
	bool LoadFromXmlElement(XmlElement const& element);

public:
	std::string m_name;
	bool m_isVisible = false;
	bool m_isSolid = false;
	bool m_isOpaque = false;
	int m_light = 0;
	Texture const* m_texture = nullptr;
	AABB2 m_topUVs = AABB2::ZERO_TO_ONE;
	AABB2 m_sideUVs = AABB2::ZERO_TO_ONE;
	AABB2 m_bottomUVs = AABB2::ZERO_TO_ONE;
	float m_breakTime = 0.f;
	
	static void InitializeBlockDefinitions();
	static const BlockDefinition* GetByName(std::string const& name);
	static const BlockDefinition* GetById(int id);
	static const uint8_t GetIndexByName(std::string const& name);
	static void CreateNewBlockDef(std::string const& name, bool isVisible, bool isSolid, bool isOpaque, int light, IntVec2 topSprite, IntVec2 sideSprite, IntVec2 bottomSprite, float breakTime);
	static std::vector<BlockDefinition*> s_definitions;
};