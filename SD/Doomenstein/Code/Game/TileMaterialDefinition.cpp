#include "Game/TileMaterialDefinition.hpp"

std::vector<TileMaterialDefinition*> TileMaterialDefinition::s_definitions;

bool TileMaterialDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name			= ParseXmlAttribute(element, "name", "none");
	bool isVisible	= ParseXmlAttribute(element, "isVisible", true);
	if (!isVisible) return true;

	std::string shaderName		= ParseXmlAttribute(element, "shader", "none");
	std::string textureName		= ParseXmlAttribute(element, "texture", "none");
	IntVec2 grid				= ParseXmlAttribute(element, "cellCount", IntVec2::ZERO);
	IntVec2 cell				= ParseXmlAttribute(element, "cell", IntVec2::ZERO);

	m_shader = g_theRenderer->GetShaderForName(shaderName.c_str());
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(textureName.c_str());
	SpriteSheet spriteSheet(*m_texture, grid);
	m_uv = spriteSheet.GetSpriteUVs(cell);
	return true;
}


void TileMaterialDefinition::InitializeDefinitions()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/TileMaterialDefinitions.xml");
	XmlElement const* root = doc.RootElement();

	XmlElement const* element = root->FirstChildElement();
	while (element)
	{
		if (std::string(element->Name()) == "TileMaterialDefinition")
		{
			TileMaterialDefinition* newMaterialTileDef = new TileMaterialDefinition();
			newMaterialTileDef->LoadFromXmlElement(*element);
			TileMaterialDefinition::s_definitions.push_back(newMaterialTileDef);
			element = element->NextSiblingElement();
		}
	}
}


const TileMaterialDefinition* TileMaterialDefinition::GetByName(const std::string& name)
{
	for (int tileMaterialDefIndex = 0; tileMaterialDefIndex < (int)s_definitions.size(); tileMaterialDefIndex++)
	{
		TileMaterialDefinition*& tileMaterialDef = TileMaterialDefinition::s_definitions[tileMaterialDefIndex];
		if (tileMaterialDef && tileMaterialDef->m_name == name)
		{
			return tileMaterialDef;
		}
	}

	ERROR_AND_DIE("Error: tile material definition.");
}


