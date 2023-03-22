#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Mesh/Mesh.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Renderer/D3D11Common.hpp"

#include "ThirdParty/stb/stb_image.h"

#ifdef ENGINE_DEBUG_RENDER
	#include <dxgidebug.h>
	#pragma comment(lib, "dxguid.lib")
#endif

#undef OPAQUE

static DXGI_FORMAT LocalToD3D11Format(TextureFormat format);
static UINT LocalToD3D11BindFlags(TextureBindFlags const flags);
static D3D11_USAGE LocalToD3D11Usage(MemoryHint hint);

Renderer::Renderer(RendererConfig const& config)
	:m_config(config)
{

}


Renderer::~Renderer()
{
}


void Renderer::Startup()
{
#ifdef ENGINE_DEBUG_RENDER
	m_dxgiDebugModule = (void*) ::LoadLibraryA("dxgidebug.dll");
	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	HRESULT hr = ((GetDebugModuleCB) ::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))(__uuidof(IDXGIDebug), &m_dxgiDebug);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Get Debug Module Get Proc Address Failed.");
		ERROR_AND_DIE("D3D11 Get Debug Module Get Proc Address Failed.");
	}
#endif

	CreateRenderContext();
	Image defaultImage(IntVec2(2, 2), Rgba8::WHITE);
	m_defaultTexture = CreateTextureFromImage(defaultImage);
	m_defaultShader = CreateShader("Default", defaultShaderSource);
	m_immediateVBO_PCU = CreateVertexBuffer(1, sizeof(Vertex_PCU));
	m_immediateVBO_PNCU = CreateVertexBuffer(1, sizeof(Vertex_PNCU));
	m_cameraCBO = CreateConstantBuffer((UINT)sizeof(CameraConstant));
	m_modelCBO = CreateConstantBuffer((UINT)sizeof(ModelConstant));
	m_lightCBO = CreateConstantBuffer((UINT)sizeof(LightConstant));

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = this;
	debugRenderConfig.m_startHidden = false;
	DebugRenderSystemStartup(debugRenderConfig);
}


void Renderer::BeginFrame()
{
	DebugRenderBeginFrame();
}


void Renderer::EndFrame()
{
	DebugRenderEndFrame();
	m_swapChain->Present(0, 0);

	CopyTexture(m_backBuffer, GetActiveColorTarget());
}


void Renderer::Shutdown()
{
	DebugRenderSystemShutdown();

	delete m_modelCBO;
	m_modelCBO = nullptr;

	delete m_cameraCBO;
	m_cameraCBO = nullptr;

	delete m_lightCBO;
	m_lightCBO = nullptr;

	delete m_immediateVBO_PNCU;
	m_immediateVBO_PNCU = nullptr;

	delete m_immediateVBO_PCU;
	m_immediateVBO_PCU = nullptr;

	for (int meshIndex = 0; meshIndex < (int)m_loadedMeshes.size(); meshIndex++)
	{
		delete m_loadedMeshes[meshIndex];
		m_loadedMeshes[meshIndex] = nullptr;
	}
	m_loadedMeshes.clear();

	for (int shaderIndex = 0; shaderIndex < (int)m_loadedShaders.size(); shaderIndex++)
	{
		delete m_loadedShaders[shaderIndex];
		m_loadedShaders[shaderIndex] = nullptr;
	}
	m_loadedShaders.clear();
	m_currentShader = nullptr;
	m_defaultShader = nullptr;

	for (int fontIndex = 0; fontIndex < (int)m_loadedFonts.size(); fontIndex++)
	{
		delete m_loadedFonts[fontIndex];
		m_loadedFonts[fontIndex] = nullptr;
	}
	m_loadedFonts.clear();

	for (int textureIndex = 0; textureIndex < (int)m_loadedTextures.size(); textureIndex++)
	{
		delete m_loadedTextures[textureIndex];
		m_loadedTextures[textureIndex] = nullptr;
	}
	m_loadedTextures.clear();
	m_defaultTexture = nullptr;

	DX_SAFE_RELEASE(m_depthStencilState);
	DX_SAFE_RELEASE(m_samplerState);
	DX_SAFE_RELEASE(m_blendState);
	DX_SAFE_RELEASE(m_rasterizerState);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_device);

#ifdef ENGINE_DEBUG_RENDER
	HRESULT hr = ((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Debug Report Live Objects Failed.");
		ERROR_AND_DIE("D3D11 Debug Report Live Objects Failed.");
	}
	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;
	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
}


void Renderer::ClearScreen(const Rgba8& clearColor) const
{
	Rgba8 screenColor = clearColor;
	float screenColorFloat[4] = {};
	screenColor.GetAsFloats(screenColorFloat);

	Texture* color = GetCurrentColorTarget();
	if (color)
	{
		TextureView* renderTargetView = color->GetRenderTargetView();
		m_deviceContext->ClearRenderTargetView(renderTargetView->m_renderTargetView, screenColorFloat);
	}
	ClearDepth();
}


