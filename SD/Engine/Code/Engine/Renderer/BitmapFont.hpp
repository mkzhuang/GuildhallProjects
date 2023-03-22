#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include <vector>

struct GlyphData
{
	int glyphIndex = 0;
	float glyphAspect = 1.f;
	AABB2 glyphBounds = AABB2::ZERO_TO_ONE;
	AABB2 glphyUvs = AABB2::ZERO_TO_ONE;
	Vec2 glyphOffset = Vec2::ZERO;
	float glyphXAdvance = 0.f;
};

struct KerningPair
{
	KerningPair(int first, int second, float kerningAmount)
		: firstIndex(first)
		, secondIndex(second)
		, amount(kerningAmount)
	{
	}

	int firstIndex = 0;
	int secondIndex = 0;
	float amount = 0.f;
};


enum class TextBoxMode
{
	SHRINK_TO_FIT,
	OVERRUN
};

class BitmapFont
{
	friend class Renderer;

private:
	BitmapFont(char const* fontFilePath, Texture const& fontTexture, bool hasMetaData = false);

public:
	Texture const& GetTexture() const;

	void AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins,
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f);
	void AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight,
		std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f,
		Vec2 const& alignment = Vec2(.5f, .5f), TextBoxMode mode = TextBoxMode::SHRINK_TO_FIT, int maxGlyphsToDraw = INT_MAX);
	float GetTextWidth(float cellHeight, std::string const& text, float cellAspect = 1.f);

protected:
	void CreateFontWithoutMetadata();
	void CreateFontWithMetadata();
	float GetGlyphAspect(int glyphUnicode) const;

protected:
	std::string	m_fontFilePathName;
	SpriteSheet m_fontGlyphsSpriteSheet;
	Texture const& m_fontTexture;
	std::vector<GlyphData> m_glyphData;
	std::vector<KerningPair> m_kerningPairs;
	float m_fontSize = 0.f;
	float m_lineHeight = 0.f;
	float m_baseHeight = 0.f;
	bool m_hasMetadata = false;
};