#include "pch.h"
#include "Renderer.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
		m_AspectRatio = static_cast<float>(m_Width) / m_Height;

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();

		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";

		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}
		
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];
		m_CurrentSystemMode = SystemMode::Software;

		InitCamera();
		InitTexture();
		InitMesh();
	}

	Renderer::~Renderer()
	{
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		//Mesh
		delete m_pVehicleMesh;
		m_pVehicleMesh = nullptr;

		delete m_pFireMesh;
		m_pFireMesh = nullptr;

		//Object
		delete m_pCamera;
		m_pCamera = nullptr;

		//Texture
		delete m_pTexture;
		m_pTexture = nullptr;

		delete m_pNormalTexture;
		m_pNormalTexture = nullptr;

		delete m_pGlossinessTexture;
		m_pGlossinessTexture = nullptr;

		delete m_pSpecularTexture;
		m_pSpecularTexture = nullptr;

		delete m_pFireTexture;
		m_pFireTexture = nullptr;


		m_pRenderTargetView->Release();
		m_pRenderTargetBuffer->Release();
		m_pDepthStencilView->Release();
		m_pDepthStencilBuffer->Release();
		m_pSwapChain->Release();
		m_pDevice->Release();

		delete[] m_pDepthBufferPixels;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);

		
		if (m_IsRotating)
		{
			const float meshRotation{ 45.0f * pTimer->GetElapsed() * TO_RADIANS };
			m_pVehicleMesh->RotateMesh(meshRotation);
			m_pFireMesh->RotateMesh(meshRotation);
		}
	}

	void Renderer::Render()
	{
		switch (m_CurrentSystemMode)
		{
		case dae::SystemMode::Hardware:
			HardwareRender();
			break;
		case dae::SystemMode::Software:
			SoftWareRender();
			break;

		}
	}

	void Renderer::HardwareRender() 
	{
		if (!m_IsInitialized)
			return;

		m_pCamera->CalculateViewMatrix();

		ColorRGB clearColor{ 135.f / 255.f, 206.f / 255.f, 235.f / 255.f };
		//1. CLEAR RTV & DSV
		if(m_IsClearColorToggled)
			clearColor = { 0.f, 0.f, 0.f };

		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAWCALLS (=RENDER)

		auto worldViewProjectionMatix = m_pVehicleMesh->GetWorldMatrix() * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();
		m_pVehicleMesh->Render(m_pDeviceContext, worldViewProjectionMatix, m_pCamera->GetInvViewMatrix());

		if(m_ShowFireMesh)
			m_pFireMesh->Render(m_pDeviceContext, worldViewProjectionMatix, m_pCamera->GetInvViewMatrix());

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);

	}

	void Renderer::InitCamera()
	{
		m_pCamera = new Camera({ 0.f, 0.f, -10.f }, m_AspectRatio, 45.f);
		m_pCamera->CalculateProjectionMatrix();
	}
	void Renderer::InitTexture()
	{
		m_pTexture = Texture::LoadFromFile("Resources/vehicle_diffuse.png", m_pDevice);
		m_pNormalTexture = Texture::LoadFromFile("Resources/vehicle_normal.png", m_pDevice);
		m_pGlossinessTexture = Texture::LoadFromFile("Resources/vehicle_gloss.png", m_pDevice);
		m_pSpecularTexture = Texture::LoadFromFile("Resources/vehicle_specular.png", m_pDevice);
		m_pFireTexture = Texture::LoadFromFile("Resources/fireFX_diffuse.png", m_pDevice);
	}
	void Renderer::InitMesh()
	{
		//Set up mesh data for vehicle
		std::vector<Vertex> vertices{};
		
		std::vector<uint32_t> indices{};
		MeshShaderEffect* shaderEffect = new MeshShaderEffect(m_pDevice, L"Resources/MeshShader.fx");

		if (m_pTexture)
			shaderEffect->SetDiffuseMap(m_pTexture);

		if (m_pNormalTexture)
			shaderEffect->SetNormalMap(m_pNormalTexture);

		if (m_pGlossinessTexture)
			shaderEffect->SetGlossinessMap(m_pGlossinessTexture);

		if (m_pSpecularTexture)
			shaderEffect->SetSpecularMap(m_pSpecularTexture);
		
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);
		m_pVehicleMesh = new Mesh(m_pDevice, vertices, indices, shaderEffect);
		const Vector3 position{ m_pCamera->GetOrigin() + Vector3{0, 0, 50}};
		const Vector3 rotation{ };
		const Vector3 scale{ Vector3{ 1, 1, 1 } };
		Matrix worldMatrix{ Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(position) };

		m_pVehicleMesh->SetWorldMatrix(worldMatrix);




		//Set up mesh data for Fire
		std::vector<Vertex> fireVertices{};

		std::vector<uint32_t> fireIndices{};
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);
		TransparancyEffect* transparancyEffect = new TransparancyEffect(m_pDevice, L"Resources/Transparancy.fx");
		m_pFireMesh = new Mesh(m_pDevice, vertices, indices, transparancyEffect);

		if (m_pFireTexture)
			transparancyEffect->SetDiffuseMap(m_pFireTexture);

		m_pFireMesh->SetWorldMatrix(worldMatrix);
	}


	void Renderer::SwitchTechnique()
	{
		m_pVehicleMesh->GetEffect()->SwitchCurrentTechnique();
		m_pFireMesh->GetEffect()->SwitchCurrentTechnique();
	}
	void Renderer::SwitchRenderMode()
	{
		m_CurrentRenderMode = static_cast<RenderMode>((static_cast<int>(m_CurrentRenderMode) + 1) % (static_cast<int>(RenderMode::END)));
	}
	void Renderer::SwitchColorMode()
	{
		m_CurrentColorMode = static_cast<ColorMode>((static_cast<int>(m_CurrentColorMode) + 1) % (static_cast<int>(ColorMode::END) ));
	}

	void Renderer::ToggleNormals()
	{
		m_UseNormals = !m_UseNormals;
	}
	void Renderer::ToggleRotation()
	{
		m_IsRotating = !m_IsRotating;
	}
	void Renderer::ToggleSystemMode()
	{
		m_CurrentSystemMode = static_cast<SystemMode>((static_cast<int>(m_CurrentSystemMode) + 1) % (static_cast<int>(SystemMode::END)));
	}
	void Renderer::ToggleCullFaceMode()
	{

	}
	void Renderer::ToggleUniformClearColor()
	{
		m_IsClearColorToggled = !m_IsClearColorToggled;
	}
	void Renderer::TogglePrintFPS()
	{
	}
	void Renderer::ToggleFireMesh()
	{
		m_ShowFireMesh = !m_ShowFireMesh;
	}
	void Renderer::ToggleBoundingBoxVisualisation()
	{
	}


	//========================================================================
	//Software
	//========================================================================

	void Renderer::SoftWareRender()
	{
		//@START
	//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);
		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

		Uint32 clearColor{ 100 };
		if (m_IsClearColorToggled)
			clearColor = 3;
		SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, clearColor, clearColor, clearColor));

		std::vector<Vector2> raster_Vertices;

		VertexTransformationFunction();

		for (Vertex_Out& vertex : m_pVehicleMesh->GetVerticesOut())
		{
			raster_Vertices.push_back(Vector2{ (vertex.position.x + 1) * 0.5f * m_Width, (1 - vertex.position.y) * 0.5f * m_Height });
		}

		std::vector<uint32_t> meshIndeces = m_pVehicleMesh->GetIndices();
		std::vector<Vertex_Out> meshVerticesOut = m_pVehicleMesh->GetVerticesOut();

		switch (m_pVehicleMesh->GetTopology())
		{
		case PrimitiveTopology::TriangleStrip:
			for (size_t i = 0; i + 2 < meshIndeces.size(); ++i)
			{
				int idx0{ static_cast<int>(i) };
				int idx1{ static_cast<int>(i + 1) };
				int idx2{ static_cast<int>(i + 2) };

				if ((i ^ 1) != i + 1)
				{
					int temp = idx1;
					idx1 = idx2;
					idx2 = temp;
				}
				RenderTriangle(idx0, idx1, idx2, raster_Vertices, meshVerticesOut, meshIndeces);

			}

			break;
		case PrimitiveTopology::TriangleList:
			for (int i = 0; i + 2 < meshIndeces.size(); i += 3)
			{
				int idx0{ i };
				int idx1{ i + 1 };
				int idx2{ i + 2 };

				RenderTriangle(idx0, idx1, idx2, raster_Vertices, meshVerticesOut, meshIndeces);
			}
			break;
		}
		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}
	void Renderer::VertexTransformationFunction()
	{
		//Todo > W1 Projection Stage
		m_pVehicleMesh->GetVerticesOut().clear();
		m_pVehicleMesh->GetVerticesOut().reserve(m_pVehicleMesh->GetVertices().size());
		Matrix worldViewProjectMatrix = m_pVehicleMesh->GetWorldMatrix() * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();

		for (Vertex currentVertex : m_pVehicleMesh->GetVertices())
		{
			Vertex_Out vertexOut{ {}, currentVertex.color, currentVertex.uv, currentVertex.normal, currentVertex.tangent, currentVertex.viewDirection };
			vertexOut.position = worldViewProjectMatrix.TransformPoint({ currentVertex.position, 1 });
			currentVertex.position.x = currentVertex.position.x / (m_pCamera->GetFOV() * m_AspectRatio);// / currentVertex.position.z;
			currentVertex.position.y = currentVertex.position.y / m_pCamera->GetFOV(); // / currentVertex.position.z;

			vertexOut.position.x /= vertexOut.position.w;
			vertexOut.position.y /= vertexOut.position.w;
			vertexOut.position.z /= vertexOut.position.w;

			vertexOut.normal = m_pVehicleMesh->GetWorldMatrix().TransformVector(vertexOut.normal).Normalized();
			vertexOut.viewDirection = Vector3{ vertexOut.position.x, vertexOut.position.y, vertexOut.position.z }.Normalized();
			m_pVehicleMesh->GetVerticesOut().push_back(vertexOut);
		}
	}
	bool dae::Renderer::IsInsideFrustrum(const Vector4& position)
	{
		return position.x < -1.f || position.x > 1.f || position.y > 1.f || position.y < -1.f || position.z > 1.0f || position.z < 0.f;
	}

	void dae::Renderer::RenderTriangle(int idx0, int idx1, int idx2, std::vector<Vector2>& screenVertices, 
		std::vector<Vertex_Out>& vertices_out, const std::vector<uint32_t>& indices)
	{

		if (IsInsideFrustrum(vertices_out[indices[idx0]].position) ||
			IsInsideFrustrum(vertices_out[indices[idx1]].position) ||
			IsInsideFrustrum(vertices_out[indices[idx2]].position))
			return;

		Vector2 p0{ screenVertices[indices[idx0]] };
		Vector2 p1{ screenVertices[indices[idx1]] };
		Vector2 p2{ screenVertices[indices[idx2]] };

		Vector2 e0{ p1 - p0 };
		Vector2 e1{ p2 - p1 };
		Vector2 e2{ p0 - p2 };


		float triangleArea = Vector2::Cross(e0, e1);

		Vector2 Min{ Vector2::Min(p0,Vector2::Min(p1,p2)) };
		Vector2 Max{ Vector2::Max(p0,Vector2::Max(p1,p2)) };

		const int startX{ std::clamp(static_cast<int>(Min.x) - 1, 0, m_Width) };
		const int startY{ std::clamp(static_cast<int>(Min.y) - 1, 0, m_Height) };
		const int endX{ std::clamp(static_cast<int>(Max.x) + 1, 0, m_Width) };
		const int endY{ std::clamp(static_cast<int>(Max.y) + 1, 0, m_Height) };

		//RENDER LOGIC
		for (int px{ startX }; px < endX; ++px)
		{
			for (int py{ startY }; py < endY; ++py)
			{


				Vector2 currentPixel{ static_cast<float>(px), static_cast<float>(py) };
				float currPixMin0Crossv0 = Vector2::Cross(e0, currentPixel - p0);
				float currPixMin1Crossv1 = Vector2::Cross(e1, currentPixel - p1);
				float currPixMin2Crossv2 = Vector2::Cross(e2, currentPixel - p2);

				if (!(currPixMin0Crossv0 > 0 && currPixMin1Crossv1 > 0 && currPixMin2Crossv2 > 0))
					continue;

				float weight0 = currPixMin1Crossv1 / triangleArea;
				float weight1 = currPixMin2Crossv2 / triangleArea;
				float weight2 = currPixMin0Crossv0 / triangleArea;

				const float depthZV0{ (vertices_out[indices[idx0]].position.z) };
				const float depthZV1{ (vertices_out[indices[idx1]].position.z) };
				const float depthZV2{ (vertices_out[indices[idx2]].position.z) };
				// Calculate the Z depth at this pixel
				const float interpolatedZDepth
				{
					1.0f /
						(weight0 / depthZV0 +
						weight1 / depthZV1 +
						weight2 / depthZV2)
				};

				int pixelIdx = px + (py * m_Width);
				if (m_pDepthBufferPixels[pixelIdx] < interpolatedZDepth)
					continue;

				m_pDepthBufferPixels[pixelIdx] = interpolatedZDepth;

				switch (m_CurrentRenderMode)
				{
				case dae::RenderMode::Texture:
				{
					//W Depth
					const float depthWV0{ (vertices_out[indices[idx0]].position.w) };
					const float depthWV1{ (vertices_out[indices[idx1]].position.w) };
					const float depthWV2{ (vertices_out[indices[idx2]].position.w) };


					// Calculate the W depth at this pixel
					const float interpolatedWDepth
					{
						1.0f /
							(weight0 / depthWV0 +
							weight1 / depthWV1 +
							weight2 / depthWV2)
					};

					Vertex_Out interpolatedVertex{};

					//UV interpolate
					Vector2 uvInterpolate1{ weight0 * (vertices_out[indices[idx0]].uv / depthWV0) };
					Vector2 uvInterpolate2{ weight1 * (vertices_out[indices[idx1]].uv / depthWV1) };
					Vector2 uvInterpolate3{ weight2 * (vertices_out[indices[idx2]].uv / depthWV2) };

					Vector2 uvInterpolateTotal{ uvInterpolate1 + uvInterpolate2 + uvInterpolate3 };

					Vector2 uvInterpolated{ interpolatedWDepth * uvInterpolateTotal };

					interpolatedVertex.uv = uvInterpolated;

					//Normal interpolate
					Vector3 normalInterpolate1{ weight0 * (vertices_out[indices[idx0]].normal / depthWV0) };
					Vector3 normalInterpolate2{ weight1 * (vertices_out[indices[idx1]].normal / depthWV1) };
					Vector3 normalInterpolate3{ weight2 * (vertices_out[indices[idx2]].normal / depthWV2) };

					Vector3 normalInterpolateTotal{ normalInterpolate1 + normalInterpolate2 + normalInterpolate3 };
					Vector3 normalInterpolated{ interpolatedWDepth * normalInterpolateTotal };

					interpolatedVertex.normal = normalInterpolated.Normalized();

					//Tangent interpolate
					Vector3 tangentInterpolate1{ weight0 * (vertices_out[indices[idx0]].tangent / depthWV0) };
					Vector3 tangentInterpolate2{ weight1 * (vertices_out[indices[idx1]].tangent / depthWV1) };
					Vector3 tangentInterpolate3{ weight2 * (vertices_out[indices[idx2]].tangent / depthWV2) };

					Vector3 tangentInterpolateTotal{ tangentInterpolate1 + tangentInterpolate2 + tangentInterpolate3 };
					Vector3 tangentInterpolated{ interpolatedWDepth * tangentInterpolateTotal };

					interpolatedVertex.tangent = tangentInterpolated.Normalized();

					//viewdirection interpolate
					Vector3 viewDirectionInterpolate1{ weight0 * (vertices_out[indices[idx0]].viewDirection / depthWV0) };
					Vector3 viewDirectionInterpolate2{ weight1 * (vertices_out[indices[idx1]].viewDirection / depthWV1) };
					Vector3 viewDirectionInterpolate3{ weight2 * (vertices_out[indices[idx2]].viewDirection / depthWV2) };

					Vector3 viewDirectionInterpolateTotal{ viewDirectionInterpolate1 + viewDirectionInterpolate2 + viewDirectionInterpolate3 };
					Vector3 viewDirectionInterpolated{ interpolatedWDepth * viewDirectionInterpolateTotal };

					interpolatedVertex.viewDirection = viewDirectionInterpolated.Normalized();


					ColorRGB finalColor{ PixelShading(interpolatedVertex) };

					finalColor.MaxToOne();


					//Update Color in Buffer
					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));

				}
				break;
				case dae::RenderMode::DepthBuffer:
				{
					float depthColor = Utils::Remap(interpolatedZDepth, 0.985f, 1.f);


					ColorRGB finalColor{ depthColor, depthColor, depthColor };


					//Update Color in Buffer
					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
				break;
				}

			}
		}
	}
	ColorRGB dae::Renderer::PixelShading(const Vertex_Out& vertex_out)
	{
		Vector3 pixelNormal{ vertex_out.normal };
		//Normal calculations
		if (m_UseNormals)
		{
			Vector3 binormal = Vector3::Cross(vertex_out.normal, vertex_out.tangent);
			Matrix tangentSpaceAxis = Matrix{ vertex_out.tangent, binormal, vertex_out.normal, Vector3::Zero };
			auto sampledNormal{ m_pNormalTexture->Sample(vertex_out.uv) };

			sampledNormal = (2.f * sampledNormal) - ColorRGB{ 1.f, 1.f, 1.f }; // [0, 1] -> [-1, 1]

			Vector3 sampledNormalVector{ sampledNormal.r, sampledNormal.g, sampledNormal.b };
			pixelNormal = tangentSpaceAxis.TransformVector(sampledNormalVector);
		}

		Vector3 lightDirection = Vector3{ .577f, -.577f , .577f }.Normalized();
		ColorRGB finalColor{ };
		float lightIntensity{ 7.f };
		float glossiness{ 25.f };
		Vector3 ambient{ .025f, .025f, .025f };
		float observedArea = std::max(Vector3::Dot(-lightDirection, pixelNormal), 0.f);


		switch (m_CurrentColorMode)
		{
		case dae::ColorMode::observedArea:
		{

			finalColor += ColorRGB{ observedArea, observedArea, observedArea };
			return finalColor;
			break;
		}
		case dae::ColorMode::Diffuse:
		{

			finalColor = Lambert(lightIntensity, m_pTexture->Sample(vertex_out.uv));
			return finalColor * observedArea;
			break;
		}
		case dae::ColorMode::Specular:
		{
			float exponent{ m_pGlossinessTexture->Sample(vertex_out.uv).r * glossiness };
			finalColor = Phong(1.0f, exponent, -lightDirection, vertex_out.viewDirection, pixelNormal) * m_pSpecularTexture->Sample(vertex_out.uv);
			return finalColor;
			break;
		}
		case dae::ColorMode::Combined:
		{

			const ColorRGB lambert{ 1.0f * m_pTexture->Sample(vertex_out.uv) / PI };
			
			const float phongExponent{ m_pGlossinessTexture->Sample(vertex_out.uv).r * glossiness };
			
			const ColorRGB specular{ m_pSpecularTexture->Sample(vertex_out.uv) * Phong(1.0f, phongExponent, -lightDirection, vertex_out.viewDirection, pixelNormal) };
			
			return (lightIntensity * lambert + specular) * observedArea;
			break;
		}
		default:
			//finalColor += ambient;
			return finalColor;
		}


	}

	ColorRGB Renderer::Lambert(float kd, const ColorRGB& cd)
	{
		return (kd * cd) / static_cast<float>(M_PI);
	}

	ColorRGB Renderer::Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
	{

		Vector3 reflect{ Vector3::Reflect(l,n) };
		float angle = std::max(Vector3::Dot(reflect, v), 0.f);

		float specularReflection = ks * powf(angle, exp);

		return ColorRGB{ specularReflection, specularReflection, specularReflection };
	}








	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContext
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

		if (FAILED(result))
			return result;

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));

		if (FAILED(result))
			return result;

		//2. Create Swapchain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

		if (FAILED(result))
			return result;

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)

		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;


		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		//4. Create renderTarget (RT) & RenderTargetView (RTV)

		//Resource 
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);

		//5. Bind RTV & DSV to output Merger Stage
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set viewport
		D3D11_VIEWPORT viewPort{};
		viewPort.Width = static_cast<float>(m_Width);
		viewPort.Height = static_cast<float>(m_Height);
		viewPort.TopLeftX = 0.f;
		viewPort.TopLeftY = 0.f;
		viewPort.MinDepth = 0.f;
		viewPort.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewPort);
		
		pDxgiFactory->Release();
		return S_OK;
	}
}
