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
		const float rotationSpeed{ 5.f };
		const float movementSpeed{ 5.f };

		bool isLeftHeld{ false };
		bool isRightHeld{ false };

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

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			const float rotSpeed{ deltaTime * rotationSpeed };
			const Vector3 forwardSpeed{ forward * deltaTime * movementSpeed };
			const Vector3 sideSpeed{ right * deltaTime * movementSpeed };
			const Vector3 upSpeed{ up * deltaTime * movementSpeed };

			if (SDL_BUTTON(mouseState) == 8) {
				totalPitch -= static_cast<float>(mouseX) * rotSpeed;
				totalYaw -= static_cast<float>(mouseY) * rotSpeed;
			}
			else if (SDL_BUTTON(mouseState) == 1) {
				origin += static_cast<float>(mouseY) * forwardSpeed;
			}
			else if (SDL_BUTTON(mouseState) == 16) {
				origin += static_cast<float>(mouseY) * upSpeed;
			}

			//reset totalPitch to 0 degrees if it reaches a full spin(360 deg)
			if (totalPitch > 350 || totalPitch < -360) totalPitch = 0;
			Matrix finalRot{ Matrix::CreateRotation(totalYaw, totalPitch, 1) };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			
			origin += pKeyboardState[SDL_SCANCODE_W] * forwardSpeed;
			origin -= pKeyboardState[SDL_SCANCODE_S] * forwardSpeed;

			origin += pKeyboardState[SDL_SCANCODE_SPACE] * upSpeed;
			origin -= pKeyboardState[SDL_SCANCODE_LCTRL] * upSpeed;

			//reverse axis when camera is looking behind, and apply correct addition/subtraction
			totalPitch > 159.f || totalPitch < -159.f ? origin -= pKeyboardState[SDL_SCANCODE_D] * sideSpeed : origin += pKeyboardState[SDL_SCANCODE_D] * sideSpeed;
			totalPitch > 159.f || totalPitch < -159.f ? origin += pKeyboardState[SDL_SCANCODE_A] * sideSpeed : origin -= pKeyboardState[SDL_SCANCODE_A] * sideSpeed;


			forward = finalRot.TransformVector(Vector3::UnitZ);
			forward.Normalize();
		}
	};
}
