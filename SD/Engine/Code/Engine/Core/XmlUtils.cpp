#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	return element.IntAttribute(attributeName, defaultValue);
}


char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	return *value;
}


bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	return element.BoolAttribute(attributeName, defaultValue);
}


float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	return element.FloatAttribute(attributeName, defaultValue);
}


Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	Rgba8 color;
	color.SetFromText(value);
	return color;
}


Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	Vec2 vec2;
	vec2.SetFromText(value);
	return vec2;
}


IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	IntVec2 intVec2;
	intVec2.SetFromText(value);
	return intVec2;
}


AABB2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, AABB2 const& defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	AABB2 aabb2;
	aabb2.SetFromText(value);
	return aabb2;
}


Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	Vec3 vec3;
	vec3.SetFromText(value);
	return vec3;
}


IntVec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec3 const& defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	IntVec3 intVec3;
	intVec3.SetFromText(value);
	return intVec3;
}


FloatRange ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange const& defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	FloatRange floatRange;
	floatRange.SetFromText(value);
	return floatRange;
}


EulerAngles ParseXmlAttribute(XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	EulerAngles eulerAngles;
	eulerAngles.SetFromText(value);
	return eulerAngles;
}


std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	return value;
}


Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValues;

	return SplitStringOnDelimiter(value, ',');
}


std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const* defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (!value) return defaultValue;

	return value;
}


