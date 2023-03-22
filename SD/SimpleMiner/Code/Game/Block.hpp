#pragma once
#include <stdint.h>

constexpr uint8_t BLOCK_BIT_IS_SKY			= 0b00000001;
constexpr uint8_t BLOCK_BIT_IS_LIGHT_DIRTY	= 0b00000010;
constexpr uint8_t BLOCK_BIT_IS_FULL_OPAQUE	= 0b00000100;

struct Block
{
public:

	bool IsBlockSky() const;
	bool IsLightDirty() const;
	bool IsBlockOpaque() const;
	int GetIndoorLightInfluence() const;
	int GetOutdoorLightInfluence() const;

	void SetBlockSky();
	void SetBlockNotSky();
	void SetLightDirty();
	void SetLightNotDirty();
	void SetBlockOpaque();
	void SetBlockNotOpaque();
	void SetIndoorLightInfluence(int indoorLightInfluence);
	void SetOutdoorLightInfluence(int outdoorLightInfluence);

public:
	uint8_t m_type = 0;
	uint8_t m_lightInfluences = 0;
	uint8_t m_bitFlags = 0;
};


