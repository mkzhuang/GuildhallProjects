#pragma once

#include <string>
#include <vector>

enum class ItemKeyType
{
	Default,
	String,
	Bool,
	Int,
	Float,
};


struct ItemKey
{
	ItemKeyType keyType = ItemKeyType::Default;
	std::string keyName ="";
	uint16_t keyIndex = 0;
};


struct ItemKeyTable
{
public:
	static uint16_t AddItemKey(ItemKeyType const& keyType, std::string const& name);
	static ItemKey const& GetItemKeyByName(std::string const& name);
	static uint16_t GetIndexByName(std::string const& name);
	static ItemKeyType const& GetTypeByName(std::string const& name);
	static ItemKey const& GetItemKeyByIndex(uint16_t index);

public:
	static std::vector<ItemKey> s_keys;
};