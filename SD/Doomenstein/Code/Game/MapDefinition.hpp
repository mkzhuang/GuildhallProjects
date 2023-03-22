#pragma once
#include "Game/GameCommon.hpp"
#include "Game/SpawnInfo.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Vec3.hpp"

#include <string>
#include <vector>

//------------------------------------------------------------------------------------------------
class TileSetDefinition;

//------------------------------------------------------------------------------------------------
class MapDefinition
{
public:
	bool LoadFromXmlElement( const XmlElement& element );

public:
	std::string m_name;
	Image* m_image = nullptr;
	const TileSetDefinition* m_tileSetDefinition = nullptr;
	std::vector<SpawnInfo> m_spawnInfos;

	static void InitializeDefinitions();
	static void ClearDefinitions();
	static const MapDefinition* GetByName( const std::string& name );
	static std::vector<MapDefinition*> s_definitions;
};


