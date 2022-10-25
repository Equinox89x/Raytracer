#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <iostream>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		//Vector3 forward{Vector3::UnitZ};
		Vector3 forward{0.266f, -0.453f, 0.860f};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};
		const float rotationSpeed{ 0.03f };
		const float movementSpeed{ 1.f };

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld() const
		{
			Vector3 right{ Vector3::Cross(Vector3::UnitY, forward).Normalized() };
			Vector3 upVector{ Vector3::Cross(forward, right).Normalized() };

			Vector4 up4{ upVector, 0 };
			Vector4 right4{ right, 0 };
			Vector4 forward4{ forward, 0 };;
			Vector4 position{ origin, 1 };

			//ONB Matrix
			return Matrix { right4, up4, forward4, position };
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			origin.z += pKeyboardState[SDL_SCANCODE_W] * deltaTime * movementSpeed;
			origin.x -= pKeyboardState[SDL_SCANCODE_A] * deltaTime * movementSpeed;
			origin.z -= pKeyboardState[SDL_SCANCODE_S] * deltaTime * movementSpeed;
			origin.x += pKeyboardState[SDL_SCANCODE_D] * deltaTime * movementSpeed;
			fovAngle -= pKeyboardState[SDL_SCANCODE_UP] * deltaTime * movementSpeed;
			fovAngle += pKeyboardState[SDL_SCANCODE_DOWN] * deltaTime * movementSpeed;

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			if (SDL_BUTTON(mouseState) == 1) {
				totalPitch -= mouseX * deltaTime * rotationSpeed;
				totalYaw -= mouseY * deltaTime * rotationSpeed;
				Matrix finalRot{ Matrix::CreateRotation(totalYaw, totalPitch, 1) };
				forward = finalRot.TransformVector(Vector3::UnitZ);
				forward.Normalize();
			}
		}
	};
}
