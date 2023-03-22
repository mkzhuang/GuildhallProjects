#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/IntVec3.hpp"

#include <stdint.h>
#include <vector>

struct TemplateBlock
{
	std::string m_blockName;
	IntVec3 m_offset = IntVec3::ZERO;
};

class TemplateDefinition
{
public:
	bool LoadFromXmlElement(XmlElement const& element);

private:
	void ParseInfoString(std::string const& infoString);

public:
	std::string m_name;
	std::vector<TemplateBlock> m_blocks;

	static void InitalizeDefinitions(char const* path);
	static void LoadFromSpriteFile(char const* filePath);
	static TemplateDefinition const* GetByName(std::string const& name);
	static std::vector<TemplateDefinition*> s_definitions;
};