#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <math.h>

Vec2 const Vec2::ZERO(0.f, 0.f);
Vec2 const Vec2::ONE(1.f, 1.f);

Vec2 const Vec2::NORTH(0.f, 1.f);
Vec2 const Vec2::SOUTH(0.f, -1.f);
Vec2 const Vec2::WEST(-1.f, 0.f);
Vec2 const Vec2::EAST(1.f, 0.f);

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}


Vec2::Vec2(IntVec2 from)
	: x(static_cast<float>(from.x))
	, y(static_cast<float>(from.y))
{
}


//static methods
Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length)
{
	float newX = length * cosf(orientationRadians);
	float newY = length * sinf(orientationRadians);
	return Vec2(newX, newY);
}


Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	float newX = length * CosDegrees(orientationDegrees);
	float newY = length * SinDegrees(orientationDegrees);
	return Vec2(newX, newY);
}


//Accessors
float Vec2::GetLength() const
{
	return sqrtf((x * x) + (y * y));
}


float Vec2::GetLengthSquared() const
{
	return (x * x) + (y * y);
}


float Vec2::GetOrientationRadians() const
{
	return atan2f(y, x);
}


float Vec2::GetOrientationDegrees() const
{
	return Atan2Degrees(y, x);
}


Vec2 const Vec2::GetRotated90Degrees() const
{
	return Vec2(-y, x);
}


Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2(y, -x);
}


Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	if (deltaRadians == 0.f) return *this;

	float length = GetLength();
	float theta = GetOrientationRadians();
	theta += deltaRadians;
	float newX = length * cosf(theta);
	float newY = length * sinf(theta);
	return Vec2(newX, newY);
}


Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	if (deltaDegrees == 0.f) return *this;

	float length = GetLength();
	float theta = GetOrientationDegrees();
	theta += deltaDegrees;
	float newX = length * CosDegrees(theta);
	float newY = length * SinDegrees(theta);
	return Vec2(newX, newY);
}


Vec2 const Vec2::GetClamped(float maxLength) const
{
	if (GetLengthSquared() <= (maxLength * maxLength)) return *this;

	float length = GetLength();
	float ratio = maxLength / length;
	float newX = x * ratio;
	float newY = y * ratio;
	return Vec2(newX, newY);
}


Vec2 const Vec2::GetNormalized() const
{
	if (GetLengthSquared() == 1.f) return Vec2(x, y);
	if (GetLengthSquared() == 0.f) return Vec2::ZERO;

	float length = GetLength();
	float ratio = 1.f / length;
	float newX = x * ratio;
	float newY = y * ratio;
	return Vec2(newX, newY);
}


Vec2 const Vec2::GetReflected(Vec2 const& impactSurfaceNormal) const
{
	Vec2 projectedLength = DotProduct2D(*this, impactSurfaceNormal) * impactSurfaceNormal;
	return *this - (2.f * projectedLength);
}


//Mutators
void Vec2::SetOrientationRadians(float newOrientationRadians)
{
	if (newOrientationRadians == 0.f) return;

	float length = GetLength();
	x = length * cosf(newOrientationRadians);
	y = length * sinf(newOrientationRadians);
}


void Vec2::SetOrientationDegrees(float newOrientationDegrees)
{
	if (newOrientationDegrees == 0.f) return;

	float length = GetLength();
	x = length * CosDegrees(newOrientationDegrees);
	y = length * SinDegrees(newOrientationDegrees);
}


void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	x = newLength * cosf(newOrientationRadians);
	y = newLength * sinf(newOrientationRadians);
}


void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	x = newLength * CosDegrees(newOrientationDegrees);
	y = newLength * SinDegrees(newOrientationDegrees);
}


void Vec2::Rotate90Degrees()
{
	float temp = x;
	x = -y;
	y = temp;
}


void Vec2::RotateMinus90Degrees()
{
	float temp = y;
	y = -x;
	x = temp;
}


void Vec2::RotateRadians(float deltaRadians)
{
	float length = GetLength();
	float theta = GetOrientationRadians();
	theta += deltaRadians;
	x = length * cosf(theta);
	y = length * sinf(theta);
}


void Vec2::RotateDegrees(float deltaDegrees)
{
	float length = GetLength();
	float theta = GetOrientationDegrees();
	theta += deltaDegrees;
	x = length * CosDegrees(theta);
	y = length * SinDegrees(theta);
}


void Vec2::SetLength(float newLength)
{ 
	float length = GetLength();
	if (length == 0.f) return;
	float ratio = newLength / length;
	*this *= ratio;
}


void Vec2::ClampLength(float maxLength)
{
	float length = GetLength();
	if (length <= maxLength) return;
	float ratio = maxLength / length;
	*this *= ratio;
}


void Vec2::Normalize()
{
	if (GetLengthSquared() == 1.f || GetLengthSquared() == 0.f) return;

	float length = GetLength();
	float ratio = 1.f / length;
	*this *= ratio;
}


float Vec2::NormalizeAndGetPreviousLength()
{
	float length = GetLength();
	float ratio = 1.f / length;
	*this *= ratio;
	return length;
}


void Vec2::Reflect(Vec2 const& impactSurfaceNormal)
{
	Vec2 projectedLength = DotProduct2D(*this, impactSurfaceNormal) * impactSurfaceNormal;
	*this -= (2.f * projectedLength);
}


void Vec2::SetFromText(std::string const& text)
{
	Strings tokens = SplitStringOnDelimiter(text, ',');

	if (tokens.size() == 2)
	{
		x = static_cast<float>(atof(tokens[0].c_str()));
		y = static_cast<float>(atof(tokens[1].c_str()));
	}
	else
	{
		ERROR_AND_DIE("Vec2 set from text input incorrect.");
	}
}


//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2( x - vecToSubtract.x, y - vecToSubtract.y );
}


//------------------------------------------------------------------------------------------------
Vec2 const Vec2::operator-() const
{
	return Vec2( -x, -y );
}


//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator*( float uniformScale ) const
{
	return Vec2( x * uniformScale, y * uniformScale );
}


//------------------------------------------------------------------------------------------------
Vec2 const Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x, y * vecToMultiply.y );
}


//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator/( float inverseScale ) const
{
	return Vec2( x / inverseScale, y / inverseScale );
}


Vec2 const Vec2::operator/(const Vec2& vecToDivide) const
{
	return Vec2(x / vecToDivide.x, y / vecToDivide.y);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
Vec2 const operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2( uniformScale * vecToScale.x,  uniformScale * vecToScale.y);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return x == compare.x && y == compare.y;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return x != compare.x || y != compare.y;
}


