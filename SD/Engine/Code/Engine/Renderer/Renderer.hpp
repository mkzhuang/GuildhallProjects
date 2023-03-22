#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Mesh/MeshBuilder.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"

#include <vector>

class Mesh;
class Window;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DeviceChild;
struct ID3D11DepthStencilState;

const int LIGHT_CONSTANT_BUFFER_SLOT = 1;
const int CAMERA_CONSTANT_BUFFER_SLOT = 2;
const int MODEL_CONSTANT_BUFFER_SLOT = 3;

enum class BlendMode
{
	OPAQUE,
	ALPHA,
	ADDITIVE
};

enum class CullMode
{
	NONE = 1,
	FRONT = 2,
	BACK = 3
};

enum class FillMode
{
	WIREFRAME = 2,
	SOLID = 3
};

enum class WindingOrder
{
	CLOCKWISE = false,
	COUNTERCLOCKWISE = true
};

enum class DepthTest
{
	ALWAYS = 8,
	NEVER = 1,
	EQUAL = 3,
	NOTEQUAL = 6,
	LESS = 2,
	LESSEQUAL = 4,
	GREATER = 5,
	GREATEREQUAL = 7
};

enum class SamplerMode
{
	POINTCLAMP,
	POINTWRAP,
	BILINEARCLAMP,
	BILINEARWRAP
};

struct LightConstant
{
	Vec3 SunDirection = Vec3(0.f, 0.f, -1.f);
	float SunIntensity = 0.f;
	Vec3 SunColor = Vec3(255.f, 255.f, 255.f);
	float AmbientIntensity = 0.f;
};

struct CameraConstant
{
	Mat44 ProjectionMatrix;
	Mat44 ViewMatrix;
};

struct ModelConstant
{
	Mat44 ModelMatrix;
	float ModelColor[4] = {};
};

struct RendererConfig
{
	Window* m_window = nullptr;
};

class Renderer
{
public:
	Renderer(RendererConfig const& config);
	~Renderer();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen(const Rgba8& clearColor) const;
	void ClearDepth(float value = 1.f) const;
	void BeginCamera(const Camera& camera);
	void EndCamera(const Camera& camera);
	void DrawVertexArray(int numVertices, const Vertex_PCU* vertices) const;
	void DrawVertexArray(std::vector<Vertex_PCU> const& vertices) const;
	void DrawVertexArray(int numVertices, const Vertex_PNCU* vertices) const;
	void DrawVertexArray(std::vector<Vertex_PNCU> const& vertices) const;
	void DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset = 0) const;
	void DrawIndexedArray(int numVertices, Vertex_PCU const* vertices, VertexBuffer* vbo, int numIndices, unsigned int const* indices, IndexBuffer* ibo) const;
	void DrawIndexedArray(std::vector<Vertex_PCU> const& vertices, VertexBuffer* vbo, std::vector<unsigned int> const& indices, IndexBuffer* ibo) const;
	void DrawIndexedArray(int numVertices, Vertex_PNCU const* vertices, VertexBuffer* vbo, int numIndices, unsigned int const* indices, IndexBuffer* ibo) const;
	void DrawIndexedArray(std::vector<Vertex_PNCU> const& vertices, VertexBuffer* vbo, std::vector<unsigned int> const& indices, IndexBuffer* ibo) const;
	void DrawIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int vertexCount, int indexOffset = 0, int vertextOffset = 0) const;

	void CreateRenderContext();
	
	Texture* CreateOrGetTextureFromFile(const char* imageFilePath);
	BitmapFont* CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtension);
	BitmapFont* CreateOrGetBitmapFontWithMetadata(const char* fontTexturePathWithNoExtension, const char* fontMetadataPath);
	Shader* CreateOrGetShader(const char* shaderName);
	Mesh* CreateOrGetMeshFromConfig(const char* meshName, MeshBuilderConfig const& config);
	Texture* GetTextureForFileName(const char* imageFilePath) const;
	BitmapFont* GetBitmapFontForFileName(const char* bitmapFontFilePath) const;
	Shader* GetShaderForName(const char* shaderName) const;
	Mesh* GetMeshForName(const char* meshName) const;
	const Camera* GetCamera() const;
	void BindTexture(const Texture* texture, uint32_t idx = 0);
	void SetSunDirection(Vec3 const& sunDirection);
	void SetSunIntensity(float sunIntensity);
	void SetSunColor(Vec3 const& sunColor);
	void SetAmbientIntensity(float ambientIntensity);
	void SetLightConstant(Vec3 const& sunDirection, float sunIntensity, Vec3 const& sunColor, float ambientIntensity);
	void SetModelMatrix(Mat44 const& matrix);
	void SetModelColor(Rgba8 const& color);
	void SetBlendMode(BlendMode blendMode);
	void SetRasterizerState(CullMode cullMode, FillMode fillMode, WindingOrder windingOrder);
	void SetDepthStencilState(DepthTest depthTest, bool writeDepth);
	void SetSamplerState(SamplerMode samplerMode);
	void BindShaderByName(const char* shaderName);
	void BindShader(Shader* shader);
	VertexBuffer* CreateVertexBuffer(const size_t size, unsigned int stride = sizeof(Vertex_PCU)) const;
	void CopyCPUToGPU(const void* data, size_t size, VertexBuffer* vbo) const;
	void BindVertexBuffer(VertexBuffer* vbo) const;
	IndexBuffer* CreateIndexBuffer(const size_t size) const;
	void CopyCPUToGPU(const void* data, size_t size, IndexBuffer* ibo) const;
	void BindIndexBuffer(IndexBuffer* ibo) const;
	ConstantBuffer* CreateConstantBuffer(const size_t size) const;
	void CopyCPUToGPU(const void* data, size_t size, ConstantBuffer* cbo) const;
	void BindConstantBuffer(int slot, ConstantBuffer* cbo) const;
	void BindLightConstantBuffer() const;
	Texture* CreateTextureFromInfo(TextureCreateInfo const& createInfo);
	void CopyTexture(Texture* destination, Texture* source);
	void CopyTextureWithShader(Texture* destination, Texture* source, Shader* effect);
	void ApplyEffect(Shader* effect);

