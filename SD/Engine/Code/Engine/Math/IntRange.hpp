#pragma once

struct IntRange
{
public:
	int m_min = 0;
	int m_max = 0;

	static IntRange const ZERO;
	static IntRange const ONE;
	static IntRange const ZERO_TO_ONE;

public:
	IntRange() {}
	~IntRange() {}
	IntRange(IntRange const& copy);
	explicit IntRange(int min, int max);

	bool IsOnRange(int point);
	bool IsOverlappingWith(IntRange range);

	bool operator==(const IntRange& compare) const;
	bool operator!=(const IntRange& compare) const;

	void operator=(const IntRange& copy);
};


