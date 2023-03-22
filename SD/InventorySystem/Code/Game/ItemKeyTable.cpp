#include "Game/ItemKeyTable.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::vector<ItemKey> ItemKeyTable::s_keys;

uint16_t ItemKeyTable::AddItemKey(ItemKeyType const& keyType, std::string const& name)
{
	ItemKey newKey;
	newKey.keyType = keyType;
	newKey.keyName = name;
	newKey.keyIndex = (uint16_t)s_keys.size();
	s_keys.push_back(newKey);
	return newKey.keyIndex;
}


ItemKey const& ItemKeyTable::GetItemKeyByName(std::string const& name)
{
	for (ItemKey const& key : s_keys)
	{
		if (key.keyName == name)
		{
			return key;
		}
	}
	ERROR_AND_DIE("Item key not found");
}


uint16_t ItemKeyTable::GetIndexByName(std::string const& name)
{
	for (ItemKey const& key : s_keys)
	{
		if (key.keyName == name)
		{
			return key.keyIndex;
		}
	}

	return 0;
}


ItemKeyType const& ItemKeyTable::GetTypeByName(std::string const& name)
{
	for (ItemKey const& key : s_keys)
	{
		if (key.keyName == name)
		{
			return key.keyType;
		}
	}
	ERROR_AND_DIE("Item key not found");
}


ItemKey const& ItemKeyTable::GetItemKeyByIndex(uint16_t index)
{
	return s_keys[index];
}