void Renderer::ClearDepth(float value) const
{
	Texture* depth = GetCurrentDepthTarget();
	if (depth)
	{
		TextureView* depthStencilView = depth->GetDepthStencilView();
		m_deviceContext->ClearDepthStencilView(depthStencilView->m_depthStencilView, D3D11_CLEAR_DEPTH, value, 0);
	}
}


void Renderer::BeginCamera(const Camera& camera)
{	
	ASSERT_OR_DIE(m_currentCamera == nullptr, "Camera is nullptr");
	m_currentCamera = (Camera*)&camera;

	Texture* color = GetCurrentColorTarget();
	TextureView* renderTargetView = nullptr;
	if (color)
	{
		renderTargetView = color->GetRenderTargetView();
	}
	
	if (m_depthBuffer)
	{
		TextureView* depthStencilView = m_depthBuffer->GetDepthStencilView();
		m_deviceContext->OMSetRenderTargets(1, &renderTargetView->m_renderTargetView, depthStencilView->m_depthStencilView);
	}
	AABB2 const& viewportBounds = camera.GetViewport();
	IntVec2 clientDimensions = m_config.m_window->GetClientDimensions();
	D3D11_VIEWPORT viewPort = { 0 };
	viewPort.TopLeftX = viewportBounds.m_mins.x * static_cast<float>(clientDimensions.x);
	viewPort.TopLeftY = viewportBounds.m_mins.y * static_cast<float>(clientDimensions.y);
	viewPort.Width = (viewportBounds.m_maxs.x - viewportBounds.m_mins.x) * static_cast<float>(clientDimensions.x);
	viewPort.Height = (viewportBounds.m_maxs.y - viewportBounds.m_mins.y)  * static_cast<float>(clientDimensions.y);
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;
	m_deviceContext->RSSetViewports(1, &viewPort);

	CameraConstant cameraConstant;
	cameraConstant.ProjectionMatrix = camera.GetProjectionMatrix();
	cameraConstant.ViewMatrix = camera.GetViewMatrix();

	CopyCPUToGPU(&cameraConstant, sizeof(CameraConstant), m_cameraCBO);
	BindConstantBuffer(CAMERA_CONSTANT_BUFFER_SLOT, m_cameraCBO);

	Mat44 identityMatrix;
	m_modelConstant.ModelMatrix = identityMatrix;
	Rgba8 white = Rgba8::WHITE;
	white.GetAsFloats(m_modelConstant.ModelColor);

	//assume all drawing is in 2D, set to 3D in game if per request
	SetBlendMode(BlendMode::ALPHA);
	SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	SetDepthStencilState(DepthTest::ALWAYS, false);
	SetSamplerState(SamplerMode::POINTCLAMP);
	BindTexture(m_defaultTexture);
	BindShader(m_defaultShader);
}


void Renderer::EndCamera(const Camera& camera)
{
	ASSERT_OR_DIE(m_currentCamera == &camera, "End camera is not same as current");
	m_currentCamera = nullptr;
	m_deviceContext->ClearState();
}


void Renderer::DrawVertexArray(int numVertices, Vertex_PCU const* vertices) const
{
	CopyCPUToGPU(vertices, sizeof(Vertex_PCU) * numVertices, m_immediateVBO_PCU);
	DrawVertexBuffer(m_immediateVBO_PCU, numVertices);
}


void Renderer::DrawVertexArray(std::vector<Vertex_PCU> const& vertices) const
{
	CopyCPUToGPU(vertices.data(), sizeof(Vertex_PCU) * vertices.size(), m_immediateVBO_PCU);
	DrawVertexBuffer(m_immediateVBO_PCU, (int)vertices.size());
}


void Renderer::DrawVertexArray(int numVertices, const Vertex_PNCU* vertices) const
{
	CopyCPUToGPU(&m_lightConstant, sizeof(m_lightConstant), m_lightCBO);
	BindConstantBuffer(LIGHT_CONSTANT_BUFFER_SLOT, m_lightCBO);

	CopyCPUToGPU(vertices, sizeof(Vertex_PNCU) * numVertices, m_immediateVBO_PNCU);
	DrawVertexBuffer(m_immediateVBO_PNCU, numVertices);
}


void Renderer::DrawVertexArray(std::vector<Vertex_PNCU> const& vertices) const
{
	CopyCPUToGPU(&m_lightConstant, sizeof(m_lightConstant), m_lightCBO);
	BindConstantBuffer(LIGHT_CONSTANT_BUFFER_SLOT, m_lightCBO);

	CopyCPUToGPU(vertices.data(), sizeof(Vertex_PNCU) * vertices.size(), m_immediateVBO_PNCU);
	DrawVertexBuffer(m_immediateVBO_PNCU, (int)vertices.size());
}


void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset) const
{
	CopyCPUToGPU(&m_modelConstant, sizeof(m_modelConstant), m_modelCBO);
	BindConstantBuffer(MODEL_CONSTANT_BUFFER_SLOT, m_modelCBO);

	BindVertexBuffer(vbo);
	m_deviceContext->Draw(vertexCount, vertexOffset);
}


