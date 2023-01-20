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
		Vector3 GetOrigin() { return origin; }
		Matrix GetInvViewMatrix() { return invViewMatrix; }
		float GetFOV() { return fov; }
		
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

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			//...
			float movementSpeed{ 50.f };
			float cameraSpeed{ 0.2f };

			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			origin += static_cast<float>((pKeyboardState[SDL_SCANCODE_W] | pKeyboardState[SDL_SCANCODE_UP])) * forward * deltaTime * movementSpeed;
			origin += static_cast<float>((pKeyboardState[SDL_SCANCODE_S] | pKeyboardState[SDL_SCANCODE_DOWN])) * -forward * deltaTime * movementSpeed;

			origin += static_cast<float>((pKeyboardState[SDL_SCANCODE_A] | pKeyboardState[SDL_SCANCODE_LEFT])) * -right * deltaTime * movementSpeed;
			origin += static_cast<float>((pKeyboardState[SDL_SCANCODE_D] | pKeyboardState[SDL_SCANCODE_RIGHT])) * right * deltaTime * movementSpeed;

			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			switch (mouseState)
			{
			case SDL_BUTTON_X2:
			{
				origin += up * static_cast<float>(mouseY) * deltaTime * movementSpeed;

				break;
			}
			case SDL_BUTTON_LMASK:
			{
				origin += forward * static_cast<float>(mouseY) * deltaTime * movementSpeed;

				totalYaw += mouseX * cameraSpeed;
				Matrix rotation = Matrix::CreateRotation(totalPitch * TO_RADIANS, 0, 0);
				forward = rotation.TransformVector(Vector3::UnitZ);
				forward.Normalize();
				break;
			}
			case SDL_BUTTON_RMASK:
			{
				totalYaw += mouseX * cameraSpeed;
				totalPitch += -mouseY * cameraSpeed;
				Matrix rotation = Matrix::CreateRotation(totalPitch * TO_RADIANS, totalYaw * TO_RADIANS, 0);
				forward = rotation.TransformVector(Vector3::UnitZ);
				forward.Normalize();
				break;
			}
			}

			//Update Matrices
			CalculateViewMatrix();
			void CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
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