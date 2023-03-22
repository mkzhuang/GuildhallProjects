#pragma once
#include <string>

struct IntVec2
{
public:
	int x = 0;
	int y = 0;

	static IntVec2 const ZERO;
	static IntVec2 const ONE;

	static IntVec2 const STEP_NORTH;
	static IntVec2 const STEP_SOUTH;
	static IntVec2 const STEP_WEST;
	static IntVec2 const STEP_EAST;

	static IntVec2 const STEP_NW;
	static IntVec2 const STEP_NE;
	static IntVec2 const STEP_SW;
	static IntVec2 const STEP_SE;

public:
	~IntVec2() {}
	IntVec2() {}
	IntVec2(IntVec2 const& copyFrom);
	explicit IntVec2(int initialX, int initialY);

	float GetLength() const;
	int GetLengthSquared() const;
	int GetTaxicabLength() const;
	float GetOrientationRadians() const;
	float GetOrientationDegrees() const;
	IntVec2 const GetRotated90Degrees() const;
	IntVec2 const GetRotatedMinus90Degrees() const;

	void Rotate90Degrees();
	void RotateMinus90Degrees();
	void SetFromText(std::string const& text);

	// Operators (const)
	bool		operator==(const IntVec2& compare) const;		// vec2 == vec2
	bool		operator!=(const IntVec2& compare) const;		// vec2 != vec2
	IntVec2 const 	operator+(const IntVec2& vecToAdd) const;		// vec2 + vec2
	IntVec2 const 	operator-(const IntVec2& vecToSubtract) const;	// vec2 - vec2
	IntVec2 const	operator-() const;								// -vec2, i.e. "unary negation"

	// Operators (self-mutating / non-const)
	void		operator+=(const IntVec2& vecToAdd);				// vec2 += vec2
	void		operator-=(const IntVec2& vecToSubtract);		// vec2 -= vec2
	void		operator=(const IntVec2& copyFrom);				// vec2 = vec2
};
