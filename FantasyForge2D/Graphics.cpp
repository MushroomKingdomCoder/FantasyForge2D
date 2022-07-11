#include "Graphics.h"
#include "Rect.h"
#include "Math.h"
#include <assert.h>
#pragma comment(lib, "d3d11.lib")

using namespace Microsoft::WRL;

Graphics::Graphics(HWND hWnd, unsigned int WindowWidth, unsigned int WindowHeight, std::vector<uint2> display_layer_dims)
{
	for (const uint2& dld : display_layer_dims)
	{
		Layers.emplace_back(dld.x, dld.y);
	}

	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferCount = 2u;
	scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scd.BufferDesc.Width = WindowWidth;
	scd.BufferDesc.Height = WindowHeight;
	scd.BufferDesc.RefreshRate.Numerator = 0u;
	scd.BufferDesc.RefreshRate.Denominator = 1u;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.Flags = 0u;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1u;
	scd.SampleDesc.Quality = 0u;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT dxFlags = 0u;

#ifdef _DEBUG
	dxFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	GFXCHECK(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, dxFlags, nullptr, 0u, D3D11_SDK_VERSION, &scd, &pFrameManager, &pDevice, nullptr, &pPipeline));

	ComPtr<ID3D11Resource> pBackBuffer = nullptr;
	GFXCHECK(pFrameManager->GetBuffer(0u, __uuidof(ID3D11Resource), &pBackBuffer));
	GFXCHECK(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pFrameBufferView));

	pPipeline->OMSetRenderTargets(1u, pFrameBufferView.GetAddressOf(), nullptr);

	ComPtr<ID3D11Buffer> pBuffer;
	D3D11_BUFFER_DESC bd = {};
	D3D11_SUBRESOURCE_DATA sd = {};

	struct Vertex
	{
		struct
		{
			float x;
			float y;
		} pos;
		struct
		{
			float u;
			float v;
		} tc;
	};
	Vertex VBuffer[4] = 
	{ 
		{ { -1.0f, 1.0f },{ 0.0f,0.0f } },
		{ {  1.0f, 1.0f },{ 1.0f,0.0f } },
		{ { -1.0f,-1.0f },{ 0.0f,1.0f } },
		{ {  1.0f,-1.0f },{ 1.0f,1.0f } } 
	};
	bd = {};
	bd.ByteWidth = UINT(std::size(VBuffer) * sizeof(Vertex));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.StructureByteStride = sizeof(Vertex);
	sd = {};
	sd.pSysMem = VBuffer;
	GFXCHECK(pDevice->CreateBuffer(&bd, &sd, &pBuffer));
	UINT offset = 0u;
	pPipeline->IASetVertexBuffers(0u, 1u, pBuffer.GetAddressOf(), &bd.StructureByteStride, &offset);

	unsigned short IBuffer[6] = { 2,0,1, 1,3,2 };
	bd = {};
	bd.ByteWidth = UINT(std::size(IBuffer) * sizeof(unsigned short));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.StructureByteStride = sizeof(unsigned short);
	sd = {};
	sd.pSysMem = IBuffer;
	GFXCHECK(pDevice->CreateBuffer(&bd, &sd, &pBuffer));
	pPipeline->IASetIndexBuffer(pBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

	ComPtr<ID3D11InputLayout> pILayout = nullptr;
	D3D11_INPUT_ELEMENT_DESC ied[2];
	ied[0].AlignedByteOffset = 0u;
	ied[0].Format = DXGI_FORMAT_R32G32_FLOAT;
	ied[0].InputSlot = 0u;
	ied[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	ied[0].InstanceDataStepRate = 0u;
	ied[0].SemanticIndex = 0u;
	ied[0].SemanticName = "Position";
	ied[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	ied[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	ied[1].InputSlot = 0u;
	ied[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	ied[1].InstanceDataStepRate = 0u;
	ied[1].SemanticIndex = 0u;
	ied[1].SemanticName = "TextureCoordinate";
	GFXCHECK(pDevice->CreateInputLayout(ied, (UINT)std::size(ied), VSS::Default.GetByteCode(), VSS::Default.GetByteCodeSize(), &pILayout));
	pPipeline->IASetInputLayout(pILayout.Get());

	ComPtr<ID3D11BlendState> pColorBlender = nullptr;
	D3D11_BLEND_DESC bld = {};
	bld.AlphaToCoverageEnable = FALSE;
	bld.IndependentBlendEnable = FALSE;
	bld.RenderTarget[0].BlendEnable = TRUE;
	bld.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bld.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bld.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bld.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	bld.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	bld.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bld.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	GFXCHECK(pDevice->CreateBlendState(&bld, &pColorBlender));
	pPipeline->OMSetBlendState(pColorBlender.Get(), nullptr, 0xFFFFFFFF);

	pPipeline->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (Layer& layer : Layers)
	{
		GFXCHECK(pDevice->CreatePixelShader(PSS::Default.GetByteCode(), PSS::Default.GetByteCodeSize(), nullptr, &layer.pPShader));
		GFXCHECK(pDevice->CreateVertexShader(VSS::Default.GetByteCode(), VSS::Default.GetByteCodeSize(), nullptr, &layer.pVShader));
		D3D11_TEXTURE2D_DESC td = {};
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.ArraySize = 1u;
		td.MipLevels = 1u;
		td.Usage = D3D11_USAGE_DYNAMIC;
		td.SampleDesc.Count = 1u;
		td.SampleDesc.Quality = 0u;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		td.Width = layer.width;
		td.Height = layer.height;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = layer.pixelMap.data();
		sd.SysMemPitch = layer.nImagePitchBytes;
		GFXCHECK(pDevice->CreateTexture2D(&td, &sd, &layer.pPixelMap));
		D3D11_SHADER_RESOURCE_VIEW_DESC vd = {};
		vd.Format = td.Format;
		vd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		vd.Texture2D.MipLevels = 1u;
		vd.Texture2D.MostDetailedMip = 0u;
		GFXCHECK(pDevice->CreateShaderResourceView(layer.pPixelMap.Get(), &vd, &layer.pPixelMapView));
		D3D11_SAMPLER_DESC smd = {};
		smd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		smd.AddressV = smd.AddressU;
		smd.AddressW = smd.AddressV;
		smd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		GFXCHECK(pDevice->CreateSamplerState(&smd, &layer.pSampler));
		layer.viewport.TopLeftX = 0.0f;
		layer.viewport.TopLeftY = 0.0f;
		layer.viewport.Width = (float)WindowWidth;
		layer.viewport.Height = (float)WindowHeight;
		layer.viewport.MaxDepth = 1.0f;
		layer.viewport.MinDepth = 0.0f;
	}
}

void Graphics::NewFrame()
{
	for (Layer& layer : Layers)
	{
		if (layer.isAutoManaged)
		{
			memset(layer.pixelMap.data(), 0u, layer.nImageBytes);
		}
	}
	pPipeline->ClearRenderTargetView(pFrameBufferView.Get(), &fBackgroundColorRGBA.x);
}

void Graphics::EndFrame() const
{
	for (const Layer& layer : Layers)
	{
#ifdef _DEBUG
		GFXCHECK(pPipeline->Map(layer.pPixelMap.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr));
#else
		pPipeline->Map(layer.pPixelMap.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr);
#endif
		if (layer.renderFlag)
		{
			memcpy(msr.pData, layer.pixelMap.data(), layer.nImageBytes);
			pPipeline->Unmap(layer.pPixelMap.Get(), 0u);
			pPipeline->PSSetShader(layer.pPShader.Get(), nullptr, 0u);
			pPipeline->PSSetConstantBuffers(0u, 1u, layer.pPSCBUF.GetAddressOf());
			pPipeline->VSSetShader(layer.pVShader.Get(), nullptr, 0u);
			pPipeline->VSSetConstantBuffers(0u, 1u, layer.pVSCBUF.GetAddressOf());
			pPipeline->PSSetShaderResources(0u, 1u, layer.pPixelMapView.GetAddressOf());
			pPipeline->PSSetSamplers(0u, 1u, layer.pSampler.GetAddressOf());
			pPipeline->RSSetViewports(1u, &layer.viewport);
			pPipeline->DrawIndexed(6u, 0u, 0);
		}
	}
#ifdef _DEBUG
	HRESULT hr;
	if (FAILED(hr = pFrameManager->Present(1u, 0u)))
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFXEXCPT(pDevice->GetDeviceRemovedReason());
		}
		else
		{
			throw GFXEXCPT("Failed to present new frame to main window! Fatal application error!");
		}
	}
#else
	pFrameManager->Present(1u, 0u);
#endif
}

const bool& Graphics::isAutoManaged(unsigned int layer) const
{
	return Layers[layer].isAutoManaged;
}

void Graphics::AutoManage(unsigned int layer)
{
	Layers[layer].isAutoManaged = true;
}

void Graphics::ManuallyManage(unsigned int layer)
{
	Layers[layer].isAutoManaged = false;
}

void Graphics::Erase(unsigned int layer)
{
	memset(Layers[layer].pixelMap.data(), 0u, Layers[layer].nImageBytes);
}

const bool& Graphics::isBeingRendered(unsigned int layer) const
{
	return Layers[layer].renderFlag;
}

void Graphics::StartRendering(unsigned int layer)
{
	Layers[layer].renderFlag = true;
}

void Graphics::StopRendering(unsigned int layer)
{
	Layers[layer].renderFlag = false;
}

void Graphics::EnableBilinearFiltering(unsigned int layer)
{
	D3D11_SAMPLER_DESC smd = {};
	smd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	smd.AddressV = smd.AddressU;
	smd.AddressW = smd.AddressV;
	smd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	GFXCHECK(pDevice->CreateSamplerState(&smd, &Layers[layer].pSampler));
}

void Graphics::DisableBilinearFiltering(unsigned int layer)
{
	D3D11_SAMPLER_DESC smd = {};
	smd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	smd.AddressV = smd.AddressU;
	smd.AddressW = smd.AddressV;
	smd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	GFXCHECK(pDevice->CreateSamplerState(&smd, &Layers[layer].pSampler));
}

void Graphics::SetPixelShader(const Shader& shader, unsigned int layer)
{
	GFXCHECK(pDevice->CreatePixelShader(shader.GetByteCode(), shader.GetByteCodeSize(), nullptr, &Layers[layer].pPShader));
}

void Graphics::SetVertexShader(const Shader& shader, unsigned int layer)
{
	GFXCHECK(pDevice->CreateVertexShader(shader.GetByteCode(), shader.GetByteCodeSize(), nullptr, &Layers[layer].pVShader));
}

void Graphics::SetPixel(unsigned int x, unsigned int y, Color color, unsigned int layer)
{
	Layer& L = Layers[layer];
	assert(x < L.width && y < L.height);
	const unsigned int pxl = y * L.width + x;
	L.pixelMap[pxl] = color;
}

const Color& Graphics::GetPixel(unsigned int x, unsigned int y, unsigned int layer) const
{
	const Layer& L = Layers[layer];
	assert(x < L.width && y < L.height);
	const unsigned int pxl = y * L.width + x;
	return L.pixelMap[pxl];
}

void Graphics::DrawLine(vec2i p0, vec2i p1, const Color& color, unsigned int layer)
{
	assert(p0 != p1);
	const iRect gfxRect = GetRect(layer);
	if (p0.x == p1.x)
	{
		int deltaY = 1;
		if (p1.y < p0.y)
		{
			deltaY = -1;
		}
		for (int y = p0.y; y != p1.y; y += deltaY)
		{
			if (gfxRect.ContainsPoint({ p0.x,y }))
			{
				SetPixel(p0.x, y, color, layer);
			}
		}
	}
	else if (p0.y == p1.y)
	{
		int deltaX = 1;
		if (p1.x < p0.x)
		{
			deltaX = -1;
		}
		for (int x = p0.x; x != p1.x; x += deltaX)
		{
			if (gfxRect.ContainsPoint({ x,p0.y }))
			{
				SetPixel(x, p0.y, color, layer);
			}
		}
	}
	else
	{
		int rise = p1.y - p0.y;
		int run = p1.x - p0.x;
		if (abs(run) >= abs(rise))
		{
			int deltaX = 1;
			if (run < 0)
			{
				deltaX = -1;
			}
			float m = (float)rise / (float)run;
			for (int x = p0.x; x != p1.x; x += deltaX)
			{
				int y = int(m * float(x - p0.x)) + p0.y;
				if (gfxRect.ContainsPoint({ x,y }))
				{
					SetPixel(x, y, color, layer);
				}
			}
		}
		else
		{
			int deltaY = 1;
			if (rise < 0)
			{
				deltaY = -1;
			}
			float m_inv = (float)run / (float)rise;
			for (int y = p0.y; y != p1.y; y += deltaY)
			{
				int x = int(m_inv * float(y - p0.y)) + p0.x;
				if (gfxRect.ContainsPoint({ x,y }))
				{
					SetPixel(x, y, color, layer);
				}
			}
		}
	}
}

void Graphics::DrawLine(vec2i p0, vec2i p1, std::function<Color(int, int)> color_func, unsigned int layer)
{
	assert(p0 != p1);
	const iRect gfxRect = GetRect(layer);
	if (p0.x == p1.x)
	{
		int deltaY = 1;
		if (p1.y < p0.y)
		{
			deltaY = -1;
		}
		for (int y = p0.y; y != p1.y; y += deltaY)
		{
			if (gfxRect.ContainsPoint({ p0.x,y }))
			{
				SetPixel(p0.x, y, color_func(p0.x, y), layer);
			}
		}
	}
	else if (p0.y == p1.y)
	{
		int deltaX = 1;
		if (p1.x < p0.x)
		{
			deltaX = -1;
		}
		for (int x = p0.x; x != p1.x; x += deltaX)
		{
			if (gfxRect.ContainsPoint({ x,p0.y }))
			{
				SetPixel(x, p0.y, color_func(x, p0.y), layer);
			}
		}
	}
	else
	{
		int rise = p1.y - p0.y;
		int run = p1.x - p0.x;
		if (abs(run) >= abs(rise))
		{
			int deltaX = 1;
			if (run < 0)
			{
				deltaX = -1;
			}
			float m = (float)rise / (float)run;
			for (int x = p0.x; x != p1.x; x += deltaX)
			{
				int y = int(m * float(x - p0.x)) + p0.y;
				if (gfxRect.ContainsPoint({ x,y }))
				{
					SetPixel(x, y, color_func(x, y), layer);
				}
			}
		}
		else
		{
			int deltaY = 1;
			if (rise < 0)
			{
				deltaY = -1;
			}
			float m_inv = (float)run / (float)rise;
			for (int y = p0.y; y != p1.y; y += deltaY)
			{
				int x = int(m_inv * float(y - p0.y)) + p0.x;
				if (gfxRect.ContainsPoint({ x,y }))
				{
					SetPixel(x, y, color_func(x, y), layer);
				}
			}
		}
	}
}

void Graphics::DrawCircle(int x, int y, int r, const Color& color, unsigned int layer)
{
	assert(r > 0);
	const int d = r * 2;
	assert(x + d >= 0 && y + d >= 0);
	const iRect gfxRect = GetRect(layer);
	vec2i center{ x + r,y + r };
	int rSQ = sq(r);
	for (int ly = y; ly < y + d; ++ly)
	{
		for (int lx = x; lx < x + d; ++lx)
		{
			const vec2i point{ lx,ly };
			if ((point - center).LengthSq() <= rSQ && gfxRect.ContainsPoint(point))
			{
				SetPixel(lx, ly, color, layer);
			}
		}
	}
}

void Graphics::DrawCircle(int x, int y, int r, std::function<Color(int, int)> color_func, unsigned int layer)
{
	assert(r > 0);
	const int d = r * 2;
	assert(x + d >= 0 && y + d >= 0);
	const iRect gfxRect = GetRect(layer);
	vec2i center{ x + r,y + r };
	int rSQ = sq(r);
	for (int ly = y; ly < y + d; ++ly)
	{
		for (int lx = x; lx < x + d; ++lx)
		{
			const vec2i point{ lx,ly };
			if ((point - center).LengthSq() <= rSQ && gfxRect.ContainsPoint(point))
			{
				SetPixel(lx, ly, color_func(lx, ly), layer);
			}
		}
	}
}

const unsigned int& Graphics::GetWidth(unsigned int layer) const
{
	return Layers[layer].width;
}

const unsigned int& Graphics::GetHeight(unsigned int layer) const
{
	return Layers[layer].height;
}

unsigned int Graphics::GetViewWidth(unsigned int layer) const
{
	return (unsigned int)Layers[layer].viewport.Width;
}

unsigned int Graphics::GetViewHeight(unsigned int layer) const
{
	return (unsigned int)Layers[layer].viewport.Height;
}

vec2u Graphics::GetDimensions(unsigned int layer) const
{
	const Layer& L = Layers[layer];
	return vec2u(L.width, L.height);
}

vec2u Graphics::GetViewDimensions(unsigned int layer) const
{
	const D3D11_VIEWPORT& vp = Layers[layer].viewport;
	return vec2u((unsigned int)vp.Width, (unsigned int)vp.Height);
}

iRect Graphics::GetRect(unsigned int layer) const
{
	const Layer& L = Layers[layer];
	return iRect({ 0,0 }, (int)L.width, (int)L.height);
}

iRect Graphics::GetViewRect(unsigned int layer) const
{
	const D3D11_VIEWPORT& vp = Layers[layer].viewport;
	return iRect({ (int)vp.TopLeftX,(int)vp.TopLeftY }, (int)vp.Width, (int)vp.Height);
}

float Graphics::GetWidth_FLOAT(unsigned int layer) const
{
	return (float)Layers[layer].width;
}

float Graphics::GetHeight_FLOAT(unsigned int layer) const
{
	return (float)Layers[layer].height;
}

const float& Graphics::GetViewWidth_FLOAT(unsigned int layer) const
{
	return Layers[layer].viewport.Width;
}

const float& Graphics::GetViewHeight_FLOAT(unsigned int layer) const
{
	return Layers[layer].viewport.Height;
}

vec2 Graphics::GetDimensions_FLOAT(unsigned int layer) const
{
	const Layer& L = Layers[layer];
	return vec2((float)L.width, (float)L.height);
}

vec2 Graphics::GetViewDimensions_FLOAT(unsigned int layer) const
{
	const D3D11_VIEWPORT& vp = Layers[layer].viewport;
	return vec2(vp.Width, vp.Height);
}

fRect Graphics::GetRect_FLOAT(unsigned int layer) const
{
	const Layer& L = Layers[layer];
	return fRect({ 0.0f,0.0f }, (float)L.width, (float)L.height);
}

fRect Graphics::GetViewRect_FLOAT(unsigned int layer) const
{
	const D3D11_VIEWPORT& vp = Layers[layer].viewport;
	return fRect({ vp.TopLeftX,vp.TopLeftY }, vp.Width, vp.Height);
}

float Graphics::GetAspectRatio(unsigned int layer) const
{
	const Layer& L = Layers[layer];
	return (float)L.width / (float)L.height;
}

float Graphics::GetViewAspectRatio(unsigned int layer) const
{
	const D3D11_VIEWPORT& vp = Layers[layer].viewport;
	return vp.Width / vp.Height;
}

float Graphics::GetInvAspectRatio(unsigned int layer) const
{
	const Layer& L = Layers[layer];
	return (float)L.height / (float)L.width;
}

float Graphics::GetInvViewAspectRatio(unsigned int layer) const
{
	const D3D11_VIEWPORT& vp = Layers[layer].viewport;
	return vp.Height / vp.Width;
}

const unsigned int& Graphics::GetPixelCount(unsigned int layer) const
{
	return Layers[layer].nPixels;
}

const unsigned int& Graphics::GetSizeInBytes(unsigned int layer) const
{
	return Layers[layer].nImageBytes;
}

const std::vector<Color>& Graphics::GetPixelMap(unsigned int layer) const
{
	return Layers[layer].pixelMap;
}

std::vector<Color>& Graphics::GetPixelMap(unsigned int layer)
{
	return Layers[layer].pixelMap;
}

void Graphics::SetViewport(int x, int y, unsigned int width, unsigned int height, unsigned int layer)
{
	Layers[layer].viewport.TopLeftX = (float)x;
	Layers[layer].viewport.TopLeftY = (float)y;
	Layers[layer].viewport.Width = (float)width;
	Layers[layer].viewport.Height = (float)height;
}

void Graphics::SetBackgroundColor(const Color& color)
{
	fBackgroundColorRGBA = color.GetStruct();
}

Color Graphics::GetBackgroundColor() const
{
	return Color(fBackgroundColorRGBA);
}

void Graphics::Move(vec2 delta, unsigned int layer)
{
	Layers[layer].position += delta * 2.0f / GetDimensions_FLOAT(layer);
}

void Graphics::SetPosition(vec2 pos, unsigned int layer)
{
	Layers[layer].position = pos * 2.0f / GetDimensions_FLOAT(layer);
}

vec2 Graphics::GetPosition(unsigned int layer) const
{
	return Layers[layer].position * GetDimensions_FLOAT(layer) / 2.0f;
}

void Graphics::Rotate(float radians, unsigned int layer)
{
	Layers[layer].rotation += radians;
}

void Graphics::SetRotation(float radians, unsigned int layer)
{
	Layers[layer].rotation = radians;
}

const float& Graphics::GetRotation(unsigned int layer) const
{
	return Layers[layer].rotation;
}

void Graphics::Scale(vec2 scalar, unsigned int layer)
{
	assert(scalar.x > 0.0f && scalar.y > 0.0f);
	Layers[layer].scale *= scalar;
}

void Graphics::SetScale(vec2 scale, unsigned int layer)
{
	assert(scale.x > 0.0f && scale.y > 0.0f);
	Layers[layer].scale = scale;
}

const vec2& Graphics::GetScale(unsigned int layer) const
{
	return Layers[layer].scale;
}

mat4 Graphics::GetTransformationMatrix(unsigned int layer) const
{
	return
		mat4(mat3::RotationZ(Layers[layer].rotation)) *
		mat4::Scaling(Layers[layer].scale.x, Layers[layer].scale.y, 1.0f, 1.0f) *
		mat4::Translation(Layers[layer].position.x, Layers[layer].position.y, 0.0f);
}

mat4 Graphics::GetPreTransformMatrix(unsigned int layer) const
{
	return mat4::Scaling(GetAspectRatio(layer), 1.0f, 1.0f, 1.0f);
}

mat4 Graphics::GetPostTransformMatrix(unsigned int layer) const
{
	return mat4::Scaling(GetInvViewAspectRatio(layer), 1.0f, 1.0f, 1.0f);
}

mat4 Graphics::GetAspectCorrectionMatrix(unsigned int layer) const
{
	return mat4::Scaling(GetAspectRatio(layer) * GetInvViewAspectRatio(layer), 1.0f, 1.0f, 1.0f);
}

mat3 Graphics::GetWorldToPixelMapTransformMatrix(unsigned int layer) const
{
	return
		mat3::Scaling(1.0f, -1.0f, 1.0f) *
		mat3::Translation((float)Layers[layer].width / 2.0f, (float)Layers[layer].height / 2.0f);
}

mat3 Graphics::GetPixelMapToWorldTransformMatrix(unsigned int layer) const
{
	return
		mat3::Translation(-(float)Layers[layer].width / 2.0f, -(float)Layers[layer].height / 2.0f) *
		mat3::Scaling(1.0f, -1.0f, 1.0f);
}



