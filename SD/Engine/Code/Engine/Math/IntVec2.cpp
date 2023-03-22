#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <math.h>

IntVec2::IntVec2(IntVec2 const& copy)
	: x(copy.x)
	, y(copy.y)
{
}


IntVec2::IntVec2(int initialX, int initialY)
	: x(initialX)
	, y(initialY)
{
}

IntVec2 const IntVec2::ZERO(0, 0);
IntVec2 const IntVec2::ONE(1, 1);

IntVec2 const IntVec2::STEP_NORTH(0, 1);
IntVec2 const IntVec2::STEP_SOUTH(0, -1);
IntVec2 const IntVec2::STEP_WEST(-1, 0);
IntVec2 const IntVec2::STEP_EAST(1, 0);

IntVec2 const IntVec2::STEP_NW(-1, 1);
IntVec2 const IntVec2::STEP_NE(1, 1);
IntVec2 const IntVec2::STEP_SW(-1, -1);
IntVec2 const IntVec2::STEP_SE(1, -1);

float IntVec2::GetLength() const
{
	return sqrtf((static_cast<float>(x) * static_cast<float>(x)) + (static_cast<float>(y) * static_cast<float>(y)));
}


int IntVec2::GetLengthSquared() const
{
	return (x * x) + (y * y);
}


int IntVec2::GetTaxicabLength() const
{
	return (abs(x) + abs(y));
}


float IntVec2::GetOrientationRadians() const
{
	return atan2f(static_cast<float>(y), static_cast<float>(x));
}


float IntVec2::GetOrientationDegrees() const
{
	return Atan2Degrees(static_cast<float>(y), static_cast<float>(x));
}


IntVec2 const IntVec2::GetRotated90Degrees() const
{
	return IntVec2(-y, x);
}


IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2(y, -x);
}


void IntVec2::Rotate90Degrees()
{
	int temp = x;
	x = -y;
	y = temp;
}


void IntVec2::RotateMinus90Degrees()
{
	int temp = y;
	y = -x;
	x = temp;
}


void IntVec2::SetFromText(std::string const& text)
{
	Strings tokens = SplitStringOnDelimiter(text, ',');

	if (tokens.size() == 2)
	{
		x = atoi(tokens[0].c_str());
		y = atoi(tokens[1].c_str());
	}
	else
	{
		ERROR_AND_DIE("IntVec2 set from text input incorrect.");
	}
}


// Operators (const)
IntVec2 const IntVec2::operator+(const IntVec2& vecToAdd) const
{
	return IntVec2( x + vecToAdd.x, y + vecToAdd.y );
}


IntVec2 const IntVec2::operator-(const IntVec2& vecToSubtract) const
{
	return IntVec2( x - vecToSubtract.x, y - vecToSubtract.y );
}


IntVec2 const IntVec2::operator-() const
{
	return IntVec2( -x, -y );
}


// Operators (self-mutating / non-const)
void IntVec2::operator+=(const IntVec2& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


void IntVec2::operator-=(const IntVec2& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


void IntVec2::operator=(const IntVec2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}


bool IntVec2::operator==(const IntVec2& compare) const
{
	return x == compare.x && y == compare.y;
}


bool IntVec2::operator!=(const IntVec2& compare) const
{
	return x != compare.x || y != compare.y;
}
