#pragma once

#include <string>

struct FloatRange
{
public:
	float m_min = 0.f;
	float m_max = 0.f;

	static FloatRange const ZERO;
	static FloatRange const ONE;
	static FloatRange const ZERO_TO_ONE;

public:
	FloatRange() {}
	~FloatRange() {}
	FloatRange(FloatRange const& copy);
	explicit FloatRange(float min, float max);

	bool IsOnRange(float point) const;
	bool IsOverlappingWith(FloatRange const& range) const;
	FloatRange const GetOverlappingRangeWith(FloatRange const& range) const;
	void SetFromText(std::string const& text);

	bool operator==(const FloatRange& compare) const;
	bool operator!=(const FloatRange& compare) const;

	void operator=(const FloatRange& copy);
};