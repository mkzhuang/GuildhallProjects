#include "Engine/Mesh/Mesh.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

Mesh::Mesh(MeshBuilder const* meshBuilder, Renderer* renderer, std::string const& name)
{
	m_vertexBuffer = renderer->CreateVertexBuffer(sizeof(Vertex_PNCU), sizeof(Vertex_PNCU));
	//m_indexBuffer = renderer->CreateIndexBuffer(sizeof(unsigned int));
	renderer->CopyCPUToGPU(meshBuilder->m_vertices.data(), sizeof(Vertex_PNCU) * meshBuilder->m_vertices.size(), m_vertexBuffer);
	//renderer->CopyCPUToGPU(meshBuilder->m_indices.data(), sizeof(unsigned int) * meshBuilder->m_indices.size(), m_indexBuffer);
	//m_size = (int)meshBuilder->m_indices.size();
	m_size = (int)meshBuilder->m_vertices.size();
	m_shader = renderer->CreateOrGetShader("Data/Shaders/SpriteLit");
	m_texture = renderer->CreateOrGetTextureFromFile(meshBuilder->m_texturePath.c_str());
	m_orientationMatrix = Mat44();
	m_modelMatrix = Mat44();
	m_name = name;
}


Mesh::~Mesh()
{
	delete m_vertexBuffer;
	//delete m_indexBuffer;
}


void Mesh::RotateAboutZ(float degrees)
{
	//m_orientation.m_yawDegrees += degrees;
	m_modelMatrix.Append(Mat44::CreateZRotationDegrees(degrees));
}


void Mesh::Rotate(Mat44& delta)
{
	m_orientationMatrix.Append(delta);
	Mat44 inverseMatrix = Mat44::GetInverse(GetModelMatrix());
	inverseMatrix.Append(m_orientationMatrix);
	m_modelMatrix.Append(inverseMatrix);

	//m_orientationMatrix.AppendYRotation(degrees.y);
	//m_orientationMatrix.AppendXRotation(degrees.x);
}


void Mesh::Rotate(Vec3 const& delta)
{
	Mat44 inverseMatrix = Mat44::GetInverse(GetModelMatrix());
	Vec3 localDelta = inverseMatrix.TransformVectorQuantity3D(delta);
	EulerAngles deltaAngles(localDelta.x, localDelta.y, localDelta.z);
	m_orientationMatrix.Append(deltaAngles.GetAsMatrix_XFwd_YLeft_ZUp());
}


Mat44 Mesh::GetModelMatrix() const
{
	//Mat44 model = Mat44::CreateTranslation3D(Vec3(0.f, 0.f, 0.f));
	//Mat44 rot = m_orientationMatrix; //m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	//model.Append(rot);
	//return model;
	return m_modelMatrix;
}


void Mesh::Render(Renderer* renderer) const
{
	EulerAngles ea = m_modelMatrix.GetEulerAngles_XFwd_YLeft_ZUp();
	std::string info = Stringf("%.2f, %.2f, %.2f", ea.m_yawDegrees, ea.m_pitchDegrees, ea.m_rollDegrees);
	//DebugAddMessage(info, 0.f, Rgba8::RED, Rgba8::RED);

	renderer->SetBlendMode(BlendMode::OPAQUE);
	renderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	renderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
	renderer->SetSamplerState(SamplerMode::POINTCLAMP);

	renderer->SetAmbientIntensity(0.8f);
	renderer->SetSunIntensity(0.4f);
	renderer->SetSunDirection(EulerAngles(0.f, 135.f, 0.f).GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D());
	renderer->BindLightConstantBuffer();

	renderer->SetModelMatrix(GetModelMatrix());
	renderer->BindTexture(m_texture);
	renderer->BindShader(m_shader);
	renderer->DrawVertexBuffer(m_vertexBuffer, m_size);
	renderer->BindShader(nullptr);
	renderer->BindTexture(nullptr);
	//renderer->DrawIndexBuffer(m_vertexBuffer, m_indexBuffer, m_indicesSize);
}