void Renderer::DrawIndexedArray(int numVertices, Vertex_PCU const* vertices, VertexBuffer* vbo, int numIndices, unsigned int const* indices, IndexBuffer* ibo) const
{
	CopyCPUToGPU(vertices, sizeof(Vertex_PCU) * numVertices, vbo);
	CopyCPUToGPU(indices, sizeof(unsigned int) * numIndices, ibo);

	DrawIndexBuffer(vbo, ibo, numVertices);
}


void Renderer::DrawIndexedArray(std::vector<Vertex_PCU> const& vertices, VertexBuffer* vbo, std::vector<unsigned int> const& indices, IndexBuffer* ibo) const
{
	CopyCPUToGPU(vertices.data(), sizeof(Vertex_PCU) * vertices.size(), vbo);
	CopyCPUToGPU(indices.data(), sizeof(unsigned int) * indices.size(), ibo);

	DrawIndexBuffer(vbo, ibo, (int)vertices.size());
}


void Renderer::DrawIndexedArray(int numVertices, Vertex_PNCU const* vertices, VertexBuffer* vbo, int numIndices, unsigned int const* indices, IndexBuffer* ibo) const
{
	CopyCPUToGPU(vertices, sizeof(Vertex_PNCU) * numVertices, vbo);
	CopyCPUToGPU(indices, sizeof(unsigned int) * numIndices, ibo);
	CopyCPUToGPU(&m_lightConstant, sizeof(m_lightConstant), m_lightCBO);
	BindConstantBuffer(LIGHT_CONSTANT_BUFFER_SLOT, m_lightCBO);

	DrawIndexBuffer(vbo, ibo, numVertices);
}


void Renderer::DrawIndexedArray(std::vector<Vertex_PNCU> const& vertices, VertexBuffer* vbo, std::vector<unsigned int> const& indices, IndexBuffer* ibo) const
{
	CopyCPUToGPU(vertices.data(), sizeof(Vertex_PNCU) * vertices.size(), vbo);
	CopyCPUToGPU(indices.data(), sizeof(unsigned int) * indices.size(), ibo);
	CopyCPUToGPU(&m_lightConstant, sizeof(m_lightConstant), m_lightCBO);
	BindConstantBuffer(LIGHT_CONSTANT_BUFFER_SLOT, m_lightCBO);

	DrawIndexBuffer(vbo, ibo, (int)vertices.size());
}


void Renderer::DrawIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int vertexCount, int indexOffset, int vertextOffset) const
{
	CopyCPUToGPU(&m_modelConstant, sizeof(m_modelConstant), m_modelCBO);
	BindConstantBuffer(MODEL_CONSTANT_BUFFER_SLOT, m_modelCBO);

	BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(vertexCount, indexOffset, vertextOffset);
}


void Renderer::CreateRenderContext()
{
	// create device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	IntVec2 clientDimensions = m_config.m_window->GetClientDimensions();
	swapChainDesc.BufferDesc.Width = clientDimensions.x;
	swapChainDesc.BufferDesc.Height = clientDimensions.y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = (HWND)m_config.m_window->GetOSWindowHandle();
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = 0;
#ifdef ENGINE_DEBUG_RENDER
	swapChainDesc.Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
	UINT sdkVersion = D3D11_SDK_VERSION;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, swapChainDesc.Flags, NULL, NULL, sdkVersion, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Device Failed.");
		ERROR_AND_DIE("D3D11 Create Device Failed.");
	}

	//save view of back buffer to clear and present
	CreateBackBuffer();

	//create and set viewport
	D3D11_VIEWPORT viewPort = { 0 };
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = static_cast<float>(clientDimensions.x);
	viewPort.Height = static_cast<float>(clientDimensions.y);
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;
	m_deviceContext->RSSetViewports(1, &viewPort);

	//create and set depth stencil
	CreateDepthBuffer();
}


Texture* Renderer::CreateOrGetTextureFromFile(const char* imageFilePath)
{
	Texture* existingTexture = GetTextureForFileName(imageFilePath);
	if (existingTexture) return existingTexture;

	return CreateTextureFromFile(imageFilePath);
}


BitmapFont* Renderer::CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtension)
{
	std::string appendPNG = std::string(bitmapFontFilePathWithNoExtension) + ".png";
	BitmapFont* existingFont = GetBitmapFontForFileName(appendPNG.c_str());
	if (existingFont) return existingFont;

	return CreateBitmapFontFromFile(appendPNG.c_str());
}


BitmapFont* Renderer::CreateOrGetBitmapFontWithMetadata(const char* fontTexturePathWithNoExtension, const char* fontMetadataPath)
{
	std::string appendPNG = std::string(fontTexturePathWithNoExtension) + ".png";
	BitmapFont* existingFont = GetBitmapFontForFileName(appendPNG.c_str());
	if (existingFont) return existingFont;

	return CreateBitmapFontFromFileWithMetadata(appendPNG.c_str(), fontMetadataPath);
}

