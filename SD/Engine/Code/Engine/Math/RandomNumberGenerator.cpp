#include "Engine/Math/RandomNumberGenerator.hpp"

#include <stdlib.h>

int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	return rand() % maxNotInclusive;
}


int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	int range = maxInclusive - minInclusive + 1;
	return (rand() % range) + minInclusive;
}


float RandomNumberGenerator::RollRandomFloatZeroToOneInclusive()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}


float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float range = maxInclusive - minInclusive;
	return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * range + minInclusive;
}


float RandomNumberGenerator::RollRandomFloatInFloatRange(FloatRange floatRange)
{
	return RollRandomFloatInRange(floatRange.m_min, floatRange.m_max);
}


