#pragma once
#include <string>
#include <vector>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/NamedProperties.hpp"

class NamedStrings;
class EventSystem;
class InputSystem;
class DevConsole;
class RemoteConsole;

#define UNUSED(x) (void)(x);

typedef NamedProperties EventArgs;

extern EventSystem* g_theEventSystem;
extern InputSystem* g_theInput;
extern DevConsole* g_theDevConsole;
extern RemoteConsole* g_theRemoteConsole;
extern NamedStrings g_gameConfigBlackboard;

typedef tinyxml2::XMLDocument XmlDocument;
typedef tinyxml2::XMLElement XmlElement;
typedef tinyxml2::XMLAttribute XmlAttribute;

constexpr float SHORT_MIN = -32768.f;
constexpr float SHORT_MAX = 32767.f;
constexpr float UNSIGNED_CHAR_MIN = 0.f;
constexpr float UNSIGNED_CHAR_MAX = 255.f;


