#pragma once
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "Engine/core/EngineCommon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/EulerAngles.hpp"

#include <string>

struct Vec2;
struct IntVec2;
struct Rgba8;

typedef tinyxml2::XMLElement XmlElement;

int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue);
char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue);
bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue);
float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue);
Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue);
Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue);
IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue);
AABB2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, AABB2 const& defaultValue);
Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue);
IntVec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec3 const& defaultValue);
FloatRange ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange const& defaultValue);
EulerAngles ParseXmlAttribute(XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue);
std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue);
Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues);
std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, char const* defaultValue);


