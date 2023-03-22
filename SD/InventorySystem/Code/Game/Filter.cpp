#include "Game/Filter.hpp"
#include "Game/Item.hpp"

#include <algorithm>

std::map<std::string, std::function<bool(Item const* a, Item const* b)>> Filter::s_sorters;

void Filter::InitializeFilter()
{
	auto nameSorter = [](Item const* a, Item const* b)
	{
		std::string const& aName = a->GetValueAsString("name");
		std::string const& bName = b->GetValueAsString("name");
		int result = aName.compare(bName);
		return result < 0 ? true : false;
	};
	s_sorters["name"] = nameSorter;

	auto weightSorter = [](Item const* a, Item const* b)
	{
		float aFloat = a->GetValueAsFloat("weight");
		float bFloat = b->GetValueAsFloat("weight");
		if (aFloat < bFloat)
		{
			return true;
		}
		else if (bFloat < aFloat)
		{
			return false;
		}
		else
		{
			std::string const& aName = a->GetValueAsString("name");
			std::string const& bName = b->GetValueAsString("name");
			int result = aName.compare(bName);
			return result < 0 ? true : false;
		}
	};
	s_sorters["weight"] = weightSorter;

	auto valueSorter = [](Item const* a, Item const* b)
	{
		float aFloat = a->GetValueAsFloat("value");
		float bFloat = b->GetValueAsFloat("value");
		if (aFloat < bFloat)
		{
			return true;
		}
		else if (bFloat < aFloat)
		{
			return false;
		}
		else
		{
			std::string const& aName = a->GetValueAsString("name");
			std::string const& bName = b->GetValueAsString("name");
			int result = aName.compare(bName);
			return result < 0 ? true : false;
		}
	};
	s_sorters["value"] = valueSorter;
}


void Filter::SortBy(std::vector<Item*>& items, std::function<bool(Item const* a, Item const* b)> sorter, bool isDescend)
{
	if (!isDescend)
	{
		std::sort(items.begin(), items.end(), sorter);
	}
	else
	{
		auto inverseSorter = [sorter](Item const* a, Item const* b) 
		{ 
			return !sorter(a, b); 
		};
		std::sort(items.begin(), items.end(), inverseSorter);
	}
}


std::function<bool(Item const* a, Item const* b)> Filter::GetSorterByName(std::string const& name)
{
	auto iter = s_sorters.find(name);
	if (iter != s_sorters.end())
	{
		return iter->second;
	}
	return s_sorters.begin()->second;
}


