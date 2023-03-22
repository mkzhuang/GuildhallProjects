#pragma once
#include "Engine/Math/IntVec2.hpp"

#include <vector>

class TileHeatMap
{
public:
	TileHeatMap(IntVec2 const& dimensions);
	~TileHeatMap() {}

	float GetValue(int index) const;
	float GetValue(IntVec2 tileCoords) const;
	int GetSize() const;

	void SetAllValues(float maxHeat);
	void SetValue(int index, float value);
	void SetValue(IntVec2 tileCoords, float value);
	void AddValue(int index, float value);
	void AddValue(IntVec2 tileCoords, float value);

protected:
	int GetTileIndexByCoordinates(IntVec2 tileCoords) const;

	IntVec2 m_dimensions = IntVec2::ZERO;
	std::vector<float> m_values;
};


