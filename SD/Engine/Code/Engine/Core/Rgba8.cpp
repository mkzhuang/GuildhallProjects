#include "Engine/Core/Rgba8.hpp"
#include "EngineCommon.hpp"

Rgba8::Rgba8(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
	: r(red)
	, g(green)
	, b(blue)
	, a(alpha)
{
}


Rgba8 const Rgba8::WHITE(255, 255, 255, 255);
Rgba8 const Rgba8::BLACK(0, 0, 0, 255);
Rgba8 const Rgba8::CLEAR(0, 0, 0, 0);
Rgba8 const Rgba8::GRAY(127, 127, 127, 255);
Rgba8 const Rgba8::RED(255, 0, 0, 255);
Rgba8 const Rgba8::CYAN(0, 255, 255, 255);
Rgba8 const Rgba8::GREEN(0, 255, 0, 255);
Rgba8 const Rgba8::MAGENTA(255, 0, 255, 255);
Rgba8 const Rgba8::BLUE(0, 0, 255, 255);
Rgba8 const Rgba8::YELLOW(255, 255, 0, 255);


void Rgba8::SetFromText(std::string const& text)
{
	Strings tokens = SplitStringOnDelimiter(text, ',');

	if (tokens.size() == 3)
	{
		r = static_cast<unsigned char>(atoi(tokens[0].c_str()));
		g = static_cast<unsigned char>(atoi(tokens[1].c_str()));
		b = static_cast<unsigned char>(atoi(tokens[2].c_str()));
		a = 255;
	} 
	else if (tokens.size() == 4) 
	{ 
		r = static_cast<unsigned char>(atoi(tokens[0].c_str()));
		g = static_cast<unsigned char>(atoi(tokens[1].c_str()));
		b = static_cast<unsigned char>(atoi(tokens[2].c_str()));
		a = static_cast<unsigned char>(atoi(tokens[3].c_str()));
	}
	else
	{
		ERROR_AND_DIE("Rgba8 set from text input incorrect.");
	}
}


void Rgba8::GetAsFloats(float* colorAsFloats)
{
	colorAsFloats[0] = NormalizeByte(r);
	colorAsFloats[1] = NormalizeByte(g);
	colorAsFloats[2] = NormalizeByte(b);
	colorAsFloats[3] = NormalizeByte(a);
}


void Rgba8::ConvertFromHexString(std::string const& colorString)
{
	std::string rString = colorString.substr(0, 2);
	std::string gString = colorString.substr(2, 2);
	std::string bString = colorString.substr(4, 2);
	r = static_cast<unsigned char>(std::stol(rString, nullptr, 16));
	g = static_cast<unsigned char>(std::stol(gString, nullptr, 16));
	b = static_cast<unsigned char>(std::stol(bString, nullptr, 16));
	a = 255;
}


bool Rgba8::operator==(const Rgba8& compare) const
{
	return r == compare.r && g == compare.g && b == compare.b && a == compare.a;
}


bool Rgba8::operator!=(const Rgba8& compare) const
{
	return r != compare.r || g != compare.g || b != compare.b || a != compare.a;
}


void Rgba8::operator=(Rgba8 const& copy)
{
	r = copy.r;
	g = copy.g;
	b = copy.b;
	a = copy.a;
}


Rgba8 InterpolateBetweenColor(Rgba8 const& startColor, Rgba8 const& endColor, float timeFraction)
{
	unsigned char outR = static_cast<unsigned char>(Interpolate(startColor.r, endColor.r, timeFraction));
	unsigned char outG = static_cast<unsigned char>(Interpolate(startColor.g, endColor.g, timeFraction));
	unsigned char outB = static_cast<unsigned char>(Interpolate(startColor.b, endColor.b, timeFraction));
	unsigned char outA = static_cast<unsigned char>(Interpolate(startColor.a, endColor.a, timeFraction));
	return Rgba8(outR, outG, outB, outA);
}