Shader* Renderer::CreateOrGetShader(const char* shaderName)
{
	Shader* existingShader = GetShaderForName(shaderName);
	if (existingShader) return existingShader;

	return CreateShader(shaderName);
}


Mesh* Renderer::CreateOrGetMeshFromConfig(const char* meshName, MeshBuilderConfig const& config)
{
	Mesh* existingMesh = GetMeshForName(meshName);
	if (existingMesh) return existingMesh;

	return CreateMesh(meshName, config);
}


Texture* Renderer::GetTextureForFileName(const char* imageFilePath) const
{
	for (int textureIndex = 0; textureIndex < (int)m_loadedTextures.size(); textureIndex++)
	{
		Texture* curTexture = m_loadedTextures[textureIndex];
		if (curTexture->m_name == imageFilePath) return curTexture;
	}

	return NULL;
}


BitmapFont* Renderer::GetBitmapFontForFileName(const char* bitmapFontFilePath) const
{
	for (int fontIndex = 0; fontIndex < (int)m_loadedFonts.size(); fontIndex++)
	{
		BitmapFont* curFont = m_loadedFonts[fontIndex];
		if (curFont->m_fontFilePathName == bitmapFontFilePath) return curFont;
	}

	return NULL;
}


Shader* Renderer::GetShaderForName(const char* shaderName) const
{
	for (int shaderIndex = 0; shaderIndex < (int)m_loadedShaders.size(); shaderIndex++)
	{
		Shader* curShader = m_loadedShaders[shaderIndex];
		if (curShader->GetName() == shaderName) return curShader;
	}

	return NULL;
}


Mesh* Renderer::GetMeshForName(const char* meshName) const
{
	for (int meshIndex = 0; meshIndex < (int)m_loadedMeshes.size(); meshIndex++)
	{
		Mesh* curMesh = m_loadedMeshes[meshIndex];
		if (curMesh->m_name == meshName) return curMesh;
	}

	return NULL;
}


const Camera* Renderer::GetCamera() const
{
	return m_currentCamera;
}


void Renderer::BindTexture(const Texture* texture, uint32_t idx)
{
	if (!texture)
	{
		texture = m_defaultTexture;
	}

	Texture* textureToBind = const_cast<Texture*>(texture);
	TextureView* view = textureToBind->GetShaderResourceView();
	m_deviceContext->PSSetShaderResources(idx, 1, &view->m_shaderResourceView);
}


void Renderer::SetSunDirection(Vec3 const& sunDirection)
{
	m_lightConstant.SunDirection = sunDirection;
}


void Renderer::SetSunIntensity(float sunIntensity)
{
	m_lightConstant.SunIntensity = sunIntensity;
}


void Renderer::SetSunColor(Vec3 const& sunColor)
{
	m_lightConstant.SunColor = sunColor;
}


void Renderer::SetAmbientIntensity(float ambientIntensity)
{
	m_lightConstant.AmbientIntensity = ambientIntensity;
}


void Renderer::SetLightConstant(Vec3 const& sunDirection, float sunIntensity, Vec3 const& sunColor, float ambientIntensity)
{
	m_lightConstant.SunDirection		= sunDirection;
	m_lightConstant.SunIntensity		= sunIntensity;
	m_lightConstant.SunColor			= sunColor;
	m_lightConstant.AmbientIntensity	= ambientIntensity;
}


void Renderer::SetModelMatrix(Mat44 const& matrix)
{
	m_modelConstant.ModelMatrix = matrix;
}


void Renderer::SetModelColor(Rgba8 const& color)
{
	Rgba8 modelColor = color;
	modelColor.GetAsFloats(m_modelConstant.ModelColor);
}


void Renderer::SetBlendMode(BlendMode blendMode)
{
	DX_SAFE_RELEASE(m_blendState);
	D3D11_BLEND_DESC blendDesc = { 0 };
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	switch (blendMode)
	{
		case BlendMode::OPAQUE:
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			break;
		case BlendMode::ALPHA:
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			break;
		case BlendMode::ADDITIVE:
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			break;
		default:
			ERROR_AND_DIE(Stringf("Unknown / unsupported blend mode #%i", blendMode));
			break;
	}
	HRESULT hr = m_device->CreateBlendState(&blendDesc, &m_blendState);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Blend State Failed");
		ERROR_AND_DIE("D3D11 Create Blend State Failed");
	}

	float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
	m_deviceContext->OMSetBlendState(m_blendState, blendFactor, 0xffffffff);
}


void Renderer::SetRasterizerState(CullMode cullMode, FillMode fillMode, WindingOrder windingOrder)
{
	DX_SAFE_RELEASE(m_rasterizerState);
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = (D3D11_FILL_MODE)fillMode;
	rasterizerDesc.CullMode = (D3D11_CULL_MODE)cullMode;
	rasterizerDesc.FrontCounterClockwise = (bool)windingOrder;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.AntialiasedLineEnable = true;
	HRESULT hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Rasterizer State Failed.");
		ERROR_AND_DIE("D3D11 Create Rasterizer State Failed.");
	}
	m_deviceContext->RSSetState(m_rasterizerState);
}


