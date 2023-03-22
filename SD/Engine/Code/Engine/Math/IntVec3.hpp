#pragma once
#include <string>

struct IntVec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

	static IntVec3 const ZERO;
	static IntVec3 const ONE;

	static IntVec3 const STEP_NORTH;
	static IntVec3 const STEP_SOUTH;
	static IntVec3 const STEP_WEST;
	static IntVec3 const STEP_EAST;
	static IntVec3 const STEP_UP;
	static IntVec3 const STEP_DOWN;

	static IntVec3 const STEP_NW;
	static IntVec3 const STEP_NE;
	static IntVec3 const STEP_SW;
	static IntVec3 const STEP_SE;

public:
	~IntVec3() {}
	IntVec3() {}
	IntVec3(IntVec3 const& copyFrom);
	explicit IntVec3(int initialX, int initialY, int initialZ);

	float GetLength() const;
	int GetLengthSquared() const;
	int GetTaxicabLength() const;
	//float GetOrientationRadians() const;
	//float GetOrientationDegrees() const;
	//IntVec3 const GetRotated90DegreesAroundZ() const;
	//IntVec3 const GetRotatedMinus90DegreesAroundZ() const;

	//void Rotate90DegreesAroundZ();
	//void RotateMinus90DegreesAroundZ();
	void SetFromText(std::string const& text);

	// Operators (const)
	bool		operator==(const IntVec3& compare) const;		// vec3 == vec3
	bool		operator!=(const IntVec3& compare) const;		// vec3 != vec3
	IntVec3 const 	operator+(const IntVec3& vecToAdd) const;		// vec3 + vec3
	IntVec3 const 	operator-(const IntVec3& vecToSubtract) const;	// vec3 - vec3
	IntVec3 const	operator-() const;								// -vec3, i.e. "unary negation"

																	// Operators (self-mutating / non-const)
	void		operator+=(const IntVec3& vecToAdd);				// vec3 += vec3
	void		operator-=(const IntVec3& vecToSubtract);		// vec3 -= vec3
	void		operator=(const IntVec3& copyFrom);				// vec3 = vec3
};
