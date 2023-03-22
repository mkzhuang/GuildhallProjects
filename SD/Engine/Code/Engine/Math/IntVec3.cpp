#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <math.h>

IntVec3::IntVec3(IntVec3 const& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
{
}


IntVec3::IntVec3(int initialX, int initialY, int initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}

IntVec3 const IntVec3::ZERO(0, 0, 0);
IntVec3 const IntVec3::ONE(1, 1, 1);

IntVec3 const IntVec3::STEP_NORTH(0, 1, 0);
IntVec3 const IntVec3::STEP_SOUTH(0, -1, 0);
IntVec3 const IntVec3::STEP_WEST(-1, 0, 0);
IntVec3 const IntVec3::STEP_EAST(1, 0, 0);
IntVec3 const IntVec3::STEP_UP(0, 0, 1);
IntVec3 const IntVec3::STEP_DOWN(0, 0, -1);

IntVec3 const IntVec3::STEP_NW(-1, 1, 0);
IntVec3 const IntVec3::STEP_NE(1, 1, 0);
IntVec3 const IntVec3::STEP_SW(-1, -1, 0);
IntVec3 const IntVec3::STEP_SE(1, -1, 0);

float IntVec3::GetLength() const
{
	return sqrtf((static_cast<float>(x) * static_cast<float>(x)) + (static_cast<float>(y) * static_cast<float>(y)) + (static_cast<float>(z) * static_cast<float>(z)));
}


int IntVec3::GetLengthSquared() const
{
	return (x * x) + (y * y) + (z * z);
}


int IntVec3::GetTaxicabLength() const
{
	return (abs(x) + abs(y) + abs(z));
}


void IntVec3::SetFromText(std::string const& text)
{
	Strings tokens = SplitStringOnDelimiter(text, ',');

	if (tokens.size() == 3)
	{
		x = atoi(tokens[0].c_str());
		y = atoi(tokens[1].c_str());
		z = atoi(tokens[2].c_str());
	}
	else
	{
		ERROR_AND_DIE("IntVec3 set from text input incorrect.");
	}
}


// Operators (const)
IntVec3 const IntVec3::operator+(const IntVec3& vecToAdd) const
{
	return IntVec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}


IntVec3 const IntVec3::operator-(const IntVec3& vecToSubtract) const
{
	return IntVec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


IntVec3 const IntVec3::operator-() const
{
	return IntVec3(-x, -y, -z);
}


// Operators (self-mutating / non-const)
void IntVec3::operator+=(const IntVec3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


void IntVec3::operator-=(const IntVec3& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


void IntVec3::operator=(const IntVec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


bool IntVec3::operator==(const IntVec3& compare) const
{
	return x == compare.x && y == compare.y && z == compare.z;
}


bool IntVec3::operator!=(const IntVec3& compare) const
{
	return x != compare.x || y != compare.y || z != compare.z;
}
