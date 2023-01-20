#pragma once
#include "pch.h"


namespace dae
{
	class Texture
	{
	public:
		Texture(SDL_Surface* pSurface):
			m_pSurface{pSurface},
			m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
		{
		}

		~Texture()
		{
			if (m_pSurface)
			{
				SDL_FreeSurface(m_pSurface);
				m_pSurface = nullptr;

				if (m_pSRV != nullptr)
				{
					m_pSRV->Release();
					m_pSRV = nullptr;
				}

				if (m_pResource != nullptr)
				{
					m_pResource->Release();
					m_pResource = nullptr;
				}

			}
		}

		static Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice)
		{
			SDL_Surface* loadSurface = IMG_Load(path.c_str());

			Texture* returnTexture{ new Texture{ loadSurface } };

			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = loadSurface->w;
			desc.Height = loadSurface->h;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = format;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;


			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = loadSurface->pixels;
			initData.SysMemPitch = static_cast<UINT>(loadSurface->pitch);
			initData.SysMemSlicePitch = static_cast<UINT>(loadSurface->pitch * loadSurface->h);

			HRESULT result = pDevice->CreateTexture2D(&desc, &initData, &returnTexture->m_pResource);

			if (FAILED(result))
			{
				delete returnTexture;
				return nullptr;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
			SRVDesc.Format = format;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = 1;

			result = pDevice->CreateShaderResourceView(returnTexture->m_pResource, &SRVDesc, &returnTexture->m_pSRV);
			if (FAILED(result))
			{
				delete returnTexture;
				return nullptr;
			}

			return returnTexture;
		}
		ColorRGB Sample(const Vector2& uv) const
		{
			//TODO
			//Sample the correct texel for the given uv
			Uint32 x{ Uint32(uv.x * m_pSurface->w) }, y{ Uint32(uv.y * m_pSurface->h) };

			//std::cout << "x: " << uv.x << " y: " << uv.y << "\n";

			uint8_t r, g, b;

			SDL_GetRGB(m_pSurfacePixels[static_cast<uint32_t>(x + (y * m_pSurface->w))],
				m_pSurface->format,
				&r,
				&g,
				&b);

			ColorRGB pixelColor{ r / 255.f, g / 255.f, b / 255.f };
			return pixelColor;
		}
		ID3D11ShaderResourceView* GetSRV() { return m_pSRV; }

	private:
		ID3D11ShaderResourceView* m_pSRV{nullptr}; //If something breaks maybe it was this?
		ID3D11Texture2D* m_pResource{nullptr}; //If something breaks maybe it was this?
		
		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}