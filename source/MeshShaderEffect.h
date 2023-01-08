#pragma once
#include "pch.h"
#include "Effect.h"

namespace dae 
{
	class MeshShaderEffect final : public Effect
	{
	public:
		MeshShaderEffect(ID3D11Device* pDevice, const std::wstring& assetFile) : Effect(pDevice, assetFile)
		{

			m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
			if (!m_pGlossinessMapVariable->IsValid())
				std::wcout << L"m_pGlossinessMapVariable is not valid \n";

			m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
			if (!m_pNormalMapVariable->IsValid())
				std::wcout << L"m_pNormalMapVariable is not valid \n";

			m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
			if (!m_pSpecularMapVariable->IsValid())
				std::wcout << L"m_pSpecularMapVariable is not valid \n";


			m_pWorldMatrixVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
			if (!m_pMatWorldViewProjVariable->IsValid())
				std::wcout << L"m_pWorldMatrixVariable not valid \n";


			m_pInvViewMatrixVariable = m_pEffect->GetVariableByName("gInvViewMatrix")->AsMatrix();
			if (!m_pMatWorldViewProjVariable->IsValid())
				std::wcout << L"m_pInvViewMatrixVariable not valid \n";
		}

		void SetNormalMap(Texture* pNormalTexture) { if (m_pNormalMapVariable) m_pNormalMapVariable->SetResource(pNormalTexture->GetSRV()); }
		void SetSpecularMap(Texture* pSpecularTexture) { if (m_pSpecularMapVariable) m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV()); }
		void SetGlossinessMap(Texture* pGlossinessTexture) { if (m_pGlossinessMapVariable) m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetSRV()); }
	private:
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;

	};
}