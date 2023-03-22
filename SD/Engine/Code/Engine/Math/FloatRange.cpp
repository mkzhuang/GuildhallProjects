#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"



FloatRange::FloatRange(FloatRange const& copy)
	: m_min(copy.m_min)
	, m_max(copy.m_max)
{

}


FloatRange::FloatRange(float min, float max)
	: m_min(min)
	, m_max(max)
{

}


FloatRange const FloatRange::ZERO = FloatRange(0.f, 0.f);
FloatRange const FloatRange::ONE = FloatRange(1.f, 1.f);
FloatRange const FloatRange::ZERO_TO_ONE = FloatRange(0.f, 1.f);


bool FloatRange::IsOnRange(float point) const
{
	return point >= m_min && point <= m_max;
}


bool FloatRange::IsOverlappingWith(FloatRange const& range) const
{
	float rangeMin = range.m_min;
	if(IsOnRange(rangeMin) || range.IsOnRange(m_min)) return true;

	float rangeMax = range.m_max;
	if(IsOnRange(rangeMax) || range.IsOnRange(m_max)) return true;

	return false;
}


FloatRange const FloatRange::GetOverlappingRangeWith(FloatRange const& range) const
{
	if (!IsOverlappingWith(range)) return FloatRange::ZERO;
	float rangeMin = range.m_min >= m_min ? range.m_min : m_min;
	float rangeMax = range.m_max <= m_max ? range.m_max : m_max;
	return FloatRange(rangeMin, rangeMax);
}


void FloatRange::SetFromText(std::string const& text)
{
	Strings tokens = SplitStringOnDelimiter(text, '~');

	if (tokens.size() == 2)
	{
		m_min = static_cast<float>(atof(tokens[0].c_str()));
		m_max = static_cast<float>(atof(tokens[1].c_str()));
	}
	else
	{
		ERROR_AND_DIE("Float range set from text input incorrect.");
	}
}


bool FloatRange::operator==(const FloatRange& compare) const
{
	return m_min == compare.m_min && m_max == compare.m_max;
}


bool FloatRange::operator!=(const FloatRange& compare) const
{
	return m_min != compare.m_min || m_max != compare.m_max;
}


void FloatRange::operator=(const FloatRange& copy)
{
	m_min = copy.m_min;
	m_max = copy.m_max;
}


