#pragma once
#include "pch.h"
#include "Effect.h"

namespace dae 
{
	class TransparancyEffect final : public Effect
	{
	public:
		TransparancyEffect(ID3D11Device* pDevice, const std::wstring& assetFile) : Effect(pDevice, assetFile)
		{

		}

	};

}