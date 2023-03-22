#include "Game/TileSetDefinition.hpp"
#include "Game/TileDefinition.hpp"

std::vector<TileSetDefinition*> TileSetDefinition::s_definitions;

bool TileMapping::LoadFromXmlElement(const XmlElement& element)
{
	m_color					= ParseXmlAttribute(element, "color", Rgba8::WHITE);
	std::string tileName	= ParseXmlAttribute(element, "tile", "none");

	m_tileDefinition = TileDefinition::GetByName(tileName);
	return true;
}


bool TileSetDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", "none");
	std::string defaultTileName = ParseXmlAttribute(element, "defaultTile", "none");
	m_defaultTile = TileDefinition::GetByName(defaultTileName);

	XmlElement const* mapping = element.FirstChildElement();
	while (mapping)
	{
		if (std::string(mapping->Name()) == "TileMapping")
		{
			TileMapping newTileMapping;
			newTileMapping.LoadFromXmlElement(*mapping);
			m_mappings.push_back(newTileMapping);
			mapping = mapping->NextSiblingElement();
		}
	}
	return true;
}


const TileDefinition* TileSetDefinition::GetTileDefinitionByColor(const Rgba8& color) const
{
	for (int tileMappingIndex = 0; tileMappingIndex < (int)m_mappings.size(); tileMappingIndex++)
	{
		TileMapping tileMapping = m_mappings[tileMappingIndex];
		if (tileMapping.m_color == color)
		{
			return tileMapping.m_tileDefinition;
		}
	}

	ERROR_AND_DIE("Error: tile mapping.");
}


void TileSetDefinition::InitializeDefinitions()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/TileSetDefinitions.xml");
	XmlElement const* root = doc.RootElement();

	XmlElement const* element = root->FirstChildElement();
	while (element)
	{
		if (std::string(element->Name()) == "TileSetDefinition")
		{
			TileSetDefinition* newTileSetDef = new TileSetDefinition();
			newTileSetDef->LoadFromXmlElement(*element);
			TileSetDefinition::s_definitions.push_back(newTileSetDef);
			element = element->NextSiblingElement();
		}
	}
}


const TileSetDefinition* TileSetDefinition::GetByName(const std::string& name)
{
	for (int tileSetDefIndex = 0; tileSetDefIndex < (int)s_definitions.size(); tileSetDefIndex++)
	{
		TileSetDefinition*& tileSetDef = TileSetDefinition::s_definitions[tileSetDefIndex];
		if (tileSetDef && tileSetDef->m_name == name)
		{
			return tileSetDef;
		}
	}

	ERROR_AND_DIE("Error: tile set definition.");
}


