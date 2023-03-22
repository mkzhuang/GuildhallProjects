#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <math.h>

Vec3::Vec3(Vec3 const& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
{
}


Vec3::Vec3(Vec2 const& pos)
	: x(pos.x)
	, y(pos.y)
	, z(0.f)
{
}


Vec3::Vec3(float initialX, float initialY, float initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}


Vec3::Vec3(IntVec3 from)
	: x(static_cast<float>(from.x))
	, y(static_cast<float>(from.y))
	, z(static_cast<float>(from.z))
{
}


Vec3 const Vec3::ZERO(0.f, 0.f, 0.f);
Vec3 const Vec3::ONE(1.f, 1.f, 1.f);
Vec3 const Vec3::UP(0.f, 0.f, 1.f);
Vec3 const Vec3::DOWN(0.f, 0.f, -1.f);
Vec3 const Vec3::FORWARD(1.f, 0.f, 0.f);
Vec3 const Vec3::BACKWARD(-1.f, 0.f, 0.f);
Vec3 const Vec3::RIGHT(0.f, 1.f, 0.f);
Vec3 const Vec3::LEFT(0.f, -1.f, 0.f);
Vec3 const Vec3::EAST(1.f, 0.f, 0.f);
Vec3 const Vec3::WEST(-1.f, 0.f, 0.f);
Vec3 const Vec3::NORTH(0.f, 1.f, 0.f);
Vec3 const Vec3::SOUTH(0.f, -1.f, 0.f);

float Vec3::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}


float Vec3::GetLengthXY() const
{
	return sqrtf((x * x) + (y * y));
}


float Vec3::GetLengthSquared() const
{
	return ((x * x) + (y * y) + (z * z));
}


float Vec3::GetLengthXYSquared() const
{
	return ((x * x) + (y * y));
}


float Vec3::GetAngleAboutZRadians() const
{
	return atan2f(y, x);
}


float Vec3::GetAngleAboutZDegrees() const
{
	return Atan2Degrees(y, x);
}


Vec3 const Vec3::GetRotatedAboutZRadians(float deltaRadians) const
{
	if (deltaRadians == 0.f) return *this;

	float length = GetLengthXY();
	float theta = GetAngleAboutZRadians();
	theta += deltaRadians;
	float newX = length * cosf(theta);
	float newY = length * sinf(theta);
	return Vec3(newX, newY, z);
}


Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	if (deltaDegrees == 0.f) return *this;

	float length = GetLengthXY();
	float theta = GetAngleAboutZDegrees();
	theta += deltaDegrees;
	float newX = length * CosDegrees(theta);
	float newY = length * SinDegrees(theta);
	return Vec3(newX, newY, z);
}


Vec3 const Vec3::GetClamped(float maxLength) const
{
	if (GetLengthSquared() <= (maxLength * maxLength)) return *this;

	float length = GetLength();
	float ratio = maxLength / length;
	float newX = x * ratio;
	float newY = y * ratio;
	float newZ = z * ratio;
	return Vec3(newX, newY, newZ);
}


Vec3 const Vec3::GetNormalized() const
{
	if (GetLengthSquared() == 1.f) return *this;
	if (GetLengthSquared() == 0.f) return Vec3::ZERO;

	float length = GetLength();
	float ratio = 1.f / length;
	float newX = x * ratio;
	float newY = y * ratio;
	float newZ = z * ratio;
	return Vec3(newX, newY, newZ);
}


Vec3 const Vec3::GetReflected(Vec3 const& impactSurfaceNormal) const
{
	Vec3 projectedLength = DotProduct3D(*this, impactSurfaceNormal) * impactSurfaceNormal;
	return *this - (2.f * projectedLength);
}


void Vec3::SetLength(float newLength)
{
	float length = GetLength();
	if (length == 0.f) return;
	float ratio = newLength / length;
	*this *= ratio;
}


void Vec3::SetFromText(std::string const& text)
{
	Strings tokens = SplitStringOnDelimiter(text, ',');

	if (tokens.size() == 3)
	{
		x = static_cast<float>(atof(tokens[0].c_str()));
		y = static_cast<float>(atof(tokens[1].c_str()));
		z = static_cast<float>(atof(tokens[2].c_str()));
	}
	else
	{
		ERROR_AND_DIE("Vec3 set from text input incorrect.");
	}
}


Vec3 const Vec3::operator+(Vec3 const& vecToAdd) const
{
	return Vec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}


Vec3 const Vec3::operator-(Vec3 const& vecToSubtract) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


Vec3 const Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}


Vec3 const Vec3::operator*(float uniformScale) const
{
	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}


Vec3 const Vec3::operator*(Vec3 nonUniformScale) const
{
	return Vec3(x * nonUniformScale.x, y * nonUniformScale.y, z * nonUniformScale.z);
}


Vec3 const Vec3::operator/(float inverseScale) const
{
	return Vec3(x / inverseScale, y / inverseScale, z / inverseScale);
}


//Operators (mutate)
void Vec3::operator+=(Vec3 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


void Vec3::operator-=(Vec3 const& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


void Vec3::operator*=(float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


void Vec3::operator*=(Vec3 nonUniformScale)
{
	x *= nonUniformScale.x;
	y *= nonUniformScale.y;
	z *= nonUniformScale.z;
}


void Vec3::operator/=(float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}


void Vec3::operator=(Vec3 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//standalone "friend" function
Vec3 const operator*(float uniformScale, Vec3 const& vecToScale)
{
	return Vec3(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z);
}


bool Vec3::operator==(Vec3 const& compare) const
{
	return x == compare.x && y == compare.y && z == compare.z;
}


bool Vec3::operator!=(Vec3 const& compare) const
{
	return x != compare.x || y != compare.y || z != compare.z;
}


