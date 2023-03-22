#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec3.hpp"

struct Vec3
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	static Vec3 const ZERO;
	static Vec3 const ONE;
	static Vec3 const UP;
	static Vec3 const DOWN;
	static Vec3 const FORWARD;
	static Vec3 const BACKWARD;
	static Vec3 const LEFT;
	static Vec3 const RIGHT;
	static Vec3 const EAST;
	static Vec3 const WEST;
	static Vec3 const NORTH;
	static Vec3 const SOUTH;

public:
	Vec3() = default;
	//~Vec3() {}
	Vec3(Vec3 const& copyFrom);
	explicit Vec3(Vec2 const& pos);
	explicit Vec3(float initialX, float initialY, float initialZ);
	explicit Vec3(IntVec3 from);

	//Accessors
	float GetLength() const;
	float GetLengthXY() const;
	float GetLengthSquared() const;
	float GetLengthXYSquared() const;
	float GetAngleAboutZRadians() const;
	float GetAngleAboutZDegrees() const;
	Vec3 const GetRotatedAboutZRadians(float deltaRadians) const;
	Vec3 const GetRotatedAboutZDegrees(float deltaDegrees) const;
	Vec3 const GetClamped(float maxLength) const;
	Vec3 const GetNormalized() const;
	Vec3 const GetReflected(Vec3 const& impactSurfaceNormal) const;
	void SetLength(float newLength);
	void SetFromText(std::string const& text);

	//Operators (const)
	bool operator==(Vec3 const& compare) const;
	bool operator!=(Vec3 const& compare) const;
	Vec3 const operator+(Vec3 const& vecToAdd) const;
	Vec3 const operator-(Vec3 const& vecToSubtract) const;
	Vec3 const operator*(float uniformScale) const;
	Vec3 const operator*(Vec3 nonUniformScale) const;
	Vec3 const operator/(float inverseScale) const;
	Vec3 const operator-() const;


	//Operators (mutate)
	void operator+=(Vec3 const& vecToAdd);
	void operator-=(Vec3 const& vecToSubtract);
	void operator*=(float uniformScale);
	void operator*=(Vec3 nonUniformScale);
	void operator/=(float uniformDivisor);
	void operator=(Vec3 const& copyFrom);

	//standalone "friend" function
	friend Vec3 const operator*(float uniformScale, Vec3 const& vecToScale);
 };