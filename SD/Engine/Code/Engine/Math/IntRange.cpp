#include "Engine/Math/IntRange.hpp"

IntRange::IntRange(IntRange const& copy)
	: m_min(copy.m_min)
	, m_max(copy.m_max)
{

}


IntRange::IntRange(int min, int max)
	: m_min(min)
	, m_max(max)
{

}


IntRange const IntRange::ZERO = IntRange(0, 0);
IntRange const IntRange::ONE = IntRange(1, 1);
IntRange const IntRange::ZERO_TO_ONE = IntRange(0, 1);


bool IntRange::IsOnRange(int point)
{
	return point > m_min && point < m_max;
}


bool IntRange::IsOverlappingWith(IntRange range)
{
	int rangeMin = range.m_min;
	if (IsOnRange(rangeMin)) return true;

	int rangeMax = range.m_max;
	if (IsOnRange(rangeMax)) return true;

	return false;
}


bool IntRange::operator==(const IntRange& compare) const
{
	return m_min == compare.m_min && m_max == compare.m_max;
}


bool IntRange::operator!=(const IntRange& compare) const
{
	return m_min != compare.m_min || m_max != compare.m_max;
}


void IntRange::operator=(const IntRange& copy)
{
	m_min = copy.m_min;
	m_max = copy.m_max;
}


