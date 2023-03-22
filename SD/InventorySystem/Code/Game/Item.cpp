#include "Game/Item.hpp"
#include "Game/ItemKeyTable.hpp"

Item::Item()
{
}


Item::Item(uint16_t id, ItemValueParis valuesParis)
	: m_id(id)
	, m_values(valuesParis)
{
}


uint16_t Item::GetID() const
{
	return m_id;
}


std::string const& Item::GetValueAsString(std::string const& keyName) const
{
	ItemKey const& key = ItemKeyTable::GetItemKeyByName(keyName);
	if (key.keyType == ItemKeyType::String)
	{
		std::map<uint16_t, std::string>::const_iterator iter = m_values.find(key.keyIndex);
		if (iter != m_values.end()) return iter->second;
	}

	return keyName;
}


bool Item::GetValueAsBool(std::string const& keyName) const
{
	ItemKey const& key = ItemKeyTable::GetItemKeyByName(keyName);
	if (key.keyType == ItemKeyType::Bool)
	{
		std::map<uint16_t, std::string>::const_iterator iter = m_values.find(key.keyIndex);
		if (iter != m_values.end())
		{
			if (iter->second == "true") return true;
		}
	}

	return false;
}


float Item::GetValueAsFloat(std::string const& keyName) const
{
	ItemKey const& key = ItemKeyTable::GetItemKeyByName(keyName);
	if (key.keyType == ItemKeyType::Float)
	{
		std::map<uint16_t, std::string>::const_iterator iter = m_values.find(key.keyIndex);
		if (iter != m_values.end()) return static_cast<float>(atof(iter->second.c_str()));
	}

	return 1.f;
}


int Item::GetValueAsInt(std::string const& keyName) const
{
	ItemKey const& key = ItemKeyTable::GetItemKeyByName(keyName);
	if (key.keyType == ItemKeyType::Int)
	{
		std::map<uint16_t, std::string>::const_iterator iter = m_values.find(key.keyIndex);
		if (iter != m_values.end()) return atoi(iter->second.c_str());
	}

	return 0;
}


void Item::CopyValuePair(ItemValueParis& out)
{
	out = m_values;
}


