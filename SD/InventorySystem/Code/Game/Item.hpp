#pragma once

#include <map>
#include <string>
#include <stdint.h>

typedef std::map<uint16_t, std::string> ItemValueParis;

struct Item
{
public:
	Item();
	Item(uint16_t id, ItemValueParis valuesParis);
	~Item() {}

	uint16_t GetID() const;
	std::string const& GetValueAsString(std::string const& keyName) const;
	bool GetValueAsBool(std::string const& keyName) const;
	float GetValueAsFloat(std::string const& keyName) const;
	int GetValueAsInt(std::string const& keyName) const;
	void CopyValuePair(ItemValueParis& out);

protected:
	uint16_t m_id = 0;
	ItemValueParis m_values;
	//std::string m_name = "";
	//float m_weight = 0.f;
	//float m_cost = 0.f;
	//std::string m_description = "";
	//std::string m_category = "";
	//std::string m_subCategory = "";
	//int m_maxStack = 1;
	//bool m_isEquipable = false;
	//bool m_isConsumable = false;
	//Mesh* m_model = nullptr;

	//std::vector<std::string> m_customCategories;
};