void Renderer::SetDepthStencilState(DepthTest depthTest, bool writeDepth)
{
	DX_SAFE_RELEASE(m_depthStencilState);
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = writeDepth ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = (D3D11_COMPARISON_FUNC)depthTest;
	HRESULT hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Depth Stencil State Failed.");
		ERROR_AND_DIE("D3D11 Create Depth Stencil State Failed.");
	}
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
}


void Renderer::SetSamplerState(SamplerMode samplerMode)
{
	DX_SAFE_RELEASE(m_samplerState);
	D3D11_SAMPLER_DESC samplerDesc = { };
	switch (samplerMode)
	{
		case SamplerMode::POINTCLAMP:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case SamplerMode::POINTWRAP:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case SamplerMode::BILINEARCLAMP:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case SamplerMode::BILINEARWRAP:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		default:
			ERROR_AND_DIE(Stringf("Unknown / unsupported sampler mode #%i", samplerMode));
			break;
	}
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Sampler State Failed.");
		ERROR_AND_DIE("D3D11 Create Sampler State Failed.");
	}
	m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
}


void Renderer::BindShaderByName(const char* shaderName)
{
	Shader* shader = GetShaderForName(shaderName);
	BindShader(shader);
}


void Renderer::BindShader(Shader* shader)
{
	if (!shader)
	{
		shader = m_defaultShader;
	}
	m_deviceContext->VSSetShader(shader->m_vertexShader, NULL, 0);
	m_deviceContext->PSSetShader(shader->m_pixelShader, NULL, 0);
	m_deviceContext->IASetInputLayout(shader->m_inputLayout);
	m_currentShader = shader;
}


VertexBuffer* Renderer::CreateVertexBuffer(const size_t size, unsigned int stride) const
{
	VertexBuffer* newBuffer = new VertexBuffer(size, stride);
	//create vertex buffer
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, NULL, &(newBuffer->m_buffer));
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Buffer Failed");
		ERROR_AND_DIE("D3D11 Create Buffer Failed");
	}

	SetDebugName(newBuffer->m_buffer, "Vertex Buffer");
	return newBuffer;
}


void Renderer::CopyCPUToGPU(const void* data, size_t size, VertexBuffer* vbo) const
{
	if (vbo->m_size < size)
	{
		DX_SAFE_RELEASE(vbo->m_buffer);
 		VertexBuffer* newVBO = CreateVertexBuffer(size, vbo->GetStride());
		vbo->m_buffer = newVBO->m_buffer;
		vbo->m_size = newVBO->m_size;
		newVBO->m_buffer = nullptr;
		delete newVBO;
	}

	//mapping and un-mapping
	D3D11_MAPPED_SUBRESOURCE pMappedSubresource = { 0 };
	HRESULT hr = m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &pMappedSubresource);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Map Subresource Failed");
		ERROR_AND_DIE("D3D11 Map Subresource Failed");
	}

	memcpy(pMappedSubresource.pData, data, size);
	m_deviceContext->Unmap(vbo->m_buffer, 0);
}


void Renderer::BindVertexBuffer(VertexBuffer* vbo) const
{
	UINT pStride = (UINT)vbo->GetStride();
	UINT pOffset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &pStride, &pOffset);
	m_deviceContext->IASetInputLayout(m_currentShader->m_inputLayout);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


IndexBuffer* Renderer::CreateIndexBuffer(const size_t size) const
{
	IndexBuffer* newBuffer = new IndexBuffer(size);
	//create index buffer
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer(&bufferDesc, NULL, &(newBuffer->m_buffer));
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Buffer Failed");
		ERROR_AND_DIE("D3D11 Create Buffer Failed");
	}

	SetDebugName(newBuffer->m_buffer, "Index Buffer");
	return newBuffer;
}


void Renderer::CopyCPUToGPU(const void* data, size_t size, IndexBuffer* ibo) const
{
	if (ibo->m_size < size)
	{
		DX_SAFE_RELEASE(ibo->m_buffer);
		IndexBuffer* newVBO = CreateIndexBuffer(size);
		ibo->m_buffer = newVBO->m_buffer;
		ibo->m_size = newVBO->m_size;
		newVBO->m_buffer = nullptr;
		delete newVBO;
	}

	//mapping and un-mapping
	D3D11_MAPPED_SUBRESOURCE pMappedSubresource = { 0 };
	HRESULT hr = m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &pMappedSubresource);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Map Subresource Failed");
		ERROR_AND_DIE("D3D11 Map Subresource Failed");
	}

	memcpy(pMappedSubresource.pData, data, size);
	m_deviceContext->Unmap(ibo->m_buffer, 0);
}


