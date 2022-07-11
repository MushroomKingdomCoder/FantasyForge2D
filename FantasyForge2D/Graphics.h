#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "Win32Includes.h"
#include "BaseException.h"
#include "Shaders.h"
#include "Color.h"
#include <optional>
#include <vector>
#include <functional>

template <typename type>
class Rect;
using fRect = Rect<float>;
using iRect = Rect<int>;
using uRect = Rect<unsigned int>;

class Graphics
{
public:
	class Exception : public BaseException
	{
	private:
		std::optional<HRESULT> hr;
	private:
		std::string GetErrorCodeString() const noexcept
		{
			if (!hr)
			{
				return "N/A";
			}
			else
			{
				std::ostringstream err;
				err << *hr;
				return err.str();
			}
		}
		std::string GetDescriptionString() const noexcept
		{
			if (!hr)
			{
				return "N/A";
			}
			else
			{
				return GetErrorCodeDesc(*hr);
			}
		}
	public:
		Exception() = delete;
		Exception(int line, std::string file, std::string note) noexcept
			:
			BaseException(line, file, note),
			hr()
		{}
		Exception(int line, std::string file, HRESULT hr, std::string note = "") noexcept
			:
			BaseException(line, file, note),
			hr(hr)
		{}
		const char* what() const noexcept override
		{
			std::ostringstream wht;
			wht << "Exception Type: " << GetType() << std::endl
				<< "Error Code: " << GetErrorCodeString() << std::endl
				<< "Description: " << GetDescriptionString() << std::endl
				<< "File Name: " << GetFile() << std::endl
				<< "Line Number: " << GetLine() << std::endl
				<< "Additional Info: " << GetNote() << std::endl;
			whatBuffer = wht.str();
			return whatBuffer.c_str();
		}
		const char* GetType() const noexcept override
		{
			return "FantasyForge Direct3D 11 Exception";
		}
	};
	#define GFXEXCPT(hr_or_note) Graphics::Exception(__LINE__, __FILE__, hr_or_note)
	#define GFXEXCPT_NOTE(hr, note) Graphics::Exception(__LINE__, __FILE__, hr, note)
	#define GFXCHECK(hr) if (FAILED(hr)) { throw GFXEXCPT(hr); }
private:
	struct Layer
	{
		friend class Graphics;
	private:
		bool isAutoManaged;
		bool renderFlag;
		const unsigned int width;
		const unsigned int height;
		const unsigned int nPixels;
		const unsigned int nImageBytes;
		const unsigned int nImagePitchBytes;
		std::vector<Color> pixelMap;
		D3D11_VIEWPORT viewport;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVShader;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pPixelMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pPixelMapView;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pPSCBUF;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pVSCBUF;
	private:
		vec2 position;
		float rotation;
		vec2 scale;
	public:
		Layer() = delete;
		Layer(unsigned int width, unsigned int height)
			:
			isAutoManaged(true),
			renderFlag(true),
			width(width),
			height(height),
			nPixels(width* height),
			nImageBytes(nPixels * sizeof(Color)),
			nImagePitchBytes(width * sizeof(Color)),
			position(0.0f, 0.0f),
			rotation(0.0f),
			scale(1.0f, 1.0f),
			pixelMap(),
			viewport()
		{
			pixelMap.resize(nPixels, Colors::Transparent);
		}
	};
private:
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pPipeline = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pFrameManager = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pFrameBufferView = nullptr;
	mutable D3D11_MAPPED_SUBRESOURCE msr = {};
	std::vector<Layer> Layers;
	float4 fBackgroundColorRGBA = { 0.0f,0.0f,0.0f,1.0f };
public:
	Graphics() = delete;
	Graphics(const Graphics& gfx) = delete;
	Graphics operator =(const Graphics& gfx) = delete;
	Graphics(HWND hWnd, unsigned int WindowWidth, unsigned int WindowHeight, std::vector<uint2> display_layer_dims);
	void NewFrame();
	void EndFrame() const;
	const bool& isAutoManaged(unsigned int layer = 0u) const;
	void AutoManage(unsigned int layer = 0u);
	void ManuallyManage(unsigned int layer = 0u);
	void Erase(unsigned int layer = 0u);
	const bool& isBeingRendered(unsigned int layer = 0u) const;
	void StartRendering(unsigned int layer = 0u);
	void StopRendering(unsigned int layer = 0u);
	void EnableBilinearFiltering(unsigned int layer = 0u);
	void DisableBilinearFiltering(unsigned int layer = 0u);
	void SetPixelShader(const Shader& shader, unsigned int layer = 0u);
	void SetVertexShader(const Shader& shader, unsigned int layer = 0u);
	template <typename cbuffer>
	void CreatePSConstantBuffer(const cbuffer& cbuf, unsigned int layer = 0u)
	{
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(cbuffer);
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0u;
		bd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = &cbuf;
		GFXCHECK(pDevice->CreateBuffer(&bd, &sd, &Layers[layer].pPSCBUF));
	}
	template <typename cbuffer>
	void UpdatePSConstantBuffer(const cbuffer& cbuf, unsigned int layer = 0u) const
	{
		GFXCHECK(pPipeline->Map(Layers[layer].pPSCBUF.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr));
		memcpy(msr.pData, &cbuf, sizeof(cbuffer));
		pPipeline->Unmap(Layers[layer].pPSCBUF.Get(), 0u);
	}
	template <typename cbuffer>
	void CreateVSConstantBuffer(const cbuffer& cbuf, unsigned int layer = 0u)
	{
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(cbuffer);
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0u;
		bd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = &cbuf;
		GFXCHECK(pDevice->CreateBuffer(&bd, &sd, &Layers[layer].pVSCBUF));
	}
	template <typename cbuffer>
	void UpdateVSConstantBuffer(const cbuffer& cbuf, unsigned int layer = 0u) const
	{
		GFXCHECK(pPipeline->Map(Layers[layer].pVSCBUF.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr));
		memcpy(msr.pData, &cbuf, sizeof(cbuffer));
		pPipeline->Unmap(Layers[layer].pVSCBUF.Get(), 0u);
	}
	void SetPixel(unsigned int x, unsigned int y, Color color, unsigned int layer = 0u);
	const Color& GetPixel(unsigned int x, unsigned int y, unsigned int layer = 0u) const;
	void DrawLine(vec2i p0, vec2i p1, const Color& color, unsigned int layer = 0u);
	void DrawLine(vec2i p0, vec2i p1, std::function<Color(int, int)> color_func, unsigned int layer = 0u);
	void DrawCircle(int x, int y, int r, const Color& color, unsigned int layer = 0u);
	void DrawCircle(int x, int y, int r, std::function<Color(int, int)> color_func, unsigned int layer = 0u);
	const unsigned int& GetWidth(unsigned int layer = 0u) const;
	const unsigned int& GetHeight(unsigned int layer = 0u) const;
	unsigned int GetViewWidth(unsigned int layer = 0u) const;
	unsigned int GetViewHeight(unsigned int layer = 0u) const;
	vec2u GetDimensions(unsigned int layer = 0u) const;
	vec2u GetViewDimensions(unsigned int layer = 0u) const;
	iRect GetRect(unsigned int layer = 0u) const;
	iRect GetViewRect(unsigned int layer = 0u) const;
	float GetWidth_FLOAT(unsigned int layer = 0u) const;
	float GetHeight_FLOAT(unsigned int layer = 0u) const;
	const float& GetViewWidth_FLOAT(unsigned int layer = 0u) const;
	const float& GetViewHeight_FLOAT(unsigned int layer = 0u) const;
	vec2 GetDimensions_FLOAT(unsigned int layer = 0u) const;
	vec2 GetViewDimensions_FLOAT(unsigned int layer = 0u) const;
	fRect GetRect_FLOAT(unsigned int layer = 0u) const;
	fRect GetViewRect_FLOAT(unsigned int layer = 0u) const;
	float GetAspectRatio(unsigned int layer = 0u) const;
	float GetViewAspectRatio(unsigned int layer = 0u) const;
	float GetInvAspectRatio(unsigned int layer = 0u) const;
	float GetInvViewAspectRatio(unsigned int layer = 0u) const;
	const unsigned int& GetPixelCount(unsigned int layer = 0u) const;
	const unsigned int& GetSizeInBytes(unsigned int layer = 0u) const;
	const std::vector<Color>& GetPixelMap(unsigned int layer = 0u) const;
	std::vector<Color>& GetPixelMap(unsigned int layer = 0u);
	void SetViewport(int x, int y, unsigned int width, unsigned int height, unsigned int layer = 0u);
	void SetBackgroundColor(const Color& color);
	Color GetBackgroundColor() const;
	void Move(vec2 delta, unsigned int layer = 0u);
	void SetPosition(vec2 pos, unsigned int layer = 0u);
	vec2 GetPosition(unsigned int layer = 0u) const;
	void Rotate(float radians, unsigned int layer = 0u);
	void SetRotation(float radians, unsigned int layer = 0u);
	const float& GetRotation(unsigned int layer = 0u) const;
	void Scale(vec2 scalar, unsigned int layer = 0u);
	void SetScale(vec2 scale, unsigned int layer = 0u);
	const vec2& GetScale(unsigned int layer = 0u) const;
	mat4 GetTransformationMatrix(unsigned int layer = 0u) const;
	mat4 GetPreTransformMatrix(unsigned int layer = 0u) const;
	mat4 GetPostTransformMatrix(unsigned int layer = 0u) const;
	mat4 GetAspectCorrectionMatrix(unsigned int layer = 0u) const;
	mat3 GetWorldToPixelMapTransformMatrix(unsigned int layer = 0u) const;
	mat3 GetPixelMapToWorldTransformMatrix(unsigned int layer = 0u) const;
};

