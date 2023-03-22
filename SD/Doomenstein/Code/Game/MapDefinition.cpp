#include "Game/MapDefinition.hpp"
#include "Game/TileSetDefinition.hpp"

std::vector<MapDefinition*> MapDefinition::s_definitions;

bool MapDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name						= ParseXmlAttribute(element, "name", "none");
	std::string imageName		= ParseXmlAttribute(element, "image", "none");
	std::string tileSetDefName	= ParseXmlAttribute(element, "tileSet", "none");

	m_image = new Image(imageName.c_str());
	m_tileSetDefinition = TileSetDefinition::GetByName(tileSetDefName);

	XmlElement const* spawnInfos = element.FirstChildElement();
	if (spawnInfos && std::string(spawnInfos->Name()) == "SpawnInfos")
	{
		XmlElement const* spawnInfo = spawnInfos->FirstChildElement();
		while (spawnInfo)
		{
			if (std::string(spawnInfo->Name()) == "SpawnInfo")
			{
				SpawnInfo newSpawnInfo;
				newSpawnInfo.LoadFromXmlElement(*spawnInfo);
				m_spawnInfos.push_back(newSpawnInfo);
				spawnInfo = spawnInfo->NextSiblingElement();
			}
		}
	}

	return true;
}


void MapDefinition::InitializeDefinitions()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/MapDefinitions.xml");
	XmlElement const* root = doc.RootElement();

	XmlElement const* element = root->FirstChildElement();
	while (element)
	{
		if (std::string(element->Name()) == "MapDefinition")
		{
			MapDefinition* newMapDef = new MapDefinition();
			newMapDef->LoadFromXmlElement(*element);
			MapDefinition::s_definitions.push_back(newMapDef);
			element = element->NextSiblingElement();
		}
	}
}


void MapDefinition::ClearDefinitions()
{
	for (int mapDefIndex = 0; mapDefIndex < (int)s_definitions.size(); mapDefIndex++)
	{
		MapDefinition*& mapDef = MapDefinition::s_definitions[mapDefIndex];
		if (mapDef)
		{
			delete mapDef;
		}
	}

	s_definitions.clear();
}


const MapDefinition* MapDefinition::GetByName(const std::string& name)
{
	for (int mapDefIndex = 0; mapDefIndex < (int)s_definitions.size(); mapDefIndex++)
	{
		MapDefinition*& mapDef = MapDefinition::s_definitions[mapDefIndex];
		if (mapDef && mapDef->m_name == name)
		{
			return mapDef;
		}
	}

	ERROR_AND_DIE("Error: map definition.");
}