private:
	BitmapFont* CreateBitmapFontFromFile(const char* bitmapFontFilePath);
	BitmapFont* CreateBitmapFontFromFileWithMetadata(const char* fontTexturePath, const char* fontMetadataPath);
	Texture* CreateTextureFromFile(const char* imageFilePath);
	Texture* CreateTextureFromImage(const Image& image);
	Texture* GetCurrentColorTarget() const;
	Texture* GetCurrentDepthTarget() const;
	void DestroyTexture(Texture* texture);
	void CreateBackBuffer();
	Texture* GetActiveColorTarget() const;
	Texture* GetBackupColorTarget() const;
	void CreateDepthBuffer();
	Shader* CreateShader(char const* shaderName, char const* shaderSource);
	Shader* CreateShader(char const* shaderName);
	Mesh* CreateMesh(char const* meshName, MeshBuilderConfig const& config);
	bool CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target);
	void SetDebugName(ID3D11DeviceChild* object, char const* name) const;

protected:
	RendererConfig m_config;
	std::vector<Texture*> m_loadedTextures;
	std::vector<BitmapFont*> m_loadedFonts;
	std::vector<Shader*> m_loadedShaders;
	std::vector<Mesh*> m_loadedMeshes;
	Shader const* m_currentShader = nullptr;
	Shader* m_defaultShader = nullptr;
	Texture* m_defaultTexture = nullptr;
	VertexBuffer* m_immediateVBO_PCU = nullptr;
	VertexBuffer* m_immediateVBO_PNCU = nullptr;
	ConstantBuffer* m_lightCBO = nullptr;
	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ModelConstant m_modelConstant;
	LightConstant m_lightConstant;

	void* m_dxgiDebugModule = nullptr;
	void* m_dxgiDebug = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11RasterizerState*	m_rasterizerState = nullptr;
	ID3D11BlendState* m_blendState = nullptr;
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	Texture* m_backBuffer = nullptr;
	Texture* m_depthBuffer = nullptr;
	Texture* m_defaultColorTarget[2] = {};
	uint16_t m_activeColorTargetIndex = 0;

	const Camera* m_currentCamera = nullptr;
};


