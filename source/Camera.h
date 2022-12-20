#pragma once
#include "pch.h"

namespace dae
{
	class Camera
	{
	public:
	
		Camera(Vector3 _origin, float aspectRatio, float _fovAngle = 90.f): origin{_origin},
			camAspectRatio{aspectRatio},
			fovAngle{_fovAngle}
		{
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);
		}
		~Camera()
		{

		}

		Matrix GetViewMatrix() { return viewMatrix; }
		Matrix GetProjectionMatrix() { return projectionMatrix; }
		
		void CalculateViewMatrix()
		{
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			invViewMatrix = Matrix
			{
				right,
				up,
				forward,
				origin
			};

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			viewMatrix = invViewMatrix.Inverse();

			//viewMatrix = Matrix::CreateLookAtLH(origin, forward, up);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, camAspectRatio, nearPlane, farPlane);
		}

	private:

		Vector3 origin{};
		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float fovAngle{ 90.f };
		float fov{ };
		float totalPitch{};
		float totalYaw{};
		float nearPlane{ .1f };
		float farPlane{ 100.f };
		float camAspectRatio{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};
	};
}