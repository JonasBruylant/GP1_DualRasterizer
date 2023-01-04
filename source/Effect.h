#pragma once
#include "pch.h"
#include "Math.h"
#include "Texture.h"

namespace dae
{


	class Effect
	{
	public:
		enum class currentTechnique
		{
			Point, //0
			Linear, //1
			Anisotropic, //2

			END
		};

		Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
		{
			m_pEffect = LoadEffect(pDevice, assetFile);

			//Load in techniques
			m_pPointTechnique = m_pEffect->GetTechniqueByName("PointTechnique");
			if (!m_pPointTechnique->IsValid())
				std::wcout << L"PointTechnique not valid \n";

			m_pLinearTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
			if (!m_pLinearTechnique->IsValid())
				std::wcout << L"LinearTechnique not valid \n";

			m_pAnisotropicTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
			if (!m_pAnisotropicTechnique->IsValid())
				std::wcout << L"AnisotropicTechnique not valid \n";



			//Load in Matrix
			m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldviewProj")->AsMatrix();
			if(!m_pMatWorldViewProjVariable->IsValid())
				std::wcout << L"m_pMatWorldViewProjVariable not valid \n";
		

			m_pWorldMatrixVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
			if (!m_pMatWorldViewProjVariable->IsValid())
				std::wcout << L"m_pWorldMatrixVariable not valid \n";


			m_pInvViewMatrixVariable = m_pEffect->GetVariableByName("gInvViewMatrix")->AsMatrix();
			if (!m_pMatWorldViewProjVariable->IsValid())
				std::wcout << L"m_pInvViewMatrixVariable not valid \n";



			//Load in Texture maps
			m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
			if (!m_pDiffuseMapVariable->IsValid())
				std::wcout << L"m_pDiffuseMapVariable is not valid \n";

			m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
			if (!m_pDiffuseMapVariable->IsValid())
				std::wcout << L"m_pGlossinessMapVariable is not valid \n";

			m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
			if (!m_pDiffuseMapVariable->IsValid())
				std::wcout << L"m_pNormalMapVariable is not valid \n";


			m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
			if (!m_pDiffuseMapVariable->IsValid())
				std::wcout << L"m_pSpecularMapVariable is not valid \n";
		}

		~Effect()
		{

			m_pGlossinessMapVariable->Release();
			m_pSpecularMapVariable->Release();
			m_pNormalMapVariable->Release();
			m_pInvViewMatrixVariable->Release();
			m_pWorldMatrixVariable->Release();


			m_pDiffuseMapVariable->Release();
			m_pMatWorldViewProjVariable->Release();
			m_pPointTechnique->Release();
			m_pLinearTechnique->Release();
			m_pAnisotropicTechnique->Release();
			m_pEffect->Release();

		}

		ID3DX11EffectTechnique* GetTechnique() 
		{ 
			switch (m_CurrentTechnique)
			{
			case dae::Effect::currentTechnique::Point:
				return m_pPointTechnique;
			case dae::Effect::currentTechnique::Linear:
				return m_pLinearTechnique;
			case dae::Effect::currentTechnique::Anisotropic:
				return m_pAnisotropicTechnique;
			}
		}

		void SwitchCurrentTechnique()
		{	
			m_CurrentTechnique = static_cast<currentTechnique>((static_cast<int>(m_CurrentTechnique) + 1) % static_cast<int>(currentTechnique::END)); 
			switch (m_CurrentTechnique)
			{
			case dae::Effect::currentTechnique::Point:
				std::cout << "Current Technique: PointTechnique \n";
				break;
			case dae::Effect::currentTechnique::Linear:
				std::cout << "Current Technique: LinearTechnique \n";
				break;
			case dae::Effect::currentTechnique::Anisotropic:
				std::cout << "Current Technique: AnisotropicTechnique \n";
				break;
			}
		}
		ID3DX11Effect* GetEffect() { return m_pEffect; }
		ID3DX11EffectMatrixVariable* GetWVPMatrix() { return m_pMatWorldViewProjVariable; }

		void SetWorldViewProjMatrixData(Matrix worldViewProjectionMatrix) { m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix)); }
		void SetWorldMatrixData(Matrix worldMatrix) { m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix)); }
		void SetInvViewMatrixData(Matrix invViewMatrix) { m_pInvViewMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&invViewMatrix)); }


		void SetDiffuseMap(Texture* pDiffuseTexture) { if (m_pDiffuseMapVariable) m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV()); }
		void SetNormalMap(Texture* pNormalTexture) { if (m_pNormalMapVariable) m_pNormalMapVariable->SetResource(pNormalTexture->GetSRV()); }
		void SetSpecularMap(Texture* pSpecularTexture) { if (m_pSpecularMapVariable) m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV()); }
		void SetGlossinessMap(Texture* pGlossinessTexture) { if (m_pGlossinessMapVariable) m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetSRV()); }


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
			//pErrorBlob->Release();
			return pEffect;
		}

	private:
		ID3DX11Effect* m_pEffect;

		ID3DX11EffectTechnique* m_pPointTechnique;
		ID3DX11EffectTechnique* m_pLinearTechnique;
		ID3DX11EffectTechnique* m_pAnisotropicTechnique;

		currentTechnique m_CurrentTechnique;

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
		ID3DX11EffectMatrixVariable* m_pWorldMatrixVariable;
		ID3DX11EffectMatrixVariable* m_pInvViewMatrixVariable;

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
	};
}

