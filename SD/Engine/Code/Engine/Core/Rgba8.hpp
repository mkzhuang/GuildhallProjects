#pragma once
#include <string>

struct Rgba8
{
public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

public:
	~Rgba8() {}
	Rgba8() {}
	explicit Rgba8(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);

	static Rgba8 const WHITE;
	static Rgba8 const BLACK;
	static Rgba8 const CLEAR;
	static Rgba8 const GRAY;
	static Rgba8 const RED;
	static Rgba8 const CYAN;
	static Rgba8 const GREEN;
	static Rgba8 const MAGENTA;
	static Rgba8 const BLUE;
	static Rgba8 const YELLOW;


	void SetFromText(std::string const& text);
	void GetAsFloats(float* colorAsFloats);
	void ConvertFromHexString(std::string const& colorString);

	bool operator==(const Rgba8& compare) const;
	bool operator!=(const Rgba8& compare) const;
	void operator=(Rgba8 const& copy);
};

Rgba8 InterpolateBetweenColor(Rgba8 const& startColor, Rgba8 const& endColor, float timeFraction);