void Renderer::BindIndexBuffer(IndexBuffer* ibo) const
{
	UINT pOffset = 0;
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, pOffset);
}


ConstantBuffer* Renderer::CreateConstantBuffer(const size_t size) const
{
	ConstantBuffer* newBuffer = new ConstantBuffer(size);
	//create constant buffer
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, NULL, &(newBuffer->m_buffer));
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Constant Buffer Failed");
		ERROR_AND_DIE("D3D11 Create Constant Buffer Failed");
	}

	SetDebugName(newBuffer->m_buffer, "Constant Buffer");
	return newBuffer;
}


void Renderer::CopyCPUToGPU(const void* data, size_t size, ConstantBuffer* cbo) const
{
	if (cbo->m_size < size)
	{
		DX_SAFE_RELEASE(cbo->m_buffer);
		ConstantBuffer* newCBO = CreateConstantBuffer(size);
		cbo->m_buffer = newCBO->m_buffer;
		cbo->m_size = newCBO->m_size;
		newCBO->m_buffer = nullptr;
		delete newCBO;
	}

	//mapping and un-mapping
	D3D11_MAPPED_SUBRESOURCE pMappedSubresource = { 0 };
	HRESULT hr = m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &pMappedSubresource);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Map Constant Buffer Subresource Failed");
		ERROR_AND_DIE("D3D11 Map Constant Buffer Subresource Failed");
	}

	memcpy(pMappedSubresource.pData, data, size);
	m_deviceContext->Unmap(cbo->m_buffer, 0);
}


void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo) const
{
	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}


void Renderer::BindLightConstantBuffer() const
{
	CopyCPUToGPU(&m_lightConstant, sizeof(m_lightConstant), m_lightCBO);
	BindConstantBuffer(LIGHT_CONSTANT_BUFFER_SLOT, m_lightCBO);
}


Texture* Renderer::CreateTextureFromInfo(TextureCreateInfo const& createInfo)
{
	ID3D11Texture2D* handle = createInfo.handle;

	if (!handle)
	{
		D3D11_TEXTURE2D_DESC texture2DDesc = { 0 };
		texture2DDesc.Width = createInfo.dimensions.x;
		texture2DDesc.Height = createInfo.dimensions.y;
		texture2DDesc.MipLevels = 1;
		texture2DDesc.ArraySize = 1;
		texture2DDesc.Format = LocalToD3D11Format(createInfo.format);
		texture2DDesc.SampleDesc.Count = 1;
		texture2DDesc.Usage = LocalToD3D11Usage(createInfo.memoryHint);
		texture2DDesc.BindFlags = LocalToD3D11BindFlags(createInfo.bindFlags);


		D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
		D3D11_SUBRESOURCE_DATA* subresourceDataPtr = nullptr;
		if (createInfo.initialData)
		{
			subresourceData.pSysMem = createInfo.initialData;
			subresourceData.SysMemPitch = (UINT)createInfo.initialDataByteSize / (UINT)createInfo.dimensions.y;
			subresourceDataPtr = &subresourceData;
		}
		HRESULT hr = m_device->CreateTexture2D(&texture2DDesc, subresourceDataPtr, &handle);
		if (!SUCCEEDED(hr))
		{
			DebuggerPrintf("D3D11 Create Texture 2D Failed");
			ERROR_AND_DIE("D3D11 Create Texture 2D Failed");
		}
	}
	else
	{
		handle->AddRef();
	}

	Texture* texture = new Texture();
	texture->m_owner = this;
	texture->m_name = createInfo.name;
	texture->m_dimensions = createInfo.dimensions;
	texture->m_texture = handle;
	texture->m_format = createInfo.format;
	texture->m_allowedBinds = createInfo.bindFlags;

	SetDebugName(texture->m_texture, texture->m_name.c_str());
	m_loadedTextures.push_back(texture);
	return texture;
}


void Renderer::CopyTexture(Texture* destination, Texture* source)
{
	m_deviceContext->CopyResource(destination->m_texture, source->m_texture);
}


void Renderer::CopyTextureWithShader(Texture* destination, Texture* source, Shader* effect)
{
	BindShader(effect);
	TextureView* renderTargetView = destination->GetRenderTargetView();
	m_deviceContext->OMSetRenderTargets(1, &renderTargetView->m_renderTargetView, nullptr);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	IntVec2 textureDimensions = source->GetDimensions();
	D3D11_VIEWPORT viewPort = { 0 };
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = static_cast<float>(textureDimensions.x);
	viewPort.Height = static_cast<float>(textureDimensions.y);
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;
	m_deviceContext->RSSetViewports(1, &viewPort);
	BindTexture(source, 0);
	BindTexture(GetCurrentDepthTarget(), 1);
	SetSamplerState(SamplerMode::POINTCLAMP);
	SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);

	m_deviceContext->Draw(3, 0);
	
	m_deviceContext->ClearState();
}


