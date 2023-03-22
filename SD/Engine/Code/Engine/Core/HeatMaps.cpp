#include "Engine/Core/HeatMaps.hpp"

TileHeatMap::TileHeatMap(IntVec2 const& dimensions)
	: m_dimensions(dimensions)
{
	int size = dimensions.x * dimensions.y;
	m_values.resize(size);
}


float TileHeatMap::GetValue(IntVec2 tileCoords) const
{
	int index = GetTileIndexByCoordinates(tileCoords);
	return m_values[index];
}


float TileHeatMap::GetValue(int index) const
{
	return m_values[index];
}


int TileHeatMap::GetSize() const
{
	return int(m_values.size());
}


void TileHeatMap::SetAllValues(float maxHeat)
{
	for (int index = 0; index < int(m_values.size()); index++)
	{
		m_values[index] = maxHeat;
	}
}


void TileHeatMap::SetValue(IntVec2 tileCoords, float value)
{
	int index = GetTileIndexByCoordinates(tileCoords);
	m_values[index] = value;
}


void TileHeatMap::SetValue(int index, float value)
{
	m_values[index] = value;
}


void TileHeatMap::AddValue(IntVec2 tileCoords, float value)
{
	int index = GetTileIndexByCoordinates(tileCoords);
	m_values[index] += value;
}


void TileHeatMap::AddValue(int index, float value)
{
	m_values[index] += value;
}

int TileHeatMap::GetTileIndexByCoordinates(IntVec2 tileCoords) const
{
	return tileCoords.x + tileCoords.y * m_dimensions.x;
}

