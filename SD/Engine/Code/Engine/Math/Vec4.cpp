#include "Engine/Math/Vec4.hpp"

Vec4::Vec4(Vec4 const& copyFrom)
	: x(copyFrom.x)
	, y(copyFrom.y)
	, z(copyFrom.z)
	, w(copyFrom.w)
{

}


Vec4::Vec4(Vec3 const& pos)
	: x(pos.x)
	, y(pos.y)
	, z(pos.z)
	, w(0.f)
{

}


Vec4::Vec4(float initX, float initY, float initZ, float initW)
	: x(initX)
	, y(initY)
	, z(initZ)
	, w(initW)
{

}

Vec4 const Vec4::ZERO(0.f, 0.f, 0.f, 0.f);


bool Vec4::operator==(Vec4 const& compare) const
{
	return x == compare.x && y == compare.y && z == compare.z && w == compare.w;
}


bool Vec4::operator!=(Vec4 const& compare) const
{
	return x != compare.x || y != compare.y || z != compare.z || w != compare.w;
}


Vec4 const Vec4::operator+(Vec4 const& vecToAdd) const
{
	return Vec4(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z, w + vecToAdd.w);
}


Vec4 const Vec4::operator-(Vec4 const& vecToSubtract) const
{
	return Vec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}


Vec4 const Vec4::operator*(float uniformScale) const
{
	return Vec4(x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale);
}


Vec4 const Vec4::operator/(float inverseScale) const
{
	return Vec4(x / inverseScale, y / inverseScale, z / inverseScale, w / inverseScale);
}


Vec4 const Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);
}


void Vec4::operator+=(Vec4 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}


void Vec4::operator-=(Vec4 const& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}


void Vec4::operator*=(float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}


void Vec4::operator/=(float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
	w /= uniformDivisor;
}


void Vec4::operator=(Vec4 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}