void Renderer::ApplyEffect(Shader* effect)
{
	Texture* active = GetActiveColorTarget();
	Texture* backup = GetBackupColorTarget();

	CopyTextureWithShader(backup, active, effect);
	m_activeColorTargetIndex = (m_activeColorTargetIndex + 1) % 2;
}


BitmapFont* Renderer::CreateBitmapFontFromFile(const char* bitmapFontFilePath)
{
	Texture* newTexture = CreateOrGetTextureFromFile(bitmapFontFilePath);
	BitmapFont* newFont = new BitmapFont(bitmapFontFilePath, *newTexture);
	m_loadedFonts.push_back(newFont);
	return newFont;
}


BitmapFont* Renderer::CreateBitmapFontFromFileWithMetadata(const char* fontTexturePath, const char* fontMetadataPath)
{
	Texture* newTexture = CreateOrGetTextureFromFile(fontTexturePath);
	BitmapFont* newFont = new BitmapFont(fontMetadataPath, *newTexture, true);
	m_loadedFonts.push_back(newFont);
	return newFont;
}


Texture* Renderer::CreateTextureFromFile(const char* imageFilePath)
{
	Image image(imageFilePath);
	return CreateTextureFromImage(image);
}


Texture* Renderer::CreateTextureFromImage(const Image& image)
{
	TextureCreateInfo info;
	info.name = image.GetImageFilePath();
	info.dimensions = image.GetDimensions();
	info.initialData = image.GetRawData();
	
	IntVec2 dimensions = image.GetDimensions();
	size_t byteSize = static_cast<size_t>(dimensions.x) * static_cast<size_t>(dimensions.y) * sizeof(Rgba8);
	info.initialDataByteSize = byteSize;

	Texture* newTexture = CreateTextureFromInfo(info);
	return newTexture;
}


Texture* Renderer::GetCurrentColorTarget() const
{
	Texture* color = m_currentCamera ? m_currentCamera->GetColorTarget() : nullptr;
	return color ? color : GetActiveColorTarget();
}


Texture* Renderer::GetCurrentDepthTarget() const
{
	Texture* depth = m_currentCamera ? m_currentCamera->GetDepthTarget() : nullptr;
	return depth ? depth : m_depthBuffer;
}


void Renderer::DestroyTexture(Texture* texture)
{
	if (texture)
	{
		delete texture;
	}
}


void Renderer::CreateBackBuffer()
{
	ID3D11Texture2D* texture2D = nullptr;
	HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&texture2D);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 SwapChain Get Buffer Failed.");
		ERROR_AND_DIE("D3D11 SwapChain Get Buffer Failed.");
	}

	D3D11_TEXTURE2D_DESC texture2DDesc = { 0 };
	texture2D->GetDesc(&texture2DDesc);

	TextureCreateInfo info;
	info.name = "Swapchain";
	info.dimensions = IntVec2(m_config.m_window->GetClientDimensions());
	ASSERT_OR_DIE(texture2DDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM, "swap chain isn't R8G8B8A8");
	info.format = TextureFormat::R8G8B8A8_UNORM;
	info.bindFlags = TEXTURE_BIND_RENDER_TARGET_BIT;
	info.memoryHint = MemoryHint::GPU;
	info.handle = texture2D;
	m_backBuffer = CreateTextureFromInfo(info);

	info.name = "DefaultColor";
	info.handle = nullptr;
	info.bindFlags |= TEXTURE_BIND_SHADER_RESOURCE_BIT;
	m_defaultColorTarget[0] = CreateTextureFromInfo(info);
	m_defaultColorTarget[1] = CreateTextureFromInfo(info);

	DX_SAFE_RELEASE(texture2D);
}


Texture* Renderer::GetActiveColorTarget() const
{
	return m_defaultColorTarget[m_activeColorTargetIndex];
}


Texture* Renderer::GetBackupColorTarget() const
{
	uint16_t backupIndex = (m_activeColorTargetIndex + 1) % 2;
	return m_defaultColorTarget[backupIndex];
}


void Renderer::CreateDepthBuffer()
{
	TextureCreateInfo info;
	info.name = "DefaultDepth";
	info.dimensions = IntVec2(m_config.m_window->GetClientDimensions());
	info.format = TextureFormat::R24G8_TYPELESS;
	info.bindFlags = TEXTURE_BIND_DEPTH_STENCIL_BIT | TEXTURE_BIND_SHADER_RESOURCE_BIT;
	info.memoryHint = MemoryHint::GPU;
	m_depthBuffer = CreateTextureFromInfo(info);
}


