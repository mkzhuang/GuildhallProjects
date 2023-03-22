#pragma once
#include "Engine/Core/EngineCommon.hpp"

struct Item;

class ItemFactory
{
public:
	static bool LoadFromXmlElement(XmlElement const& element);
	static void LoadFromFile(std::string const& filePath);
	static bool LoadFromJsonStringElement(std::string const& element);
	static void LoadFromJsonString(std::string const& jsonString);
	static Item* CreateByName(std::string const& name);
	static Item* CreateById(uint16_t id);

public:
	static uint16_t s_itemIndex;
	static std::vector<Item*> s_items;
};