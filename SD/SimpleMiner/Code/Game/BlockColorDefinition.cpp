#include "Game/BlockColorDefinition.hpp"

std::vector<BlockColorDefinition*> BlockColorDefinition::s_definitions;


void BlockColorDefinition::InitializeBlockColorDefinitions()
{
	CreateNewBlockColorDef("FFFF00", "glowstone");
	CreateNewBlockColorDef("00FF00", "oak_leaf");
	CreateNewBlockColorDef("964B00", "oak_log");
	CreateNewBlockColorDef("00BB00", "spruce_leaf");
	CreateNewBlockColorDef("824100", "spruce_log");
	CreateNewBlockColorDef("BB9911", "dirt");
	CreateNewBlockColorDef("66BB00", "grass");
	CreateNewBlockColorDef("FFAA33", "plank");
	CreateNewBlockColorDef("777777", "bricks");
	CreateNewBlockColorDef("FFFFFF", "air");
}


const BlockColorDefinition* BlockColorDefinition::GetColorByType(std::string name)
{
	for (int blockColorDefIndex = 0; blockColorDefIndex < (int)s_definitions.size(); blockColorDefIndex++)
	{
		BlockColorDefinition*& blockColorDef = BlockColorDefinition::s_definitions[blockColorDefIndex];
		if (blockColorDef && blockColorDef->m_name == name)
		{
			return blockColorDef;
		}
	}

	ERROR_AND_DIE("Error: invalid block definition name.");
}


const std::string BlockColorDefinition::GetTypeByColor(std::string colorString)
{
	Rgba8 color;
	color.ConvertFromHexString(colorString);
	for (int blockColorDefIndex = 0; blockColorDefIndex < (int)s_definitions.size(); blockColorDefIndex++)
	{
		BlockColorDefinition*& blockColorDef = BlockColorDefinition::s_definitions[blockColorDefIndex];
		if (blockColorDef && blockColorDef->m_color == color)
		{
			return blockColorDef->m_name;
		}
	}

	ERROR_AND_DIE("Error: invalid block definition name.");
}


void BlockColorDefinition::CreateNewBlockColorDef(std::string colorString, std::string blockName)
{
	BlockColorDefinition* newBlockColorDef = new BlockColorDefinition();
	Rgba8 color;
	color.ConvertFromHexString(colorString);
	newBlockColorDef->m_color = color;
	newBlockColorDef->m_name = blockName;
	BlockColorDefinition::s_definitions.push_back(newBlockColorDef);
}


