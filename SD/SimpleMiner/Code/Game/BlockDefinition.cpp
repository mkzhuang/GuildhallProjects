#include "Game/BlockDefinition.hpp"

std::vector<BlockDefinition*> BlockDefinition::s_definitions;

bool BlockDefinition::LoadFromXmlElement(XmlElement const& element)
{
	UNUSED(element)
	return false;
}


void BlockDefinition::InitializeBlockDefinitions()
{
	//				  name				visible	solid	opaque light	top				side			bottom		break time
	CreateNewBlockDef("air",			false,	false,	false,	0,	IntVec2(0, 0),	 IntVec2(0, 0),	  IntVec2(0, 0),	0.0f);
	CreateNewBlockDef("grass",			true,	true,	true,	0,	IntVec2(32, 33), IntVec2(33, 33), IntVec2(32, 34),	0.5f);
	CreateNewBlockDef("dirt",			true,	true,	true,	0,	IntVec2(32, 34), IntVec2(32, 34), IntVec2(32, 34),	0.5f);
	CreateNewBlockDef("stone",			true,	true,	true,	0,	IntVec2(33, 32), IntVec2(33, 32), IntVec2(33, 32),	2.0f);
	CreateNewBlockDef("bricks",			true,	true,	true,	0,	IntVec2(34, 32), IntVec2(34, 32), IntVec2(34, 32),	2.0f);
	CreateNewBlockDef("glowstone",		true,	true,	true,	15,	IntVec2(46, 34), IntVec2(46, 34), IntVec2(46, 34),	1.0f);
	CreateNewBlockDef("water",			true,	false,	false,	0,	IntVec2(32, 44), IntVec2(32, 44), IntVec2(32, 44),	1.0f);
	CreateNewBlockDef("coal",			true,	true,	true,	0,	IntVec2(63, 34), IntVec2(63, 34), IntVec2(63, 34),	3.0f);
	CreateNewBlockDef("iron",			true,	true,	true,	0,	IntVec2(63, 35), IntVec2(63, 35), IntVec2(63, 35),	4.0f);
	CreateNewBlockDef("gold",			true,	true,	true,	0,	IntVec2(63, 36), IntVec2(63, 36), IntVec2(63, 36),	5.0f);
	CreateNewBlockDef("diamond",		true,	true,	true,	0,	IntVec2(63, 37), IntVec2(63, 37), IntVec2(63, 37),	6.0f);
	CreateNewBlockDef("sand",			true,	true,	true,	0,	IntVec2(34, 34), IntVec2(34, 34), IntVec2(34, 34),	0.75f);
	CreateNewBlockDef("ice",			true,	true,	true,	0,	IntVec2(45, 34), IntVec2(45, 34), IntVec2(45, 34),	1.0f);
	CreateNewBlockDef("oak_leaf",		true,	true,	true,	0,	IntVec2(32, 35), IntVec2(32, 35), IntVec2(32, 35),	0.25f);
	CreateNewBlockDef("oak_log",		true,	true,	true,	0,	IntVec2(38, 33), IntVec2(36, 33), IntVec2(38, 33),	2.0f);
	CreateNewBlockDef("spruce_leaf",	true,	true,	true,	0,	IntVec2(62, 41), IntVec2(62, 41), IntVec2(62, 41),	0.25f);
	CreateNewBlockDef("spruce_log",		true,	true,	true,	0,	IntVec2(38, 33), IntVec2(37, 33), IntVec2(38, 33),	2.0f);
	CreateNewBlockDef("cactus",			true,	true,	true,	0,	IntVec2(39, 36), IntVec2(37, 36), IntVec2(38, 36),	1.0f);
	CreateNewBlockDef("red",			true,	true,	true,	0,	IntVec2(39, 34), IntVec2(39, 34), IntVec2(39, 34),	1.0f);
	CreateNewBlockDef("snow_grass",		true,	true,	true,	0,	IntVec2(36, 35), IntVec2(33, 35), IntVec2(32, 34),	0.5f);
	CreateNewBlockDef("snow",			true,	true,	true,	0,	IntVec2(36, 35), IntVec2(36, 35), IntVec2(36, 35),	0.5f);
	CreateNewBlockDef("plank",			true,	true,	true,	0,	IntVec2(39, 33), IntVec2(39, 33), IntVec2(39, 33),	1.5f);
}


const BlockDefinition* BlockDefinition::GetByName(std::string const& name)
{
	for (int blockDefIndex = 0; blockDefIndex < (int)s_definitions.size(); blockDefIndex++)
	{
		BlockDefinition*& blockDef = BlockDefinition::s_definitions[blockDefIndex];
		if (blockDef && blockDef->m_name == name)
		{
			return blockDef;
		}
	}

	ERROR_AND_DIE("Error: invalid block definition name.");
}


const BlockDefinition* BlockDefinition::GetById(int id)
{
	if (id < 0 || id >= BlockDefinition::s_definitions.size())
	{
		return nullptr;
	}

	return BlockDefinition::s_definitions[id];
}


const uint8_t BlockDefinition::GetIndexByName(std::string const& name)
{
	for (int blockDefIndex = 0; blockDefIndex < (int)s_definitions.size(); blockDefIndex++)
	{
		BlockDefinition*& blockDef = BlockDefinition::s_definitions[blockDefIndex];
		if (blockDef && blockDef->m_name == name)
		{
			return (uint8_t)blockDefIndex;
		}
	}

	ERROR_AND_DIE("Error: invalid block definition name.");
}


void BlockDefinition::CreateNewBlockDef(std::string const& name, bool isVisible, bool isSolid, bool isOpaque, int light, IntVec2 topSprite, IntVec2 sideSprite, IntVec2 bottomSprite, float breakTime)
{
	Texture const* texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BasicSprites_64x64.png");
	if (g_gameConfigBlackboard.GetValue("disableTexture", false))
	{
		topSprite = IntVec2(2, 4);
		sideSprite = IntVec2(2, 4);
		bottomSprite = IntVec2(2, 4);
	}
	SpriteSheet spriteSheet(*texture, IntVec2(64, 64));
	BlockDefinition* newBlockDef = new BlockDefinition();
	newBlockDef->m_name = name;
	newBlockDef->m_isVisible = isVisible;
	newBlockDef->m_isSolid = isSolid;
	newBlockDef->m_isOpaque = isOpaque;
	newBlockDef->m_texture = texture;
	newBlockDef->m_light = light;
	newBlockDef->m_topUVs = spriteSheet.GetSpriteUVs(topSprite);
	newBlockDef->m_sideUVs = spriteSheet.GetSpriteUVs(sideSprite);
	newBlockDef->m_bottomUVs = spriteSheet.GetSpriteUVs(bottomSprite);
	newBlockDef->m_breakTime = breakTime;
	BlockDefinition::s_definitions.push_back(newBlockDef);
}


