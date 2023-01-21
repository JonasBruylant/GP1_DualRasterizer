#pragma once
#include "Math.h"
#include "Mesh.h"
#include "Camera.h"
#include "Utils.h"
#include "MeshShaderEffect.h"
#include "TransparancyEffect.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{

	enum class SystemMode
	{
		Hardware,
		Software,

		END
	};

	enum class RenderMode
	{
		Texture,
		DepthBuffer,

		END
	};

	enum class ColorMode
	{
		observedArea,
		Diffuse,
		Specular,
		Combined,

		END
	};

	enum class CullFaceMode
	{
		Front,
		Back,
		None,

		END
	};

	
	class Renderer final
	{


	public:

		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render();
		void SoftWareRender();
		void HardwareRender();
		void InitMesh(); 
		void InitCamera();
		void InitTexture();


		void SwitchTechnique();
		void SwitchRenderMode();
		void SwitchColorMode();

		void ToggleNormals();
		void ToggleRotation();
		void ToggleSystemMode();
		void ToggleCullFaceMode();
		void ToggleUniformClearColor();
		void ToggleFireMesh();
		void ToggleBoundingBoxVisualisation();

		SystemMode GetSystemMode() { return m_CurrentSystemMode; }

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		float m_AspectRatio;

		bool m_IsInitialized{ false };
		bool m_IsRotating{ true };
		bool m_UseNormals{ true };
		bool m_ShowFireMesh{ true };
		bool m_IsClearColorToggled{ false };
		bool m_ShowBoundingBox{ false };
		//DIRECTX
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;
		ID3D11RasterizerState* m_pRasterizerState;

		//Objects
		Mesh* m_pVehicleMesh;
		Mesh* m_pFireMesh;
		Camera* m_pCamera;

		//Modes
		RenderMode m_CurrentRenderMode;
		ColorMode m_CurrentColorMode;
		SystemMode m_CurrentSystemMode;
		CullFaceMode m_CurrentCullMode;

		//Textures
		Texture* m_pTexture{ nullptr };
		Texture* m_pNormalTexture{ nullptr };
		Texture* m_pGlossinessTexture{ nullptr };
		Texture* m_pSpecularTexture{ nullptr };
		Texture* m_pFireTexture{ nullptr };

		//Software data
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};

		void VertexTransformationFunction(); //W1 Version
		bool IsInsideFrustrum(const Vector4& position);
		void RenderTriangle(int idx0, int idx1, int idx2, std::vector<Vector2>& screenVerticesl, std::vector<Vertex_Out>& vertices_out, const std::vector<uint32_t>& indices);
		ColorRGB PixelShading(const Vertex_Out& vertex_out);
		ColorRGB Lambert(float kd, const ColorRGB& cd);
		ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n);





		//SDL_Surface* m_pFrontBuffer{ nullptr };
		//SDL_Surface* m_pBackBuffer{ nullptr };
		//uint32_t* m_pBackBufferPixels{};
		//
		//float* m_pDepthBufferPixels{};
		//
		//void VertexTransformationFunction(); //W1 Version
		//bool IsInsideFrustrum(const Vector4& position);
		//void RenderTriangle(int idx0, int idx1, int idx2, std::vector<Vector2>& screenVertices);
		//ColorRGB PixelShading(const Vertex_Out& vertex_out);
		//ColorRGB Lambert(float kd, const ColorRGB& cd);
		//ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n);

		HRESULT InitializeDirectX();
		//...
	};
}
