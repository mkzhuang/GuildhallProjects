#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#define UNUSED(x) (void)(x);

class InputSystem;
class Window;
class Renderer;
class AudioSystem;
class App;
class RandomNumberGenerator;
struct Rgba8;
struct Vec2;

extern bool g_isQuitting;
extern bool g_isDebugging;
extern float g_gameSoundVolume;
extern float g_gameSoundPlaySpeed;

extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudio;
extern App* g_theApp;
extern RandomNumberGenerator RNG;

typedef tinyxml2::XMLDocument XmlDocument;
typedef tinyxml2::XMLElement XmlElement;
typedef tinyxml2::XMLAttribute XmlAttribute;

SoundPlaybackID PlaySound(SoundID sound, bool loop, float volume);
void DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float lineWidth, Rgba8 color);
void DebugDrawRing(Vec2 const& center, float radius, float lineWidth, Rgba8 color);