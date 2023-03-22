#pragma once
#include "Engine/Math/Vec3.hpp"

struct Vec4
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	static Vec4 const ZERO;

public:
	Vec4() = default;
	Vec4(Vec4 const& copyFrom);
	explicit Vec4(Vec3 const& pos);
	explicit Vec4(float initX, float initY, float initZ, float initW);

	//Operators (const)
	bool operator==(Vec4 const& compare) const;
	bool operator!=(Vec4 const& compare) const;
	Vec4 const operator+(Vec4 const& vecToAdd) const;
	Vec4 const operator-(Vec4 const& vecToSubtract) const;
	Vec4 const operator*(float uniformScale) const;
	Vec4 const operator/(float inverseScale) const;
	Vec4 const operator-() const;


	//Operators (mutate)
	void operator+=(Vec4 const& vecToAdd);
	void operator-=(Vec4 const& vecToSubtract);
	void operator*=(float uniformScale);
	void operator/=(float uniformDivisor);
	void operator=(Vec4 const& copyFrom);
};