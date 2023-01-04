#pragma once
#include "pch.h"
#include "Effect.h"

namespace dae
{
	//Structs
	struct Vertex
	{
		Vector3 Position;
		Vector3 Color;
		Vector2 Uv;
		Vector3 Normal;
		Vector3 Tangent;
	};

	class Mesh
	{
	public:


		Mesh(ID3D11Device* pDevice, std::vector<Vertex> vertices, std::vector<uint32_t> indices) : m_NumIndices{indices.size()}
		{
			m_pEffect = new Effect(pDevice, L"Resources/MeshShader.fx");



			//Create Vertex Layout
			static constexpr uint32_t numElements{ 5 };
			D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

			vertexDesc[0].SemanticName = "POSITION";
			vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexDesc[0].AlignedByteOffset = 0;
			vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

			vertexDesc[1].SemanticName = "COLOR";
			vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

			vertexDesc[2].SemanticName = "TEXCOORD";
			vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
			vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

			vertexDesc[3].SemanticName = "NORMAL";
			vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
			vertexDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

			vertexDesc[4].SemanticName = "TANGENT";
			vertexDesc[4].Format = DXGI_FORMAT_R32G32_FLOAT;
			vertexDesc[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

			//Create input Layout
			D3DX11_PASS_DESC passDesc{};
			m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

			HRESULT result = pDevice->CreateInputLayout(
				vertexDesc,
				numElements,
				passDesc.pIAInputSignature,
				passDesc.IAInputSignatureSize,
				&m_pInputLayout);

			//Create vertex buffer
			D3D11_BUFFER_DESC bd = {};
			bd.Usage = D3D11_USAGE_IMMUTABLE;
			bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;
			bd.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = vertices.data();

			result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
			if (FAILED(result))
				return;

			//Create index buffer 
			m_NumIndices = static_cast<uint32_t>(indices.size());
			bd.Usage = D3D11_USAGE_IMMUTABLE;
			bd.ByteWidth = UINT(sizeof(uint32_t) * m_NumIndices);
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;
			bd.MiscFlags = 0;
			initData.pSysMem = indices.data();
			result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
			if (FAILED(result))
				return;
		}

		~Mesh()
		{

			delete m_pEffect;
			m_pEffect = nullptr;
			if (m_pIndexBuffer != nullptr)
			{
				m_pIndexBuffer->Release();
			}

			if (m_pVertexBuffer != nullptr)
			{
				m_pVertexBuffer->Release();
			}
			if (m_pInputLayout != nullptr)
			{
				m_pInputLayout->Release();
			}
		}

		void Render(ID3D11DeviceContext* pDeviceContext, Matrix worldViewProjectionMatrix, Matrix invViewMatrix)
		{
			m_pEffect->SetWorldViewProjMatrixData(worldViewProjectionMatrix);
			m_pEffect->SetWorldMatrixData(m_WorldMatrix);
			m_pEffect->SetInvViewMatrixData(invViewMatrix);


			//1. Set Primitive Topology
			pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			//2. Set Input Layout
			pDeviceContext->IASetInputLayout(m_pInputLayout);
		
			//3. Set VertexBuffer
			constexpr UINT stride = sizeof(Vertex);
			constexpr UINT offset = 0;
			pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
				
			//4. Set IndexBuffer
			pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);


			//5. Draw
			D3DX11_TECHNIQUE_DESC techDesc;
			m_pEffect->GetTechnique()->GetDesc(&techDesc);
			for (UINT p = 0; p < techDesc.Passes; ++p)
			{
				m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
				pDeviceContext->DrawIndexed(UINT(m_NumIndices), UINT(0), INT(0));
			}
		}

		void RotateMesh(float rotationSpeed)
		{
			m_WorldMatrix = Matrix::CreateRotationY(rotationSpeed) * m_WorldMatrix;
		}

		Effect* GetEffect() const { return m_pEffect; }
		Matrix GetWorldMatrix() const { return m_WorldMatrix; }
		void SetWorldMatrix(Matrix wMatrix) { m_WorldMatrix = wMatrix; }
	private:
		ID3D11InputLayout* m_pInputLayout;
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		size_t m_NumIndices;
		Effect* m_pEffect;
		Matrix m_WorldMatrix;
	};
}