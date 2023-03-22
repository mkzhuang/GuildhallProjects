#pragma once
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Math/Mat44.hpp"

#include <vector>

struct MeshBuilderConfig
{
	Mat44 m_transform = Mat44();
	float m_scale = 1.f;
	bool m_reversedWinding = false;
	bool m_invertedTextureV = false;
	std::string m_modelPath = "";
	std::string m_texturePath = "";
};

class MeshBuilder
{
public:
	MeshBuilder();
	~MeshBuilder();
	bool LoadFromConfig(MeshBuilderConfig config);
	bool ParseDataFromOBJFile(const std::string& filename, MeshBuilderConfig config);
	bool SaveToBinaryFile(const std::string& filename);
	bool ReadFromBinaryFile(const std::string& filename);

public:
	std::string m_texturePath = "";
	std::vector<Vertex_PNCU> m_vertices;
	//std::vector<unsigned int> m_indices;
};


