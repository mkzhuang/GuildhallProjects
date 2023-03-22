#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Image.hpp"

BitmapFont::BitmapFont(char const* fontFilePath, Texture const& fontTexture, bool hasMetadata)
	: m_fontFilePathName(fontFilePath)
	, m_fontTexture(fontTexture)
	, m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16, 16))
	, m_hasMetadata(hasMetadata)
{
	m_glyphData.resize(256);

	for (int index = 0; index < (int)m_glyphData.size(); index++)
	{
		m_glyphData[index].glyphIndex = index;
	}

	if (!m_hasMetadata)
	{
		CreateFontWithoutMetadata();
	}
	else
	{
		CreateFontWithMetadata();
	}
}


Texture const& BitmapFont::GetTexture() const
{
	return m_fontTexture;
}


void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect)
{
	float currentWidth = 0.f;
	if (!m_hasMetadata)
	{
		for (int letterIndex = 0; letterIndex < int(text.size()); letterIndex++)
		{
			unsigned char letterCharacter = text[letterIndex];
			float glyphAspect = GetGlyphAspect(letterCharacter);
			float cellWidth = cellHeight * glyphAspect * cellAspect;
			Vec2 letterMin = textMins + Vec2(currentWidth, 0.f);
			Vec2 letterMax = letterMin + Vec2(cellWidth, cellHeight);
			currentWidth += cellWidth;
			AABB2 bounds(letterMin, letterMax);
			AABB2 const& glyphUVs = m_glyphData[letterCharacter].glphyUvs;
			AddVertsForAABB2D(vertexArray, bounds, tint, glyphUVs.m_mins, glyphUVs.m_maxs);
		}
	}
	else
	{
		float fontRatio = cellHeight / m_lineHeight;
		for (int letterIndex = 0; letterIndex < int(text.size()); letterIndex++)
		{
			unsigned char letterCharacter = text[letterIndex];

			float glyphWidth = m_glyphData[letterCharacter].glyphXAdvance;
			if (letterIndex >= 1)
			{
				for (KerningPair const& p : m_kerningPairs)
				{
					unsigned char prevLetterCharacter = text[(size_t)letterIndex - 1];
					if (p.firstIndex == prevLetterCharacter && p.secondIndex == letterCharacter)
					{
						glyphWidth += p.amount;
						break;
					}
				}
			}

			float cellWidth = glyphWidth * fontRatio * cellAspect;
			float characterHeight = m_glyphData[letterCharacter].glyphBounds.GetDimensions().y * fontRatio * cellAspect;
			Vec2 letterMin = textMins + Vec2(currentWidth, cellHeight * (m_baseHeight / m_lineHeight) - characterHeight);
			Vec2 letterMax = letterMin + Vec2(cellWidth, characterHeight);
			currentWidth += cellWidth;
			AABB2 bounds(letterMin, letterMax);
			bounds.Translate(m_glyphData[letterCharacter].glyphOffset * fontRatio * cellAspect);
			AABB2 const& glyphUVs = m_glyphData[letterCharacter].glphyUvs;
			AddVertsForAABB2D(vertexArray, bounds, tint, glyphUVs.m_mins, glyphUVs.m_maxs);
		}
	}
}


void BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, 
	Rgba8 const& tint, float cellAspect, Vec2 const& alignment, TextBoxMode mode, int maxGlyphsToDraw)
{

	//calculate textbox height and width
	Strings strings = SplitStringOnDelimiter(text, '\n');
	float textBoxHeight = cellHeight * static_cast<float>(strings.size());
	int longestLine = INT_MIN;
	int totalCharacters = 0;
	for (int lineIndex = 0; lineIndex < int(strings.size()); lineIndex++)
	{
		int lineSize = static_cast<int>(strings[lineIndex].size());
		totalCharacters += lineSize;
		if (lineSize > longestLine)
		{
			longestLine = lineSize;
		}
	}
	float textBoxWidth = GetTextWidth(cellHeight, text, cellAspect);
	
	//shrink text if mode is shrink to fit
	if (mode == TextBoxMode::SHRINK_TO_FIT)
	{
		Vec2 boxDimensions = box.GetDimensions();
		float widthRatio = textBoxWidth / boxDimensions.x;
		float heightRatio = textBoxHeight / boxDimensions.y;
		
		if (widthRatio > 1.f || heightRatio > 1.f)
		{
			float scale = 0.f;
			if (widthRatio > heightRatio)
			{
				scale = 1.f / widthRatio;
			}
			else
			{
				scale = 1.f / heightRatio;
			}
			textBoxHeight *= scale;
			textBoxWidth *= scale;
			cellHeight *= scale;
		}
	}

	//align text inside the bounding box
	AABB2 fitBox(Vec2::ZERO, Vec2(textBoxWidth, textBoxHeight));
	box.AlignBoxWithin(fitBox, alignment);

	int remainingGlyphsToDraw = maxGlyphsToDraw;
	for (int lineIndex = 0; lineIndex < int(strings.size()); lineIndex++)
	{
		std::string line = strings[lineIndex];
		int lineSize = static_cast<int>(line.size());
		if (remainingGlyphsToDraw < lineSize)
		{
			line.resize(remainingGlyphsToDraw);
		}
		remainingGlyphsToDraw -= lineSize;

		//align each line within its line box
		Vec2 lineMins = fitBox.m_mins + Vec2(0.f, static_cast<float>(strings.size() - lineIndex - 1) * cellHeight);
		AABB2 lineBoundBox(lineMins, lineMins + Vec2(textBoxWidth, cellHeight));
		AABB2 lineFitBox(Vec2::ZERO, Vec2(GetTextWidth(cellHeight, line, cellAspect), cellHeight));
		lineBoundBox.AlignBoxWithin(lineFitBox, alignment);
		AddVertsForText2D(vertexArray, lineFitBox.m_mins, cellHeight, line, tint, cellAspect);

		if (remainingGlyphsToDraw <= 0)
		{
			break;
		}
	}
}


float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspect)
{
	float totalWidth = 0.f;
	if (!m_hasMetadata)
	{
		for (int letterIndex = 0; letterIndex < int(text.size()); letterIndex++)
		{
			unsigned char letterCharacter = text[letterIndex];
			float glyphAspect = GetGlyphAspect(letterCharacter);
			float cellWidth = cellHeight * glyphAspect * cellAspect;
			totalWidth += cellWidth;
		}
	}
	else
	{
		float fontRatio = cellHeight / m_fontSize;
		for (int letterIndex = 0; letterIndex < int(text.size()); letterIndex++)
		{
			unsigned char letterCharacter = text[letterIndex];

			float glyphWidth = m_glyphData[letterCharacter].glyphXAdvance;
			if (letterIndex >= 1)
			{
				for (KerningPair const& p : m_kerningPairs)
				{
					unsigned char prevLetterCharacter = text[(size_t)letterIndex - 1];
					if (p.firstIndex == prevLetterCharacter && p.secondIndex == letterCharacter)
					{
						glyphWidth += p.amount;
						break;
					}
				}
			}

			float cellWidth = glyphWidth * fontRatio * cellAspect;
			totalWidth += cellWidth;
		}
	}

	return totalWidth;
}


void BitmapFont::CreateFontWithoutMetadata()
{
	Image image(m_fontFilePathName.c_str());
	IntVec2 imageDimensions = image.GetDimensions();
	IntVec2 gridSize = m_fontGlyphsSpriteSheet.GetGridSize();
	int dimensionXPerCell = imageDimensions.x / gridSize.x;
	int dimensionYPerCell = imageDimensions.y / gridSize.y;
	for (int j = 8; j < 14; j++)
	{
		for (int i = 0; i < 16; i++)
		{
			int offset = -1;
			for (int column = 0; column < dimensionXPerCell / 2; column++)
			{
				for (int row = 0; row < dimensionYPerCell; row++)
				{
					if (offset == -1)
					{
						IntVec2 leftTexelPosition(i * dimensionXPerCell + column, j * dimensionYPerCell + row);
						Rgba8 leftTexelColor = image.GetTexelColor(leftTexelPosition);
						if (leftTexelColor.a > 0)
						{
							offset = column;
							break;
						}
					}
					if (offset == -1)
					{
						IntVec2 rightTexelPosition((i + 1) * dimensionXPerCell - column - 1, j * dimensionYPerCell + row);
						Rgba8 rightTexelColor = image.GetTexelColor(rightTexelPosition);
						if (rightTexelColor.a > 0)
						{
							offset = column;
							break;
						}
					}
				}
				if (offset != -1)
				{
					break;
				}
			}
			unsigned char glyphUnicode = (15 - (unsigned char)j) * 16 + (unsigned char)i;

			if (offset != -1)
			{
				float ratio = ((float)dimensionXPerCell - 2.f * (float)offset) / (float)dimensionXPerCell;
				ratio += 0.05f;
				if ((glyphUnicode >= 48 && glyphUnicode <= 57) || (glyphUnicode >= 65 && glyphUnicode <= 90) || (glyphUnicode >= 97 && glyphUnicode <= 122))
				{

				}
				else
				{
					ratio = Clamp(ratio, 0.65f, 1.f);
				}

				m_glyphData[glyphUnicode].glyphAspect = ratio;
			}
			else
			{
				m_glyphData[glyphUnicode].glyphAspect = 1.f;
			}
		}
	}

	m_glyphData[' '].glyphAspect = 0.5f;

	for (int glyphIndex = 32; glyphIndex < 256; glyphIndex++)
	{
		Vec2 uvMins, uvMaxs;
		m_fontGlyphsSpriteSheet.GetSpriteUVs(uvMins, uvMaxs, glyphIndex);
		float uDiff = uvMaxs.x - uvMins.x;
		float uOffset = 0.5f * (uDiff - uDiff * m_glyphData[glyphIndex].glyphAspect);
		uvMins.x += uOffset;
		uvMaxs.x -= uOffset;
		m_glyphData[glyphIndex].glphyUvs = AABB2(uvMins, uvMaxs);
	}
}


