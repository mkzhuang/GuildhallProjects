#include "Game/Block.hpp"

bool Block::IsBlockSky() const
{
	return m_bitFlags & BLOCK_BIT_IS_SKY;
}


bool Block::IsLightDirty() const
{
	return m_bitFlags & BLOCK_BIT_IS_LIGHT_DIRTY;
}


bool Block::IsBlockOpaque() const
{
	return m_bitFlags & BLOCK_BIT_IS_FULL_OPAQUE;
}


int Block::GetIndoorLightInfluence() const
{
	return m_lightInfluences & 0b00001111;
}


int Block::GetOutdoorLightInfluence() const
{
	return (m_lightInfluences >> 4);
}


void Block::SetBlockSky()
{
	m_bitFlags |= BLOCK_BIT_IS_SKY;
	SetBlockNotOpaque();
}


void Block::SetBlockNotSky()
{
	m_bitFlags &= ~BLOCK_BIT_IS_SKY;
}


void Block::SetLightDirty()
{
	m_bitFlags |= BLOCK_BIT_IS_LIGHT_DIRTY;
}


void Block::SetLightNotDirty()
{
	m_bitFlags &= ~BLOCK_BIT_IS_LIGHT_DIRTY;
}


void Block::SetBlockOpaque()
{
	m_bitFlags |= BLOCK_BIT_IS_FULL_OPAQUE;
	SetBlockNotSky();
}


void Block::SetBlockNotOpaque()
{
	m_bitFlags &= ~BLOCK_BIT_IS_FULL_OPAQUE;
}


void Block::SetIndoorLightInfluence(int indoorLightInfluence)
{
	m_lightInfluences &= 0b11110000;
	m_lightInfluences |= indoorLightInfluence;
}


void Block::SetOutdoorLightInfluence(int outdoorLightInfluence)
{
	m_lightInfluences &= 0b00001111;
	m_lightInfluences |= (outdoorLightInfluence << 4);
}


