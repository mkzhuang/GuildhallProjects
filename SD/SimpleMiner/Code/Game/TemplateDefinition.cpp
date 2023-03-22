#include "Game/TemplateDefinition.hpp"
#include "Game/BlockColorDefinition.hpp"
#include "Engine/Core/FileUtils.hpp"

std::vector<TemplateDefinition*> TemplateDefinition::s_definitions;

bool TemplateDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", "none");

	XmlElement const* child = element.FirstChildElement();
	while (child)
	{
		std::string blockName = ParseXmlAttribute(*child, "block", "air");
		IntVec3 offset = ParseXmlAttribute(*child, "offset", IntVec3::ZERO);
		TemplateBlock block;
		block.m_blockName = blockName;
		block.m_offset = offset;
		m_blocks.push_back(block);
		child = child->NextSiblingElement();
	}

	return true;
}


void TemplateDefinition::ParseInfoString(std::string const& infoString)
{
	size_t nameBegin = infoString.find("name") + 7;
	std::string name;
	for (size_t charIndex = nameBegin; charIndex < infoString.size(); charIndex++)
	{
		char const& charAtIndex = infoString[charIndex];
		if (charAtIndex != '\"')
		{
			name += charAtIndex;
		}
		else
		{
			break;
		}
	}
	m_name = name;

	size_t dataIndex = infoString.find("data");
	size_t lastCloseBracketIndex = 0;
	for (size_t charIndex = 0; charIndex < infoString.size(); charIndex++)
	{
		if (infoString[charIndex] == ']')
		{
			lastCloseBracketIndex = charIndex;
		}
	}

	size_t dataBegin = dataIndex + 7;
	std::string dataString = infoString.substr(dataBegin, lastCloseBracketIndex - dataBegin);

	Strings blockInfoStrings = SplitStringOnDelimiter(dataString, '[');
	for (size_t blockInfoIndex = 0; blockInfoIndex < blockInfoStrings.size(); blockInfoIndex++)
	{
		std::string blockInfo = blockInfoStrings[blockInfoIndex];
		if (blockInfo.empty()) continue;
		size_t length = blockInfo.size() - 2;
		blockInfo = blockInfo.substr(0, length);
		Strings blockData = SplitStringOnDelimiter(blockInfo, ',');
		int x = std::stoi(blockData[0]);
		int y = std::stoi(blockData[2]);
		int z = std::stoi(blockData[1]);
		IntVec3 offset(x, -y, z);
		std::string colorString = blockData[3].substr(2, 6);
		std::string blockName = BlockColorDefinition::GetTypeByColor(colorString);
		TemplateBlock block;
		block.m_blockName = blockName;
		block.m_offset = offset;
		m_blocks.push_back(block);
	}
}


void TemplateDefinition::InitalizeDefinitions(char const* path)
{
	XmlDocument doc;
	doc.LoadFile(path);
	XmlElement const* root = doc.RootElement();

	XmlElement const* element = root->FirstChildElement();
	while (element)
	{
		TemplateDefinition* newTemplateDef = new TemplateDefinition();
		newTemplateDef->LoadFromXmlElement(*element);
		TemplateDefinition::s_definitions.push_back(newTemplateDef);
		element = element->NextSiblingElement();
	}
}


void TemplateDefinition::LoadFromSpriteFile(char const* filePath)
{
	if (!FileExists(filePath)) return;
	std::string fileString;
	FileReadToString(fileString, filePath);
	TemplateDefinition* newTemplateDef = new TemplateDefinition();
	newTemplateDef->ParseInfoString(fileString);
	TemplateDefinition::s_definitions.push_back(newTemplateDef);
}


TemplateDefinition const* TemplateDefinition::GetByName(std::string const& name)
{
	for (int defIndex = 0; defIndex < (int)s_definitions.size(); defIndex++)
	{
		TemplateDefinition*& templateDef = TemplateDefinition::s_definitions[defIndex];
		if (templateDef && templateDef->m_name == name)
		{
			return templateDef;
		}
	}

	ERROR_AND_DIE("Error: template definition not found.");
}


