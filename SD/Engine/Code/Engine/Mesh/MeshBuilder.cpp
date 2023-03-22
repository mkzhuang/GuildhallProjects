#include "Engine/Mesh/MeshBuilder.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <stdio.h>

MeshBuilder::MeshBuilder()
{

}


MeshBuilder::~MeshBuilder()
{

}


bool MeshBuilder::LoadFromConfig(MeshBuilderConfig config)
{
	return ParseDataFromOBJFile(config.m_modelPath, config);
}


bool MeshBuilder::ParseDataFromOBJFile(const std::string& filename, MeshBuilderConfig config)
{
	m_texturePath = config.m_texturePath;
	m_vertices.clear();
	std::string fileString;
	int result = FileReadToString(fileString, filename);
	if (result != 0) return false;
	Strings lines = SplitStringOnDelimiter(fileString, '\n');

	std::vector<Vec3> positions;
	std::vector<Vec2> uvs;
	std::vector<Vec3> normals;

	int verticesEstimateCounts = static_cast<int>(0.1f * lines.size());
	positions.reserve(verticesEstimateCounts);
	uvs.reserve(verticesEstimateCounts);
	normals.reserve(verticesEstimateCounts);

	// parsing file
	for (std::string line : lines)
	{
		Strings tokens = SplitStringOnDelimiter(line, ' ');
		std::string firstToken = tokens[0];
		if (firstToken == "v") // add position
		{
			float x = static_cast<float>(atof(tokens[1].c_str()));
			float y = static_cast<float>(atof(tokens[2].c_str()));
			float z = static_cast<float>(atof(tokens[3].c_str()));
			Vec3 iBasis = config.m_transform.GetIBasis3D();
			Vec3 jBasis = config.m_transform.GetJBasis3D();
			Vec3 kBasis = config.m_transform.GetKBasis3D();
			Vec3 translation = config.m_transform.GetTranslation3D();
			Vec3 position = x * iBasis + y * jBasis + z * kBasis + translation;
			position *= config.m_scale;
			positions.push_back(position);
		}
		else if (firstToken == "vt") // add uv
		{
			float u = static_cast<float>(atof(tokens[1].c_str()));
			float v = static_cast<float>(atof(tokens[2].c_str()));
			if (config.m_invertedTextureV)
			{
				v = 1.f - v;
			}
			uvs.emplace_back(u, v);
		}
		else if (firstToken == "vn") // add normal
		{
			float x = static_cast<float>(atof(tokens[1].c_str()));
			float y = static_cast<float>(atof(tokens[2].c_str()));
			float z = static_cast<float>(atof(tokens[3].c_str()));
			normals.emplace_back(x, y, z);
		}
		else if (firstToken == "f") // read face and add vertices and indices
		{
			std::vector<Vertex_PNCU> verticesForFace;
			// read faces
			for (int count = 1; count < (int)tokens.size(); count++)
			{
				Strings indices = SplitStringOnDelimiter(tokens[count], '/');
				if (indices.size() == 1) // v
				{
					int positionIndex = atoi(indices[0].c_str()) - 1;
					verticesForFace.emplace_back(positions[positionIndex], Vec3::ZERO, Rgba8::WHITE, Vec2::ZERO);
				}
				else if (indices.size() == 2) // v/vt
				{
					int positionIndex = atoi(indices[0].c_str()) - 1;
					int uvIndex = atoi(indices[1].c_str()) - 1;
					verticesForFace.emplace_back(positions[positionIndex], Vec3::ZERO, Rgba8::WHITE, uvs[uvIndex]);
				}
				else if (indices.size() == 3) // v/vt/vn
				{
					int positionIndex = atoi(indices[0].c_str()) - 1;
					int uvIndex = atoi(indices[1].c_str()) - 1;
					int normalIndex = atoi(indices[2].c_str()) - 1;
					verticesForFace.emplace_back(positions[positionIndex], normals[normalIndex], Rgba8::WHITE, uvs[uvIndex]);
				}
			}

			if (verticesForFace.size() == 3)
			{
				if (config.m_reversedWinding)
				{
					m_vertices.push_back(verticesForFace[2]);
					m_vertices.push_back(verticesForFace[1]);
					m_vertices.push_back(verticesForFace[0]);
				}
				else
				{
					m_vertices.push_back(verticesForFace[0]);
					m_vertices.push_back(verticesForFace[1]);
					m_vertices.push_back(verticesForFace[2]);
				}
			}
			else if (verticesForFace.size() == 4)
			{
				if (config.m_reversedWinding)
				{
					m_vertices.push_back(verticesForFace[2]);
					m_vertices.push_back(verticesForFace[1]);
					m_vertices.push_back(verticesForFace[0]);
					m_vertices.push_back(verticesForFace[3]);
					m_vertices.push_back(verticesForFace[2]);
					m_vertices.push_back(verticesForFace[0]);
				}
				else
				{
					m_vertices.push_back(verticesForFace[0]);
					m_vertices.push_back(verticesForFace[1]);
					m_vertices.push_back(verticesForFace[2]);
					m_vertices.push_back(verticesForFace[0]);
					m_vertices.push_back(verticesForFace[2]);
					m_vertices.push_back(verticesForFace[3]);
				}
			}
		}
	}
	return m_vertices.size() > 0;
}


bool MeshBuilder::SaveToBinaryFile(const std::string& filename)
{
	std::FILE* file = nullptr;
	fopen_s(&file, filename.c_str(), "wb");
	if (file)
	{
		size_t size = m_vertices.size();
		fwrite(&size, sizeof(size_t), 1, file);
		fwrite(m_vertices.data(), sizeof(Vertex_PNCU), m_vertices.size(), file);
		fclose(file);
		return true;
	}

	return false;
}


bool MeshBuilder::ReadFromBinaryFile(const std::string& filename)
{
	std::FILE* file = nullptr;
	fopen_s(&file, filename.c_str(), "rb");
	if (file)
	{
		size_t size = 0;
		fread(&size, sizeof(size_t), 1, file);
		m_vertices.resize(size);
		fread(m_vertices.data(), sizeof(Vertex_PNCU), size, file);
		fclose(file);
		return true;
	}

	return false;
}


