#include "Game/ItemFactory.hpp"
#include "Game/Item.hpp"
#include "Game/ItemKeyTable.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Mesh/MeshBuilder.hpp"

std::vector<Item*> ItemFactory::s_items;
uint16_t ItemFactory::s_itemIndex = 0;

bool ItemFactory::LoadFromXmlElement(XmlElement const& element)
{
	ItemValueParis valuePairs;

	XmlElement const* child = element.FirstChildElement();
	while (child)
	{
		std::string const& key = ParseXmlAttribute(*child, "key", "");
		std::string const& value = ParseXmlAttribute(*child, "value", "");
		uint16_t keyIndex = ItemKeyTable::GetIndexByName(key);
		valuePairs[keyIndex] = value;
		child = child->NextSiblingElement();
	}

	Item* item = new Item(s_itemIndex, valuePairs);
	s_items.push_back(item);
	s_itemIndex++;

	std::string name = item->GetValueAsString("name");
	std::string modelPath = item->GetValueAsString("model");
	std::string texturePath = item->GetValueAsString("texture");
	float scale = item->GetValueAsFloat("scale");
	Mat44 transform(Vec3::FORWARD, Vec3::RIGHT, Vec3::UP, Vec3::ZERO);
	MeshBuilderConfig meshBuilderConfig;
	meshBuilderConfig.m_transform = transform;
	meshBuilderConfig.m_scale = scale;
	meshBuilderConfig.m_reversedWinding = false;
	meshBuilderConfig.m_invertedTextureV = false;
	meshBuilderConfig.m_modelPath = modelPath;
	meshBuilderConfig.m_texturePath = texturePath;
	g_theRenderer->CreateOrGetMeshFromConfig(name.c_str(), meshBuilderConfig);

	return true;
}


void ItemFactory::LoadFromFile(std::string const& filePath)
{
	XmlDocument doc;
	doc.LoadFile(filePath.c_str());
	XmlElement const* root = doc.RootElement();

	XmlElement const* child = root->FirstChildElement();
	while (child)
	{
		LoadFromXmlElement(*child);
		child = child->NextSiblingElement();
	}
}


bool ItemFactory::LoadFromJsonStringElement(std::string const& element)
{
	ItemValueParis valuePairs;
	Strings pairs = SplitStringOnDelimiter(element, ',');
	for (size_t index = 1; index < pairs.size(); index++)
	{
		std::string pairString = pairs[index];
		Strings pairValues = SplitStringOnDelimiter(pairString, ':');
		std::string key = pairValues[0].substr(2, pairValues[0].size() - 4);
		std::string value = pairValues[1].substr(2, pairValues[1].size() - 3);
		uint16_t keyIndex = ItemKeyTable::GetIndexByName(key);
		valuePairs[keyIndex] = value;
	}

	Item* item = new Item(s_itemIndex, valuePairs);
	s_items.push_back(item);
	s_itemIndex++;
	return true;
}


void ItemFactory::LoadFromJsonString(std::string const& jsonString)
{
	Strings lines = SplitStringOnDelimiter(jsonString, '\n');
	for (std::string line : lines)
	{
		std::string element = line.substr(1, line.size() - 3);
		LoadFromJsonStringElement(element);
	}
}


Item* ItemFactory::CreateByName(std::string const& name)
{
	for (size_t itemIndex = 0; itemIndex < s_items.size(); itemIndex++)
	{
		Item*& item = s_items[itemIndex];
		if (item->GetValueAsString("name") == name)
		{
			uint16_t id = item->GetID();
			ItemValueParis itemValuePairs;
			item->CopyValuePair(itemValuePairs);
			Item* newItem = new Item(id, itemValuePairs);
			return newItem;
		}
	}

	ERROR_AND_DIE("Error: invalid item.");
}


Item* ItemFactory::CreateById(uint16_t id)
{
	for (size_t itemIndex = 0; itemIndex < s_items.size(); itemIndex++)
	{
		Item*& item = s_items[itemIndex];
		uint16_t itemId = item->GetID();
		if (itemId == id)
		{
			ItemValueParis itemValuePairs;
			item->CopyValuePair(itemValuePairs);
			Item* newItem = new Item(id, itemValuePairs);
			return newItem;
		}
	}

	ERROR_AND_DIE("Error: invalid item.");
}