Shader* Renderer::CreateShader(char const* shaderName, char const* shaderSource)
{
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;

	Shader* newShader = new Shader(shaderConfig);

	std::vector<uint8_t> vertexShaderByteCode;
	std::vector<uint8_t> pixelShaderByteCode;

	CompileShaderToByteCode(vertexShaderByteCode, newShader->GetName().c_str(), shaderSource, newShader->m_config.m_vertexEntryPoint.c_str(), "vs_5_0");
	HRESULT hr = m_device->CreateVertexShader(vertexShaderByteCode.data(), vertexShaderByteCode.size(), NULL, &(newShader->m_vertexShader));
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Vertex Shader Failed");
		ERROR_AND_DIE("D3D11 Create Vertex Shader Failed");
	}

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC inputElementDescLit[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (strstr(shaderName, "Lit"))
	{
		hr = m_device->CreateInputLayout(inputElementDescLit, ARRAYSIZE(inputElementDescLit), (void*)vertexShaderByteCode.data(), vertexShaderByteCode.size(), &newShader->m_inputLayout);
	}
	else
	{
		hr = m_device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), (void*)vertexShaderByteCode.data(), vertexShaderByteCode.size(), &newShader->m_inputLayout);
	}
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Input Layout Failed");
		ERROR_AND_DIE("D3D11 Create Input Layout Failed");
	}


	CompileShaderToByteCode(pixelShaderByteCode, newShader->GetName().c_str(), shaderSource, newShader->m_config.m_pixelEntryPoint.c_str(), "ps_5_0");
	hr = m_device->CreatePixelShader(pixelShaderByteCode.data(), pixelShaderByteCode.size(), NULL, &(newShader->m_pixelShader));
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("D3D11 Create Pixel Shader Failed");
		ERROR_AND_DIE("D3D11 Create Pixel Shader Failed");
	}

	SetDebugName(newShader->m_vertexShader, newShader->GetName().c_str());
	SetDebugName(newShader->m_pixelShader, newShader->GetName().c_str());
	SetDebugName(newShader->m_inputLayout, newShader->GetName().c_str());
	m_loadedShaders.push_back(newShader);
	return newShader;
}


Shader* Renderer::CreateShader(char const* shaderName)
{
	std::string shaderPath = std::string(shaderName) + ".hlsl";
	std::string shaderSource;
	FileReadToString(shaderSource, shaderPath);
	return CreateShader(shaderName, shaderSource.c_str());
}


Mesh* Renderer::CreateMesh(char const* meshName, MeshBuilderConfig const& config)
{
	MeshBuilder meshBuilder;
	meshBuilder.LoadFromConfig(config);
	Mesh* newMesh = new Mesh(&meshBuilder, this, meshName);
	m_loadedMeshes.push_back(newMesh);
	return newMesh;
}


bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target)
{
	UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef ENGINE_DEBUG_RENDER
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DInclude* include = D3D_COMPILE_STANDARD_FILE_INCLUDE;
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompile(source, strlen(source), name, NULL, include, entryPoint, target, shaderFlags, 0, &shaderBlob, &errorBlob);
	if (!SUCCEEDED(hr))
	{
		if (errorBlob)
		{
			DebuggerPrintf((char*)errorBlob->GetBufferPointer());
		}

		if (shaderBlob)
		{
			DebuggerPrintf((char*)shaderBlob->GetBufferPointer());
		}

		errorBlob->Release();
 		shaderBlob->Release();
		DebuggerPrintf("D3D11 Compile Vertex Shader Failed.");
		ERROR_AND_DIE("D3D11 Compile Vertex Shader Failed.");
	}

	outByteCode.resize(shaderBlob->GetBufferSize());
	memcpy(outByteCode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
	shaderBlob->Release();

	return true;
}


void Renderer::SetDebugName(ID3D11DeviceChild* object, char const* name) const
{
#ifdef ENGINE_DEBUG_RENDER
	object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT) strlen(name), name);
#else
	UNUSED(object)
	UNUSED(name)
#endif
}


DXGI_FORMAT LocalToD3D11Format(TextureFormat format)
{
	switch (format)
	{
		case TextureFormat::R8G8B8A8_UNORM:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::D24_UNORM_S8_UINT:	return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::R24G8_TYPELESS:		return DXGI_FORMAT_R24G8_TYPELESS;
		default:								ERROR_AND_DIE("unsupport format");
	}
}


UINT LocalToD3D11BindFlags(TextureBindFlags const flags)
{
	UINT result = 0;

	if (flags & TEXTURE_BIND_SHADER_RESOURCE_BIT)
	{
		result |= D3D11_BIND_SHADER_RESOURCE;
	}

	if (flags & TEXTURE_BIND_DEPTH_STENCIL_BIT)
	{
		result |= D3D11_BIND_DEPTH_STENCIL;
	}

	if (flags & TEXTURE_BIND_RENDER_TARGET_BIT)
	{
		result |= D3D11_BIND_RENDER_TARGET;
	}

	return result;
}


D3D11_USAGE LocalToD3D11Usage(MemoryHint hint)
{
	switch (hint)
	{
		case MemoryHint::STATIC:	return D3D11_USAGE_IMMUTABLE;
		case MemoryHint::GPU:		return D3D11_USAGE_DEFAULT;
		case MemoryHint::DYNAMIC:	return D3D11_USAGE_DYNAMIC;
		default:					ERROR_AND_DIE("unsupported usage");
	}
}


