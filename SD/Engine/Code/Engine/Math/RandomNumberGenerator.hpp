#pragma once
#include "Engine/Math/FloatRange.hpp"

class RandomNumberGenerator
{
public:
	int RollRandomIntLessThan(int maxNotInclusive);
	int RollRandomIntInRange(int minInclusive, int MaxInclusive);
	float RollRandomFloatZeroToOneInclusive();
	float RollRandomFloatInRange(float minInclusive, float maxInclusive);
	float RollRandomFloatInFloatRange(FloatRange floatRange);

private:
	//unsigned int m_seed = 0;
	//int m_position = 0;
};