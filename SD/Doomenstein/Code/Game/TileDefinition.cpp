#include "Game/TileDefinition.hpp"
#include "Game/TileMaterialDefinition.hpp"

std::vector<TileDefinition*> TileDefinition::s_definitions;

bool TileDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name								= ParseXmlAttribute(element, "name", "none");
	m_isSolid							= ParseXmlAttribute(element, "isSolid", false);
	std::string ceilingMaterialName		= ParseXmlAttribute(element, "ceilingMaterial", "none");
	std::string floorMaterialName		= ParseXmlAttribute(element, "floorMaterial", "none");
	std::string wallMaterialName		= ParseXmlAttribute(element, "wallMaterial", "none");

	if (ceilingMaterialName != "none") m_ceilingMaterialDefinition = TileMaterialDefinition::GetByName(ceilingMaterialName);
	if (floorMaterialName != "none") m_floorMaterialDefinition = TileMaterialDefinition::GetByName(floorMaterialName);
	if (wallMaterialName != "none") m_wallMaterialDefinition = TileMaterialDefinition::GetByName(wallMaterialName);
	return true;
}


void TileDefinition::InitializeDefinitions()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/TileDefinitions.xml");
	XmlElement const* root = doc.RootElement();

	XmlElement const* element = root->FirstChildElement();
	while (element)
	{
		if (std::string(element->Name()) == "TileDefinition")
		{
			TileDefinition* newTileDef = new TileDefinition();
			newTileDef->LoadFromXmlElement(*element);
			TileDefinition::s_definitions.push_back(newTileDef);
			element = element->NextSiblingElement();
		}
	}
}


const TileDefinition* TileDefinition::GetByName(const std::string& name)
{
	for (int tileDefIndex = 0; tileDefIndex < (int)s_definitions.size(); tileDefIndex++)
	{
		TileDefinition*& tileDef = TileDefinition::s_definitions[tileDefIndex];
		if (tileDef && tileDef->m_name == name)
		{
			return tileDef;
		}
	}

	ERROR_AND_DIE("Error: tile definition.");
}


