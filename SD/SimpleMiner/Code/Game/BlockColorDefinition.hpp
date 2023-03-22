#pragma once
#include "Game/GameCommon.hpp"

class BlockColorDefinition
{
public:
	Rgba8 m_color;
	std::string m_name;

	static void InitializeBlockColorDefinitions();
	static const BlockColorDefinition* GetColorByType(std::string name);
	static const std::string GetTypeByColor(std::string colorString);
	static void CreateNewBlockColorDef(std::string colorString, std::string name);
	static std::vector<BlockColorDefinition*> s_definitions;
};


