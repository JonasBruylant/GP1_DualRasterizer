#pragma once
#include "pch.h"
#include "Math.h"

class Effect
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		m_pEffect = LoadEffect(pDevice, assetFile);

		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		if (!m_pTechnique->IsValid())
			std::wcout << L"Technique not valid \n";
	}
	~Effect()
	{
		m_pTechnique->Release();
		m_pEffect->Release();
	}

	ID3DX11EffectTechnique* GetTechnique() { return m_pTechnique; }
	ID3DX11Effect* GetEffect() { return m_pEffect; }

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
					ss << pErrors[i];

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile! \nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;
				return nullptr;
			}
		}

		return pEffect;
	}
private:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
};

