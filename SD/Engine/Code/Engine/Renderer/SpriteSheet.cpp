#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"

SpriteDefinition::SpriteDefinition(SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs)
	: m_spriteSheet(spriteSheet)
	, m_spriteIndex(spriteIndex)
	, m_uvAtMins(uvAtMins)
	, m_uvAtMaxs(uvAtMaxs)
{
}


void SpriteDefinition::GetUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const
{
	out_uvAtMins = m_uvAtMins;
	out_uvAtMaxs = m_uvAtMaxs;
}


AABB2 SpriteDefinition::GetUVs() const
{
	return AABB2(m_uvAtMins, m_uvAtMaxs);
}


SpriteSheet const& SpriteDefinition::GetSpriteSheet() const
{
	return m_spriteSheet;
}


Texture const& SpriteDefinition::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}


float SpriteDefinition::GetAspect() const
{
	Vec2 uvSize = m_uvAtMaxs - m_uvAtMins;
	IntVec2 textureDimensions = m_spriteSheet.GetTexture().GetDimensions();
	float texelsWidth = uvSize.x / static_cast<float>(textureDimensions.x);
	float texelsHeight = uvSize.y / static_cast<float>(textureDimensions.y);
	return texelsWidth / texelsHeight;
}


SpriteSheet::SpriteSheet(Texture const& texture, IntVec2 const& simpleGridLayout)
	: m_texture(&texture)
	, m_gridSize(simpleGridLayout)
{
	float gridHeight = 1.f / simpleGridLayout.y;
	float gridWidth = 1.f / simpleGridLayout.x;
	m_spriteDefs.reserve(simpleGridLayout.x * simpleGridLayout.y);

	IntVec2 textureDimensions = texture.GetDimensions();
	float uCorrection = 1.f / (static_cast<float>(textureDimensions.x) * 100.f);
	float vCorrection = 1.f / (static_cast<float>(textureDimensions.y) * 100.f); 
	Vec2 uvCorrection(uCorrection, vCorrection);

	for (int gridRow = 0; gridRow < simpleGridLayout.y; gridRow++)
	{
		for (int gridColumn = 0; gridColumn < simpleGridLayout.x; gridColumn++)
		{
			int index = gridColumn + gridRow * simpleGridLayout.y;
			Vec2 uvAtMins(gridWidth * static_cast<float>(gridColumn), 1.f - gridHeight * static_cast<float>(gridRow + 1));
			Vec2 uvAtMaxs(uvAtMins.x + gridWidth, uvAtMins.y + gridHeight);
			m_spriteDefs.emplace_back(*this, index, uvAtMins + uvCorrection, uvAtMaxs - uvCorrection);
		}
	}
}


Texture const& SpriteSheet::GetTexture() const
{
	return *m_texture;
}


int SpriteSheet::GetNumSprites() const
{
	return int(m_spriteDefs.size());
}


SpriteDefinition const& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}


void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	m_spriteDefs[spriteIndex].GetUVs(out_uvAtMins, out_uvAtMaxs);
}


AABB2 const SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex].GetUVs();
}


AABB2 const SpriteSheet::GetSpriteUVs(IntVec2 spriteLocation) const
{
	int index = spriteLocation.x + spriteLocation.y * m_gridSize.x;
	return GetSpriteUVs(index);
}


IntVec2 const& SpriteSheet::GetGridSize() const
{
	return m_gridSize;
}