void BitmapFont::CreateFontWithMetadata()
{
	
	Image image(m_fontTexture.GetImageFilePath().c_str());
	IntVec2 imageDimensions = image.GetDimensions();
	AABB2 imageBounds(Vec2::ZERO, Vec2(imageDimensions));

	XmlDocument doc;
	doc.LoadFile(m_fontFilePathName.c_str());
	XmlElement const* root = doc.RootElement();

	XmlElement const* child = root->FirstChildElement();

	while (child)
	{
		if (strcmp(child->Name(), "info") == 0)
		{
			m_fontSize = fabsf(ParseXmlAttribute(*child, "size", 0.f));
		}
		if (strcmp(child->Name(), "common") == 0)
		{
			m_lineHeight = fabsf(ParseXmlAttribute(*child, "lineHeight", 0.f));
			m_baseHeight = fabsf(ParseXmlAttribute(*child, "base", 0.f));
		}
		if (strcmp(child->Name(), "chars") == 0)
		{
			XmlElement const* glyph = child->FirstChildElement();
			while (glyph)
			{
				int index = ParseXmlAttribute(*glyph, "id", 0);
				float x = ParseXmlAttribute(*glyph, "x", 0.f);
				float y = imageDimensions.y - ParseXmlAttribute(*glyph, "y", 0.f);
				float w = ParseXmlAttribute(*glyph, "width", 0.f);
				float h = ParseXmlAttribute(*glyph, "height", 0.f);

				Vec2 mins(x, y);
				Vec2 maxs = mins + Vec2(w, -h);
				Vec2 uvMins = imageBounds.GetUVForPoint(Vec2(mins.x, maxs.y));
				Vec2 uvMaxs = imageBounds.GetUVForPoint(Vec2(maxs.x, mins.y));
				m_glyphData[index].glyphBounds = AABB2(Vec2(mins.x, maxs.y), Vec2(maxs.x, mins.y));
				m_glyphData[index].glphyUvs = AABB2(uvMins, uvMaxs);

				float xoffset = ParseXmlAttribute(*glyph, "xoffset", 0.f);
				float yoffset = ParseXmlAttribute(*glyph, "yoffset", 0.f);
				Vec2 offset(xoffset, -yoffset);
				m_glyphData[index].glyphOffset = offset;

				float xAdvance = ParseXmlAttribute(*glyph, "xadvance", 0.f);
				m_glyphData[index].glyphXAdvance = xAdvance;

				glyph = glyph->NextSiblingElement();
			}
		}

		if (strcmp(child->Name(), "kernings") == 0)
		{
			XmlElement const* kerning = child->FirstChildElement();
			while (kerning)
			{
				int first = ParseXmlAttribute(*kerning, "first", 0);
				int second = ParseXmlAttribute(*kerning, "second", 0);
				float amount = ParseXmlAttribute(*kerning, "amount", 0.f);
				
				m_kerningPairs.emplace_back(first, second, amount);

				kerning = kerning->NextSiblingElement();
			}
		}

		child = child->NextSiblingElement();
	}
}


float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	return m_glyphData[glyphUnicode].glyphAspect;
}


