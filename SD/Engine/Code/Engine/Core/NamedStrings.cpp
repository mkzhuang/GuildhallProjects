#include "NamedStrings.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	const XmlAttribute* attribute = element.FirstAttribute();
	while (attribute != nullptr)
	{
		SetValue(attribute->Name(), attribute->Value());
		attribute = attribute->Next();
	}
}


void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}


std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iter = m_keyValuePairs.find(keyName);

	if (iter == m_keyValuePairs.end()) return defaultValue;

	return iter->second;
}


bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iter = m_keyValuePairs.find(keyName);

	if (iter == m_keyValuePairs.end()) return defaultValue;

	if (iter->second == "true")
	{
		return true;
	}
	else if (iter->second == "false")
	{
		return false;
	}
	else
	{
		ERROR_AND_DIE("bool value should be true or false.");
	}
}


int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iter = m_keyValuePairs.find(keyName);

	if (iter == m_keyValuePairs.end()) return defaultValue;

	return atoi(iter->second.c_str());
}


float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iter = m_keyValuePairs.find(keyName);

	if (iter == m_keyValuePairs.end()) return defaultValue;

	return static_cast<float>(atof(iter->second.c_str()));
}


std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	return GetValue(keyName, std::string(defaultValue));
}


Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iter = m_keyValuePairs.find(keyName);

	if (iter == m_keyValuePairs.end()) return defaultValue;

	Rgba8 color;
	color.SetFromText(iter->second);
	return color;
}


Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iter = m_keyValuePairs.find(keyName);

	if (iter == m_keyValuePairs.end()) return defaultValue;

	Vec2 vec2;
	vec2.SetFromText(iter->second);
	return vec2;
}


IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iter = m_keyValuePairs.find(keyName);

	if (iter == m_keyValuePairs.end()) return defaultValue;

	IntVec2 intVec2;
	intVec2.SetFromText(iter->second);
	return intVec2;
}


